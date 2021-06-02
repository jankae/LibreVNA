#include <HW_HAL.hpp>
#include "Hardware.hpp"
#include "Si5351C.hpp"
#include "max2871.hpp"
#include "FPGA/FPGA.hpp"
#include "Exti.hpp"
#include "VNA.hpp"
#include "Manual.hpp"
#include "SpectrumAnalyzer.hpp"
#include <cstring>

#define LOG_LEVEL	LOG_LEVEL_INFO
#define LOG_MODULE	"HW"
#include "Log.h"

static uint32_t extOutFreq = 0;
static bool extRefInUse = false;
HW::Mode activeMode;
static bool unlevel = false;

static Protocol::ReferenceSettings ref;
static uint32_t lastISR;

using namespace HWHAL;

static void HaltedCallback() {
	switch(activeMode) {
	case HW::Mode::VNA:
		VNA::SweepHalted();
		break;
	default:
		break;
	}
}

static void ReadComplete(const FPGA::SamplingResult &result) {
	bool needs_work = false;
	switch(activeMode) {
	case HW::Mode::VNA:
		needs_work = VNA::MeasurementDone(result);
		break;
	case HW::Mode::Manual:
		needs_work = Manual::MeasurementDone(result);
		break;
	case HW::Mode::SA:
		needs_work = SA::MeasurementDone(result);
		break;
	default:
		break;
	}
	if(needs_work) {
		STM::DispatchToInterrupt(HW::Work);
	}
}

static void FPGA_Interrupt(void*) {
	FPGA::InitiateSampleRead(ReadComplete);
	lastISR = HAL_GetTick();
}

void HW::Work() {
	switch(activeMode) {
	case HW::Mode::VNA:
		VNA::Work();
		break;
	case HW::Mode::Manual:
		Manual::Work();
		break;
	case HW::Mode::SA:
		SA::Work();
		break;
	default:
		break;
	}
}

bool HW::Init() {
#ifdef USE_DEBUG_PINS
	// initialize debug pins
	GPIO_InitTypeDef gpio;
	gpio.Pin = DEBUG1_PIN;
	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(DEBUG1_GPIO, &gpio);
	gpio.Pin = DEBUG2_PIN;
	HAL_GPIO_Init(DEBUG2_GPIO, &gpio);
#endif
	LOG_DEBUG("Initializing...");

	activeMode = Mode::Idle;

	Si5351.Init();

	// Use Si5351 to generate reference frequencies for other PLLs and ADC
	Si5351.SetPLL(Si5351C::PLL::A, HW::SI5351CPLLConstantFrequency, Si5351C::PLLSource::XTAL);
	while(!Si5351.Locked(Si5351C::PLL::A));

	Si5351.SetPLL(Si5351C::PLL::B, HW::SI5351CPLLAlignedFrequency, Si5351C::PLLSource::XTAL);
	while(!Si5351.Locked(Si5351C::PLL::B));

	extRefInUse = 0;
	extOutFreq = 0;
	Si5351.Disable(SiChannel::ReferenceOut);

	// Both MAX2871 get a 100MHz reference
//	Si5351.SetBypass(SiChannel::Source, Si5351C::PLLSource::XTAL);
	Si5351.SetCLK(SiChannel::Source, HW::PLLRef, Si5351C::PLL::B, Si5351C::DriveStrength::mA2);
	Si5351.Enable(SiChannel::Source);
//	Si5351.SetBypass(SiChannel::LO1, Si5351C::PLLSource::XTAL);
	Si5351.SetCLK(SiChannel::LO1, HW::PLLRef, Si5351C::PLL::B, Si5351C::DriveStrength::mA2);
	Si5351.Enable(SiChannel::LO1);
	// 16MHz FPGA clock
	Si5351.SetCLK(SiChannel::FPGA, HW::FPGAClkInFrequency, Si5351C::PLL::A, Si5351C::DriveStrength::mA2);
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

	// Set default ADC samplerate
	FPGA::WriteRegister(FPGA::Reg::ADCPrescaler, HW::ADCprescaler);
	// Set phase increment according to
	FPGA::WriteRegister(FPGA::Reg::PhaseIncrement, HW::DFTphaseInc);

	Exti::SetCallback(FPGA_INTR_GPIO_Port, FPGA_INTR_Pin, Exti::EdgeType::Rising, Exti::Pull::Down, FPGA_Interrupt);

	// Initialize PLLs and build VCO maps
	// enable source synthesizer
	FPGA::Enable(FPGA::Periphery::SourceChip);
	FPGA::SetMode(FPGA::Mode::SourcePLL);
	Source.Init(HW::PLLRef, false, 1, false);
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
	LO1.Init(HW::PLLRef, false, 1, false);
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

	Ref::update();
	return true;
}

void HW::SetMode(Mode mode) {
	if(activeMode == mode) {
		// already the correct mode
		return;
	}
	unlevel = false;
	switch(activeMode) {
	case Mode::Manual:
		Manual::Stop();
		break;
	case Mode::VNA:
		VNA::Stop();
		break;
	default:
		break;
	}
	if(mode != Mode::Idle) {
		// do a full initialization when switching directly between modes
		HW::Init();
		lastISR = HAL_GetTick();
	}
	SetIdle();
	activeMode = mode;
}

bool HW::GetTemps(uint8_t *source, uint8_t *lo) {
	FPGA::SetMode(FPGA::Mode::SourcePLL);
	*source = Source.GetTemp();
	FPGA::SetMode(FPGA::Mode::LOPLL);
	*lo = LO1.GetTemp();
	FPGA::SetMode(FPGA::Mode::FPGA);
	return true;
}

void HW::SetIdle() {
	unlevel = false;
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
	FPGA::Enable(FPGA::Periphery::PortSwitch, false);
}

HW::AmplitudeSettings HW::GetAmplitudeSettings(int16_t cdbm, uint64_t freq, bool applyCorrections, bool port2) {
	if (applyCorrections) {
		auto correction = Cal::SourceCorrection(freq);
		if (port2) {
			cdbm += correction.port2;
		} else {
			cdbm += correction.port1;
		}
	}
	AmplitudeSettings ret;
	if(freq < BandSwitchFrequency) {
		if(cdbm <= HW::LowBandMinPower) {
			// can use the low power setting
			ret.lowBandPower = Si5351C::DriveStrength::mA2;
			cdbm -= HW::LowBandMinPower;
		} else {
			// needs the high power setting
			ret.lowBandPower = Si5351C::DriveStrength::mA8;
			cdbm -= HW::LowBandMaxPower;
		}
	} else {
		if(cdbm <= HW::HighBandMinPower) {
			// can use the low power setting
			ret.highBandPower = MAX2871::Power::n4dbm;
			cdbm -= HW::HighBandMinPower;
		} else {
			// needs the high power setting
			ret.highBandPower = MAX2871::Power::p5dbm;
			cdbm -= HW::HighBandMaxPower;
		}
	}
	// calculate required attenuation
	int16_t attval = -cdbm / 25;
	if(attval > 127) {
		attval = 127;
		ret.unlevel = true;
	} else if(attval < 0) {
		attval = 0;
		ret.unlevel = true;
	} else {
		ret.unlevel = false;
	}
	ret.attenuator = attval;
	return ret;
}

bool HW::TimedOut() {
	constexpr uint32_t timeout = 1000;
	if(activeMode != Mode::Idle && HAL_GetTick() - lastISR > timeout) {
		return true;
	} else {
		return false;
	}
}

void HW::SetOutputUnlevel(bool unlev) {
	unlevel = unlev;
}

void HW::fillDeviceInfo(Protocol::DeviceInfo *info, bool updateEvenWhenBusy) {
	// copy constant default values
	memcpy(info, &HW::Info, sizeof(HW::Info));
	if(activeMode == Mode::Idle || updateEvenWhenBusy) {
		// updating values from FPGA allowed

		// read PLL temperatures
		uint8_t tempSource, tempLO;
		GetTemps(&tempSource, &tempLO);
		LOG_INFO("PLL temperatures: %u/%u", tempSource, tempLO);
		// Read ADC min/max
		auto limits = FPGA::GetADCLimits();
		LOG_INFO("ADC limits: P1: %d/%d P2: %d/%d R: %d/%d",
				limits.P1min, limits.P1max, limits.P2min, limits.P2max,
				limits.Rmin, limits.Rmax);
	#define ADC_LIMIT 		30000
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
		info->unlevel = unlevel;
		info->temp_LO1 = tempLO;
		info->temp_source = tempSource;
		FPGA::ResetADCLimits();
	}
	info->temp_MCU = STM::getTemperature();
}

bool HW::Ref::available() {
	return Si5351.ExtCLKAvailable();
}

void HW::Ref::set(Protocol::ReferenceSettings s) {
	ref = s;
}

bool HW::Ref::usingExternal() {
	return extRefInUse;
}

void HW::Ref::update() {
	if(extOutFreq != ref.ExtRefOuputFreq) {
		extOutFreq = ref.ExtRefOuputFreq;
		if(extOutFreq == 0) {
			Si5351.Disable(SiChannel::ReferenceOut);
			LOG_INFO("External reference output disabled");
		} else {
			Si5351.SetCLK(SiChannel::ReferenceOut, extOutFreq, Si5351C::PLL::A);
			Si5351.Enable(SiChannel::ReferenceOut);
			LOG_INFO("External reference output set to %luHz", extOutFreq);
		}
	}
	bool useExternal = ref.UseExternalRef;
	if (ref.AutomaticSwitch) {
		useExternal = Ref::available();
	}
	if(useExternal != extRefInUse) {
		// switch between internal and external reference
		extRefInUse = useExternal;
		if(extRefInUse) {
			if(!Ref::available()) {
				LOG_WARN("Forced switch to external reference but no signal detected");
			}
			Si5351.ConfigureCLKIn(10000000);
			Si5351.SetPLL(Si5351C::PLL::A, HW::SI5351CPLLConstantFrequency, Si5351C::PLLSource::CLKIN);
			Si5351.SetPLL(Si5351C::PLL::B, HW::SI5351CPLLAlignedFrequency, Si5351C::PLLSource::CLKIN);
			LOG_INFO("Switched to external reference");
			FPGA::Enable(FPGA::Periphery::ExtRefLED);
		} else {
			Si5351.SetPLL(Si5351C::PLL::A, HW::SI5351CPLLConstantFrequency, Si5351C::PLLSource::XTAL);
			Si5351.SetPLL(Si5351C::PLL::B, HW::SI5351CPLLAlignedFrequency, Si5351C::PLLSource::XTAL);
			LOG_INFO("Switched to internal reference");
			FPGA::Disable(FPGA::Periphery::ExtRefLED);
		}
	}
	constexpr uint32_t lock_timeout = 100;
	uint32_t start = HAL_GetTick();
	while(!Si5351.Locked(Si5351C::PLL::A) || !Si5351.Locked(Si5351C::PLL::A)) {
		if(HAL_GetTick() - start > lock_timeout) {
			LOG_ERR("Clock distributor PLLs failed to lock");
			return;
		}
	}
}
