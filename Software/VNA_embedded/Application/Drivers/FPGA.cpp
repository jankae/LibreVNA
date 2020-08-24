#include "FPGA.hpp"
#include "delay.hpp"
#include "stm.hpp"
#include "main.h"

#define LOG_LEVEL	LOG_LEVEL_DEBUG
#define LOG_MODULE	"FPGA"
#include "Log.h"

#define FPGA_SPI		hspi1
extern SPI_HandleTypeDef FPGA_SPI;

static inline void Low(GPIO_TypeDef *gpio, uint16_t pin) {
	gpio->BSRR = pin << 16;
}
static inline void High(GPIO_TypeDef *gpio, uint16_t pin) {
	gpio->BSRR = pin;
}

static FPGA::HaltedCallback halted_cb;
static uint16_t SysCtrlReg = 0x0000;
static uint16_t ISRMaskReg = 0x0000;

void WriteRegister(FPGA::Reg reg, uint16_t value) {
	uint16_t cmd[2] = {(uint16_t) (0x8000 | (uint16_t) reg), value};
	Low(FPGA_CS_GPIO_Port, FPGA_CS_Pin);
	HAL_SPI_Transmit(&FPGA_SPI, (uint8_t*) cmd, 2, 100);
	High(FPGA_CS_GPIO_Port, FPGA_CS_Pin);
}

bool FPGA::Init(HaltedCallback cb) {
	halted_cb = cb;
	// Reset FPGA
	High(FPGA_RESET_GPIO_Port, FPGA_RESET_Pin);
	SetMode(Mode::FPGA);
	Delay::us(1);
	Low(FPGA_RESET_GPIO_Port, FPGA_RESET_Pin);
	Delay::ms(10);

	// Check if FPGA response is as expected
	uint16_t cmd[2] = {0x4000, 0x0000};
	uint16_t recv[2];
	Low(FPGA_CS_GPIO_Port, FPGA_CS_Pin);
	HAL_SPI_TransmitReceive(&FPGA_SPI, (uint8_t*) cmd, (uint8_t*) recv, 2, 100);
	High(FPGA_CS_GPIO_Port, FPGA_CS_Pin);

	if(recv[1] != 0xF0A5) {
		LOG_ERR("Initialization failed, got 0x%04x instead of 0xF0A5", recv[1]);
	}

	LOG_DEBUG("Initialized, status register: 0x%04x", recv[0]);
	return true;
}

void FPGA::SetNumberOfPoints(uint16_t npoints) {
	// register has to be set to number of points - 1
	npoints--;
	WriteRegister(Reg::SweepPoints, npoints);
}

void FPGA::SetSamplesPerPoint(uint32_t nsamples) {
	// register has to be set to number of nsamples - 1
	nsamples--;
	// constrain to maximum value
	nsamples &= 0x1FFFF;
	// highest bit is located at the system control register
	SysCtrlReg &= ~0x0001;
	SysCtrlReg |= nsamples >> 16;
	WriteRegister(Reg::SystemControl, SysCtrlReg);
	WriteRegister(Reg::SamplesPerPoint, nsamples & 0xFFFF);
}

void FPGA::SetSettlingTime(uint16_t us) {
	if (us > 639) {
		us = 639;
	}
	uint16_t regval = (uint32_t) us * 1024 / 10;
	WriteRegister(Reg::SettlingTime, regval);
}

void FPGA::Enable(Periphery p, bool enable) {
	if (enable) {
		SysCtrlReg |= (uint16_t) p;
		WriteRegister(Reg::SystemControl, SysCtrlReg);
	} else {
		Disable(p);
	}
}

void FPGA::Disable(Periphery p) {
	SysCtrlReg &= ~(uint16_t) p;
	WriteRegister(Reg::SystemControl, SysCtrlReg);
}

void FPGA::EnableInterrupt(Interrupt i) {
	ISRMaskReg |= (uint16_t) i;
	WriteRegister(Reg::InterruptMask, ISRMaskReg);
}

void FPGA::DisableInterrupt(Interrupt i) {
	ISRMaskReg &= ~(uint16_t) i;
	WriteRegister(Reg::InterruptMask, ISRMaskReg);
}

void FPGA::WriteMAX2871Default(uint32_t *DefaultRegs) {
	WriteRegister(Reg::MAX2871Def0LSB, DefaultRegs[0] & 0xFFFF);
	WriteRegister(Reg::MAX2871Def0MSB, DefaultRegs[0] >> 16);
	WriteRegister(Reg::MAX2871Def1LSB, DefaultRegs[1] & 0xFFFF);
	WriteRegister(Reg::MAX2871Def1MSB, DefaultRegs[1] >> 16);
	WriteRegister(Reg::MAX2871Def3LSB, DefaultRegs[3] & 0xFFFF);
	WriteRegister(Reg::MAX2871Def3MSB, DefaultRegs[3] >> 16);
	WriteRegister(Reg::MAX2871Def4LSB, DefaultRegs[4] & 0xFFFF);
	WriteRegister(Reg::MAX2871Def4MSB, DefaultRegs[4] >> 16);
}

void FPGA::WriteSweepConfig(uint16_t pointnum, bool lowband, uint32_t *SourceRegs, uint32_t *LORegs,
		uint8_t attenuation, uint64_t frequency, bool halt, LowpassFilter filter) {
	uint16_t send[8];
	// select which point this sweep config is for
	send[0] = pointnum & 0x1FFF;
	// assemble sweep config from required fields of PLL registers
	send[1] = (LORegs[4] & 0x00700000) >> 14 | (LORegs[3] & 0xFC000000) >> 26;
	if (halt) {
		send[1] |= 0x8000;
	}
	if (lowband) {
		send[1] |= 0x4000;
	}
	switch(filter) {
	case LowpassFilter::Auto:
		// Select source LP filter
		if (frequency >= 3500000000) {
			send[1] |= 0x0600;
		} else if (frequency >= 1800000000) {
			send[1] |= 0x0400;
		} else if (frequency >= 900000000) {
			send[1] |= 0x0200;
		}
		break;
	case LowpassFilter::M947: break;
	case LowpassFilter::M1880: send[1] |= 0x0200; break;
	case LowpassFilter::M3500: send[1] |= 0x0400; break;
	case LowpassFilter::None: send[1] |= 0x0600; break;
	}
	send[2] = (LORegs[1] & 0x00007FF8) << 1 | (LORegs[0] & 0x00007800) >> 11;
	send[3] = (LORegs[0] & 0x000007F8) << 5 | (LORegs[0] & 0x7F800000) >> 23;
	send[4] = (LORegs[0] & 0x007F8000) >> 7 | (attenuation & 0x7F) << 1 | (SourceRegs[4] & 0x00400000) >> 22;
	send[5] = (SourceRegs[4] & 0x00300000) >> 6 | (SourceRegs[3] & 0xFC000000) >> 18 | (SourceRegs[1] & 0x00007F80) >> 7;
	send[6] = (SourceRegs[1] & 0x00000078) << 9 | (SourceRegs[0] & 0x00007FF8) >> 3;
	send[7] = (SourceRegs[0] & 0x7FFF8000) >> 15;
	Low(FPGA_CS_GPIO_Port, FPGA_CS_Pin);
	HAL_SPI_Transmit(&FPGA_SPI, (uint8_t*) send, 8, 100);
	High(FPGA_CS_GPIO_Port, FPGA_CS_Pin);
}

static inline int64_t sign_extend_64(int64_t x, uint16_t bits) {
	int64_t m = 1ULL << (bits - 1);
	return (x ^ m) - m;
}

static FPGA::ReadCallback callback;
static uint16_t raw[18];
static bool halted;

bool FPGA::InitiateSampleRead(ReadCallback cb) {
	callback = cb;
	uint16_t cmd = 0xC000;
	uint16_t status;

	Low(FPGA_CS_GPIO_Port, FPGA_CS_Pin);
	HAL_SPI_TransmitReceive(&FPGA_SPI, (uint8_t*) &cmd, (uint8_t*) &status, 1,
			100);

	if (status & 0x0010) {
		halted = true;
	} else {
		halted = false;
	}

	if (!(status & 0x0004)) {
		// no new data available yet
		High(FPGA_CS_GPIO_Port, FPGA_CS_Pin);

		if (halted) {
			if (halted_cb) {
				halted_cb();
			}
		} else {
			LOG_WARN("ISR without new data, status: 0x%04x", status);
		}
		return false;
	}

	// Start data read
	HAL_SPI_Receive_DMA(&FPGA_SPI, (uint8_t*) raw, 18);
	return true;
}

extern "C" {
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {
	FPGA::SamplingResult result;
	High(FPGA_CS_GPIO_Port, FPGA_CS_Pin);
	// Assemble data from words
	result.P1I = sign_extend_64(
			(uint64_t) raw[17] << 32 | (uint32_t) raw[16] << 16 | raw[15], 48);
	result.P1Q = sign_extend_64(
			(uint64_t) raw[14] << 32 | (uint32_t) raw[13] << 16 | raw[12], 48);
	result.P2I = sign_extend_64(
			(uint64_t) raw[11] << 32 | (uint32_t) raw[10] << 16 | raw[9], 48);
	result.P2Q = sign_extend_64(
			(uint64_t) raw[8] << 32 | (uint32_t) raw[7] << 16 | raw[6], 48);
	result.RefI = sign_extend_64(
			(uint64_t) raw[5] << 32 | (uint32_t) raw[4] << 16 | raw[3], 48);
	result.RefQ = sign_extend_64(
			(uint64_t) raw[2] << 32 | (uint32_t) raw[1] << 16 | raw[0], 48);
	if (callback) {
		callback(result);
	}
	if (halted && halted_cb) {
		halted_cb();
	}
}
}

void FPGA::StartSweep() {
	Low(FPGA_AUX3_GPIO_Port, FPGA_AUX3_Pin);
	Delay::us(1);
	High(FPGA_AUX3_GPIO_Port, FPGA_AUX3_Pin);
}

void FPGA::AbortSweep() {
	Low(FPGA_AUX3_GPIO_Port, FPGA_AUX3_Pin);
}

void FPGA::SetMode(Mode mode) {
	switch(mode) {
	case Mode::FPGA:
		// Both AUX1/2 low
		Low(FPGA_AUX1_GPIO_Port, FPGA_AUX1_Pin);
		Low(FPGA_AUX2_GPIO_Port, FPGA_AUX2_Pin);
		Delay::us(1);
		High(FPGA_CS_GPIO_Port, FPGA_CS_Pin);
		break;
	case Mode::SourcePLL:
		Low(FPGA_CS_GPIO_Port, FPGA_CS_Pin);
		Low(FPGA_AUX2_GPIO_Port, FPGA_AUX2_Pin);
		Delay::us(1);
		High(FPGA_AUX1_GPIO_Port, FPGA_AUX1_Pin);
		break;
	case Mode::LOPLL:
		Low(FPGA_CS_GPIO_Port, FPGA_CS_Pin);
		Low(FPGA_AUX1_GPIO_Port, FPGA_AUX1_Pin);
		Delay::us(1);
		High(FPGA_AUX2_GPIO_Port, FPGA_AUX2_Pin);
		break;
	}
	Delay::us(1);
}

uint16_t FPGA::GetStatus() {
	uint16_t cmd = 0x4000;
	uint16_t status;
	Low(FPGA_CS_GPIO_Port, FPGA_CS_Pin);
	HAL_SPI_TransmitReceive(&FPGA_SPI, (uint8_t*) &cmd, (uint8_t*) &status, 1,
			100);
	High(FPGA_CS_GPIO_Port, FPGA_CS_Pin);
	return status;
}

FPGA::ADCLimits FPGA::GetADCLimits() {
	uint16_t cmd = 0xE000;
	Low(FPGA_CS_GPIO_Port, FPGA_CS_Pin);
	HAL_SPI_Transmit(&FPGA_SPI, (uint8_t*) &cmd, 1, 100);
	ADCLimits limits;
	HAL_SPI_Receive(&FPGA_SPI, (uint8_t*) &limits, 6, 100);
	High(FPGA_CS_GPIO_Port, FPGA_CS_Pin);
	return limits;
}

void FPGA::ResetADCLimits() {
	uint16_t cmd = 0x6000;
	Low(FPGA_CS_GPIO_Port, FPGA_CS_Pin);
	HAL_SPI_Transmit(&FPGA_SPI, (uint8_t*) &cmd, 1, 100);
	High(FPGA_CS_GPIO_Port, FPGA_CS_Pin);
}

void FPGA::ResumeHaltedSweep() {
	uint16_t cmd = 0x2000;
	Low(FPGA_CS_GPIO_Port, FPGA_CS_Pin);
	HAL_SPI_Transmit(&FPGA_SPI, (uint8_t*) &cmd, 1, 100);
	High(FPGA_CS_GPIO_Port, FPGA_CS_Pin);
}

