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

#define LOG_LEVEL	LOG_LEVEL_INFO
#define LOG_MODULE	"VNA"
#include "Log.h"

static constexpr uint32_t IF1 = 60100000;
static constexpr uint32_t IF1_alternate = 57000000;
static constexpr uint32_t IF2 = 250000;

static VNA::SweepCallback sweepCallback;
static Protocol::SweepSettings settings;
static uint16_t pointCnt;
static bool excitingPort1;
static Protocol::Datapoint data;
static bool active = false;

using IFTableEntry = struct {
	uint16_t pointCnt;
	uint32_t IF1;
	uint8_t clkconfig[8];
};

static constexpr uint16_t IFTableNumEntries = 100;
static IFTableEntry IFTable[IFTableNumEntries];
static uint16_t IFTableIndexCnt = 0;

static constexpr uint32_t BandSwitchFrequency = 25000000;

using namespace HWHAL;

bool VNA::Setup(Protocol::SweepSettings s, SweepCallback cb) {
	HW::SetMode(HW::Mode::VNA);
	if(s.excitePort1 == 0 && s.excitePort2 == 0) {
		// both ports disabled, nothing to do
		HW::SetIdle();
		active = false;
		return false;
	}
	sweepCallback = cb;
	settings = s;
	// Abort possible active sweep first
	FPGA::AbortSweep();
	FPGA::SetMode(FPGA::Mode::FPGA);
	uint16_t points = settings.points <= FPGA::MaxPoints ? settings.points : FPGA::MaxPoints;
	// Configure sweep
	FPGA::SetNumberOfPoints(points);
	uint32_t samplesPerPoint = (HW::ADCSamplerate / s.if_bandwidth);
	// round up to next multiple of 128 (128 samples are spread across 35 IF2 periods)
	samplesPerPoint = ((uint32_t) ((samplesPerPoint + 127) / 128)) * 128;
	// has to be one less than actual number of samples
	FPGA::SetSamplesPerPoint(samplesPerPoint);

	uint8_t attenuator;
	if(s.cdbm_excitation >= -1000) {
		attenuator = 0;
	} else if (s.cdbm_excitation <= -4175){
		attenuator = 127;
	} else {
		attenuator = (-1000 - s.cdbm_excitation) / 25;
	}

	uint32_t last_IF1 = IF1;

	IFTableIndexCnt = 0;

	bool last_lowband = false;

	// Transfer PLL configuration to FPGA
	for (uint16_t i = 0; i < points; i++) {
		uint64_t freq = s.f_start + (s.f_stop - s.f_start) * i / (points - 1);
		// SetFrequency only manipulates the register content in RAM, no SPI communication is done.
		// No mode-switch of FPGA necessary here.

		// Check which IF frequency is a better fit
		uint32_t used_IF = IF1;
//		if (freq < 290000000) {
//			// for low frequencies the harmonics of the IF and source frequency should not be too close
//			uint32_t dist_primary;
//			if(freq < IF1) {
//				dist_primary = IF1 - freq * (IF1 / freq);
//				if (dist_primary > freq / 2) {
//					dist_primary = freq - dist_primary;
//				}
//			} else {
//				dist_primary = freq - IF1 * (freq / IF1);
//				if (dist_primary > IF1 / 2) {
//					dist_primary = IF1 - dist_primary;
//				}
//			}
//			uint32_t dist_alternate;
//			if(freq < IF1_alternate) {
//				dist_alternate = IF1_alternate - freq * (IF1_alternate / freq);
//				if (dist_alternate > freq / 2) {
//					dist_alternate = freq - dist_primary;
//				}
//			} else {
//				dist_alternate = freq - IF1_alternate * (freq / IF1_alternate);
//				if (dist_alternate > IF1_alternate / 2) {
//					dist_alternate = IF1_alternate - dist_primary;
//				}
//			}
//			if(dist_alternate > dist_primary) {
//				used_IF = IF1_alternate;
//			}
//			LOG_INFO("Distance: %lu/%lu", dist_primary, dist_alternate);
//		}
		bool needs_halt = false;
		if (used_IF != last_IF1) {
			last_IF1 = used_IF;
			LOG_INFO("Changing IF1 to %lu at point %u (f=%lu)", used_IF, i, (uint32_t) freq);
			needs_halt = true;
			if (IFTableIndexCnt >= IFTableNumEntries) {
				LOG_ERR("IF table full, unable to add new entry");
				return false;
			}
			IFTable[IFTableIndexCnt].pointCnt = i;
			IFTable[IFTableIndexCnt].IF1 = used_IF;
			// Configure LO2 for the changed IF1. This is not necessary right now but it will generate
			// the correct clock settings
			Si5351.SetCLK(SiChannel::RefLO2, used_IF + IF2, Si5351C::PLL::A, Si5351C::DriveStrength::mA2);
			// store calculated clock configuration for later change
			Si5351.ReadRawCLKConfig(1, IFTable[IFTableIndexCnt].clkconfig);
			IFTableIndexCnt++;
		}
		bool lowband = false;
		if (freq < BandSwitchFrequency) {
			needs_halt = true;
			lowband = true;
		} else {
			Source.SetFrequency(freq);
		}
		if (last_lowband && !lowband) {
			// additional halt before first highband point to enable highband source
			needs_halt = true;
		}
		LO1.SetFrequency(freq + used_IF);
		FPGA::WriteSweepConfig(i, lowband, Source.GetRegisters(),
				LO1.GetRegisters(), attenuator, freq, FPGA::SettlingTime::us20,
				FPGA::Samples::SPPRegister, needs_halt);
		last_lowband = lowband;
	}
//	// revert clk configuration to previous value (might have been changed in sweep calculation)
//	Si5351.SetCLK(1, IF1 + IF2, Si5351C::PLL::B, Si5351C::DriveStrength::mA2);
//	Si5351.ResetPLL(Si5351C::PLL::B);
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
	pointCnt = 0;
	// starting port depends on whether port 1 is active in sweep
	excitingPort1 = s.excitePort1;
	IFTableIndexCnt = 0;
	active = true;
	// Start the sweep
	FPGA::StartSweep();
	return true;
}

bool VNA::MeasurementDone(FPGA::SamplingResult result) {
	if(!active) {
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
		if (sweepCallback) {
			sweepCallback(data);
		}
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
	packet.info.FPGA_configured = 1;
	packet.info.FW_major = FW_MAJOR;
	packet.info.FW_minor = FW_MINOR;
	packet.info.HW_Revision = HW_REVISION;
	HW::fillDeviceInfo(&packet.info);
	Communication::Send(packet);
	FPGA::ResetADCLimits();
	// Start next sweep
	FPGA::StartSweep();
}

void VNA::SweepHalted() {
	if(!active) {
		return;
	}
	LOG_DEBUG("Halted before point %d", pointCnt);
	// Check if IF table has entry at this point
//	if (IFTable[IFTableIndexCnt].pointCnt == pointCnt) {
//		LOG_DEBUG("Shifting IF to %lu at point %u",
//				IFTable[IFTableIndexCnt].IF1, pointCnt);
//		Si5351.WriteRawCLKConfig(1, IFTable[IFTableIndexCnt].clkconfig);
//		Si5351.WriteRawCLKConfig(4, IFTable[IFTableIndexCnt].clkconfig);
//		Si5351.WriteRawCLKConfig(5, IFTable[IFTableIndexCnt].clkconfig);
//		Si5351.ResetPLL(Si5351C::PLL::B);
//		IFTableIndexCnt++;
//	}
	uint64_t frequency = settings.f_start
			+ (settings.f_stop - settings.f_start) * pointCnt
					/ (settings.points - 1);
	if (frequency < BandSwitchFrequency) {
		// need the Si5351 as Source
		Si5351.SetCLK(SiChannel::LowbandSource, frequency, Si5351C::PLL::B,
				Si5351C::DriveStrength::mA2);
		if (pointCnt == 0) {
			// First point in sweep, enable CLK
			Si5351.Enable(SiChannel::LowbandSource);
			FPGA::Disable(FPGA::Periphery::SourceRF);
		}
	} else {
		// first sweep point in highband is also halted, disable lowband source
		Si5351.Disable(SiChannel::LowbandSource);
		FPGA::Enable(FPGA::Periphery::SourceRF);
	}

	FPGA::ResumeHaltedSweep();
}

void VNA::Stop() {
	active = false;
	FPGA::AbortSweep();
}
