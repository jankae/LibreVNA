#include "App.h"

#include "delay.hpp"
#include "Communication.h"
#include "main.h"
#include "Exti.hpp"
#include <complex>
#include <cstring>
#include "USB/usb.h"
#include "FreeRTOS.h"
#include "task.h"
#include "Led.hpp"
#include "RegisterDevice.hpp"
#include "Hardware.hpp"

#define LOG_LEVEL	LOG_LEVEL_INFO
#define LOG_MODULE	"App"
#include "Log.h"

static Protocol::PacketInfo recv_packet;
static TaskHandle_t handle;

#define FLAG_USB_PACKET		0x01

static void USBPacketReceived(const Protocol::PacketInfo &p) {
	recv_packet = p;
	BaseType_t woken = false;
	xTaskNotifyFromISR(handle, FLAG_USB_PACKET, eSetBits, &woken);
	portYIELD_FROM_ISR(woken);
}

void App_Start() {
	STM::Init();
	handle = xTaskGetCurrentTaskHandle();
	usb_init(communication_usb_input);
	Log_Init();
	LED::Init();
	LED::Pulsating(3);
	Communication::SetCallback(USBPacketReceived);
	// Pass on logging output to USB
	Log_SetRedirect(usb_log);
	LOG_INFO("Start");
	Exti::Init();

	if (!HW::Init()) {
		LOG_CRIT("Initialization failed, unable to start");
		LED::Error(3, 4);
	}

	LED::Off(3);
	LED::On(0);
	while (1) {
		uint32_t notification;
		if(xTaskNotifyWait(0x00, UINT32_MAX, &notification, 100) == pdPASS) {
			// something happened
			if(notification & FLAG_USB_PACKET) {
				switch(recv_packet.type) {
				case Protocol::PacketType::RequestDeviceInfo:
					Communication::SendWithoutPayload(Protocol::PacketType::Ack);
					Protocol::PacketInfo p;
					p.type = Protocol::PacketType::DeviceInfo;
					HW::fillDeviceInfo(&p.info);
					Communication::Send(p);
					break;
				case Protocol::PacketType::RequestDirectRegisterInfo:
					Communication::SendWithoutPayload(Protocol::PacketType::Ack);
					LOG_INFO("Requested register devices, sending %d packets", RegisterDevice::getNumDevices());
					for(uint8_t i=0;i<RegisterDevice::getNumDevices();i++) {
						auto dev = RegisterDevice::getDevice(i);
						Protocol::PacketInfo send;
						send.type = Protocol::PacketType::DirectRegisterInfo;
						send.directRegInfo = dev->getInfo();
						Communication::Send(send);
					}
					break;
				case Protocol::PacketType::DirectRegisterWrite:
					if(recv_packet.directRegWrite.device >= RegisterDevice::getNumDevices()) {
						// invalid device
						Communication::SendWithoutPayload(Protocol::PacketType::Nack);
					} else {
						LOG_INFO(
								"Register write: dev %u, address %u, data 0x%08x",
								recv_packet.directRegWrite.device,
								recv_packet.directRegWrite.address,
								(uint32_t) recv_packet.directRegWrite.data);
						Communication::SendWithoutPayload(Protocol::PacketType::Ack);
						auto dev = RegisterDevice::getDevice(recv_packet.directRegWrite.device);
						dev->writeRegister(recv_packet.directRegWrite.address, recv_packet.directRegWrite.data);
					}
					break;
				case Protocol::PacketType::DirectRegisterRead:
					if(recv_packet.directRegWrite.device >= RegisterDevice::getNumDevices()) {
						// invalid device
						Communication::SendWithoutPayload(Protocol::PacketType::Nack);
					} else {
						Communication::SendWithoutPayload(Protocol::PacketType::Ack);
						auto dev = RegisterDevice::getDevice(recv_packet.directRegWrite.device);
						Protocol::PacketInfo send;
						send.type = Protocol::PacketType::DirectRegisterWrite;
						send.directRegWrite.device = recv_packet.directRegRead.device;
						send.directRegWrite.address = recv_packet.directRegRead.address;
						send.directRegWrite.data = dev->readRegister(recv_packet.directRegRead.address);
						Communication::Send(send);
					}
					break;
				default:
					// this packet type is not supported
					Communication::SendWithoutPayload(Protocol::PacketType::Nack);
					break;
				}
			}
		}
	}
}
