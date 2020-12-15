#include "Manual.hpp"
#include "HW_HAL.hpp"
#include "Hardware.hpp"
#include "Communication.h"
#include <cstring>

static bool active = false;
static uint32_t samples;
static Protocol::ManualStatus status;

using namespace HWHAL;

void Manual::Setup(Protocol::ManualControl m) {
	HW::SetMode(HW::Mode::Manual);
	samples = m.Samples;
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

	FPGA::SetWindow((FPGA::Window) m.WindowType);

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
	FPGA::Enable(FPGA::Periphery::PortSwitch);

	// Enable new data and sweep halt interrupt
	FPGA::EnableInterrupt(FPGA::Interrupt::NewData);

	active = true;
	FPGA::StartSweep();
}

bool Manual::MeasurementDone(const FPGA::SamplingResult &result) {
	if(!active) {
		return false;
	}
	// save measurement
	status.port1real = (float) result.P1I / samples;
	status.port1imag = (float) result.P1Q / samples;
	status.port2real = (float) result.P2I / samples;
	status.port2imag = (float) result.P2Q / samples;
	status.refreal = (float) result.RefI / samples;
	status.refimag = (float) result.RefQ / samples;
	return true;
}


void Manual::Work() {
	if(!active) {
		return;
	}
	Protocol::PacketInfo p;
	p.type = Protocol::PacketType::Status;
	p.status = status;
	uint16_t isr_flags = FPGA::GetStatus();
	if (!(isr_flags & 0x0002)) {
		p.status.source_locked = 1;
	} else {
		p.status.source_locked = 0;
	}
	if (!(isr_flags & 0x0001)) {
		p.status.LO_locked = 1;
	} else {
		p.status.LO_locked = 0;
	}
	auto limits = FPGA::GetADCLimits();
	FPGA::ResetADCLimits();
	p.status.port1min = limits.P1min;
	p.status.port1max = limits.P1max;
	p.status.port2min = limits.P2min;
	p.status.port2max = limits.P2max;
	p.status.refmin = limits.Rmin;
	p.status.refmax = limits.Rmax;
	HW::GetTemps(&p.status.temp_source, &p.status.temp_LO);
	Communication::Send(p);
	HW::Ref::update();
	Protocol::PacketInfo packet;
	packet.type = Protocol::PacketType::DeviceInfo;
	// Enable PLL chips for temperature reading
	bool srcEn = FPGA::IsEnabled(FPGA::Periphery::SourceChip);
	bool LOEn = FPGA::IsEnabled(FPGA::Periphery::LO1Chip);
	FPGA::Enable(FPGA::Periphery::SourceChip);
	FPGA::Enable(FPGA::Periphery::LO1Chip);
	HW::fillDeviceInfo(&packet.info, true);
	// restore PLL state
	FPGA::Enable(FPGA::Periphery::SourceChip, srcEn);
	FPGA::Enable(FPGA::Periphery::LO1Chip, LOEn);
	Communication::Send(packet);
	// Trigger next status update
	FPGA::StartSweep();
}

void Manual::Stop() {
	active = false;
}

