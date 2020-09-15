#pragma once

#include <cstdint>

namespace Protocol {

// When changing/adding/removing variables from these structs also adjust the decode/encode functions in Protocol.cpp

using Datapoint = struct _datapoint {
	float real_S11, imag_S11;
	float real_S21, imag_S21;
	float real_S12, imag_S12;
	float real_S22, imag_S22;
	uint64_t frequency;
	uint16_t pointNum;
};

using SweepSettings = struct _sweepSettings {
	uint64_t f_start;
	uint64_t f_stop;
    uint16_t points;
    uint32_t if_bandwidth;
    int16_t cdbm_excitation; // in 1/100 dbm
	uint8_t excitePort1:1;
	uint8_t excitePort2:1;
};

using ReferenceSettings = struct _referenceSettings {
	uint32_t ExtRefOuputFreq;
	uint8_t AutomaticSwitch:1;
	uint8_t UseExternalRef:1;
};

using GeneratorSettings = struct _generatorSettings {
	uint64_t frequency;
	int16_t cdbm_level;
	uint8_t activePort;
};

using DeviceInfo = struct _deviceInfo {
    uint16_t FW_major;
    uint16_t FW_minor;
    char HW_Revision;
    uint8_t extRefAvailable:1;
    uint8_t extRefInUse:1;
    uint8_t FPGA_configured:1;
    uint8_t source_locked:1;
    uint8_t LO1_locked:1;
    uint8_t ADC_overload:1;
    struct {
        uint8_t source;
        uint8_t LO1;
        uint8_t MCU;
    } temperatures;
};

using ManualStatus = struct _manualstatus {
        int16_t port1min, port1max;
        int16_t port2min, port2max;
        int16_t refmin, refmax;
        float port1real, port1imag;
        float port2real, port2imag;
        float refreal, refimag;
        uint8_t temp_source;
        uint8_t temp_LO;
        uint8_t source_locked :1;
        uint8_t LO_locked :1;
};

using ManualControl = struct _manualControl {
    // Highband Source
    uint8_t SourceHighCE :1;
    uint8_t SourceHighRFEN :1;
    uint8_t SourceHighPower :2;
    uint8_t SourceHighLowpass :2;
    uint64_t SourceHighFrequency;
    // Lowband Source
    uint8_t SourceLowEN :1;
    uint8_t SourceLowPower :2;
    uint32_t SourceLowFrequency;
    // Source signal path
    uint8_t attenuator :7;
    uint8_t SourceHighband :1;
    uint8_t AmplifierEN :1;
    uint8_t PortSwitch :1;
    // LO1
    uint8_t LO1CE :1;
    uint8_t LO1RFEN :1;
    uint64_t LO1Frequency;
    // LO2
    uint8_t LO2EN :1;
    uint32_t LO2Frequency;
    // Acquisition
    uint8_t Port1EN :1;
    uint8_t Port2EN :1;
    uint8_t RefEN :1;
    uint32_t Samples;
};


static constexpr uint16_t FirmwareChunkSize = 256;
using FirmwarePacket = struct _firmwarePacket {
    uint32_t address;
    uint8_t data[FirmwareChunkSize];
};

enum class PacketType : uint8_t {
	None = 0,
	Datapoint = 1,
	SweepSettings = 2,
    Status = 3,
    ManualControl = 4,
    DeviceInfo = 5,
    FirmwarePacket = 6,
    Ack = 7,
	ClearFlash = 8,
	PerformFirmwareUpdate = 9,
	Nack = 10,
	Reference = 11,
	Generator = 12,
};

using PacketInfo = struct _packetinfo {
	PacketType type;
	union {
		Datapoint datapoint;
		SweepSettings settings;
		ReferenceSettings reference;
		GeneratorSettings generator;
        DeviceInfo info;
        ManualControl manual;
        ManualStatus status;
        FirmwarePacket firmware;
	};
};

uint32_t CRC32(uint32_t crc, const void *data, uint32_t len);
uint16_t DecodeBuffer(uint8_t *buf, uint16_t len, PacketInfo *info);
uint16_t EncodePacket(PacketInfo packet, uint8_t *dest, uint16_t destsize);

}
