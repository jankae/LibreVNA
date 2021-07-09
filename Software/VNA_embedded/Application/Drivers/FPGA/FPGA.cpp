#include "FPGA.hpp"
#include "delay.hpp"
#include "stm.hpp"
#include "main.h"
#include "FPGA_HAL.hpp"
#include <complex>
#include "HW_HAL.hpp"

#define LOG_LEVEL	LOG_LEVEL_DEBUG
#define LOG_MODULE	"FPGA"
#include "Log.h"

static FPGA::HaltedCallback halted_cb;
static uint16_t SysCtrlReg = 0x0000;
static uint16_t ISRMaskReg = 0x0000;
static uint32_t ADC_samplerate;

using namespace FPGAHAL;

static void SwitchBytes(uint16_t &value) {
	value = (value & 0xFF00) >> 8 | (value & 0x00FF) << 8;
}
static void SwitchBytes(int16_t &value) {
	value = (value & 0xFF00) >> 8 | (value & 0x00FF) << 8;
}

void FPGA::WriteRegister(FPGA::Reg reg, uint16_t value) {
	uint8_t cmd[4] = {0x80, (uint8_t) reg, (uint8_t) (value >> 8), (uint8_t) (value & 0xFF)};
	Low(CS);
	HAL_SPI_Transmit(&FPGA_SPI, (uint8_t*) cmd, 4, 100);
	High(CS);
	if(reg == Reg::ADCPrescaler) {
		ADC_samplerate = Clockrate / value;
	}
}

bool FPGA::Configure(uint32_t start_address, uint32_t bitstream_size) {
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

	if(isHigh(DONE)) {
		LOG_ERR("DONE not asserted, aborting configuration");
		return false;
	}

	uint32_t lastmessage = HAL_GetTick();
	uint8_t buf[256];
	while(bitstream_size > 0) {
		if(HAL_GetTick() - lastmessage > 100) {
			LOG_DEBUG("Remaining: %lu", bitstream_size);
			lastmessage = HAL_GetTick();
		}
		uint16_t size = sizeof(buf);
		if(size > bitstream_size) {
			size = bitstream_size;
		}
		// TODO this part might be doable with the DMA instead of the buffer
		// get chunk of bitstream from flash...
		HWHAL::flash.read(start_address, size, buf);
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
	AbortSweep();
	Delay::us(1);
	Low(FPGA_RESET);
	Delay::ms(10);

	// Check if FPGA response is as expected
	uint8_t cmd[4] = {0x40, 0x00, 0x00, 0x00};
	uint8_t recv[4];
	Low(CS);
	HAL_SPI_TransmitReceive(&FPGA_SPI, (uint8_t*) cmd, (uint8_t*) recv, 4, 100);
	High(CS);

	uint16_t resp = (uint16_t) recv[2] << 8 | recv[3];
	uint16_t status = (uint16_t) recv[0] << 8 | recv[1];
	if(resp != 0xF0A5) {
		LOG_ERR("Initialization failed, got 0x%04x instead of 0xF0A5", resp);
		return false;
	}

	LOG_DEBUG("Initialized, status register: 0x%04x", status);
	return true;
}

void FPGA::SetNumberOfPoints(uint16_t npoints) {
	// register has to be set to number of points - 1
	npoints--;
	WriteRegister(Reg::SweepPoints, npoints);
}

void FPGA::SetSamplesPerPoint(uint32_t nsamples) {
	// register is in multiples of 16
	nsamples /= 16;
	// constrain to maximum value
	if(nsamples >= 8192) {
		nsamples = 8191;
	}
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

bool FPGA::IsEnabled(Periphery p) {
	if(SysCtrlReg & (uint16_t) p) {
		return true;
	} else {
		return false;
	}
}

void FPGA::SetWindow(Window w) {
	SysCtrlReg &= ~0x0060;
	SysCtrlReg |= (int) w << 5;
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

	uint16_t Source_Power = (SourceRegs[4] & 0x00000018) >> 3;

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
	send[3] = LO_DIV_A << 13 | LO_VCO << 7 | LO_N << 1;
	if (lowband) {
		send[3] |= 0x0001;
	}
	send[4] = Source_Power << 14 | (uint16_t) attenuation << 7 | Source_M >> 5;
	send[5] = (Source_M & 0x001F) << 11 | Source_FRAC >> 1;
	send[6] = (Source_FRAC & 0x0001) << 15 | Source_DIV_A << 12 | Source_VCO << 6 | Source_N;
	SwitchBytes(send[0]);
	SwitchBytes(send[1]);
	SwitchBytes(send[2]);
	SwitchBytes(send[3]);
	SwitchBytes(send[4]);
	SwitchBytes(send[5]);
	SwitchBytes(send[6]);
	Low(CS);
	HAL_SPI_Transmit(&FPGA_SPI, (uint8_t*) send, 14, 100);
	High(CS);
}

static inline int64_t sign_extend_64(int64_t x, uint16_t bits) {
	int64_t m = 1ULL << (bits - 1);
	return (x ^ m) - m;
}

static FPGA::ReadCallback callback;
static uint8_t raw[40];
static FPGA::SamplingResult result;
static bool busy_reading = false;

bool FPGA::InitiateSampleRead(ReadCallback cb) {
	if(busy_reading) {
		LOG_ERR("ISR while still reading old data");
		return false;
	}
	callback = cb;
	uint8_t cmd[40] = {0xC0, 0x00};
	// Start data read
	Low(CS);
	busy_reading = true;
	HAL_SPI_TransmitReceive_DMA(&FPGA_SPI, cmd, raw, 40);
	return true;
}

static int64_t assembleSampleResultValue(uint8_t *raw) {
//	LOG_DEBUG("Raw: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", raw[4], raw[5], raw[2], raw[3], raw[1], raw[0]);
	return sign_extend_64(
			(uint16_t) raw[0] << 8 | raw[1] | (uint32_t) raw[2] << 24
					| (uint32_t) raw[3] << 16 | (uint64_t) raw[4] << 40
					| (uint64_t) raw[5] << 32, 48);
}

extern "C" {
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
	uint16_t status = (uint16_t) raw[0] << 8 | raw[1];
	// Assemble data from words
	result.P1I = assembleSampleResultValue(&raw[32]);
	result.P1Q = assembleSampleResultValue(&raw[26]);
	result.P2I = assembleSampleResultValue(&raw[20]);
	result.P2Q = assembleSampleResultValue(&raw[14]);
	result.RefI = assembleSampleResultValue(&raw[8]);
	result.RefQ = assembleSampleResultValue(&raw[2]);
	result.pointNum = (uint16_t)(raw[38]&0x1F) << 8 | raw[39];
	result.activePort = raw[38] & 0x80 ? 1 : 0;
	High(CS);
	busy_reading = false;
	if ((status & 0x0004) && callback) {
		callback(result);
	}
	if ((status & 0x0010) && halted_cb) {
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
		// Configure SPI to use faster speed of 32MHz
		FPGA_SPI.Instance->CR1 = (FPGA_SPI.Instance->CR1 & ~SPI_CR1_BR_Msk) | SPI_BAUDRATEPRESCALER_4;
		break;
	case Mode::SourcePLL:
		Low(CS);
		Low(AUX2);
		Delay::us(1);
		High(AUX1);
		// Configure SPI to use slower speed of 16MHz (MAX2871 is limited to 20MHz)
		FPGA_SPI.Instance->CR1 = (FPGA_SPI.Instance->CR1 & ~SPI_CR1_BR_Msk) | SPI_BAUDRATEPRESCALER_8;
		break;
	case Mode::LOPLL:
		Low(CS);
		Low(AUX1);
		Delay::us(1);
		High(AUX2);
		// Configure SPI to use slower speed of 16MHz (MAX2871 is limited to 20MHz)
		FPGA_SPI.Instance->CR1 = (FPGA_SPI.Instance->CR1 & ~SPI_CR1_BR_Msk) | SPI_BAUDRATEPRESCALER_8;
		break;
	}
	Delay::us(1);
}

uint16_t FPGA::GetStatus() {
	uint8_t cmd[2] = {0x40, 0x00};
	uint8_t status[2];
	Low(CS);
	HAL_SPI_TransmitReceive(&FPGA_SPI, (uint8_t*) &cmd, (uint8_t*) &status, 2,
			100);
	High(CS);
	return (uint16_t) status[0] << 8 | status[1];
}

FPGA::ADCLimits FPGA::GetADCLimits() {
	uint16_t cmd = 0xE000;
	SwitchBytes(cmd);
	Low(CS);
	HAL_SPI_Transmit(&FPGA_SPI, (uint8_t*) &cmd, 2, 100);
	ADCLimits limits;
	HAL_SPI_Receive(&FPGA_SPI, (uint8_t*) &limits, 12, 100);
	High(CS);
	SwitchBytes(limits.P1max);
	SwitchBytes(limits.P1min);
	SwitchBytes(limits.P2max);
	SwitchBytes(limits.P2min);
	SwitchBytes(limits.Rmax);
	SwitchBytes(limits.Rmin);
	return limits;
}

void FPGA::ResetADCLimits() {
	uint16_t cmd = 0x6000;
	SwitchBytes(cmd);
	Low(CS);
	HAL_SPI_Transmit(&FPGA_SPI, (uint8_t*) &cmd, 2, 100);
	High(CS);
}

void FPGA::ResumeHaltedSweep() {
	uint16_t cmd = 0x2000;
	SwitchBytes(cmd);
	Low(CS);
	HAL_SPI_Transmit(&FPGA_SPI, (uint8_t*) &cmd, 2, 100);
	High(CS);
}

void FPGA::SetupDFT(uint32_t f_firstBin, uint32_t f_binSpacing) {
	// see FPGA protocol for formulas
	uint16_t firstBin = f_firstBin * (1ULL << 16) / ADC_samplerate;
	uint16_t binSpacing = f_binSpacing * (1ULL << 24) / ADC_samplerate;
	WriteRegister(Reg::DFTFirstBin, firstBin);
	WriteRegister(Reg::DFTFreqSpacing, binSpacing);
}

void FPGA::StopDFT() {
	DisableInterrupt(Interrupt::DFTReady);
}

void FPGA::StartDFT() {
	StopDFT();
	EnableInterrupt(Interrupt::DFTReady);
}

FPGA::DFTResult FPGA::ReadDFTResult() {
	uint8_t cmd[2] = {0xA0, 0x00};
	uint8_t recv[24];
	Low(CS);
	HAL_SPI_Transmit(&FPGA_SPI, cmd, 2, 100);
	HAL_SPI_Receive(&FPGA_SPI, recv, 24, 100);
	High(CS);
	// assemble words
	int64_t p2imag = assembleSampleResultValue(&recv[0]);
	int64_t p2real = assembleSampleResultValue(&recv[6]);
	int64_t p1imag = assembleSampleResultValue(&recv[12]);
	int64_t p1real = assembleSampleResultValue(&recv[18]);
//	LOG_INFO("DFT raw: %ld, %ld, %ld, %ld", (int32_t) p1real, (int32_t) p1imag, (int32_t) p2real, (int32_t) p2imag);
//	Log_Flush();
	auto p1 = std::complex<float>(p1real, p1imag);
	auto p2 = std::complex<float>(p2real, p2imag);
	DFTResult res;
//	LOG_INFO("DFT: %ld, %ld, %ld, %ld", (int32_t) p1.real(), (int32_t) p1.imag(), (int32_t) p2.real(), (int32_t) p2.imag());
	res.P1 = std::abs(p1);
	res.P2 = std::abs(p2);
	return res;
}
