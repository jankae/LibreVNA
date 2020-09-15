#include <VNA.hpp>
#include "Si5351C.hpp"
#include "max2871.hpp"
#include "main.h"
#include "delay.hpp"
#include "FPGA/FPGA.hpp"
#include <complex>
#include "Exti.hpp"
#include "VNA_HAL.hpp"

#define LOG_LEVEL	LOG_LEVEL_INFO
#define LOG_MODULE	"VNA"
#include "Log.h"

static constexpr uint32_t IF1 = 60100000;
static constexpr uint32_t IF1_alternate = 57000000;
static constexpr uint32_t IF2 = 250000;

static VNA::SweepCallback sweepCallback;
static VNA::StatusCallback statusCallback;
static Protocol::SweepSettings settings;
static uint16_t pointCnt;
static bool excitingPort1;
static Protocol::Datapoint data;
static bool manualMode = false;

using IFTableEntry = struct {
	uint16_t pointCnt;
	uint32_t IF1;
	uint8_t clkconfig[8];
};

static constexpr uint16_t IFTableNumEntries = 100;
static IFTableEntry IFTable[IFTableNumEntries];
static uint16_t IFTableIndexCnt = 0;

static constexpr uint32_t BandSwitchFrequency = 25000000;

static uint32_t extOutFreq = 0;
static bool extRefInUse = false;

using namespace VNAHAL;

static void HaltedCallback() {
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

static void ReadComplete(FPGA::SamplingResult result) {
	if(!manualMode) {
		// normal sweep mode
		auto port1_raw = std::complex<float>(result.P1I, result.P1Q);
		auto port2_raw = std::complex<float>(result.P2I, result.P2Q);
		auto ref = std::complex<float>(result.RefI, result.RefQ);
		auto port1 = port1_raw / ref;
		auto port2 = port2_raw / ref;
		if(excitingPort1) {
			data.pointNum = pointCnt;
			data.frequency = settings.f_start + (settings.f_stop - settings.f_start) * pointCnt / (settings.points - 1);
			data.real_S11 = port1.real();
			data.imag_S11 = port1.imag();
			data.real_S21 = port2.real();
			data.imag_S21 = port2.imag();
		} else {
			data.real_S12 = port1.real();
			data.imag_S12 = port1.imag();
			data.real_S22 = port2.real();
			data.imag_S22 = port2.imag();
			if (sweepCallback) {
				sweepCallback(data);
			}
			pointCnt++;
			if (pointCnt >= settings.points) {
				// reached end of sweep, start again
				pointCnt = 0;
				IFTableIndexCnt = 0;
	//			FPGA::StartSweep();
			}
		}
		excitingPort1 = !excitingPort1;
	} else {
		// Manual control mode, simply pass on raw result
		if(statusCallback) {
			statusCallback(result);
		}
	}
}

static void FPGA_Interrupt(void*) {
	FPGA::InitiateSampleRead(ReadComplete);
}

bool VNA::Init() {
	LOG_DEBUG("Initializing...");

	manualMode = false;

	Si5351.Init();

	// Use Si5351 to generate reference frequencies for other PLLs and ADC
	Si5351.SetPLL(Si5351C::PLL::A, 800000000, Si5351C::PLLSource::XTAL);
	while(!Si5351.Locked(Si5351C::PLL::A));

	Si5351.SetPLL(Si5351C::PLL::B, 800000000, Si5351C::PLLSource::XTAL);
	while(!Si5351.Locked(Si5351C::PLL::B));

	extRefInUse = 0;
	extOutFreq = 0;
	Si5351.Disable(SiChannel::ReferenceOut);

	// Both MAX2871 get a 100MHz reference
	Si5351.SetCLK(SiChannel::Source, 100000000, Si5351C::PLL::A, Si5351C::DriveStrength::mA2);
	Si5351.Enable(SiChannel::Source);
	Si5351.SetCLK(SiChannel::LO1, 100000000, Si5351C::PLL::A, Si5351C::DriveStrength::mA2);
	Si5351.Enable(SiChannel::LO1);
	// 16MHz FPGA clock
	Si5351.SetCLK(SiChannel::FPGA, 16000000, Si5351C::PLL::A, Si5351C::DriveStrength::mA2);
	Si5351.Enable(SiChannel::FPGA);

	// Generate second LO with Si5351
	Si5351.SetCLK(SiChannel::Port1LO2, IF1 - IF2, Si5351C::PLL::B, Si5351C::DriveStrength::mA2);
	Si5351.Enable(SiChannel::Port1LO2);
	Si5351.SetCLK(SiChannel::Port2LO2, IF1 - IF2, Si5351C::PLL::B, Si5351C::DriveStrength::mA2);
	Si5351.Enable(SiChannel::Port2LO2);
	Si5351.SetCLK(SiChannel::RefLO2, IF1 - IF2, Si5351C::PLL::B, Si5351C::DriveStrength::mA2);
	Si5351.Enable(SiChannel::RefLO2);

	// PLL reset appears to realign phases of clock signals
	Si5351.ResetPLL(Si5351C::PLL::B);

	LOG_DEBUG("Si5351 locked");

	// FPGA clock is now present, can initialize
	if (!FPGA::Init(HaltedCallback)) {
		LOG_ERR("Aborting due to uninitialized FPGA");
		return false;
	}

	// Enable new data and sweep halt interrupt
	FPGA::EnableInterrupt(FPGA::Interrupt::NewData);
	FPGA::EnableInterrupt(FPGA::Interrupt::SweepHalted);

	Exti::SetCallback(FPGA_INTR_GPIO_Port, FPGA_INTR_Pin, Exti::EdgeType::Rising, Exti::Pull::Down, FPGA_Interrupt);

	// Initialize PLLs and build VCO maps
	// enable source synthesizer
	FPGA::Enable(FPGA::Periphery::SourceChip);
	FPGA::SetMode(FPGA::Mode::SourcePLL);
	Source.Init(100000000, false, 1, false);
	Source.SetPowerOutA(MAX2871::Power::n4dbm);
	// output B is not used
	Source.SetPowerOutB(MAX2871::Power::n4dbm, false);
	if(!Source.BuildVCOMap()) {
		LOG_WARN("Source VCO map failed");
	} else {
		LOG_INFO("Source VCO map complete");
	}
	Source.SetFrequency(1000000000);
	Source.UpdateFrequency();
	LOG_DEBUG("Source temp: %u", Source.GetTemp());
	// disable source synthesizer/enable LO synthesizer
	FPGA::SetMode(FPGA::Mode::FPGA);
	FPGA::Disable(FPGA::Periphery::SourceChip);
	FPGA::Enable(FPGA::Periphery::LO1Chip);
	FPGA::SetMode(FPGA::Mode::LOPLL);
	LO1.Init(100000000, false, 1, false);
	LO1.SetPowerOutA(MAX2871::Power::n4dbm);
	LO1.SetPowerOutB(MAX2871::Power::n4dbm);
	if(!LO1.BuildVCOMap()) {
		LOG_WARN("LO1 VCO map failed");
	} else {
		LOG_INFO("LO1 VCO map complete");
	}
	LO1.SetFrequency(1000000000 + IF1);
	LO1.UpdateFrequency();
	LOG_DEBUG("LO temp: %u", LO1.GetTemp());

	FPGA::SetMode(FPGA::Mode::FPGA);
	// disable both synthesizers
	FPGA::Disable(FPGA::Periphery::LO1Chip);
	FPGA::WriteMAX2871Default(Source.GetRegisters());

	LOG_INFO("Initialized");
	FPGA::Enable(FPGA::Periphery::ReadyLED);
	return true;
}

bool VNA::ConfigureSweep(Protocol::SweepSettings s, SweepCallback cb) {
	if (manualMode) {
		// was used in manual mode last, do full initialization before starting sweep
		VNA::Init();
	}
	sweepCallback = cb;
	settings = s;
	// Abort possible active sweep first
	FPGA::AbortSweep();
	uint16_t points = settings.points <= FPGA::MaxPoints ? settings.points : FPGA::MaxPoints;
	// Configure sweep
	FPGA::SetNumberOfPoints(points);
	uint32_t samplesPerPoint = (1000000 / s.if_bandwidth);
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
		uint64_t freq = s.f_start + (s.f_stop - s.f_start) * i / (s.points - 1);
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
				LO1.GetRegisters(), attenuator, freq, FPGA::SettlingTime::us540,
				FPGA::Samples::SPPRegister, needs_halt);
		last_lowband = lowband;
	}
//	// revert clk configuration to previous value (might have been changed in sweep calculation)
//	Si5351.SetCLK(1, IF1 + IF2, Si5351C::PLL::B, Si5351C::DriveStrength::mA2);
//	Si5351.ResetPLL(Si5351C::PLL::B);
	// Enable mixers/amplifier/PLLs
	FPGA::Enable(FPGA::Periphery::Port1Mixer);
	FPGA::Enable(FPGA::Periphery::Port2Mixer);
	FPGA::Enable(FPGA::Periphery::RefMixer);
	FPGA::Enable(FPGA::Periphery::Amplifier);
	FPGA::Enable(FPGA::Periphery::SourceChip);
	FPGA::Enable(FPGA::Periphery::SourceRF);
	FPGA::Enable(FPGA::Periphery::LO1Chip);
	FPGA::Enable(FPGA::Periphery::LO1RF);
	FPGA::Enable(FPGA::Periphery::ExcitePort1);
	FPGA::Enable(FPGA::Periphery::ExcitePort2);
	pointCnt = 0;
	excitingPort1 = true;
	IFTableIndexCnt = 0;
	// Start the sweep
	FPGA::StartSweep();
	return true;
}

bool VNA::ConfigureManual(Protocol::ManualControl m, StatusCallback cb) {
	manualMode = true;
	statusCallback = cb;
	FPGA::AbortSweep();
	// Configure lowband source
	if (m.SourceLowEN) {
		Si5351.SetCLK(SiChannel::LowbandSource, m.SourceLowFrequency, Si5351C::PLL::B,
				(Si5351C::DriveStrength) m.SourceLowPower);
		Si5351.Enable(SiChannel::LowbandSource);
	} else {
		Si5351.Disable(SiChannel::LowbandSource);
	}
	// Configure highband source
	Source.SetFrequency(m.SourceHighFrequency);
	Source.SetPowerOutA((MAX2871::Power) m.SourceHighPower);

	// Configure LO1
	LO1.SetFrequency(m.LO1Frequency);

	// Configure LO2
	if(m.LO2EN) {
		// Generate second LO with Si5351
		Si5351.SetCLK(SiChannel::Port1LO2, m.LO2Frequency, Si5351C::PLL::B, Si5351C::DriveStrength::mA2);
		Si5351.Enable(SiChannel::Port1LO2);
		Si5351.SetCLK(SiChannel::Port2LO2, m.LO2Frequency, Si5351C::PLL::B, Si5351C::DriveStrength::mA2);
		Si5351.Enable(SiChannel::Port2LO2);
		Si5351.SetCLK(SiChannel::RefLO2, m.LO2Frequency, Si5351C::PLL::B, Si5351C::DriveStrength::mA2);
		Si5351.Enable(SiChannel::RefLO2);

		// PLL reset appears to realign phases of clock signals
		Si5351.ResetPLL(Si5351C::PLL::B);
	} else {
		Si5351.Disable(SiChannel::Port1LO2);
		Si5351.Disable(SiChannel::Port2LO2);
		Si5351.Disable(SiChannel::RefLO2);
	}

	FPGA::WriteMAX2871Default(Source.GetRegisters());

	FPGA::SetNumberOfPoints(1);
	FPGA::SetSamplesPerPoint(m.Samples);

	// Configure single sweep point
	FPGA::WriteSweepConfig(0, !m.SourceHighband, Source.GetRegisters(),
			LO1.GetRegisters(), m.attenuator, 0, FPGA::SettlingTime::us20,
			FPGA::Samples::SPPRegister, 0,
			(FPGA::LowpassFilter) m.SourceHighLowpass);

	// Enable/Disable periphery
	FPGA::Enable(FPGA::Periphery::SourceChip, m.SourceHighCE);
	FPGA::Enable(FPGA::Periphery::SourceRF, m.SourceHighRFEN);
	FPGA::Enable(FPGA::Periphery::LO1Chip, m.LO1CE);
	FPGA::Enable(FPGA::Periphery::LO1RF, m.LO1RFEN);
	FPGA::Enable(FPGA::Periphery::Amplifier, m.AmplifierEN);
	FPGA::Enable(FPGA::Periphery::Port1Mixer, m.Port1EN);
	FPGA::Enable(FPGA::Periphery::Port2Mixer, m.Port2EN);
	FPGA::Enable(FPGA::Periphery::RefMixer, m.RefEN);
	FPGA::Enable(FPGA::Periphery::ExcitePort1, m.PortSwitch == 0);
	FPGA::Enable(FPGA::Periphery::ExcitePort2, m.PortSwitch == 1);

	FPGA::StartSweep();
	return true;
}

bool VNA::GetTemps(uint8_t *source, uint8_t *lo) {
	FPGA::SetMode(FPGA::Mode::SourcePLL);
	*source = Source.GetTemp();
	FPGA::SetMode(FPGA::Mode::LOPLL);
	*lo = LO1.GetTemp();
	FPGA::SetMode(FPGA::Mode::FPGA);
	return true;
}

void VNA::fillDeviceInfo(Protocol::DeviceInfo *info) {
	// read PLL temperatures
	uint8_t tempSource, tempLO;
	VNA::GetTemps(&tempSource, &tempLO);
	LOG_INFO("PLL temperatures: %u/%u", tempSource, tempLO);
	// Read ADC min/max
	auto limits = FPGA::GetADCLimits();
	LOG_INFO("ADC limits: P1: %d/%d P2: %d/%d R: %d/%d",
			limits.P1min, limits.P1max, limits.P2min, limits.P2max,
			limits.Rmin, limits.Rmax);
#define ADC_LIMIT 		30000
	// Set VNA related member of info struct
	if(limits.P1min < -ADC_LIMIT || limits.P1max > ADC_LIMIT
			|| limits.P2min < -ADC_LIMIT || limits.P2max > ADC_LIMIT
			|| limits.Rmin < -ADC_LIMIT || limits.Rmax > ADC_LIMIT) {
		info->ADC_overload = true;
	} else {
		info->ADC_overload = false;
	}
	auto status = FPGA::GetStatus();
	info->LO1_locked = (status & (int) FPGA::Interrupt::LO1Unlock) ? 0 : 1;
	info->source_locked = (status & (int) FPGA::Interrupt::SourceUnlock) ? 0 : 1;
	info->extRefAvailable = Ref::available();
	info->extRefInUse = extRefInUse;
	info->temperatures.LO1 = tempLO;
	info->temperatures.source = tempSource;
	info->temperatures.MCU = 0;
	FPGA::ResetADCLimits();
}

bool VNA::Ref::available() {
	return Si5351.ExtCLKAvailable();
}

bool VNA::Ref::applySettings(Protocol::ReferenceSettings s) {
	if(extOutFreq != s.ExtRefOuputFreq) {
		extOutFreq = s.ExtRefOuputFreq;
		if(extOutFreq == 0) {
			Si5351.Disable(SiChannel::ReferenceOut);
			LOG_INFO("External reference output disabled");
		} else {
			Si5351.SetCLK(SiChannel::ReferenceOut, extOutFreq, Si5351C::PLL::A);
			Si5351.Enable(SiChannel::ReferenceOut);
			LOG_INFO("External reference output set to %luHz", extOutFreq);
		}
	}
	bool useExternal = s.UseExternalRef || (s.AutomaticSwitch && Ref::available());
	if(useExternal != extRefInUse) {
		// switch between internal and external reference
		extRefInUse = useExternal;
		if(extRefInUse) {
			if(!Ref::available()) {
				LOG_WARN("Forced switch to external reference but no signal detected");
			}
			Si5351.ConfigureCLKIn(10000000);
			Si5351.SetPLL(Si5351C::PLL::A, 800000000, Si5351C::PLLSource::CLKIN);
			Si5351.SetPLL(Si5351C::PLL::B, 800000000, Si5351C::PLLSource::CLKIN);
			LOG_INFO("Switched to external reference");
			FPGA::Enable(FPGA::Periphery::ExtRefLED);
		} else {
			Si5351.SetPLL(Si5351C::PLL::A, 800000000, Si5351C::PLLSource::XTAL);
			Si5351.SetPLL(Si5351C::PLL::B, 800000000, Si5351C::PLLSource::XTAL);
			LOG_INFO("Switched to internal reference");
			FPGA::Disable(FPGA::Periphery::ExtRefLED);
		}
	}
	constexpr uint32_t lock_timeout = 10;
	uint32_t start = HAL_GetTick();
	while(!Si5351.Locked(Si5351C::PLL::A) || !Si5351.Locked(Si5351C::PLL::A)) {
		if(HAL_GetTick() - start > lock_timeout) {
			LOG_ERR("Clock distributor PLLs failed to lock");
			return false;
		}
	}
	return false;
}

bool VNA::ConfigureGenerator(Protocol::GeneratorSettings g) {
	if(g.activePort == 0) {
		// both ports disabled, no need to configure PLLs
		SetIdle();
		return true;
	}
	Protocol::ManualControl m;
	// LOs not required
	m.LO1CE = 0;
	m.LO1Frequency = 1000000000;
	m.LO1RFEN = 0;
	m.LO1RFEN = 0;
	m.LO2EN = 0;
	m.LO2Frequency = 60000000;
	m.Port1EN = 0;
	m.Port2EN = 0;
	m.RefEN = 0;
	m.Samples = 131072;
	// Select correct source
	if(g.frequency < BandSwitchFrequency) {
		m.SourceLowEN = 1;
		m.SourceLowFrequency = g.frequency;
		m.SourceHighCE = 0;
		m.SourceHighRFEN = 0;
		m.SourceHighFrequency = BandSwitchFrequency;
		m.SourceHighLowpass = (int) FPGA::LowpassFilter::M947;
		m.SourceHighPower = (int) MAX2871::Power::n4dbm;
		m.SourceHighband = false;
	} else {
		m.SourceLowEN = 0;
		m.SourceLowFrequency = BandSwitchFrequency;
		m.SourceHighCE = 1;
		m.SourceHighRFEN = 1;
		m.SourceHighFrequency = g.frequency;
		if(g.frequency < 900000000UL) {
			m.SourceHighLowpass = (int) FPGA::LowpassFilter::M947;
		} else if(g.frequency < 1800000000UL) {
			m.SourceHighLowpass = (int) FPGA::LowpassFilter::M1880;
		} else if(g.frequency < 3500000000UL) {
			m.SourceHighLowpass = (int) FPGA::LowpassFilter::M3500;
		} else {
			m.SourceHighLowpass = (int) FPGA::LowpassFilter::None;
		}
		m.SourceHighband = true;
	}
	switch(g.activePort) {
	case 1:
		m.AmplifierEN = 1;
		m.PortSwitch = 0;
		break;
	case 2:
		m.AmplifierEN = 1;
		m.PortSwitch = 1;
		break;
	}
	// Set level (not very accurate)
	if(g.cdbm_level > -1000) {
		// use higher source power (approx 0dbm with no attenuation)
		m.SourceHighPower = (int) MAX2871::Power::p5dbm;
		m.SourceLowPower = (int) Si5351C::DriveStrength::mA8;
	} else {
		// use lower source power (approx -10dbm with no attenuation)
		m.SourceHighPower = (int) MAX2871::Power::n4dbm;
		m.SourceLowPower = (int) Si5351C::DriveStrength::mA2;
		g.cdbm_level += 1000;
	}
	// calculate required attenuation
	uint16_t attval = -g.cdbm_level / 25;
	if(attval > 127) {
		attval = 127;
	}
	m.attenuator = attval;
	return ConfigureManual(m, nullptr);
}

void VNA::SetIdle() {
	FPGA::AbortSweep();
	FPGA::SetMode(FPGA::Mode::FPGA);
	FPGA::Enable(FPGA::Periphery::SourceChip, false);
	FPGA::Enable(FPGA::Periphery::SourceRF, false);
	FPGA::Enable(FPGA::Periphery::LO1Chip, false);
	FPGA::Enable(FPGA::Periphery::LO1RF, false);
	FPGA::Enable(FPGA::Periphery::Amplifier, false);
	FPGA::Enable(FPGA::Periphery::Port1Mixer, false);
	FPGA::Enable(FPGA::Periphery::Port2Mixer, false);
	FPGA::Enable(FPGA::Periphery::RefMixer, false);
}
