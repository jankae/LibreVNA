#pragma once


#include <cstdint>


namespace PacketConstants {
	//  USB protocol packet field constants

	static constexpr uint8_t PCKT_HEADER_DATA = 0x5A;


	/////////////////////////////////////////////////////////////
	//  General packet structure field sizes and offsets in bytes
	static constexpr uint8_t PCKT_HEADER_OFFSET = 0;
	static constexpr uint8_t PCKT_HEADER_LEN = 1;
	static constexpr uint8_t PCKT_LENGTH_OFFSET = PCKT_HEADER_OFFSET + PCKT_HEADER_LEN;  // offset one byte
	static constexpr uint8_t PCKT_LENGTH_LEN = 2;
	static constexpr uint8_t PCKT_TYPE_OFFSET = PCKT_LENGTH_OFFSET + PCKT_LENGTH_LEN;  // offset three bytes
	static constexpr uint8_t PCKT_TYPE_LEN = 1;
	static constexpr uint8_t PCKT_PAYLOAD_OFFSET = PCKT_TYPE_OFFSET + PCKT_TYPE_LEN;  // offset four bytes
	static constexpr uint8_t PCKT_CRC_LEN = 4;

	static constexpr uint8_t PCKT_COMBINED_HEADER_LEN = PCKT_HEADER_LEN + PCKT_LENGTH_LEN + PCKT_TYPE_LEN;  //  combined length of fields preceding payload
	static constexpr uint8_t PCKT_EXCL_PAYLOAD_LEN = PCKT_COMBINED_HEADER_LEN + PCKT_CRC_LEN;  // combined length of all packet fields, excluding payload

	/////////////////////////////////////////////////////////////
	//  Payload content

	//  Firmware packets
	static constexpr uint16_t FW_CHUNK_SIZE = 256;

	//  VNADataPoint payload fields in bytes
	static constexpr uint8_t DPNT_FREQ_LEN = 8;
	static constexpr uint8_t DPNT_POW_LVL_LEN = 2;
	static constexpr uint8_t DPNT_PNT_NUM_LEN = 2;
	static constexpr uint8_t DPNT_REAL_PART_LEN = 4;
	static constexpr uint8_t DPNT_IMAG_PART_LEN = 4;
	static constexpr uint8_t DPNT_DESC_LEN = 1;

	// VNADataPoint configuration bitmask offsets in bits
	static constexpr uint8_t DPNT_CONF_P1_OFFSET = 0;
	static constexpr uint8_t DPNT_CONF_P2_OFFSET = 1;
	static constexpr uint8_t DPNT_CONF_P3_OFFSET = 2;
	static constexpr uint8_t DPNT_CONF_P4_OFFSET = 3;
	static constexpr uint8_t DPNT_CONF_REF_OFFSET = 4;
	static constexpr uint8_t DPNT_CONF_STAGE_OFFSET = 5;

}
