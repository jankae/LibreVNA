#pragma once

#include <Cal.hpp>
#include <cstdint>
#include "Protocol.hpp"
#include "FPGA/FPGA.hpp"
#include "max2871.hpp"
#include "Si5351C.hpp"

#define USE_DEBUG_PINS	0

#if USE_DEBUG_PINS
#define DEBUG1_GPIO		GPIOA
#define DEBUG1_PIN		GPIO_PIN_13
#define DEBUG2_GPIO		GPIOA
#define DEBUG2_PIN		GPIO_PIN_14

#define DEBUG1_LOW() do {DEBUG1_GPIO->BSRR = DEBUG1_PIN<<16; }while(0)
#define DEBUG1_HIGH() do {DEBUG1_GPIO->BSRR = DEBUG1_PIN; }while(0)
#define DEBUG2_LOW() do {DEBUG2_GPIO->BSRR = DEBUG2_PIN<<16; }while(0)
#define DEBUG2_HIGH() do {DEBUG2_GPIO->BSRR = DEBUG2_PIN; }while(0)
#else
#define DEBUG1_LOW()
#define DEBUG1_HIGH()
#define DEBUG2_LOW()
#define DEBUG2_HIGH()
#endif

namespace HW {

static constexpr uint32_t TCXOFrequency = 26000000;
static constexpr uint32_t ExtRefInFrequency = 10000000;
static constexpr uint32_t ExtRefOut1Frequency = 10000000;
static constexpr uint32_t ExtRefOut2Frequency = 10000000;
static constexpr uint32_t SI5351CPLLConstantFrequency = 800000000;
static constexpr uint32_t FPGAClkInFrequency = 16000000;
static constexpr uint32_t DefaultADCSamplerate = 800000;
static constexpr uint32_t DefaultIF1 = 62000000;
static constexpr uint32_t DefaultIF2 = 250000;
static constexpr uint32_t LO1_minFreq = 25000000;
static constexpr uint32_t MaxSamples = 130944;
static constexpr uint32_t MinSamples = 16;
static constexpr uint32_t PLLRef = 100000000;
static constexpr uint32_t BandSwitchFrequency = 25000000;
static constexpr uint32_t DefaultLO2 = DefaultIF1 - DefaultIF2;
static constexpr uint8_t LO2Multiplier = 13;
static constexpr uint32_t SI5351CPLLAlignedFrequency = DefaultLO2 * LO2Multiplier;
static constexpr uint16_t DefaultPLLSettlingDelay = 60;
static constexpr uint16_t MinPLLSettlingDelay = 10;

static constexpr uint8_t DefaultADCprescaler = FPGA::Clockrate / DefaultADCSamplerate;
static_assert(DefaultADCprescaler * DefaultADCSamplerate == FPGA::Clockrate, "ADCSamplerate can not be reached exactly");
static constexpr uint16_t DefaultDFTphaseInc = 4096 * DefaultIF2 / DefaultADCSamplerate;
static_assert(DefaultDFTphaseInc * DefaultADCSamplerate == 4096 * DefaultIF2, "DFT can not be computed for 2.IF");

static constexpr uint16_t _fpga_div = SI5351CPLLConstantFrequency / FPGAClkInFrequency;
static_assert(_fpga_div * FPGAClkInFrequency == SI5351CPLLConstantFrequency && _fpga_div >= 6 && _fpga_div <= 254 && (_fpga_div & 0x01) == 0, "Unable to generate FPGA clock input frequency");

static constexpr uint16_t _ref_out1_div = SI5351CPLLConstantFrequency / ExtRefOut1Frequency;
static_assert(_ref_out1_div * ExtRefOut1Frequency == SI5351CPLLConstantFrequency && _ref_out1_div >= 6 && _ref_out1_div <= 254 && (_ref_out1_div & 0x01) == 0, "Unable to generate first reference output frequency");

static constexpr uint16_t _ref_out2_div = SI5351CPLLConstantFrequency / ExtRefOut2Frequency;
static_assert(_ref_out2_div * ExtRefOut2Frequency == SI5351CPLLConstantFrequency && _ref_out2_div >= 6 && _ref_out2_div <= 254 && (_ref_out2_div & 0x01) == 0, "Unable to generate first reference output frequency");


// approximate output power at low frequencies with different source strength settings (attenuator = 0) in cdbm
static constexpr int16_t LowBandMinPower = -1350;
static constexpr int16_t LowBandMaxPower = -190;
static constexpr int16_t HighBandMinPower = -1060;
static constexpr int16_t HighBandMaxPower = -160;

static constexpr Protocol::DeviceInfo Info = {
		.ProtocolVersion = Protocol::Version,
		.FW_major = FW_MAJOR,
		.FW_minor = FW_MINOR,
		.FW_patch = FW_PATCH,
		.hardware_version = 1,
		.HW_Revision = HW_REVISION,
		.limits_minFreq = 0,
		.limits_maxFreq = 6000000000,
		.limits_minIFBW = DefaultADCSamplerate / MaxSamples,
		.limits_maxIFBW = DefaultADCSamplerate / MinSamples,
		.limits_maxPoints = FPGA::MaxPoints,
		.limits_cdbm_min = -4000,
		.limits_cdbm_max = 0,
		.limits_minRBW = (uint32_t) (DefaultADCSamplerate * 2.23f / MaxSamples),
		.limits_maxRBW = (uint32_t) (DefaultADCSamplerate * 2.23f / MinSamples),
		.limits_maxAmplitudePoints = Cal::maxPoints,
		.limits_maxFreqHarmonic = 18000000000,
		.num_ports = 2,
		.limits_maxDwellTime = 10239,
};

enum class Mode {
	Idle,
	Manual,
	Generator,
	VNA,
	SA,
};

bool Init();
void SetMode(Mode mode);
void SetIdle();
void Work();
bool TimedOut();
uint64_t getLastISRTimestamp();

void SetOutputUnlevel(bool unlev);

bool getStatusUpdateFlag();

void setStatusUpdateFlag(bool flag);

void updateDeviceStatus();

using AmplitudeSettings = struct _amplitudeSettings {
	uint8_t attenuator;
	union {
		MAX2871::Power highBandPower;
		Si5351C::DriveStrength lowBandPower;
	};
	bool unlevel;
};
AmplitudeSettings GetAmplitudeSettings(int16_t cdbm, uint64_t freq = 0, bool applyCorrections = false, bool port2 = false);

bool GetTemps(uint8_t *source, uint8_t *lo);
void getDeviceStatus(Protocol::DeviceStatus *status, bool updateEvenWhenBusy = false);
namespace Ref {
	bool available();
	bool usingExternal();
	// reference won't change until update is called
	void set(Protocol::ReferenceSettings s);
	void update();
	Si5351C::PLLSource getSource();
}

// Device configuration settings
constexpr uint32_t flash_address = Firmware::maxSize + Cal::flash_size; // stored directly behind calibration in flash
constexpr uint32_t flash_size = 4096; // reserve one sector for now

bool LoadDeviceConfig();
bool SaveDeviceConfig();
void SetDefaultDeviceConfig();

void setDeviceConfig(Protocol::DeviceConfig s);
Protocol::DeviceConfig getDeviceConfig();
uint32_t getIF1();
uint32_t getIF2();
uint32_t getADCRate();
uint8_t getADCPrescaler();
uint16_t getDFTPhaseInc();
uint8_t getPLLSettlingDelay();

}
