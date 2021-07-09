#include <Cal.hpp>
#include "Generator.hpp"
#include "Manual.hpp"
#include "Hardware.hpp"
#include "max2871.hpp"
#include "Si5351C.hpp"

void Generator::Setup(Protocol::GeneratorSettings g) {
	if(g.activePort == 0) {
			// both ports disabled, no need to configure PLLs
			HW::SetIdle();
			return;
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
	m.WindowType = (int) FPGA::Window::None;

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
	g.frequency = Cal::FrequencyCorrectionToDevice(g.frequency);
	auto amplitude = HW::GetAmplitudeSettings(g.cdbm_level, g.frequency, g.applyAmplitudeCorrection, g.activePort == 2);
	// Select correct source
	if(g.frequency < HW::BandSwitchFrequency) {
		m.SourceLowEN = 1;
		m.SourceLowFrequency = g.frequency;
		m.SourceHighCE = 0;
		m.SourceHighRFEN = 0;
		m.SourceHighFrequency = HW::BandSwitchFrequency;
		m.SourceHighLowpass = (int) FPGA::LowpassFilter::M947;
		m.SourceHighPower = (int) MAX2871::Power::n4dbm;
		m.SourceHighband = false;
		m.SourceLowPower = (int) amplitude.lowBandPower;
	} else {
		m.SourceLowEN = 0;
		m.SourceLowFrequency = HW::BandSwitchFrequency;
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
		m.SourceHighPower = (int) amplitude.highBandPower;
		m.SourceLowPower = (int) Si5351C::DriveStrength::mA2;
	}

	m.attenuator = amplitude.attenuator;
	Manual::Setup(m);
	HW::SetOutputUnlevel(amplitude.unlevel);
}
