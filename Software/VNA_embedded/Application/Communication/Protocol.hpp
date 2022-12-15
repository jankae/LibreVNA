#pragma once


#include <cstdint>
#include <cstring>
#include <limits>
#include <complex>
#include "PacketConstants.h"

using namespace PacketConstants;

namespace Protocol {

static constexpr uint16_t Version = 12;

#pragma pack(push, 1)

enum class Source : uint8_t {
	Port1 = 0x01,
	Port2 = 0x02,
	Port3 = 0x04,
	Port4 = 0x08,
	Reference = 0x10,
};

template<int s> class VNADatapoint {
public:
	VNADatapoint() {
		clear();
	}

	void clear() {
		num_values = 0;
		pointNum = 0;
		cdBm = 0;
		frequency = 0;
	}
	bool addValue(float real, float imag, uint8_t stage, int sourceMask) {
		if(num_values >= s) {
			return false;
		}
		real_values[num_values] = real;
		imag_values[num_values] = imag;
		descr_values[num_values] = stage << DPNT_CONF_STAGE_OFFSET | sourceMask;
		num_values++;
		return true;
	}

	bool encode(uint8_t *dest, uint16_t destSize) {
		if(requiredBufferSize() > destSize) {
			return false;
		}
		memcpy(dest, &frequency, DPNT_FREQ_LEN);
		dest += DPNT_FREQ_LEN;
		memcpy(dest, &cdBm, DPNT_POW_LVL_LEN);
		dest += DPNT_POW_LVL_LEN;
		memcpy(dest, &pointNum, DPNT_PNT_NUM_LEN);
        dest += DPNT_PNT_NUM_LEN;
		memcpy(dest, real_values, num_values * DPNT_REAL_PART_LEN);
		dest += num_values * DPNT_REAL_PART_LEN;
		memcpy(dest, imag_values, num_values * DPNT_IMAG_PART_LEN);
		dest += num_values * DPNT_IMAG_PART_LEN;
		memcpy(dest, descr_values, num_values);
		return true;
	}
    void decode(const uint8_t *buffer, uint16_t size) {
        num_values = (size - (DPNT_FREQ_LEN + DPNT_POW_LVL_LEN + DPNT_PNT_NUM_LEN)) /
        		(DPNT_REAL_PART_LEN + DPNT_IMAG_PART_LEN + DPNT_DESC_LEN);
        memcpy(&frequency, buffer, DPNT_FREQ_LEN);
        buffer += DPNT_FREQ_LEN;
        memcpy(&cdBm, buffer, DPNT_POW_LVL_LEN);
        buffer += DPNT_POW_LVL_LEN;
        memcpy(&pointNum, buffer, DPNT_PNT_NUM_LEN);
        buffer += DPNT_PNT_NUM_LEN;
        memcpy(real_values, buffer, num_values * DPNT_REAL_PART_LEN);
        buffer += num_values * DPNT_REAL_PART_LEN;
        memcpy(imag_values, buffer, num_values * DPNT_IMAG_PART_LEN);
        buffer += num_values * DPNT_IMAG_PART_LEN;
        memcpy(descr_values, buffer, num_values);
	}

	std::complex<double> getValue(uint8_t stage, uint8_t port, bool reference) {
		uint8_t sourceMask = 0;
		sourceMask |= 0x01 << port;
		if(reference) {
			sourceMask |= (int) Source::Reference;
		}
		for(int i=0;i<num_values;i++) {
			if(descr_values[i] >> DPNT_CONF_STAGE_OFFSET != stage) {
				continue;
			}
			if((descr_values[i] & sourceMask) != sourceMask) {
				continue;
			}
			return std::complex<double>(real_values[i], imag_values[i]);
		}
		return std::numeric_limits<std::complex<double>>::quiet_NaN();
	}

    class Value {
    public:
        std::complex<double> value;
        uint8_t flags;
    };
    Value getValue(unsigned int index) {
        Value v;
        v.value = 0.0;
        v.flags = 0;
        if(index <= num_values) {
            v.value = std::complex<double>(real_values[index], imag_values[index]);
            v.flags = descr_values[index];
        }
        return v;
    }
    unsigned int getNumValues() {
        return num_values;
    }

	uint16_t requiredBufferSize() {
		return DPNT_FREQ_LEN + DPNT_POW_LVL_LEN + DPNT_PNT_NUM_LEN +
				num_values * (DPNT_REAL_PART_LEN + DPNT_IMAG_PART_LEN + DPNT_DESC_LEN);
	}

	union {
		uint64_t frequency;
		uint64_t us;
	};
	int16_t cdBm;
	uint16_t pointNum;
private:
	float real_values[s];
	float imag_values[s];
	uint8_t descr_values[s];
	uint8_t num_values;
};

using Datapoint = struct _datapoint {
	float real_S11, imag_S11;
	float real_S21, imag_S21;
	float real_S12, imag_S12;
	float real_S22, imag_S22;
	union {
		struct {
			// for non-zero span
			uint64_t frequency;
			int16_t cdbm;
		};
		struct {
			// for zero span
			uint64_t us; // time in us since first datapoint
		};
	};
    uint16_t pointNum;
};

using SweepSettings = struct _sweepSettings {
	uint64_t f_start;
	uint64_t f_stop;
    uint16_t points;
    uint32_t if_bandwidth;
    int16_t cdbm_excitation_start; // in 1/100 dbm
	uint16_t unused:1;
	uint16_t syncMaster:1;
	uint16_t suppressPeaks:1;
	uint16_t fixedPowerSetting:1; // if set the attenuator and source PLL power will not be changed across the sweep
	uint16_t logSweep:1;
	uint16_t stages:3;
	uint16_t port1Stage:3;
	uint16_t port2Stage:3;
	/*
	 * 0: no synchronization
	 * 1: USB synchronization
	 * 2: External reference synchronization
	 * 3: Trigger synchronization (not supported yet by hardware)
	 */
	uint16_t syncMode:2;
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
    uint8_t hardware_version;
    char HW_Revision;
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

using DeviceStatusV1 = struct _deviceStatusV1 {
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
};


using ManualStatusV1 = struct _manualstatusV1 {
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

using ManualControlV1 = struct _manualControlV1 {
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
	/*
	 * 0: no synchronization
	 * 1: USB synchronization
	 * 2: External reference synchronization
	 * 3: Trigger synchronization (not supported yet by hardware)
	 */
    uint8_t syncMode :2;
    uint8_t syncMaster :1;
    int64_t trackingGeneratorOffset;
    int16_t trackingPower;
};

using SpectrumAnalyzerResult = struct _spectrumAnalyzerResult {
	float port1;
	float port2;
	union {
		struct {
			// for non-zero span
			uint64_t frequency;
		};
		struct {
			// for zero span
			uint64_t us; // time in us since first datapoint
		};
	};
	uint16_t pointNum;
};


using FirmwarePacket = struct _firmwarePacket {
    uint32_t address;
    uint8_t data[FW_CHUNK_SIZE];
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

using AcquisitionFrequencySettings = struct _acquisitionfrequencysettigns {
	uint32_t IF1;
	uint8_t ADCprescaler;
	uint16_t DFTphaseInc;
};

enum class PacketType : uint8_t {
	None = 0,
	//Datapoint = 1, // Deprecated, replaced by VNADatapoint
	SweepSettings = 2,
    ManualStatusV1 = 3,
    ManualControlV1 = 4,
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
	RequestAcquisitionFrequencySettings = 23,
	AcquisitionFrequencySettings = 24,
	DeviceStatusV1 = 25,
	RequestDeviceStatus = 26,
	VNADatapoint = 27,
	SetTrigger = 28,
	ClearTrigger = 29,
	StopStatusUpdates = 30,
	StartStatusUpdates = 31,
	StopAutoIdle = 32,
	StartAutoIdle = 33
};

using PacketInfo = struct _packetinfo {
	PacketType type;
	union {
//		Datapoint datapoint; // Deprecated, use VNADatapoint instead
		SweepSettings settings;
		ReferenceSettings reference;
		GeneratorSettings generator;
		DeviceStatusV1 statusV1;
        DeviceInfo info;
        ManualControlV1 manual;
        FirmwarePacket firmware;
        ManualStatusV1 manualStatusV1;
        SpectrumAnalyzerSettings spectrumSettings;
        SpectrumAnalyzerResult spectrumResult;
        AmplitudeCorrectionPoint amplitudePoint;
        FrequencyCorrection frequencyCorrection;
        AcquisitionFrequencySettings acquisitionFrequencySettings;
        /*
         * When encoding: Pointer may go invalid after call to EncodePacket
         * When decoding: VNADatapoint is created on heap by DecodeBuffer, freeing is up to the caller
         */
        VNADatapoint<32> *VNAdatapoint;
	};
};

#pragma pack(pop)

uint32_t CRC32(uint32_t crc, const void *data, uint32_t len);
uint16_t DecodeBuffer(uint8_t *buf, uint16_t len, PacketInfo *info);
uint16_t EncodePacket(const PacketInfo &packet, uint8_t *dest, uint16_t destsize);

}
