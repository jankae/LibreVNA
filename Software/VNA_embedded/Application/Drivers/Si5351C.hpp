#pragma once

#include "stm.hpp"

class Si5351C {
public:
	enum class PLL : uint8_t {
		A = 0,
		B = 1,
	};
	enum class DriveStrength : uint8_t {
		mA2 = 0x00,
		mA4 = 0x01,
		mA6 = 0x02,
		mA8 = 0x03,
	};
	enum class PLLSource : uint8_t {
		XTAL,
		CLKIN,
	};
	constexpr Si5351C(I2C_HandleTypeDef *i2c, uint32_t XTAL_freq, GPIO_InitTypeDef *intr_gpio = nullptr,
			uint16_t intr_pin = 0, GPIO_InitTypeDef *oeb_gpio = nullptr, uint16_t oeb_pin = 0):
		i2c(i2c),
		intr_gpio(intr_gpio),
		intr_pin(intr_pin),
		oeb_gpio(oeb_gpio),
		oeb_pin(oeb_pin),
		FreqPLL{},
		FreqXTAL(XTAL_freq),
		FreqCLKINDiv(0) {
	};
	bool Init(uint32_t clkin_freq = 0);
	bool ConfigureCLKIn(uint32_t clkin_freq);
	bool SetPLL(PLL pll, uint32_t frequency, PLLSource src);
	bool SetCLK(uint8_t clknum, uint32_t frequency, PLL source, DriveStrength strength = DriveStrength::mA2, uint32_t PLLFreqOverride = 0);
	bool SetBypass(uint8_t clknum, PLLSource source, DriveStrength strength = DriveStrength::mA2);
	bool SetCLKtoXTAL(uint8_t clknum);
	bool SetCLKToCLKIN(uint8_t clknum);
	bool Enable(uint8_t clknum);
	bool Disable(uint8_t clknum);
	bool Locked(PLL pll);
	bool ResetPLL(PLL pll);
	bool ExtCLKAvailable();

	// Direct register access of clk configuration registers
	// config has to point to a buffer containing at least 8 bytes
	bool WriteRawCLKConfig(uint8_t clknum, const uint8_t *config);
	bool ReadRawCLKConfig(uint8_t clknum, uint8_t *config);
private:
	void FindOptimalDivider(uint32_t f_pll, uint32_t f, uint32_t &P1, uint32_t &P2, uint32_t &P3);
	enum class Reg : uint8_t {
		DeviceStatus = 0,
		InterruptStatusSticky = 1,
		InterruptStatusMask = 2,
		OutputEnableControl = 3,
		OEBPinMask = 9,
		PLLInputSource = 15,
		CLK0Control = 16,
		CLK1Control = 17,
		CLK2Control = 18,
		CLK3Control = 19,
		CLK4Control = 20,
		CLK5Control = 21,
		CLK6Control = 22,
		CLK7Control = 23,
		CLK3_0DisableState = 24,
		CLK7_4DisableState = 25,
		MSNA_CONFIG  = 26,
		MSNB_CONFIG = 34,
		MS0_CONFIG = 42,
		MS1_CONFIG = 50,
		MS2_CONFIG = 58,
		MS3_CONFIG = 66,
		MS4_CONFIG = 74,
		MS5_CONFIG = 82,
		MS6_CONFIG = 90,
		MS7_CONFIG = 91,
		R6_7_Divider = 92,
		// Left out: Spread Spectrum and VCXO parameters
		CLK0_Offset = 165,
		CLK1_Offset = 166,
		CLK2_Offset = 167,
		CLK3_Offset = 168,
		CLK4_Offset = 169,
		CLK5_Offset = 170,
		PLLReset = 177,
		CrystalLoadCapacitance = 183,
		FanoutEnable = 187,
	};

	using PLLConfig = struct {
		uint32_t P1, P2, P3;
		bool IntegerMode;
		PLLSource source;
	};
	bool WritePLLConfig(PLLConfig config, PLL pll);

	using ClkConfig = struct {
		uint32_t P1, P2, P3;
		uint8_t RDiv; // 1 to 128, only 2^n
		bool DivideBy4;
		bool PoweredDown;
		bool IntegerMode;
		PLL source;
		bool Inverted;
		DriveStrength strength;
	};
	bool WriteClkConfig(ClkConfig config, uint8_t clknum);

	static constexpr uint8_t address = 0xC0;
	bool WriteRegister(Reg reg, uint8_t data);
	bool ReadRegister(Reg reg, uint8_t *data);
	bool SetBits(Reg reg, uint8_t bits);
	bool ClearBits(Reg reg, uint8_t bits);
	bool WriteRegisterRange(Reg start, const uint8_t *data, uint8_t len);
	bool ReadRegisterRange(Reg start, uint8_t *data, uint8_t len);
	I2C_HandleTypeDef *i2c;
	GPIO_InitTypeDef *intr_gpio;
	uint16_t intr_pin;
	GPIO_InitTypeDef *oeb_gpio;
	uint16_t oeb_pin;
	uint32_t FreqPLL[2];
	uint32_t FreqXTAL, FreqCLKINDiv;
};
