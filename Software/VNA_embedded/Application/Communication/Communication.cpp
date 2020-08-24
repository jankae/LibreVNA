#include <Communication.h>

#include "stm.hpp"
#include "../App.h"
#include <string.h>
#include "USB/usb.h"

static uint8_t inputBuffer[1024];
uint16_t inputCnt = 0;
static uint8_t outputBuffer[1024];

//#include "usbd_def.h"
//#include "usbd_cdc_if.h"

//extern USBD_HandleTypeDef hUsbDeviceFS;

void Communication::Input(const uint8_t *buf, uint16_t len) {
	if (inputCnt + len < sizeof(inputBuffer)) {
		// add received data to input buffer
		memcpy(&inputBuffer[inputCnt], buf, len);
		inputCnt += len;
	}
	Protocol::PacketInfo packet;
	uint16_t handled_len;
	do {
		handled_len = Protocol::DecodeBuffer(inputBuffer, inputCnt, &packet);
		if (handled_len == inputCnt) {
			// complete input buffer used up, reset counter
			inputCnt = 0;
		} else {
			// only used part of the buffer, move up remaining bytes
			uint16_t remaining = inputCnt - handled_len;
			memmove(inputBuffer, &inputBuffer[handled_len], remaining);
			inputCnt = remaining;
		}
		switch(packet.type) {
		case Protocol::PacketType::SweepSettings:
			App::NewSettings(packet.settings);
			break;
		case Protocol::PacketType::ManualControl:
			App::SetManual(packet.manual);
			break;
		}
	} while (handled_len > 0);
}

bool Communication::Send(Protocol::PacketInfo packet) {
	uint16_t len = Protocol::EncodePacket(packet, outputBuffer,
					sizeof(outputBuffer));
	return usb_transmit(outputBuffer, len);
//	if (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED) {
//		uint16_t len = Protocol::EncodePacket(packet, outputBuffer,
//				sizeof(outputBuffer));
//		while (CDC_Transmit_FS(outputBuffer, len) != USBD_OK)
//			;
//		return true;
//	} else {
//		// not connected, do not attempt to send
//		return false;
//	}
}

void communication_usb_input(const uint8_t *buf, uint16_t len) {
	Communication::Input(buf, len);
}
