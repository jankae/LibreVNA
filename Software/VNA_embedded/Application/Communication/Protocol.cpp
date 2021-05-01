#include "Protocol.hpp"

#include <cstring>

/*
 * General packet format:
 * 1. 1 byte header
 * 2. 2 byte overall packet length (with header and checksum)
 * 3. packet type
 * 4. packet payload
 * 5. 4 byte CRC32 (with header)
 */

static constexpr uint8_t header = 0x5A;
static constexpr uint8_t header_size = 4;

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
	while (*data != header) {
		data++;
		if(--len == 0) {
			/* Reached end of data */
			/* No frame contained in data */
			info->type = PacketType::None;
			return data - buf;
		}
	}
	/* At this point, data points to the beginning of the frame */
	if(len < header_size) {
		/* the frame header has not been completely received */
		info->type = PacketType::None;
		return data - buf;
	}

	/* Evaluate frame size */
	uint16_t length = *(uint16_t*) &data[1];
	if(len < length) {
		/* The frame payload has not been completely received */
		info->type = PacketType::None;
		return data - buf;
	}

	/* The complete frame has been received, check checksum */
	auto type = (PacketType) data[3];
	uint32_t crc = *(uint32_t*) &data[length - 4];
	if(type != PacketType::Datapoint) {
		uint32_t compare = CRC32(0, data, length - 4);
		if(crc != compare) {
			// CRC mismatch, remove header
			data += 1;
			info->type = PacketType::None;
			return data - buf;
		}
	} else {
		// Datapoint has the CRC set to zero
		if(crc != 0x00000000) {
			data += 1;
			info->type = PacketType::None;
			return data - buf;
		}
	}

	// Valid packet, copy packet type and payload
	memcpy(info, &data[3], length - 7);
	return data - buf + length;
}

uint16_t Protocol::EncodePacket(const PacketInfo &packet, uint8_t *dest, uint16_t destsize) {
   int16_t payload_size = 0;
	switch (packet.type) {
	case PacketType::Datapoint: payload_size = sizeof(packet.datapoint); break;
	case PacketType::SweepSettings: payload_size = sizeof(packet.settings); break;
	case PacketType::Reference:	payload_size = sizeof(packet.reference); break;
    case PacketType::DeviceInfo: payload_size = sizeof(packet.info); break;
    case PacketType::Status: payload_size = sizeof(packet.status); break;
    case PacketType::ManualControl: payload_size = sizeof(packet.manual); break;
    case PacketType::FirmwarePacket: payload_size = sizeof(packet.firmware); break;
    case PacketType::Generator:	payload_size = sizeof(packet.generator); break;
    case PacketType::SpectrumAnalyzerSettings: payload_size = sizeof(packet.spectrumSettings); break;
    case PacketType::SpectrumAnalyzerResult: payload_size = sizeof(packet.spectrumResult); break;
    case PacketType::SourceCalPoint:
    case PacketType::ReceiverCalPoint: payload_size = sizeof(packet.amplitudePoint); break;
    case PacketType::FrequencyCorrection: payload_size = sizeof(packet.frequencyCorrection); break;
    case PacketType::Ack:
    case PacketType::PerformFirmwareUpdate:
    case PacketType::ClearFlash:
    case PacketType::Nack:
    case PacketType::RequestDeviceInfo:
    case PacketType::RequestSourceCal:
    case PacketType::RequestReceiverCal:
    case PacketType::SetIdle:
    case PacketType::RequestFrequencyCorrection:
        // no payload
        break;
    case PacketType::None:
        break;
    }
    if (payload_size < 0 || payload_size + 8 > destsize) {
		// encoding failed, buffer too small
		return 0;
	}
	// Write header
	dest[0] = header;
	uint16_t overall_size = payload_size + 8;
	memcpy(&dest[1], &overall_size, 2);
	memcpy(&dest[3], &packet, payload_size + 1); // one additional byte for the packet type
	// Calculate checksum
	uint32_t crc = 0x00000000;
	if(packet.type == PacketType::Datapoint) {
		// CRC calculation takes about 18us which is the bulk of the time required to encode and transmit a datapoint.
		// Skip CRC for data points to optimize throughput
		crc = 0x00000000;
	} else {
		crc = CRC32(0, dest, overall_size - 4);
	}
	memcpy(&dest[overall_size - 4], &crc, 4);
	return overall_size;
}

