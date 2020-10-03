#pragma once

#include <cstdint>
#include "Protocol.hpp"

#define USE_DEBUG_PINS

#ifdef USE_DEBUG_PINS
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

static constexpr uint32_t ADCSamplerate = 800000;
static constexpr uint32_t IF1 = 62000000;
static constexpr uint32_t IF2 = 250000;
static constexpr uint32_t LO1_minFreq = 25000000;
static constexpr uint32_t MaxSamples = 130944;
static constexpr uint32_t MinSamples = 16;
static constexpr uint32_t PLLRef = 100000000;
static constexpr uint16_t MaxPoints = 4501;

static constexpr Protocol::DeviceLimits Limits = {
		.minFreq = 1000000,
		.maxFreq = 6000000000,
		.minIFBW = ADCSamplerate / MaxSamples,
		.maxIFBW = ADCSamplerate / MinSamples,
		.maxPoints = MaxPoints,
		.cdbm_min = -4000,
		.cdbm_max = 0,
		.minRBW = (uint32_t) (ADCSamplerate * 2.23f / MaxSamples),
		.maxRBW = (uint32_t) (ADCSamplerate * 2.23f / MinSamples),
};

enum class Mode {
	Idle,
	Manual,
	VNA,
	SA,
};

bool Init();
void SetMode(Mode mode);
void SetIdle();
void Work();

bool GetTemps(uint8_t *source, uint8_t *lo);
void fillDeviceInfo(Protocol::DeviceInfo *info);
namespace Ref {
	bool available();
	// reference won't change until update is called
	void set(Protocol::ReferenceSettings s);
	void update();
}

}
