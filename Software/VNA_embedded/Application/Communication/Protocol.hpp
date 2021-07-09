#pragma once

#include <cstdint>

namespace Protocol {

static constexpr uint16_t Version = 7;

#pragma pack(push, 1)

using Datapoint = struct _datapoint {
	float real_S11, imag_S11;
	float real_S21, imag_S21;
	float real_S12, imag_S12;
	float real_S22, imag_S22;
	uint64_t frequency;
    int16_t cdbm;
	uint16_t pointNum;
};

using SweepSettings = struct _sweepSettings {
	uint64_t f_start;
	uint64_t f_stop;
    uint16_t points;
    uint32_t if_bandwidth;
    int16_t cdbm_excitation_start; // in 1/100 dbm
	uint8_t excitePort1:1;
	uint8_t excitePort2:1;
	uint8_t suppressPeaks:1;
	uint8_t fixedPowerSetting:1; // if set the attenuator and source PLL power will not be changed across the sweep
    int16_t cdbm_excitation_stop; // in 1/100 dbm
};

using ReferenceSettings = struct _referenceSettings {
	uint32_t ExtRefOuputFreq;
	uint8_t AutomaticSwitch:1;
	uint8_t UseExternalRef:1;
};

using GeneratorSettings = struct _generatorSettings {
	uint64_t frequency;
	int16_t cdbm_level;
    uint8_t activePort :2;
    uint8_t applyAmplitudeCorrection :1;
};

using DeviceInfo = struct _deviceInfo {
	uint16_t ProtocolVersion;
    uint8_t FW_major;
    uint8_t FW_minor;
    uint8_t FW_patch;
    char HW_Revision;
    uint8_t extRefAvailable:1;
    uint8_t extRefInUse:1;
    uint8_t FPGA_configured:1;
    uint8_t source_locked:1;
    uint8_t LO1_locked:1;
    uint8_t ADC_overload:1;
    uint8_t unlevel:1;
	uint8_t temp_source;
	uint8_t temp_LO1;
	uint8_t temp_MCU;
	uint64_t limits_minFreq;
	uint64_t limits_maxFreq;
	uint32_t limits_minIFBW;
	uint32_t limits_maxIFBW;
	uint16_t limits_maxPoints;
	int16_t limits_cdbm_min;
	int16_t limits_cdbm_max;
	uint32_t limits_minRBW;
	uint32_t limits_maxRBW;
    uint8_t limits_maxAmplitudePoints;
    uint64_t limits_maxFreqHarmonic;
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
    uint8_t WindowType :2;
};

using SpectrumAnalyzerSettings = struct _spectrumAnalyzerSettings {
	uint64_t f_start;
	uint64_t f_stop;
	uint32_t RBW;
	uint16_t pointNum;
	uint8_t WindowType :2;
	uint8_t SignalID :1;
	uint8_t Detector :3;
	uint8_t UseDFT :1;
    uint8_t applyReceiverCorrection :1;
    uint8_t trackingGenerator :1;
    uint8_t applySourceCorrection :1;
    uint8_t trackingGeneratorPort :1; // 0 for port1, 1 for port2
    int64_t trackingGeneratorOffset;
    int16_t trackingPower;
};

using SpectrumAnalyzerResult = struct _spectrumAnalyzerResult {
	float port1;
	float port2;
	uint64_t frequency;
	uint16_t pointNum;
};

static constexpr uint16_t FirmwareChunkSize = 256;
using FirmwarePacket = struct _firmwarePacket {
    uint32_t address;
    uint8_t data[FirmwareChunkSize];
};

using AmplitudeCorrectionPoint = struct _amplitudecorrectionpoint {
	uint8_t totalPoints;
	uint8_t pointNum;
	uint32_t freq;
	int16_t port1;
	int16_t port2;
};

using FrequencyCorrection = struct _frequencycorrection {
	float ppm;
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
	SpectrumAnalyzerSettings = 13,
	SpectrumAnalyzerResult =  14,
    RequestDeviceInfo = 15,
	RequestSourceCal = 16,
	RequestReceiverCal = 17,
	SourceCalPoint = 18,
	ReceiverCalPoint = 19,
	SetIdle = 20,
	RequestFrequencyCorrection = 21,
	FrequencyCorrection = 22,
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
        FirmwarePacket firmware;
        ManualStatus status;
        SpectrumAnalyzerSettings spectrumSettings;
        SpectrumAnalyzerResult spectrumResult;
        AmplitudeCorrectionPoint amplitudePoint;
        FrequencyCorrection frequencyCorrection;
	};
};

#pragma pack(pop)

uint32_t CRC32(uint32_t crc, const void *data, uint32_t len);
uint16_t DecodeBuffer(uint8_t *buf, uint16_t len, PacketInfo *info);
uint16_t EncodePacket(const PacketInfo &packet, uint8_t *dest, uint16_t destsize);

}
