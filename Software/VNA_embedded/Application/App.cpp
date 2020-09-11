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
	Communication::SetCallback(USBPacketReceived);
	// Pass on logging output to USB
	Log_SetRedirect(usb_log);
	LOG_INFO("Start");
	Exti::Init();
#ifdef HAS_FLASH
	if(!flash.isPresent()) {
		LOG_CRIT("Failed to detect onboard FLASH");
	}
	auto fw_info = Firmware::GetFlashContentInfo(&flash);
	if(fw_info.valid) {
		if(fw_info.CPU_need_update) {
			// Function will not return, the device will reboot with the new firmware instead
			Firmware::PerformUpdate(&flash);
		}
		FPGA::Configure(&flash, fw_info.FPGA_bitstream_address, fw_info.FPGA_bitstream_size);
	} else {
		LOG_CRIT("Invalid bitstream/firmware, not configuring FPGA");
	}
#else
	// The FPGA configures itself from the flash, allow time for this
	vTaskDelay(2000);
#endif
	if (!VNA::Init()) {
		LOG_CRIT("Initialization failed, unable to start");
	}

#if HW_REVISION == 'A'
	// Allow USB enumeration
	USB_EN_GPIO_Port->BSRR = USB_EN_Pin;
#endif

	uint32_t lastNewPoint = HAL_GetTick();
	bool sweepActive = false;

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
					// read PLL temperatures
					uint8_t tempSource, tempLO;
					VNA::GetTemps(&tempSource, &tempLO);
					LOG_INFO("PLL temperatures: %u/%u", tempSource, tempLO);
					// Read ADC min/max
					auto limits = FPGA::GetADCLimits();
	#define ADC_LIMIT 		30000
					// Compile info packet
					packet.type = Protocol::PacketType::DeviceInfo;
					packet.info.FPGA_configured = 1;
					if(limits.P1min < -ADC_LIMIT || limits.P1max > ADC_LIMIT
							|| limits.P2min < -ADC_LIMIT || limits.P2max > ADC_LIMIT
							|| limits.Rmin < -ADC_LIMIT || limits.Rmax > ADC_LIMIT) {
						packet.info.ADC_overload = true;
					} else {
						packet.info.ADC_overload = false;
					}
					packet.info.FW_major = FW_MAJOR;
					packet.info.FW_minor = FW_MINOR;
					packet.info.HW_Revision = HW_REVISION;
					auto status = FPGA::GetStatus();
					packet.info.LO1_locked = (status & (int) FPGA::Interrupt::LO1Unlock) ? 0 : 1;
					packet.info.source_locked = (status & (int) FPGA::Interrupt::SourceUnlock) ? 0 : 1;
					packet.info.extRefAvailable = 0;
					packet.info.extRefInUse = 0;
					packet.info.temperatures.LO1 = tempLO;
					packet.info.temperatures.source = tempSource;
					packet.info.temperatures.MCU = 0;
					Communication::Send(packet);
					FPGA::ResetADCLimits();
					LOG_INFO("ADC limits: P1: %d/%d P2: %d/%d R: %d/%d",
							limits.P1min, limits.P1max, limits.P2min, limits.P2max,
							limits.Rmin, limits.Rmax);
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
					VNA::ConfigureSweep(settings, VNACallback);
					sweepActive = true;
					lastNewPoint = HAL_GetTick();
					break;
				case Protocol::PacketType::ManualControl:
					sweepActive = false;
					manual = packet.manual;
					VNA::ConfigureManual(manual, VNAStatusCallback);
					break;
#ifdef HAS_FLASH
				case Protocol::PacketType::ClearFlash:
					FPGA::AbortSweep();
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
					flash.write(packet.firmware.address, sizeof(packet.firmware.data), packet.firmware.data);
					Communication::SendWithoutPayload(Protocol::PacketType::Ack);
					break;
				case Protocol::PacketType::PerformFirmwareUpdate: {
					auto fw_info = Firmware::GetFlashContentInfo(&flash);
					if(fw_info.valid) {
						Protocol::PacketInfo p;
						p.type = Protocol::PacketType::Ack;
						Communication::Send(p);
						// Some delay to allow communication to finish
						vTaskDelay(1000);
						Firmware::PerformUpdate(&flash);
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
			LOG_WARN("Timed out waiting for point, last received point was %d", result.pointNum);
			LOG_WARN("FPGA status: 0x%04x", FPGA::GetStatus());
			// restart the current sweep
			VNA::ConfigureSweep(settings, VNACallback);
			sweepActive = true;
			lastNewPoint = HAL_GetTick();
		}
	}
}
