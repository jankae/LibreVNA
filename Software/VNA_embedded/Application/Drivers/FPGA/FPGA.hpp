#pragma once

#include <cstdint>
#include "Flash.hpp"
#include "max2871.hpp"

namespace FPGA {

static constexpr uint16_t MaxPoints = 4501;
static constexpr uint16_t DFTbins = 96;
static constexpr uint32_t Clockrate = 102400000UL;

enum class Reg {
	InterruptMask = 0x00,
	SweepPoints = 0x01,
	SamplesPerPoint = 0x02,
	SystemControl = 0x03,
	ADCPrescaler = 0x04,
	PhaseIncrement = 0x05,
	SweepSetup = 0x06,
	HardwareOverwrite = 0x07,
	MAX2871Def0LSB = 0x08,
	MAX2871Def0MSB = 0x09,
	MAX2871Def1LSB = 0x0A,
	MAX2871Def1MSB = 0x0B,
	MAX2871Def3LSB = 0x0C,
	MAX2871Def3MSB = 0x0D,
	MAX2871Def4LSB = 0x0E,
	MAX2871Def4MSB = 0x0F,
	DFTFirstBin = 0x12,
	DFTFreqSpacing = 0x13,
	SettlingTimeLow = 0x14,
	SettlingTimeHigh = 0x15,
};

using SamplingResult = struct _samplingresult {
	int64_t P1I, P1Q;
	int64_t P2I, P2Q;
	int64_t RefI, RefQ;
	uint16_t pointNum :13;
	uint16_t stageNum :3;
};

using DFTResult = struct _dftresult {
	float P1, P2;
};

using ADCLimits = struct _adclimits {
	int16_t P1min, P1max;
	int16_t P2min, P2max;
	int16_t Rmin, Rmax;
};

enum class Periphery {
	Port1Mixer = 0x8000,
	Port2Mixer = 0x4000,
	RefMixer = 0x2000,
	Amplifier = 0x1000,
	SourceRF = 0x0800,
	LO1RF = 0x0400,
	ExtRefLED = 0x0200,
	ReadyLED = 0x0100,
	DebugLED = 0x0080,
	SourceChip = 0x0010,
	LO1Chip = 0x0008,
	SyncMaster = 0x0002,
	PortSwitch = 0x0001,
};

enum class Interrupt {
	LO1Unlock = 0x0001,
	SourceUnlock = 0x0002,
	NewData = 0x0004,
	DataOverrun = 0x0008,
	SweepHalted = 0x0010,
	DFTReady = 0x0020,
};

enum class LowpassFilter {
	M947 = 0x00,
	M1880 = 0x01,
	M3500 = 0x02,
	None = 0x03,
	Auto = 0xFF,
};

enum class Samples {
	SPPRegister = 0x00,
    S96 = 0x01,
	S304 = 0x02,
	S912 = 0x03,
	S3040 = 0x04,
	S9136 = 0x05,
	S30464 = 0x06,
	S91392 = 0x07,
};

enum class Window {
	None = 0x00,
	Kaiser = 0x01,
	Hann = 0x02,
	Flattop = 0x03,
};

bool Configure(uint32_t start_address, uint32_t bitstream_size);

using HaltedCallback = void(*)(void);
bool Init(HaltedCallback cb = nullptr);
void WriteRegister(FPGA::Reg reg, uint16_t value);
void SetNumberOfPoints(uint16_t npoints);
void SetSamplesPerPoint(uint32_t nsamples);
void SetSettlingTime(uint16_t us);
void SetupSweep(uint8_t stages, uint8_t port1_stage, uint8_t port2_stage, bool synchronize = false, bool syncMaster = false);
void Enable(Periphery p, bool enable = true);
void Disable(Periphery p);
bool IsEnabled(Periphery p);
void SetWindow(Window w);
void EnableInterrupt(Interrupt i);
void DisableInterrupt(Interrupt i);
void DisableAllInterrupts();
void WriteMAX2871Default(uint32_t *DefaultRegs);
void WriteSweepConfig(uint16_t pointnum, bool lowband, uint32_t *SourceRegs, uint32_t *LORegs,
		uint8_t attenuation, uint64_t frequency, Samples samples, bool halt = false, LowpassFilter filter = LowpassFilter::Auto);
using ReadCallback = void(*)(const SamplingResult &result);
bool InitiateSampleRead(ReadCallback cb);
void SetupDFT(uint32_t f_firstBin, uint32_t f_binSpacing);
void StopDFT();
void StartDFT();
DFTResult ReadDFTResult();
ADCLimits GetADCLimits();
void ResetADCLimits();
bool ResumeHaltedSweep();
uint16_t GetStatus();

void OverwriteHardware(uint8_t attenuation, LowpassFilter filter, bool lowband, bool port1_enabled, bool port2_enabled);
void DisableHardwareOverwrite();

void StopSweep();
void StartSweep();
void AbortSweep();

enum class Mode {
	FPGA,
	SourcePLL,
	LOPLL,
};
void SetMode(Mode mode);


}
