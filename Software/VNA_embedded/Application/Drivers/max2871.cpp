#include "max2871.hpp"
#include <string.h>
#include <algorithm.hpp>

#include "delay.hpp"
#include <cmath>
#define LOG_LEVEL	LOG_LEVEL_ERR
#define LOG_MODULE	"MAX2871"
#include "Log.h"

bool MAX2871::Init() {
	return Init(10000000, true, 1, false);
}

bool MAX2871::Init(uint32_t f_ref, bool doubler, uint16_t r, bool div2) {
	for (uint8_t i = 0; i < 6; i++) {
		regs[i] = 0;
	}

	ChipEnable(false);
	RFEnable(false);

	SetReference(f_ref, doubler, r, div2);

	// non-inverting loop filter
	regs[2] |= (1UL << 6);
	// select digital lock detect
	regs[5] |= (0x1UL << 22);

	// fundamental VCO feedback
	regs[4] |= (1UL << 23);

	// reserved, set to 0x03
	regs[4] |= (0x3UL << 29);

	// enable double buffering for register 4
	regs[2] |= (1UL << 13);

	// automatically switch to integer mode if F = 0
	regs[5] |= (1UL << 24);

	// recommended phase setting
	regs[1] |= (1UL << 15);

	SetMode(Mode::LowNoise);
	// for all other CP modes the PLL reports unlock condition (output signal appears to be locked)
	SetCPMode(CPMode::CP20);
	SetCPCurrent(15);
	SetFrequency(1000000000);

	// initial register write according to datasheet timing
	ChipEnable(true);
	Write(5, regs[5]);
	Delay::ms(20);
	Write(4, regs[4]);
	Write(3, regs[3]);
	Write(2, regs[2]);
	Write(1, regs[1]);
	Write(0, regs[0]);
	Write(5, regs[5]);
	Delay::ms(20);
	Write(4, regs[4]);
	Write(3, regs[3]);
	Write(2, regs[2]);
	Write(1, regs[1]);
	Write(0, regs[0]);

	return true;
}

void MAX2871::ChipEnable(bool on) {
	if(!CE) {
		return;
	}
	if (on) {
		CE->BSRR = CEpin;
	} else {
		CE->BSRR = CEpin << 16;
	}
}

void MAX2871::RFEnable(bool on) {
	if(!RF_EN) {
		return;
	}
	if (on) {
		RF_EN->BSRR = RF_ENpin;
		Read();
	} else {
		RF_EN->BSRR = RF_ENpin << 16;
	}
}

bool MAX2871::Locked() {
	return LD->IDR & LDpin;
}

void MAX2871::SetPowerOutA(Power p, bool enabled) {
	// only set power of port A
	regs[4] &= ~0x38;
	regs[4] |= ((uint16_t) p << 3);
	if (enabled) {
		regs[4] |= 0x20;
	}
}

void MAX2871::SetPowerOutB(Power p, bool enabled) {
	// only set power of port B
	regs[4] &= ~0x1C0;
	regs[4] |= ((uint16_t) p << 6);
	if (enabled) {
		regs[4] |= 0x100;
	}
}

void MAX2871::SetMode(Mode m) {
	regs[2] &= ~0x60000000;
	regs[2] |= ((uint32_t) m << 29);
}

void MAX2871::SetCPMode(CPMode m) {
	regs[1] &= ~0x60000000;
	regs[1] |= ((uint32_t) m << 29);
}

void MAX2871::SetCPCurrent(uint8_t mA) {
	if(mA > 15) {
		LOG_WARN("Clipping charge pump current to 15mA");
		mA = 15;
	}
	regs[2] &= ~0x1E00;
	regs[2] |= ((uint16_t) mA << 9);
}

bool MAX2871::SetFrequency(uint64_t f) {
	if (f < 23500000 || f > MaxFreq) {
		LOG_ERR("Frequency must be between 23.5MHz and 6GHz");
		return false;
	}
	LOG_DEBUG("Setting frequency to %lu%06luHz...", (uint32_t ) (f / 1000000),
			(uint32_t ) (f % 1000000));
	// select divider
	uint64_t f_vco = f;
	uint8_t div = 0;
	if (f < 46875000) {
		div = 0x07;
		f_vco *= 128;
	} else if (f < 93750000) {
		div = 0x06;
		f_vco *= 64;
	} else if (f < 187500000) {
		div = 0x05;
		f_vco *= 32;
	} else if (f < 375000000) {
		div = 0x04;
		f_vco *= 16;
	} else if (f < 750000000) {
		div = 0x03;
		f_vco *= 8;
	} else if (f < 1500000000) {
		div = 0x02;
		f_vco *= 4;
	} else if (f < 3000000000) {
		div = 0x01;
		f_vco *= 2;
	}
	LOG_DEBUG("F_VCO: %lu%06luHz",
			(uint32_t ) (f_vco / 1000000), (uint32_t ) (f_vco % 1000000));
	if (gotVCOMap) {
		// manual VCO selection for lock time improvement
		uint16_t compare = f_vco / 100000;
		uint8_t vco = 0;
		for (; vco < 64; vco++) {
			if (VCOmax[vco] >= compare) {
				break;
			}
		} LOG_DEBUG("Manually selected VCO %d", vco);
		regs[3] &= ~0xFC000000;
		regs[3] |= (uint32_t) vco << 26;
	}
	uint16_t N = f_vco / f_PFD;
	if (N < 19 || N > 4091) {
		LOG_ERR("Invalid N value, should be between 19 and 4091, got %lu", N);
		return false;
	}
	uint32_t rem_f = f_vco - N * f_PFD;
	LOG_DEBUG("Remaining fractional frequency: %lu", rem_f);
	LOG_DEBUG("Looking for best fractional match");
	float fraction = (float) rem_f / f_PFD;

	auto approx = Algorithm::BestRationalApproximation(fraction, 4095);

	if(approx.denom == 1) {
		// M value must be at least 2
		approx.denom = 2;
	}

	int32_t rem_approx = ((uint64_t) f_PFD * approx.num) / approx.denom;
	if(rem_approx != rem_f) {
		LOG_WARN("Best match is F=%u/M=%u, deviation of %luHz",
				approx.num, approx.denom, abs(rem_f - rem_approx));
	}

	uint64_t f_set = (uint64_t) N * f_PFD + rem_approx;
	f_set /= (1UL << div);

	// write values to registers
	regs[4] &= ~0x00700000;
	regs[4] |= ((uint32_t) div << 20);
	regs[0] &= ~0x7FFFFFF8;
	regs[0] |= ((uint32_t) N << 15) | ((uint32_t) approx.num << 3);
	regs[1] &= ~0x00007FF8;
	regs[1] |= ((uint32_t) approx.denom << 3);

	LOG_DEBUG("Set frequency to %lu%06luHz...",
			(uint32_t ) (f_set / 1000000), (uint32_t ) (f_set % 1000000));
	outputFrequency = f_set;
	return true;
}

bool MAX2871::SetReference(uint32_t f_ref, bool doubler, uint16_t r,
		bool div2) {
	if (f_ref < 10000000) {
		LOG_ERR("Reference frequency must be >=10MHz, is %lu", f_ref);
		return false;
	} else if (f_ref > 105000000 && doubler) {
		LOG_ERR(
				"Reference frequency must be <=105MHz when used with doubler, is %lu",
				f_ref);
		return false;
	} else if (f_ref > 210000000) {
		LOG_ERR("Reference frequency must be <=210MHz, is %lu", f_ref);
		return false;
	}
	if (r < 1 || r > 1023) {
		LOG_ERR("Reference divider must be between 1 and 1023, is %d", r);
		return false;
	}
	// calculate PFD frequency
	uint32_t pfd = f_ref;
	if (doubler) {
		pfd *= 2;
	}
	pfd /= r;
	if (div2) {
		pfd /= 2;
	}
	if (pfd > 125000000) {
		LOG_ERR("PFD frequency must be <=125MHz, is %d",
				pfd);
		return false;
	}
	if(pfd > 32000000) {
		regs[2] |= (1UL << 31);
	} else {
		regs[2] &= ~(1UL << 31);
	}
	// input values are valid, adjust registers
	regs[2] &= ~0x03FFC000;
	if (doubler) {
		regs[2] |= (1UL << 25);
	}
	if (div2) {
		regs[2] |= (1UL << 24);
	}
	regs[2] |= (r << 14);
	f_PFD = pfd;
	LOG_INFO("Set PFD frequency to %lu", f_PFD);

	// updating VAS state machine clock
	uint16_t BS = f_PFD / 50000;
	if (BS > 1023) {
		BS = 1023;
	} else if (BS < 1) {
		BS = 1;
	}
	LOG_DEBUG("BS set to %lu", BS);
	regs[4] &= ~0x030FF000;
	regs[4] |= ((BS & 0xFF) << 12);
	regs[4] |= (((BS >> 8) & 0x03) << 24);

	// update ADC clock
	uint16_t cdiv = f_PFD/100000;
	LOG_DEBUG("CDIV set to %u", cdiv);
	regs[3] &= ~0x00007FF8;
	regs[3] |= (cdiv & 0xFFF) << 3;
	return true;
}

void MAX2871::Update() {
	Write(5, regs[5]);
	Write(4, regs[4]);
	Write(3, regs[3]);
	Write(2, regs[2]);
	Write(1, regs[1]);
	Write(0, regs[0]);
}

void MAX2871::UpdateFrequency() {
	Write(4, regs[4]);
	Write(3, regs[3]);
	Write(1, regs[1]);
	Write(0, regs[0]);
}

void MAX2871::Write(uint8_t reg, uint32_t val) {
	uint8_t data[4];
	// split value into two 16 bit words
	data[0] = (val >> 24) & 0xFF;
	data[1] = (val >> 16) & 0xFF;
	data[2] = (val >> 8) & 0xFF;
	data[3] = (val & 0xF8) | reg;
	Delay::us(1);
	HAL_SPI_Transmit(hspi, (uint8_t*) data, 4, 20);
	LE->BSRR = LEpin;
	Delay::us(1);
	LE->BSRR = LEpin << 16;
}

// Assumes that the MUX pin is already configured as "Read register 6" and connected to MISO
uint32_t MAX2871::Read() {
	uint8_t transmit[4] = {0x00, 0x00, 0x00, 0x06};
	HAL_SPI_Transmit(hspi, (uint8_t*) transmit, 4, 20);
	LE->BSRR = LEpin;
	memset(transmit, 0, sizeof(transmit));
	uint8_t recv[4];
	HAL_SPI_TransmitReceive(hspi, (uint8_t*) transmit, (uint8_t*) recv, 4, 20);
	LE->BSRR = LEpin << 16;
	// assemble readback result
	uint32_t result = ((uint32_t) recv[0] << 24) | ((uint32_t) recv[1] << 16
	) | ((uint32_t) recv[2] << 8) | (recv[3] & 0xFF);
	result <<= 2;
	LOG_DEBUG("Readback: 0x%08x", result);
	return result;
}

bool MAX2871::BuildVCOMap() {
	memset(VCOmax, 0, sizeof(VCOmax));
	// save output frequency
	uint64_t oldFreq = outputFrequency;
	constexpr uint32_t step = 10000000;
	for (uint64_t freq = 3000000000; freq <= MaxFreq; freq += step) {
		SetFrequency(freq);
		UpdateFrequency();
		uint32_t start = HAL_GetTick();
		// set MUX to LD
		regs[2] &= ~(7UL << 26);
		regs[5] &= ~(1UL << 18);
		regs[2] |= (6UL << 26);
		Write(5, regs[5]);
		Write(2, regs[2]);
		while (!(MUX->IDR & MUXpin)) {
			if (HAL_GetTick() - start > 100) {
				LOG_ERR(
						"Failed to lock during VCO map build process, aborting (f=%lu%06luHz)",
						(uint32_t )(freq / 1000000),
						(uint32_t ) (freq % 1000000));
				gotVCOMap = false;
				// revert back to previous frequency
				SetFrequency(oldFreq);
				LE->BSRR = LEpin << 16;
				// Mux pin back to high impedance
				regs[2] &= ~(7UL << 26);
				regs[5] &= ~(1UL << 18);
				Update();
				return false;
			}
		}
		// set MUX to SPI read
		regs[2] &= ~(7UL << 26);
		regs[5] &= ~(1UL << 18);
		regs[2] |= (4UL << 26);
		regs[5] |= (1UL << 18);
		Write(5, regs[5]);
		Write(2, regs[2]);
		auto readback = Read();
		uint8_t vco = (readback & 0x01F8) >> 3;
		VCOmax[vco] = freq / 100000;
		LOG_INFO("VCO map: %lu%06luHz uses VCO %d",
			(uint32_t ) (freq / 1000000), (uint32_t ) (freq % 1000000), vco);
	}
	gotVCOMap = true;
	// revert back to previous frequency
	SetFrequency(oldFreq);
	// Mux pin back to high impedance
	regs[2] &= ~(7UL << 26);
	regs[5] &= ~(1UL << 18);

	// Turn off VAS, select VCO manually from now on
	regs[3] |= (1UL << 25);
	Update();
	return true;
}

uint8_t MAX2871::GetTemp() {
	// select temperature channel and start ADC
	regs[5] &= ~0x00000078;
	regs[5] |= 0x00000048;
	Write(5, regs[5]);
	Delay::us(100);
	// set MUX to SPI read
	regs[2] &= ~(7UL << 26);
	regs[5] &= ~(1UL << 18);
	regs[2] |= (4UL << 26);
	regs[5] |= (1UL << 18);
	Write(5, regs[5]);
	Write(2, regs[2]);
	uint8_t ADC_raw = (Read() >> 16) & 0x7F;
	LOG_DEBUG("Raw temp ADC: %d", ADC_raw);
	// Disable ADC
	regs[5] &= ~0x00000078;
	// Mux pin back to high impedance
	regs[2] &= ~(7UL << 26);
	regs[5] &= ~(1UL << 18);
	Write(5, regs[5]);
	Write(2, regs[2]);
	// convert to celsius and return
	return 95 - 1.14f * ADC_raw;
}
