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

#define LOG_LEVEL	LOG_LEVEL_INFO
#define LOG_MODULE	"App"
#include "Log.h"

static Protocol::Datapoint result;
static Protocol::SweepSettings settings;

static FPGA::SamplingResult statusResult;
static Protocol::ManualControl manual;

static Protocol::PacketInfo packet;
static TaskHandle_t handle;

// TODO set proper values
//#define HW_REVISION			'A'
#define FW_MAJOR			0
#define FW_MINOR			01

#if HW_REVISION >= 'B'
// has MCU controllable flash chip, firmware update supported
#define HAS_FLASH
#include "Firmware.hpp"
extern SPI_HandleTypeDef hspi1;
static Flash flash = Flash(&hspi1, FLASH_CS_GPIO_Port, FLASH_CS_Pin);
#endif

#define FLAG_USB_PACKET		0x01
#define FLAG_DATAPOINT		0x02
#define FLAG_STATUSRESULT	0x04

static void VNACallback(Protocol::Datapoint res) {
	result = res;
	BaseType_t woken = false;
	xTaskNotifyFromISR(handle, FLAG_DATAPOINT, eSetBits, &woken);
	portYIELD_FROM_ISR(woken);
}
static void VNAStatusCallback(FPGA::SamplingResult res) {
	statusResult = res;
	BaseType_t woken = false;
	xTaskNotifyFromISR(handle, FLAG_STATUSRESULT, eSetBits, &woken);
	portYIELD_FROM_ISR(woken);
}
static void USBPacketReceived(Protocol::PacketInfo p) {
	packet = p;
	BaseType_t woken = false;
	xTaskNotifyFromISR(handle, FLAG_USB_PACKET, eSetBits, &woken);
	portYIELD_FROM_ISR(woken);
}

void App_Start() {
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

	if (!VNA::Init()) {
		LOG_CRIT("Initialization failed, unable to start");
		LED::Error(4);
	}

#if HW_REVISION == 'A'
	// Allow USB enumeration
	USB_EN_GPIO_Port->BSRR = USB_EN_Pin;
#endif

	uint32_t lastNewPoint = HAL_GetTick();
	bool sweepActive = false;
	Protocol::ReferenceSettings reference;

	LED::Off();
	while (1) {
		uint32_t notification;
		if(xTaskNotifyWait(0x00, UINT32_MAX, &notification, 100) == pdPASS) {
			// something happened
			if(notification & FLAG_DATAPOINT) {
				Protocol::PacketInfo packet;
				packet.type = Protocol::PacketType::Datapoint;
				packet.datapoint = result;
				Communication::Send(packet);
				lastNewPoint = HAL_GetTick();
				if(result.pointNum == settings.points - 1) {
					// end of sweep
					VNA::Ref::applySettings(reference);
					// Compile info packet
					packet.type = Protocol::PacketType::DeviceInfo;
					packet.info.FPGA_configured = 1;
					packet.info.FW_major = FW_MAJOR;
					packet.info.FW_minor = FW_MINOR;
					packet.info.HW_Revision = HW_REVISION;
					VNA::fillDeviceInfo(&packet.info);
					Communication::Send(packet);
					FPGA::ResetADCLimits();
					// Start next sweep
					FPGA::StartSweep();
				}
			}
			if(notification & FLAG_STATUSRESULT) {
				Protocol::PacketInfo p;
				p.type = Protocol::PacketType::Status;
				memset(&p.status, 0, sizeof(p.status));
				uint16_t isr_flags = FPGA::GetStatus();
				if (!(isr_flags & 0x0002)) {
					p.status.source_locked = 1;
				}
				if (!(isr_flags & 0x0001)) {
					p.status.LO_locked = 1;
				}
				auto limits = FPGA::GetADCLimits();
				FPGA::ResetADCLimits();
				p.status.port1min = limits.P1min;
				p.status.port1max = limits.P1max;
				p.status.port2min = limits.P2min;
				p.status.port2max = limits.P2max;
				p.status.refmin = limits.Rmin;
				p.status.refmax = limits.Rmax;
				p.status.port1real = (float) statusResult.P1I / manual.Samples;
				p.status.port1imag = (float) statusResult.P1Q / manual.Samples;
				p.status.port2real = (float) statusResult.P2I / manual.Samples;
				p.status.port2imag = (float) statusResult.P2Q / manual.Samples;
				p.status.refreal = (float) statusResult.RefI / manual.Samples;
				p.status.refimag = (float) statusResult.RefQ / manual.Samples;
				VNA::GetTemps(&p.status.temp_source, &p.status.temp_LO);
				Communication::Send(p);
				// Trigger next status update
				FPGA::StartSweep();
			}
			if(notification & FLAG_USB_PACKET) {
				switch(packet.type) {
				case Protocol::PacketType::SweepSettings:
					LOG_INFO("New settings received");
					settings = packet.settings;
					sweepActive = VNA::ConfigureSweep(settings, VNACallback);
					lastNewPoint = HAL_GetTick();
					Communication::SendWithoutPayload(Protocol::PacketType::Ack);
					break;
				case Protocol::PacketType::ManualControl:
					sweepActive = false;
					manual = packet.manual;
					VNA::ConfigureManual(manual, VNAStatusCallback);
					Communication::SendWithoutPayload(Protocol::PacketType::Ack);
					break;
				case Protocol::PacketType::Reference:
					reference = packet.reference;
					if(!sweepActive) {
						// can update right now
						VNA::Ref::applySettings(reference);
					}
					Communication::SendWithoutPayload(Protocol::PacketType::Ack);
					break;
				case Protocol::PacketType::Generator:
					sweepActive = false;
					LOG_INFO("Updating generator setting");
					VNA::ConfigureGenerator(packet.generator);
					Communication::SendWithoutPayload(Protocol::PacketType::Ack);
					break;
#ifdef HAS_FLASH
				case Protocol::PacketType::ClearFlash:
					VNA::SetIdle();
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
			VNA::Init();
			VNA::Ref::applySettings(reference);
			VNA::ConfigureSweep(settings, VNACallback);
			sweepActive = true;
			lastNewPoint = HAL_GetTick();
		}
	}
}
