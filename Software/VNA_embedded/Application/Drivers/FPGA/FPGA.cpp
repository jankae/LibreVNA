#include "FPGA.hpp"
#include "delay.hpp"
#include "stm.hpp"
#include "main.h"
#include "FPGA_HAL.hpp"

#define LOG_LEVEL	LOG_LEVEL_DEBUG
#define LOG_MODULE	"FPGA"
#include "Log.h"

static FPGA::HaltedCallback halted_cb;
static uint16_t SysCtrlReg = 0x0000;
static uint16_t ISRMaskReg = 0x0000;

using namespace FPGAHAL;

void WriteRegister(FPGA::Reg reg, uint16_t value) {
	uint16_t cmd[2] = {(uint16_t) (0x8000 | (uint16_t) reg), value};
	Low(CS);
	HAL_SPI_Transmit(&FPGA_SPI, (uint8_t*) cmd, 2, 100);
	High(CS);
}

bool FPGA::Configure(Flash *f, uint32_t start_address, uint32_t bitstream_size) {
	if(!PROGRAM_B.gpio) {
		LOG_WARN("PROGRAM_B not defined, assuming FPGA configures itself in master configuration");
		// wait too allow enough time for FPGA configuration
		HAL_Delay(2000);
		return true;
	}
	LOG_INFO("Loading bitstream of size %lu...", bitstream_size);
	Low(PROGRAM_B);
	while(isHigh(INIT_B));
	High(PROGRAM_B);
	while(!isHigh(INIT_B));

	uint8_t buf[256];
	while(bitstream_size > 0) {
		uint16_t size = sizeof(buf);
		if(size > bitstream_size) {
			size = bitstream_size;
		}
		// TODO this part might be doable with the DMA instead of the buffer
		// get chunk of bitstream from flash...
		f->read(start_address, size, buf);
		// ... and pass it on to FPGA
		HAL_SPI_Transmit(&CONFIGURATION_SPI, buf, size, 100);
		bitstream_size -= size;
		start_address += size;
	}
	Delay::ms(1);
	if(!isHigh(INIT_B)) {
		LOG_CRIT("INIT_B asserted after configuration, CRC error occurred");
		return false;
	}
	if(!isHigh(DONE)) {
		LOG_CRIT("DONE still low after configuration");
		return false;
	}
	LOG_INFO("...configured");
	return true;
}

bool FPGA::Init(HaltedCallback cb) {
	halted_cb = cb;
	SysCtrlReg = 0;
	ISRMaskReg = 0;
	// Reset FPGA
	High(FPGA_RESET);
	SetMode(Mode::FPGA);
	Delay::us(1);
	Low(FPGA_RESET);
	Delay::ms(10);

	// Check if FPGA response is as expected
	uint16_t cmd[2] = {0x4000, 0x0000};
	uint16_t recv[2];
	Low(CS);
	HAL_SPI_TransmitReceive(&FPGA_SPI, (uint8_t*) cmd, (uint8_t*) recv, 2, 100);
	High(CS);

	if(recv[1] != 0xF0A5) {
		LOG_ERR("Initialization failed, got 0x%04x instead of 0xF0A5", recv[1]);
		return false;
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
	// register is in multiples of 128
	nsamples /= 128;
	// constrain to maximum value
	nsamples &= 0x03FF;
	WriteRegister(Reg::SamplesPerPoint, nsamples);
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
		uint8_t attenuation, uint64_t frequency, SettlingTime settling, Samples samples, bool halt, LowpassFilter filter) {
	uint16_t send[7];
	// select which point this sweep config is for
	send[0] = pointnum & 0x1FFF;
	// assemble sweep config from required fields of PLL registers
	uint16_t LO_N = (LORegs[0] & 0x7FFF8000) >> 15;
	uint16_t LO_FRAC = (LORegs[0] & 0x00007FF8) >> 3;
	uint16_t LO_M = (LORegs[1] & 0x00007FF8) >> 3;
	uint16_t LO_VCO = (LORegs[3] & 0xFC000000) >> 26;
	uint16_t LO_DIV_A = (LORegs[4] & 0x00700000) >> 20;

	uint16_t Source_N = (SourceRegs[0] & 0x7FFF8000) >> 15;
	uint16_t Source_FRAC = (SourceRegs[0] & 0x00007FF8) >> 3;
	uint16_t Source_M = (SourceRegs[1] & 0x00007FF8) >> 3;
	uint16_t Source_VCO = (SourceRegs[3] & 0xFC000000) >> 26;
	uint16_t Source_DIV_A = (SourceRegs[4] & 0x00700000) >> 20;

	send[1] = LO_M >> 4;
	if (halt) {
		send[1] |= 0x8000;
	}
	send[1] |= (int) settling << 13;
	send[1] |= (int) samples << 10;
	if(filter == LowpassFilter::Auto) {
		// Select source LP filter
		if (frequency >= 3500000000) {
			send[1] |= (int) LowpassFilter::None << 8;
		} else if (frequency >= 1800000000) {
			send[1] |= (int) LowpassFilter::M3500 << 8;
		} else if (frequency >= 900000000) {
			send[1] |= (int) LowpassFilter::M1880 << 8;
		} else {
			send[1] |= (int) LowpassFilter::M947 << 8;
		}
	} else {
		send[1] |= (int) filter << 8;
	}
	send[2] = (LO_M & 0x000F) << 12 | LO_FRAC;
	send[3] = LO_DIV_A << 13 | LO_VCO << 7 | LO_N;
	send[4] = (uint16_t) attenuation << 8 | Source_M >> 4;
	if (lowband) {
		send[4] |= 0x8000;
	}
	send[5] = (Source_M & 0x000F) << 12 | Source_FRAC;
	send[6] = Source_DIV_A << 13 | Source_VCO << 7 | Source_N;
	Low(CS);
	HAL_SPI_Transmit(&FPGA_SPI, (uint8_t*) send, 7, 100);
	High(CS);
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

	Low(CS);
	HAL_SPI_TransmitReceive(&FPGA_SPI, (uint8_t*) &cmd, (uint8_t*) &status, 1,
			100);

	if (status & 0x0010) {
		halted = true;
	} else {
		halted = false;
	}

	if (!(status & 0x0004)) {
		// no new data available yet
		High(CS);

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
	High(CS);
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
	Low(AUX3);
	Delay::us(1);
	High(AUX3);
}

void FPGA::AbortSweep() {
	Low(AUX3);
}

void FPGA::SetMode(Mode mode) {
	switch(mode) {
	case Mode::FPGA:
		// Both AUX1/2 low
		Low(AUX1);
		Low(AUX2);
		Delay::us(1);
		High(CS);
		break;
	case Mode::SourcePLL:
		Low(CS);
		Low(AUX2);
		Delay::us(1);
		High(AUX1);
		break;
	case Mode::LOPLL:
		Low(CS);
		Low(AUX1);
		Delay::us(1);
		High(AUX2);
		break;
	}
	Delay::us(1);
}

uint16_t FPGA::GetStatus() {
	uint16_t cmd = 0x4000;
	uint16_t status;
	Low(CS);
	HAL_SPI_TransmitReceive(&FPGA_SPI, (uint8_t*) &cmd, (uint8_t*) &status, 1,
			100);
	High(CS);
	return status;
}

FPGA::ADCLimits FPGA::GetADCLimits() {
	uint16_t cmd = 0xE000;
	Low(CS);
	HAL_SPI_Transmit(&FPGA_SPI, (uint8_t*) &cmd, 1, 100);
	ADCLimits limits;
	HAL_SPI_Receive(&FPGA_SPI, (uint8_t*) &limits, 6, 100);
	High(CS);
	return limits;
}

void FPGA::ResetADCLimits() {
	uint16_t cmd = 0x6000;
	Low(CS);
	HAL_SPI_Transmit(&FPGA_SPI, (uint8_t*) &cmd, 1, 100);
	High(CS);
}

void FPGA::ResumeHaltedSweep() {
	uint16_t cmd = 0x2000;
	Low(CS);
	HAL_SPI_Transmit(&FPGA_SPI, (uint8_t*) &cmd, 1, 100);
	High(CS);
}


