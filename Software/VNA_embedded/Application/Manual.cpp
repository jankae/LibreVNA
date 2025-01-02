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
	samples = m.V1.Samples;
	FPGA::AbortSweep();
	// Configure lowband source
	if (m.V1.SourceLowEN) {
		Si5351.SetCLK(SiChannel::LowbandSource, m.V1.SourceLowFrequency, Si5351C::PLL::B,
				(Si5351C::DriveStrength) m.V1.SourceLowPower);
		Si5351.Enable(SiChannel::LowbandSource);
	} else {
		Si5351.Disable(SiChannel::LowbandSource);
	}
	// Configure highband source
	Source.SetFrequency(m.V1.SourceHighFrequency);
	Source.SetPowerOutA((MAX2871::Power) m.V1.SourceHighPower);

	// Configure LO1
	LO1.SetFrequency(m.V1.LO1Frequency);

	// Configure LO2
	if(m.V1.LO2EN) {
		// Generate second LO with Si5351
		Si5351.Enable(SiChannel::Port1LO2);
		Si5351.Enable(SiChannel::Port2LO2);
		Si5351.Enable(SiChannel::RefLO2);

		Si5351.SetPLL(Si5351C::PLL::B, m.V1.LO2Frequency*HW::LO2Multiplier, HW::Ref::getSource());

		// PLL reset appears to realign phases of clock signals
		Si5351.ResetPLL(Si5351C::PLL::B);
	} else {
		Si5351.Disable(SiChannel::Port1LO2);
		Si5351.Disable(SiChannel::Port2LO2);
		Si5351.Disable(SiChannel::RefLO2);
	}

	FPGA::WriteMAX2871Default(Source.GetRegisters());

	FPGA::SetNumberOfPoints(1);
	FPGA::SetSamplesPerPoint(m.V1.Samples);

	// Configure single sweep point
	FPGA::WriteSweepConfig(0, !m.V1.SourceHighband, Source.GetRegisters(),
			LO1.GetRegisters(), m.V1.attenuator, 0, FPGA::Samples::SPPRegister, 0,
			(FPGA::LowpassFilter) m.V1.SourceHighLowpass);

	FPGA::SetWindow((FPGA::Window) m.V1.WindowType);

	// Enable/Disable periphery
	FPGA::Enable(FPGA::Periphery::SourceChip, m.V1.SourceHighCE);
	FPGA::Enable(FPGA::Periphery::SourceRF, m.V1.SourceHighRFEN);
	FPGA::Enable(FPGA::Periphery::LO1Chip, m.V1.LO1CE);
	FPGA::Enable(FPGA::Periphery::LO1RF, m.V1.LO1RFEN);
	FPGA::Enable(FPGA::Periphery::Amplifier, m.V1.AmplifierEN);
	FPGA::Enable(FPGA::Periphery::Port1Mixer, m.V1.Port1EN);
	FPGA::Enable(FPGA::Periphery::Port2Mixer, m.V1.Port2EN);
	FPGA::Enable(FPGA::Periphery::RefMixer, m.V1.RefEN);
	FPGA::SetupSweep(0, m.V1.PortSwitch == 1, m.V1.PortSwitch == 0);
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
	status.V1.port1real = (float) result.P1I / samples;
	status.V1.port1imag = (float) result.P1Q / samples;
	status.V1.port2real = (float) result.P2I / samples;
	status.V1.port2imag = (float) result.P2Q / samples;
	status.V1.refreal = (float) result.RefI / samples;
	status.V1.refimag = (float) result.RefQ / samples;
	return true;
}


void Manual::Work() {
	if(!active) {
		return;
	}
	Protocol::PacketInfo p;
	p.type = Protocol::PacketType::ManualStatus;
	p.manualStatus = status;
	uint16_t isr_flags = FPGA::GetStatus();
	if (!(isr_flags & 0x0002)) {
		p.manualStatus.V1.source_locked = 1;
	} else {
		p.manualStatus.V1.source_locked = 0;
	}
	if (!(isr_flags & 0x0001)) {
		p.manualStatus.V1.LO_locked = 1;
	} else {
		p.manualStatus.V1.LO_locked = 0;
	}
	auto limits = FPGA::GetADCLimits();
	FPGA::ResetADCLimits();
	p.manualStatus.V1.port1min = limits.P1min;
	p.manualStatus.V1.port1max = limits.P1max;
	p.manualStatus.V1.port2min = limits.P2min;
	p.manualStatus.V1.port2max = limits.P2max;
	p.manualStatus.V1.refmin = limits.Rmin;
	p.manualStatus.V1.refmax = limits.Rmax;
	HW::GetTemps(&p.manualStatus.V1.temp_source, &p.manualStatus.V1.temp_LO);
	Communication::Send(p);
	HW::Ref::update();
	if(HW::getStatusUpdateFlag()) {
		Protocol::PacketInfo packet;
		packet.type = Protocol::PacketType::DeviceStatus;
		// Enable PLL chips for temperature reading
		bool srcEn = FPGA::IsEnabled(FPGA::Periphery::SourceChip);
		bool LOEn = FPGA::IsEnabled(FPGA::Periphery::LO1Chip);
		FPGA::Enable(FPGA::Periphery::SourceChip);
		FPGA::Enable(FPGA::Periphery::LO1Chip);
		HW::getDeviceStatus(&packet.status, true);
		// restore PLL state
		FPGA::Enable(FPGA::Periphery::SourceChip, srcEn);
		FPGA::Enable(FPGA::Periphery::LO1Chip, LOEn);
		Communication::Send(packet);
	}

	// Trigger next status update
	FPGA::StartSweep();
}

void Manual::Stop() {
	active = false;
}

