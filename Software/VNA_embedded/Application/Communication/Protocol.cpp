#include "Protocol.hpp"
#include <cstring>
#include "PacketConstants.h"

/*
 * General packet format:
 * 1. 1 byte header
 * 2. 2 byte overall packet length (with header and checksum)
 * 3. 1 byte packet type
 * 4. variable length packet payload
 * 5. 4 byte CRC32 (with header)
 */

#define CRC32_POLYGON 0xEDB88320
uint32_t Protocol::CRC32(uint32_t crc, const void *data, uint32_t len) {
	uint8_t *u8buf = (uint8_t*) data;
	int k;

	crc = ~crc;
	while (len--) {
		crc ^= *u8buf++;
		for (k = 0; k < 8; k++)
			crc = crc & 1 ? (crc >> 1) ^ CRC32_POLYGON : crc >> 1;
	}
	return ~crc;
}

uint16_t Protocol::DecodeBuffer(uint8_t *buf, uint16_t len, PacketInfo *info) {
    if (!info || !len) {
        info->type = PacketType::None;
		return 0;
	}
	uint8_t *data = buf;
	/* Remove any out-of-order bytes in front of the frame */
	while (*data != PCKT_HEADER_DATA) {
		data++;
		if(--len == 0) {
			/* Reached end of data */
			/* No frame contained in data */
			info->type = PacketType::None;
			return data - buf;
		}
	}
	/* At this point, data points to the beginning of the frame */
	if(len < PCKT_COMBINED_HEADER_LEN) {
		/* the frame header has not been completely received */
		info->type = PacketType::None;
		return data - buf;
	}

	/* Evaluate frame size */
    uint16_t length = data[PCKT_LENGTH_OFFSET] | ((uint16_t) data[2] << 8);

    if(length > sizeof(PacketInfo) * 2 || length < PCKT_EXCL_PAYLOAD_LEN) {
        // larger than twice the maximum expected packet size or too small, probably an error, ignore
        info->type = PacketType::None;
        return 1;
    }

    if(len < length) {
		/* The frame payload has not been completely received */
		info->type = PacketType::None;
		return data - buf;
	}

	/* The complete frame has been received, check checksum */
	auto type = (PacketType) data[PCKT_TYPE_OFFSET];
    uint32_t crc = (uint32_t) data[length-4] | ((uint32_t) data[length-3] << 8) | ((uint32_t) data[length-2] << 16) | ((uint32_t) data[length-1] << 24);
	if(type != PacketType::VNADatapoint) {
		uint32_t compare = CRC32(0, data, length - PCKT_CRC_LEN);
		if(crc != compare) {
			// CRC mismatch, remove header
			data += 1;
			info->type = PacketType::None;
			return data - buf;
		}
		// Valid packet, copy packet type and payload
		memcpy(info, &data[PCKT_TYPE_OFFSET], length - 7);
	} else {
		// Datapoint has the CRC set to zero
		if(crc != 0x00000000) {
			data += 1;
			info->type = PacketType::None;
			return data - buf;
		}
		// Create the datapoint
		info->type = (PacketType) data[PCKT_TYPE_OFFSET];
		info->VNAdatapoint = new VNADatapoint<32>;
		info->VNAdatapoint->decode(&data[PCKT_PAYLOAD_OFFSET], length - PCKT_EXCL_PAYLOAD_LEN);
	}

	return data - buf + length;
}

uint16_t Protocol::EncodePacket(const PacketInfo &packet, uint8_t *dest, uint16_t destsize) {
   int16_t payload_size = 0;
	switch (packet.type) {
//	case PacketType::Datapoint: payload_size = sizeof(packet.datapoint); break;
	case PacketType::SweepSettings: payload_size = sizeof(packet.settings); break;
	case PacketType::Reference:	payload_size = sizeof(packet.reference); break;
    case PacketType::DeviceInfo: payload_size = sizeof(packet.info); break;
    case PacketType::DeviceStatus: payload_size = sizeof(packet.status); break;
    case PacketType::ManualStatus: payload_size = sizeof(packet.manualStatus); break;
    case PacketType::ManualControl: payload_size = sizeof(packet.manual); break;
    case PacketType::FirmwarePacket: payload_size = sizeof(packet.firmware); break;
    case PacketType::Generator:	payload_size = sizeof(packet.generator); break;
    case PacketType::SpectrumAnalyzerSettings: payload_size = sizeof(packet.spectrumSettings); break;
    case PacketType::SpectrumAnalyzerResult: payload_size = sizeof(packet.spectrumResult); break;
    case PacketType::SourceCalPoint:
    case PacketType::ReceiverCalPoint: payload_size = sizeof(packet.amplitudePoint); break;
    case PacketType::FrequencyCorrection: payload_size = sizeof(packet.frequencyCorrection); break;
    case PacketType::DeviceConfiguration: payload_size = sizeof(packet.deviceConfig); break;
    case PacketType::Ack:
    case PacketType::PerformFirmwareUpdate:
    case PacketType::ClearFlash:
    case PacketType::Nack:
    case PacketType::RequestDeviceInfo:
    case PacketType::RequestSourceCal:
    case PacketType::RequestReceiverCal:
    case PacketType::SetIdle:
    case PacketType::RequestFrequencyCorrection:
    case PacketType::RequestDeviceConfiguration:
    case PacketType::RequestDeviceStatus:
    case PacketType::SetTrigger:
    case PacketType::ClearTrigger:
    case PacketType::StopStatusUpdates:
    case PacketType::StartStatusUpdates:
    case PacketType::InitiateSweep:
        // no payload
        break;
    case PacketType::VNADatapoint: payload_size = packet.VNAdatapoint->requiredBufferSize(); break;
    case PacketType::None:
        break;
    }
    if (payload_size < 0 || payload_size + PCKT_EXCL_PAYLOAD_LEN > destsize) {
		// encoding failed, buffer too small
		return 0;
	}
	// Write header
	dest[PCKT_HEADER_OFFSET] = PCKT_HEADER_DATA;
	uint16_t overall_size = payload_size + PCKT_EXCL_PAYLOAD_LEN;
	memcpy(&dest[PCKT_LENGTH_OFFSET], &overall_size, PCKT_LENGTH_LEN);
	// Further encoding uses a special case for VNADatapoint packettype
	uint32_t crc = 0x00000000;
	if(packet.type == PacketType::VNADatapoint) {
		// CRC calculation takes about 18us which is the bulk of the time required to encode and transmit a datapoint.
		// Skip CRC for data points to optimize throughput
		dest[PCKT_TYPE_OFFSET] = (uint8_t) packet.type;
		packet.VNAdatapoint->encode(&dest[PCKT_PAYLOAD_OFFSET], destsize - PCKT_EXCL_PAYLOAD_LEN);
		crc = 0x00000000;
	} else {
		// Copy rest of the packet
		memcpy(&dest[PCKT_TYPE_OFFSET], &packet, payload_size + PCKT_TYPE_LEN); // one additional byte for the packet type
		// Calculate the CRC
		crc = CRC32(0, dest, overall_size - PCKT_CRC_LEN);
	}
	memcpy(&dest[overall_size - PCKT_CRC_LEN], &crc, PCKT_CRC_LEN);
	return overall_size;
}

