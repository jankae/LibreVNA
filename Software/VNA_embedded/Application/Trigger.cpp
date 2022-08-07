#include "Trigger.hpp"

#include "Drivers/Exti.hpp"
#include "main.h"
#include "HW_HAL.hpp"
#include "Communication/Protocol.hpp"
#include "Hardware.hpp"

static Trigger::Mode mode;
static Trigger::CallbackISR callback = nullptr;

void Trigger::Init(CallbackISR cb) {
	mode = Mode::Off;
	callback = cb;
	Exti::SetCallback(FPGA_TRIGGER_OUT_GPIO_Port, FPGA_TRIGGER_OUT_Pin, Exti::EdgeType::Both, Exti::Pull::Down, [](void*){
		STM::DispatchToInterrupt(callback);
	}, nullptr);
}

extern "C" {
void vApplicationIdleHook() {
	if(mode == Trigger::Mode::ExtRef) {
		STM::DispatchToInterrupt([](){
			Trigger::SetInput(HW::Ref::available());
		});
	}
}
}

void Trigger::SetMode(Mode m) {
	if(mode == m) {
		// already in the correct mdoe
		return;
	}
	if(mode == Mode::ExtRef) {
		// reset reference to default settings
		HWHAL::Si5351.Disable(HWHAL::SiChannel::ReferenceOut);
	}
	mode = m;
	if(mode == Mode::ExtRef) {
		// Disable the external reference
		Protocol::ReferenceSettings s;
		s.AutomaticSwitch = 0;
		s.ExtRefOuputFreq = 0;
		s.UseExternalRef = 0;
		HW::Ref::set(s);
		HW::Ref::update();

		HWHAL::Si5351.SetCLK(HWHAL::SiChannel::ReferenceOut, 10000000, Si5351C::PLL::A);
		if(GetOutput()) {
			HWHAL::Si5351.Enable(HWHAL::SiChannel::ReferenceOut);
		} else {
			HWHAL::Si5351.Disable(HWHAL::SiChannel::ReferenceOut);
		}
	}
}
Trigger::Mode Trigger::GetMode() {
	return mode;
}

void Trigger::SetInput(bool high) {
	if(high) {
		FPGA_TRIGGER_IN_GPIO_Port->BSRR = FPGA_TRIGGER_IN_Pin;
	} else {
		FPGA_TRIGGER_IN_GPIO_Port->BSRR = FPGA_TRIGGER_IN_Pin << 16;
	}
}
bool Trigger::GetOutput() {
	return FPGA_TRIGGER_OUT_GPIO_Port->IDR & FPGA_TRIGGER_OUT_Pin;
}
bool Trigger::GetInput() {
	return FPGA_TRIGGER_IN_GPIO_Port->IDR & FPGA_TRIGGER_IN_Pin;
}
