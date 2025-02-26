#pragma once

#include "stm.hpp"

class MAX2871 {
public:
	constexpr MAX2871(SPI_HandleTypeDef *hspi, GPIO_TypeDef *LE = nullptr,
			uint16_t LEpin = 0, GPIO_TypeDef *RF_EN = nullptr,
			uint16_t RF_ENpin = 0, GPIO_TypeDef *LD = nullptr, uint16_t LDpin =	0,
			GPIO_TypeDef *CE = nullptr, uint16_t CEpin = 0,
			GPIO_TypeDef *MUX = nullptr, uint16_t MUXpin = 0) :
			regs(), f_PFD(0),
		hspi(hspi),
		CE(CE), CEpin(CEpin),
		LE(LE), LEpin(LEpin),
		MUX(MUX), MUXpin(MUXpin),
		RF_EN(RF_EN), RF_ENpin(RF_ENpin),
		LD(LD), LDpin(LDpin),
		outputFrequency(0),
		VCOmax(),
		gotVCOMap(false)
		{};

	bool Init();
	bool Init(uint32_t f_ref, bool doubler, uint16_t r, bool div2);
	bool SetReference(uint32_t f_ref, bool doubler, uint16_t r, bool div2);
	void ChipEnable(bool on);
	void RFEnable(bool on);
	bool Locked();
	enum class Power : uint8_t {
		n4dbm = 0x00,
		n1dbm = 0x01,
		p2dbm = 0x02,
		p5dbm = 0x03,
	};
	void SetPowerOutA(Power p, bool enabled = true);
	void SetPowerOutB(Power p, bool enabled = true);
	enum class Mode : uint8_t {
		LowNoise = 0x00,
		LowSpur1 = 0x02,
		LowSpur2 = 0x03,
	};
	void SetMode(Mode m);
	enum class CPMode : uint8_t {
		Disabled = 0x00,
		CP10 = 0x01,
		CP20 = 0x02,
		CP30 = 0x03,
	};
	void SetCPMode(CPMode m);
	void SetCPCurrent(uint8_t mA);
	bool SetFrequency(uint64_t f);
	void Update();
	void UpdateFrequency();
	bool BuildVCOMap();
	uint8_t GetTemp();
	uint32_t* GetRegisters() {
		return regs;
	}
	uint64_t GetActualFrequency() {
		return outputFrequency;
	}
	uint32_t DistanceToIntegerSpur() {
		uint32_t dist = outputFrequency % f_PFD;
		if(dist > f_PFD / 2) {
			return f_PFD - dist;
		} else {
			return dist;
		}
	}
private:
	static constexpr uint64_t MaxFreq = 6100000000; // 6GHz according to datasheet, but slight overclocking is possible

	uint32_t Read();
	void Write(uint8_t reg, uint32_t val);
	uint32_t regs[6];
	uint32_t f_PFD;
	SPI_HandleTypeDef *hspi;
	GPIO_TypeDef *CE;
	uint16_t CEpin;
	GPIO_TypeDef *LE;
	uint16_t LEpin;
	GPIO_TypeDef *MUX;
	uint16_t MUXpin;
	GPIO_TypeDef *RF_EN;
	uint16_t RF_ENpin;
	GPIO_TypeDef *LD;
	uint16_t LDpin;
	uint64_t outputFrequency;
	uint16_t VCOmax[64];
	bool gotVCOMap;
};
