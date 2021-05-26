#pragma once

#include <cstdint>
#include <array>
#include "Protocol.hpp"

extern int global;

class RegisterDevice {
public:
	constexpr RegisterDevice(const char *type, const char *name) :
	type(type),
	name(name),
	num(0)
	{
		num = cnt;
		if(cnt < maxDevices) {
			devices[cnt] = this;
			cnt++;
		} else {
			// not enough room in array. A debug message would be useful here
			// but the constructor is called before any hardware initialization
			// so we can do nothing here
		}
	}
	virtual void writeRegister(uint32_t address, uint64_t data) = 0;
	virtual uint64_t readRegister(uint32_t address) = 0;

	Protocol::DirectRegisterInfo getInfo();

	static uint8_t getNumDevices() { return cnt;};
	static RegisterDevice *getDevice(uint8_t num);

private:
	static constexpr uint8_t maxDevices = 10;
	static std::array<RegisterDevice*,maxDevices> devices;
	static uint8_t cnt;
	const char *type;
	const char *name;
	uint8_t num;
};
