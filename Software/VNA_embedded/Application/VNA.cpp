#include <HW_HAL.hpp>
#include <VNA.hpp>
#include "Si5351C.hpp"
#include "max2871.hpp"
#include "main.h"
#include "delay.hpp"
#include "FPGA/FPGA.hpp"
#include <complex>
#include "Exti.hpp"
#include "Hardware.hpp"
#include "Communication.h"
#include "FreeRTOS.h"
#include "task.h"
#include "Util.hpp"
#include "usb.h"

#define LOG_LEVEL	LOG_LEVEL_INFO
#define LOG_MODULE	"VNA"
#include "Log.h"

static Protocol::SweepSettings settings;
static uint16_t pointCnt;
static bool excitingPort1;
static Protocol::Datapoint data;
static bool active = false;
static bool sourceHighPower;
static bool adcShifted;
static uint32_t actualBandwidth;

static constexpr uint8_t sourceHarmonic = 5;
static constexpr uint8_t LOHarmonic = 3;

using IFTableEntry = struct {
	uint16_t pointCnt;
	uint8_t clkconfig[8];
};

static constexpr uint16_t IFTableNumEntries = 500;
static IFTableEntry IFTable[IFTableNumEntries];
static uint16_t IFTableIndexCnt = 0;

static constexpr float alternativeSamplerate = 914285.7143f;
static constexpr uint8_t alternativePrescaler = 102400000UL / alternativeSamplerate;
static_assert(alternativePrescaler * alternativeSamplerate == 102400000UL, "alternative ADCSamplerate can not be reached exactly");
static constexpr uint16_t alternativePhaseInc = 4096 * HW::IF2 / alternativeSamplerate;
static_assert(alternativePhaseInc * alternativeSamplerate == 4096 * HW::IF2, "DFT can not be computed for 2.IF when using alternative samplerate");

// Constants for USB buffer overflow prevention
static constexpr uint16_t maxPointsBetweenHalts = 40;
static constexpr uint32_t reservedUSBbuffer = maxPointsBetweenHalts * (sizeof(Protocol::Datapoint) + 8 /*USB packet overhead*/);

using namespace HWHAL;

bool VNA::Setup(Protocol::SweepSettings s) {
	VNA::Stop();
	vTaskDelay(5);
	HW::SetMode(HW::Mode::VNA);
	if(s.excitePort1 == 0 && s.excitePort2 == 0) {
		// both ports disabled, nothing to do
		HW::SetIdle();
		active = false;
		return false;
	}
	settings = s;
	// Abort possible active sweep first
	FPGA::SetMode(FPGA::Mode::FPGA);
	uint16_t points = settings.points <= FPGA::MaxPoints ? settings.points : FPGA::MaxPoints;
	// Configure sweep
	FPGA::SetNumberOfPoints(points);
	uint32_t samplesPerPoint = (HW::ADCSamplerate / s.if_bandwidth);
	// round up to next multiple of 16 (16 samples are spread across 5 IF2 periods)
	if(samplesPerPoint%16) {
		samplesPerPoint += 16 - samplesPerPoint%16;
	}
	actualBandwidth = HW::ADCSamplerate / samplesPerPoint;
	// has to be one less than actual number of samples
	FPGA::SetSamplesPerPoint(samplesPerPoint);

	// Set level (not very accurate)
	int16_t cdbm = s.cdbm_excitation;
	if(cdbm > -1000) {
		// use higher source power (approx 0dbm with no attenuation)
		sourceHighPower = true;
		Source.SetPowerOutA(MAX2871::Power::p5dbm, true);
	} else {
		// use lower source power (approx -10dbm with no attenuation)
		sourceHighPower = false;
		Source.SetPowerOutA(MAX2871::Power::n4dbm, true);
		cdbm += 1000;
	}
	uint8_t attenuator;
	if(cdbm >= 0) {
		attenuator = 0;
	} else if (cdbm <= -3175){
		attenuator = 127;
	} else {
		attenuator = (-cdbm) / 25;
	}
	FPGA::WriteMAX2871Default(Source.GetRegisters());

	uint32_t last_LO2 = HW::IF1 - HW::IF2;
	Si5351.SetCLK(SiChannel::Port1LO2, last_LO2, Si5351C::PLL::B, Si5351C::DriveStrength::mA2);
	Si5351.SetCLK(SiChannel::Port2LO2, last_LO2, Si5351C::PLL::B, Si5351C::DriveStrength::mA2);
	Si5351.SetCLK(SiChannel::RefLO2, last_LO2, Si5351C::PLL::B, Si5351C::DriveStrength::mA2);
	Si5351.ResetPLL(Si5351C::PLL::B);

	IFTableIndexCnt = 0;

	bool last_lowband = false;

	// invalidate first entry of IFTable, preventing switing of 2.LO in halted callback
	IFTable[0].pointCnt = 0xFFFF;

	uint16_t pointsWithoutHalt = 0;

	// Transfer PLL configuration to FPGA
	for (uint16_t i = 0; i < points; i++) {
		bool harmonic_mixing = false;
		uint64_t freq = s.f_start + (s.f_stop - s.f_start) * i / (points - 1);
		freq = Cal::FrequencyCorrectionToDevice(freq);

		if(freq > 6000000000ULL) {
			harmonic_mixing = true;
		}

		// SetFrequency only manipulates the register content in RAM, no SPI communication is done.
		// No mode-switch of FPGA necessary here.

		bool needs_halt = false;
		uint64_t actualSourceFreq;
		bool lowband = false;
		if (freq < HW::BandSwitchFrequency) {
			needs_halt = true;
			lowband = true;
			actualSourceFreq = freq;
		} else {
			uint64_t srcFreq = freq;
			if(harmonic_mixing) {
				srcFreq /= sourceHarmonic;
			}
			Source.SetFrequency(srcFreq);
			actualSourceFreq = Source.GetActualFrequency();
			if(harmonic_mixing) {
				actualSourceFreq *= sourceHarmonic;
			}
		}
		if (last_lowband && !lowband) {
			// additional halt before first highband point to enable highband source
			needs_halt = true;
		}
		uint64_t LOFreq = freq + HW::IF1;
		if(harmonic_mixing) {
			LOFreq /= LOHarmonic;
		}
		LO1.SetFrequency(LOFreq);
		uint64_t actualLO1 = LO1.GetActualFrequency();
		if(harmonic_mixing) {
			actualLO1 *= LOHarmonic;
		}
		uint32_t actualFirstIF = actualLO1 - actualSourceFreq;
		uint32_t actualFinalIF = actualFirstIF - last_LO2;
		uint32_t IFdeviation = abs(actualFinalIF - HW::IF2);
		bool needs_LO2_shift = false;
		if(IFdeviation > actualBandwidth / 2) {
			needs_LO2_shift = true;
		}
		if (s.suppressPeaks && needs_LO2_shift) {
			if (IFTableIndexCnt < IFTableNumEntries) {
				// still room in table
				needs_halt = true;
				IFTable[IFTableIndexCnt].pointCnt = i;
				if(IFTableIndexCnt < IFTableNumEntries - 1) {
					// Configure LO2 for the changed IF1. This is not necessary right now but it will generate
					// the correct clock settings
					last_LO2 = actualFirstIF - HW::IF2;
					LOG_INFO("Changing 2.LO to %lu at point %lu (%lu%06luHz) to reach correct 2.IF frequency",
							last_LO2, i, (uint32_t ) (freq / 1000000),
							(uint32_t ) (freq % 1000000));
				} else {
					// last entry in IF table, revert LO2 to default
					last_LO2 = HW::IF1 - HW::IF2;
				}
				Si5351.SetCLK(SiChannel::RefLO2, last_LO2,
						Si5351C::PLL::B, Si5351C::DriveStrength::mA2);
				// store calculated clock configuration for later change
				Si5351.ReadRawCLKConfig(SiChannel::RefLO2, IFTable[IFTableIndexCnt].clkconfig);
				IFTableIndexCnt++;
				needs_LO2_shift = false;
			}
		}
		if(needs_LO2_shift) {
			// if shift is still needed either peak suppression is disabled or no more room in IFTable was available
			LOG_WARN(
					"PLL deviation of %luHz for measurement at %lu%06luHz, will cause a peak",
					IFdeviation, (uint32_t ) (freq / 1000000), (uint32_t ) (freq % 1000000));
		}

		// halt on regular intervals to prevent USB buffer overflow
		if(!needs_halt) {
			pointsWithoutHalt++;
			if(pointsWithoutHalt > maxPointsBetweenHalts) {
				needs_halt = true;
			}
		}
		if(needs_halt) {
			pointsWithoutHalt = 0;
		}

		FPGA::WriteSweepConfig(i, lowband, Source.GetRegisters(),
				LO1.GetRegisters(), attenuator, freq, FPGA::SettlingTime::us20,
				FPGA::Samples::SPPRegister, needs_halt);
		last_lowband = lowband;
	}
	// revert clk configuration to previous value (might have been changed in sweep calculation)
	Si5351.SetCLK(SiChannel::RefLO2, HW::IF1 - HW::IF2, Si5351C::PLL::B, Si5351C::DriveStrength::mA2);
	Si5351.ResetPLL(Si5351C::PLL::B);
	// Enable mixers/amplifier/PLLs
	FPGA::SetWindow(FPGA::Window::None);
	FPGA::Enable(FPGA::Periphery::Port1Mixer);
	FPGA::Enable(FPGA::Periphery::Port2Mixer);
	FPGA::Enable(FPGA::Periphery::RefMixer);
	FPGA::Enable(FPGA::Periphery::Amplifier);
	FPGA::Enable(FPGA::Periphery::SourceChip);
	FPGA::Enable(FPGA::Periphery::SourceRF);
	FPGA::Enable(FPGA::Periphery::LO1Chip);
	FPGA::Enable(FPGA::Periphery::LO1RF);
	FPGA::Enable(FPGA::Periphery::ExcitePort1, s.excitePort1);
	FPGA::Enable(FPGA::Periphery::ExcitePort2, s.excitePort2);
	FPGA::Enable(FPGA::Periphery::PortSwitch);
	pointCnt = 0;
	// starting port depends on whether port 1 is active in sweep
	excitingPort1 = s.excitePort1;
	IFTableIndexCnt = 0;
	adcShifted = false;
	active = true;
	// Enable new data and sweep halt interrupt
	FPGA::EnableInterrupt(FPGA::Interrupt::NewData);
	FPGA::EnableInterrupt(FPGA::Interrupt::SweepHalted);
	// Start the sweep
	FPGA::StartSweep();
	return true;
}

static void PassOnData() {
	Protocol::PacketInfo info;
	info.type = Protocol::PacketType::Datapoint;
	info.datapoint = data;
	Communication::Send(info);
}

bool VNA::MeasurementDone(const FPGA::SamplingResult &result) {
	if(!active) {
		return false;
	}
	if(result.pointNum != pointCnt || !result.activePort != excitingPort1) {
		LOG_WARN("Indicated point does not match (%u != %u, %d != %d)", result.pointNum, pointCnt, result.activePort, !excitingPort1);
		FPGA::AbortSweep();
		return false;
	}
	// normal sweep mode
	auto port1_raw = std::complex<float>(result.P1I, result.P1Q);
	auto port2_raw = std::complex<float>(result.P2I, result.P2Q);
	auto ref = std::complex<float>(result.RefI, result.RefQ);
	auto port1 = port1_raw / ref;
	auto port2 = port2_raw / ref;
	data.pointNum = pointCnt;
	data.frequency = settings.f_start + (settings.f_stop - settings.f_start) * pointCnt / (settings.points - 1);
	if(excitingPort1) {
		data.real_S11 = port1.real();
		data.imag_S11 = port1.imag();
		data.real_S21 = port2.real();
		data.imag_S21 = port2.imag();
	} else {
		data.real_S12 = port1.real();
		data.imag_S12 = port1.imag();
		data.real_S22 = port2.real();
		data.imag_S22 = port2.imag();
	}
	// figure out whether this sweep point is complete and which port gets excited next
	bool pointComplete = false;
	if(settings.excitePort1 == 1 && settings.excitePort2 == 1) {
		// point is complete when port 2 was active
		pointComplete = !excitingPort1;
		// next measurement will be from other port
		excitingPort1 = !excitingPort1;
	} else {
		// only one port active, point is complete after every measurement
		pointComplete = true;
	}
	if(pointComplete) {
		STM::DispatchToInterrupt(PassOnData);
		pointCnt++;
		if (pointCnt >= settings.points) {
			// reached end of sweep, start again
			pointCnt = 0;
			IFTableIndexCnt = 0;
			// request to trigger work function
			return true;
		}
	}
	return false;
}

void VNA::Work() {
	// end of sweep
	HW::Ref::update();
	// Compile info packet
	Protocol::PacketInfo packet;
	packet.type = Protocol::PacketType::DeviceInfo;
	HW::fillDeviceInfo(&packet.info, true);
	Communication::Send(packet);
	// Start next sweep
	FPGA::StartSweep();
}

void VNA::SweepHalted() {
	if(!active) {
		return;
	}
	LOG_DEBUG("Halted before point %d", pointCnt);
	// Check if IF table has entry at this point
	if (IFTableIndexCnt < IFTableNumEntries && IFTable[IFTableIndexCnt].pointCnt == pointCnt) {
		Si5351.WriteRawCLKConfig(SiChannel::Port1LO2, IFTable[IFTableIndexCnt].clkconfig);
		Si5351.WriteRawCLKConfig(SiChannel::Port2LO2, IFTable[IFTableIndexCnt].clkconfig);
		Si5351.WriteRawCLKConfig(SiChannel::RefLO2, IFTable[IFTableIndexCnt].clkconfig);
		Si5351.ResetPLL(Si5351C::PLL::B);
		IFTableIndexCnt++;
		// PLL reset causes the 2.LO to turn off briefly and then ramp on back, needs delay before next point
		Delay::us(1300);
	}
	uint64_t frequency = settings.f_start
			+ (settings.f_stop - settings.f_start) * pointCnt
					/ (settings.points - 1);
	bool adcShiftRequired = false;
	if (frequency < HW::BandSwitchFrequency) {
		// need the Si5351 as Source
		Si5351.SetCLK(SiChannel::LowbandSource, frequency, Si5351C::PLL::B,
				sourceHighPower ? Si5351C::DriveStrength::mA8 : Si5351C::DriveStrength::mA4);
		if (pointCnt == 0) {
			// First point in sweep, enable CLK
			Si5351.Enable(SiChannel::LowbandSource);
			FPGA::Disable(FPGA::Periphery::SourceRF);
			Delay::us(1300);
		}

		// At low frequencies the 1.LO feedtrough mixes with the 2.LO in the second mixer.
		// Depending on the stimulus frequency, the resulting mixing product might alias to the 2.IF
		// in the ADC which causes a spike. Check for this and shift the ADC sampling frequency if necessary
		uint32_t LO_mixing = (HW::IF1 + frequency) - (HW::IF1 - HW::IF2);
		if(abs(Util::Alias(LO_mixing, HW::ADCSamplerate) - HW::IF2) <= actualBandwidth * 2) {
			// the image is in or near the IF bandwidth and would cause a peak
			// Use a slightly different ADC samplerate
			adcShiftRequired = true;
		}
	} else if(!FPGA::IsEnabled(FPGA::Periphery::SourceRF)){
		// first sweep point in highband is also halted, disable lowband source
		Si5351.Disable(SiChannel::LowbandSource);
		FPGA::Enable(FPGA::Periphery::SourceRF);
	}

	if(adcShiftRequired) {
		FPGA::WriteRegister(FPGA::Reg::ADCPrescaler, alternativePrescaler);
		FPGA::WriteRegister(FPGA::Reg::PhaseIncrement, alternativePhaseInc);
		adcShifted = true;
	} else if(adcShifted) {
		// reset to default value
		FPGA::WriteRegister(FPGA::Reg::ADCPrescaler, HW::ADCprescaler);
		FPGA::WriteRegister(FPGA::Reg::PhaseIncrement, HW::DFTphaseInc);
		adcShifted = false;
	}

	if(usb_available_buffer() >= reservedUSBbuffer) {
		// enough space available, can resume immediately
		FPGA::ResumeHaltedSweep();
	} else {
		// USB buffer could potentially overflow before next halted point, wait until more space is available.
		// This function is called from a low level interrupt, need to dispatch to lower priority to allow USB
		// handling to continue
		STM::DispatchToInterrupt([](){
			while(usb_available_buffer() < reservedUSBbuffer);
			FPGA::ResumeHaltedSweep();
		});
	}
}

void VNA::Stop() {
	active = false;
	FPGA::AbortSweep();
}
