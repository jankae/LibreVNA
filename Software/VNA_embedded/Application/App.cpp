#include <Cal.hpp>
#include <VNA.hpp>
#include "App.h"

#include "delay.hpp"
#include "Communication.h"
#include "main.h"
#include "Exti.hpp"
#include "FPGA/FPGA.hpp"
#include <complex>
#include <cstring>
#include "USB/usb.h"
#include "Flash.hpp"
#include "FreeRTOS.h"
#include "task.h"
#include "Led.hpp"
#include "Hardware.hpp"
#include "Manual.hpp"
#include "Generator.hpp"
#include "SpectrumAnalyzer.hpp"
#include "HW_HAL.hpp"
#include "Trigger.hpp"

#define LOG_LEVEL	LOG_LEVEL_INFO
#define LOG_MODULE	"App"
#include "Log.h"

static Protocol::PacketInfo recv_packet;
static Protocol::PacketInfo last_measure_packet; // contains the command that started the last measured (replay in case of timeout)
static TaskHandle_t handle;
static bool sweepActive;

#if HW_REVISION >= 'B'
// has MCU controllable flash chip, firmware update supported
#define HAS_FLASH
#include "Firmware.hpp"
#endif

extern ADC_HandleTypeDef hadc1;

#define FLAG_USB_PACKET			0x01
#define FLAG_TRIGGER_OUT_ISR	0x02

static bool lastReportedTrigger;

static void USBPacketReceived(const Protocol::PacketInfo &p) {
	recv_packet = p;
	BaseType_t woken = false;
	xTaskNotifyFromISR(handle, FLAG_USB_PACKET, eSetBits, &woken);
	portYIELD_FROM_ISR(woken);
}

static void TriggerOutISR() {
	BaseType_t woken = false;
	xTaskNotifyFromISR(handle, FLAG_TRIGGER_OUT_ISR, eSetBits, &woken);
	portYIELD_FROM_ISR(woken);
}

inline void App_Init() {
	STM::Init();
	Delay::Init();
	HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
	handle = xTaskGetCurrentTaskHandle();
	usb_init(communication_usb_input);
	Log_Init();
	LED::Init();
	LED::Pulsating();
	Communication::SetCallback(USBPacketReceived);
	// Pass on logging output to USB
	Log_SetRedirect(usb_log);
	LOG_INFO("Start");
	Exti::Init();
	Trigger::Init(TriggerOutISR);
#ifdef HAS_FLASH
	if(!HWHAL::flash.isPresent()) {
		LOG_CRIT("Failed to detect onboard FLASH");
		LED::Error(1);
	}
	auto fw_info = Firmware::GetFlashContentInfo();
	if(fw_info.valid) {
		if(fw_info.CPU_need_update) {
			// Function will not return, the device will reboot with the new firmware instead
//			Firmware::PerformUpdate(&flash, fw_info);
		}
		if(!FPGA::Configure(fw_info.FPGA_bitstream_address, fw_info.FPGA_bitstream_size)) {
			LOG_CRIT("FPGA configuration failed");
			LED::Error(3);
		}
	} else {
		LOG_CRIT("Invalid bitstream/firmware, not configuring FPGA");
		LED::Error(2);
	}
#else
	// The FPGA configures itself from the flash, allow time for this
	vTaskDelay(2000);
#endif
#if HW_REVISION == 'B'
	// Enable supply to RF circuit
	EN_6V_GPIO_Port->BSRR = EN_6V_Pin;
#endif

	Cal::Load();

	if (!HW::Init()) {
		LOG_CRIT("Initialization failed, unable to start");
		LED::Error(4);
	}

#if HW_REVISION == 'A'
	// Allow USB enumeration
	USB_EN_GPIO_Port->BSRR = USB_EN_Pin;
#endif

	LED::Off();

	sweepActive = false;
}

inline void App_Process() {
	while(1) {
		uint32_t notification;
		if(xTaskNotifyWait(0x00, UINT32_MAX, &notification, 100) == pdPASS) {
			// something happened
			if(notification & FLAG_USB_PACKET) {
				switch(recv_packet.type) {
				case Protocol::PacketType::SweepSettings:
					LOG_INFO("New settings received");
					last_measure_packet = recv_packet;
					sweepActive = VNA::Setup(recv_packet.settings);
					Communication::SendWithoutPayload(Protocol::PacketType::Ack);
					break;
				case Protocol::PacketType::ManualControl:
					sweepActive = false;
					last_measure_packet = recv_packet;
					Manual::Setup(recv_packet.manual);
					Communication::SendWithoutPayload(Protocol::PacketType::Ack);
					break;
				case Protocol::PacketType::Reference:
					HW::Ref::set(recv_packet.reference);
					if(!sweepActive) {
						// can update right now
						HW::Ref::update();
					}
					Communication::SendWithoutPayload(Protocol::PacketType::Ack);
					break;
				case Protocol::PacketType::Generator:
					sweepActive = true;
					last_measure_packet = recv_packet;
					LOG_INFO("Updating generator setting");
					Generator::Setup(recv_packet.generator);
					Communication::SendWithoutPayload(Protocol::PacketType::Ack);
					break;
				case Protocol::PacketType::SpectrumAnalyzerSettings:
					sweepActive = true;
					last_measure_packet = recv_packet;
					LOG_INFO("Updating spectrum analyzer settings");
					SA::Setup(recv_packet.spectrumSettings);
					Communication::SendWithoutPayload(Protocol::PacketType::Ack);
					break;
				case Protocol::PacketType::RequestDeviceInfo: {
					Communication::SendWithoutPayload(Protocol::PacketType::Ack);
					Protocol::PacketInfo p;
					p.type = Protocol::PacketType::DeviceInfo;
					p.info = HW::Info;
					Communication::Send(p);
				}
					break;
				case Protocol::PacketType::RequestDeviceStatus: {
					Communication::SendWithoutPayload(Protocol::PacketType::Ack);
					Protocol::PacketInfo p;
					p.type = Protocol::PacketType::DeviceStatus;
					HW::getDeviceStatus(&p.status);
					Communication::Send(p);
				}
					break;
				case Protocol::PacketType::StopStatusUpdates: {
					HW::setStatusUpdateFlag(false);
					Communication::SendWithoutPayload(Protocol::PacketType::Ack);
				}
					break;
				case Protocol::PacketType::StartStatusUpdates: {
					HW::setStatusUpdateFlag(true);
					Communication::SendWithoutPayload(Protocol::PacketType::Ack);
				}
					break;
				case Protocol::PacketType::InitiateSweep: {
					if(VNA::GetStandbyMode()) {
						VNA::InitiateSweep();
						Communication::SendWithoutPayload(Protocol::PacketType::Ack);
					} else {
						Communication::SendWithoutPayload(Protocol::PacketType::Nack);
					}

				}
					break;
				case Protocol::PacketType::SetIdle:
					HW::SetMode(HW::Mode::Idle);
					sweepActive = false;
					Communication::SendWithoutPayload(Protocol::PacketType::Ack);
					break;
		#ifdef HAS_FLASH
				case Protocol::PacketType::ClearFlash:
					HW::SetMode(HW::Mode::Idle);
					sweepActive = false;
					LOG_DEBUG("Erasing FLASH in preparation for firmware update...");
					if(HWHAL::flash.eraseRange(0, Firmware::maxSize)) {
						LOG_DEBUG("...FLASH erased")
						Communication::SendWithoutPayload(Protocol::PacketType::Ack);
					} else {
						LOG_ERR("Failed to erase FLASH");
						Communication::SendWithoutPayload(Protocol::PacketType::Nack);
					}
					break;
				case Protocol::PacketType::FirmwarePacket:
					LOG_INFO("Writing firmware packet at address %u", recv_packet.firmware.address);
					if(HWHAL::flash.write(recv_packet.firmware.address, sizeof(recv_packet.firmware.data), recv_packet.firmware.data)) {
						Communication::SendWithoutPayload(Protocol::PacketType::Ack);
					} else {
						LOG_ERR("Failed to write FLASH");
						Communication::SendWithoutPayload(Protocol::PacketType::Nack);
					}
					break;
				case Protocol::PacketType::PerformFirmwareUpdate: {
					LOG_INFO("Firmware update process triggered");
					auto fw_info = Firmware::GetFlashContentInfo();
					if(fw_info.valid) {
						Communication::SendWithoutPayload(Protocol::PacketType::Ack);
						// Some delay to allow communication to finish
						vTaskDelay(100);
						Firmware::PerformUpdate(fw_info);
						// should never get here
						Communication::SendWithoutPayload(Protocol::PacketType::Nack);
					}
				}
					break;
		#endif
				case Protocol::PacketType::RequestSourceCal:
					Communication::SendWithoutPayload(Protocol::PacketType::Ack);
					Cal::SendSource();
					break;
				case Protocol::PacketType::RequestReceiverCal:
					Communication::SendWithoutPayload(Protocol::PacketType::Ack);
					Cal::SendReceiver();
					break;
				case Protocol::PacketType::SourceCalPoint:
					Cal::AddSourcePoint(recv_packet.amplitudePoint);
					Communication::SendWithoutPayload(Protocol::PacketType::Ack);
					break;
				case Protocol::PacketType::ReceiverCalPoint:
					Cal::AddReceiverPoint(recv_packet.amplitudePoint);
					Communication::SendWithoutPayload(Protocol::PacketType::Ack);
					break;
				case Protocol::PacketType::RequestFrequencyCorrection:
					Communication::SendWithoutPayload(Protocol::PacketType::Ack);
					{
						Protocol::PacketInfo send;
						send.type = Protocol::PacketType::FrequencyCorrection;
						send.frequencyCorrection.ppm = Cal::getFrequencyCal();
						Communication::Send(send);
					}
					break;
				case Protocol::PacketType::FrequencyCorrection:
					Cal::setFrequencyCal(recv_packet.frequencyCorrection.ppm);
					Communication::SendWithoutPayload(Protocol::PacketType::Ack);
					break;
				case Protocol::PacketType::RequestDeviceConfiguration:
					Communication::SendWithoutPayload(Protocol::PacketType::Ack);
					{
						Protocol::PacketInfo send;
						send.type = Protocol::PacketType::DeviceConfiguration;
						send.deviceConfig = HW::getDeviceConfig();
						Communication::Send(send);
					}
					break;
				case Protocol::PacketType::DeviceConfiguration:
					HW::setDeviceConfig(recv_packet.deviceConfig);
					Communication::SendWithoutPayload(Protocol::PacketType::Ack);
					break;
				case Protocol::PacketType::SetTrigger:
					if(Trigger::GetMode() == Trigger::Mode::USB_GUI) {
						Trigger::SetInput(true);
						Communication::SendWithoutPayload(Protocol::PacketType::Ack);
					} else {
						Communication::SendWithoutPayload(Protocol::PacketType::Nack);
					}
					break;
				case Protocol::PacketType::ClearTrigger:
					if(Trigger::GetMode() == Trigger::Mode::USB_GUI) {
						Trigger::SetInput(false);
						Communication::SendWithoutPayload(Protocol::PacketType::Ack);
					} else {
						Communication::SendWithoutPayload(Protocol::PacketType::Nack);
					}
					break;
				default:
					// this packet type is not supported
					Communication::SendWithoutPayload(Protocol::PacketType::Nack);
					break;
				}
			}
			if(notification & FLAG_TRIGGER_OUT_ISR) {
				// trigger output (from FPGA) changed level
				bool set = Trigger::GetOutput();
				switch(Trigger::GetMode()) {
				case Trigger::Mode::Off:
					// nothing to do
					break;
				case Trigger::Mode::USB_GUI:
					lastReportedTrigger = set;
					Communication::SendWithoutPayload(set ? Protocol::PacketType::SetTrigger : Protocol::PacketType::ClearTrigger);
					break;
				case Trigger::Mode::ExtRef:
					if(set) {
						HWHAL::Si5351.Enable(HWHAL::SiChannel::ReferenceOut);
					} else {
						HWHAL::Si5351.Disable(HWHAL::SiChannel::ReferenceOut);
					}
					break;
				case Trigger::Mode::Trigger:
					// not supported by the hardware, nothing to do
					break;
				}
			}
		}
		if(HW::TimedOut()) {
			HW::SetMode(HW::Mode::Idle);
			// insert the last received packet (restarts the timed out operation)
			Communication::BlockNextAck();
			USBPacketReceived(last_measure_packet);
		}
		HW::updateDeviceStatus();
	}
}

#include "HW_HAL.hpp"

void App_Start() {
	App_Init();

//	uint32_t LO2_1 = 61750000;
//	uint32_t LO2_2 = 61752000;
//
//	HWHAL::Si5351.Enable(HWHAL::SiChannel::Port1LO2);
//	HWHAL::Si5351.Enable(HWHAL::SiChannel::Port2LO2);
//	HWHAL::Si5351.Enable(HWHAL::SiChannel::RefLO2);
//
////	HWHAL::Si5351.Disable(HWHAL::SiChannel::Port1LO2);
////	HWHAL::Si5351.Disable(HWHAL::SiChannel::Port2LO2);
////	HWHAL::Si5351.Disable(HWHAL::SiChannel::RefLO2);
//
////	FPGA::Enable(FPGA::Periphery::Port1Mixer);
////	FPGA::Enable(FPGA::Periphery::Port2Mixer);
////	FPGA::Enable(FPGA::Periphery::RefMixer);
//
//	uint32_t i=64000000;
//	HWHAL::Si5351.SetCLK(HWHAL::SiChannel::Port1LO2, i, Si5351C::PLL::B, Si5351C::DriveStrength::mA2);
//	HWHAL::Si5351.SetCLK(HWHAL::SiChannel::Port2LO2, i, Si5351C::PLL::B, Si5351C::DriveStrength::mA2);
//	HWHAL::Si5351.SetCLK(HWHAL::SiChannel::RefLO2, i, Si5351C::PLL::B, Si5351C::DriveStrength::mA2);
//	HWHAL::Si5351.ResetPLL(Si5351C::PLL::B);
//	while(1) {
//		for(i=61000000;i<62000000;i++) {
//			LOG_INFO("Setting LO2=%lu", i);
//			HWHAL::Si5351.SetPLL(Si5351C::PLL::B, i*13, Si5351C::PLLSource::XTAL);
////			HWHAL::Si5351.Disable(HWHAL::SiChannel::Port1LO2);
////			HWHAL::Si5351.Disable(HWHAL::SiChannel::Port2LO2);
////			HWHAL::Si5351.Disable(HWHAL::SiChannel::RefLO2);
////			HWHAL::Si5351.SetCLK(HWHAL::SiChannel::Port1LO2, i, Si5351C::PLL::B, Si5351C::DriveStrength::mA2);
////			HWHAL::Si5351.SetCLK(HWHAL::SiChannel::Port2LO2, i, Si5351C::PLL::B, Si5351C::DriveStrength::mA2);
////			HWHAL::Si5351.SetCLK(HWHAL::SiChannel::RefLO2, i, Si5351C::PLL::B, Si5351C::DriveStrength::mA2);
//
////			HWHAL::Si5351.Enable(HWHAL::SiChannel::Port1LO2);
////			HWHAL::Si5351.Enable(HWHAL::SiChannel::Port2LO2);
////			HWHAL::Si5351.Enable(HWHAL::SiChannel::RefLO2);
////			HWHAL::Si5351.ResetPLL(Si5351C::PLL::B);
////			HWHAL::Si5351.WaitForLock(Si5351C::PLL::B, 10);
//			vTaskDelay(1);
//		}
//	}

	App_Process();
}
