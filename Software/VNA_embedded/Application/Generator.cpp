#include <Cal.hpp>
#include "Generator.hpp"
#include "Manual.hpp"
#include "Hardware.hpp"
#include "HW_HAL.hpp"
#include "max2871.hpp"
#include "Si5351C.hpp"

using namespace HWHAL;

void Generator::Setup(Protocol::GeneratorSettings g) {
	// Disable 2.LO
	Si5351.Disable(SiChannel::Port1LO2);
	Si5351.Disable(SiChannel::Port2LO2);
	Si5351.Disable(SiChannel::RefLO2);

	HW::SetMode(HW::Mode::Generator);
	if(g.activePort == 0) {
		// both ports disabled, no need to configure PLLs
		Si5351.Disable(SiChannel::LowbandSource);
		FPGA::Disable(FPGA::Periphery::SourceChip);
		FPGA::Disable(FPGA::Periphery::Amplifier);
		FPGA::Disable(FPGA::Periphery::SourceRF);
		FPGA::Disable(FPGA::Periphery::PortSwitch);
		FPGA::DisableHardwareOverwrite();
		return;
	}

	g.frequency = Cal::FrequencyCorrectionToDevice(g.frequency);
	auto amplitude = HW::GetAmplitudeSettings(g.cdbm_level, g.frequency, g.applyAmplitudeCorrection, g.activePort == 2);
	// Select correct source
	bool bandSelect;
	FPGA::LowpassFilter lp = FPGA::LowpassFilter::M947;
	if(g.frequency < HW::BandSwitchFrequency) {
		bandSelect = true;
		FPGA::Disable(FPGA::Periphery::SourceChip);
		Si5351.SetCLK(SiChannel::LowbandSource, g.frequency, Si5351C::PLL::A,
				amplitude.lowBandPower);
		Si5351.Enable(SiChannel::LowbandSource);
	} else {
		bandSelect = false;
		Si5351.Disable(SiChannel::LowbandSource);
		FPGA::Enable(FPGA::Periphery::SourceChip);
		FPGA::SetMode(FPGA::Mode::SourcePLL);
		Source.SetPowerOutA(amplitude.highBandPower);
		Source.SetFrequency(g.frequency);
		Source.Update();
		FPGA::SetMode(FPGA::Mode::FPGA);
		if(g.frequency < 900000000UL) {
			lp = FPGA::LowpassFilter::M947;
		} else if(g.frequency < 1800000000UL) {
			lp = FPGA::LowpassFilter::M1880;
		} else if(g.frequency < 3500000000UL) {
			lp = FPGA::LowpassFilter::M3500;
		} else {
			lp = FPGA::LowpassFilter::None;
		}
	}

	FPGA::OverwriteHardware(amplitude.attenuator, lp, bandSelect, g.activePort == 1, g.activePort == 2);
	HW::SetOutputUnlevel(amplitude.unlevel);
	FPGA::Enable(FPGA::Periphery::Amplifier, true);
	FPGA::Enable(FPGA::Periphery::SourceRF, true);
	FPGA::Enable(FPGA::Periphery::PortSwitch, true);
}
