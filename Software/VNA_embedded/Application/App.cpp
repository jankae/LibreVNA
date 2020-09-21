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

#define LOG_LEVEL	LOG_LEVEL_INFO
#define LOG_MODULE	"App"
#include "Log.h"

static Protocol::Datapoint result;
static Protocol::SweepSettings settings;

static Protocol::PacketInfo packet;
static TaskHandle_t handle;

#if HW_REVISION >= 'B'
// has MCU controllable flash chip, firmware update supported
#define HAS_FLASH
#include "Firmware.hpp"
extern SPI_HandleTypeDef hspi1;
static Flash flash = Flash(&hspi1, FLASH_CS_GPIO_Port, FLASH_CS_Pin);
#endif

extern ADC_HandleTypeDef hadc1;

#define FLAG_USB_PACKET		0x01
#define FLAG_DATAPOINT		0x02
#define FLAG_WORK_REQUIRED	0x04

static void VNACallback(Protocol::Datapoint res) {
	result = res;
	BaseType_t woken = false;
	xTaskNotifyFromISR(handle, FLAG_DATAPOINT, eSetBits, &woken);
	portYIELD_FROM_ISR(woken);
}
static void USBPacketReceived(Protocol::PacketInfo p) {
	packet = p;
	BaseType_t woken = false;
	xTaskNotifyFromISR(handle, FLAG_USB_PACKET, eSetBits, &woken);
	portYIELD_FROM_ISR(woken);
}
static void HardwareWorkRequired() {
	BaseType_t woken = false;
	xTaskNotifyFromISR(handle, FLAG_WORK_REQUIRED, eSetBits, &woken);
	portYIELD_FROM_ISR(woken);
}

void App_Start() {
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
#ifdef HAS_FLASH
	if(!flash.isPresent()) {
		LOG_CRIT("Failed to detect onboard FLASH");
		LED::Error(1);
	}
	auto fw_info = Firmware::GetFlashContentInfo(&flash);
	if(fw_info.valid) {
		if(fw_info.CPU_need_update) {
			// Function will not return, the device will reboot with the new firmware instead
//			Firmware::PerformUpdate(&flash, fw_info);
		}
		if(!FPGA::Configure(&flash, fw_info.FPGA_bitstream_address, fw_info.FPGA_bitstream_size)) {
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

	if (!HW::Init(HardwareWorkRequired)) {
		LOG_CRIT("Initialization failed, unable to start");
		LED::Error(4);
	}

#if HW_REVISION == 'A'
	// Allow USB enumeration
	USB_EN_GPIO_Port->BSRR = USB_EN_Pin;
#endif

	uint32_t lastNewPoint = HAL_GetTick();
	bool sweepActive = false;

	LED::Off();
	while (1) {
		uint32_t notification;
		if(xTaskNotifyWait(0x00, UINT32_MAX, &notification, 100) == pdPASS) {
			// something happened
			if(notification & FLAG_WORK_REQUIRED) {
				HW::Work();
			}
			if(notification & FLAG_DATAPOINT) {
				Protocol::PacketInfo packet;
				packet.type = Protocol::PacketType::Datapoint;
				packet.datapoint = result;
				Communication::Send(packet);
				lastNewPoint = HAL_GetTick();
			}
			if(notification & FLAG_USB_PACKET) {
				switch(packet.type) {
				case Protocol::PacketType::SweepSettings:
					LOG_INFO("New settings received");
					settings = packet.settings;
					sweepActive = VNA::Setup(settings, VNACallback);
					lastNewPoint = HAL_GetTick();
					Communication::SendWithoutPayload(Protocol::PacketType::Ack);
					break;
				case Protocol::PacketType::ManualControl:
					sweepActive = false;
					Manual::Setup(packet.manual);
					Communication::SendWithoutPayload(Protocol::PacketType::Ack);
					break;
				case Protocol::PacketType::Reference:
					HW::Ref::set(packet.reference);
					if(!sweepActive) {
						// can update right now
						HW::Ref::update();
					}
					Communication::SendWithoutPayload(Protocol::PacketType::Ack);
					break;
				case Protocol::PacketType::Generator:
					sweepActive = false;
					LOG_INFO("Updating generator setting");
					Generator::Setup(packet.generator);
					Communication::SendWithoutPayload(Protocol::PacketType::Ack);
					break;
				case Protocol::PacketType::SpectrumAnalyzerSettings:
					sweepActive = false;
					LOG_INFO("Updating spectrum analyzer settings");
					SA::Setup(packet.spectrumSettings);
					Communication::SendWithoutPayload(Protocol::PacketType::Ack);
					break;
#ifdef HAS_FLASH
				case Protocol::PacketType::ClearFlash:
					HW::SetMode(HW::Mode::Idle);
					sweepActive = false;
					LOG_DEBUG("Erasing FLASH in preparation for firmware update...");
					if(flash.eraseChip()) {
						LOG_DEBUG("...FLASH erased")
						Communication::SendWithoutPayload(Protocol::PacketType::Ack);
					} else {
						LOG_ERR("Failed to erase FLASH");
						Communication::SendWithoutPayload(Protocol::PacketType::Nack);
					}
					break;
				case Protocol::PacketType::FirmwarePacket:
					LOG_INFO("Writing firmware packet at address %u", packet.firmware.address);
					if(flash.write(packet.firmware.address, sizeof(packet.firmware.data), packet.firmware.data)) {
						Communication::SendWithoutPayload(Protocol::PacketType::Ack);
					} else {
						LOG_ERR("Failed to write FLASH");
						Communication::SendWithoutPayload(Protocol::PacketType::Nack);
					}
					break;
				case Protocol::PacketType::PerformFirmwareUpdate: {
					LOG_INFO("Firmware update process triggered");
					auto fw_info = Firmware::GetFlashContentInfo(&flash);
					if(fw_info.valid) {
						Communication::SendWithoutPayload(Protocol::PacketType::Ack);
						// Some delay to allow communication to finish
						vTaskDelay(100);
						Firmware::PerformUpdate(&flash, fw_info);
						// should never get here
						Communication::SendWithoutPayload(Protocol::PacketType::Nack);
					}
				}
					break;
#endif
				default:
					// this packet type is not supported
					Communication::SendWithoutPayload(Protocol::PacketType::Nack);
					break;
				}
			}
		}

		if(sweepActive && HAL_GetTick() - lastNewPoint > 1000) {
			LOG_WARN("Timed out waiting for point, last received point was %d (Status 0x%04x)", result.pointNum, FPGA::GetStatus());
			FPGA::AbortSweep();
			// restart the current sweep
			HW::Init(HardwareWorkRequired);
			HW::Ref::update();
			VNA::Setup(settings, VNACallback);
			sweepActive = true;
			lastNewPoint = HAL_GetTick();
		}
	}
}
