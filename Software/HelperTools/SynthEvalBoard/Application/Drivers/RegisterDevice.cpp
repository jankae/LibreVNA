#include "RegisterDevice.hpp"

#include <cstring>

uint8_t RegisterDevice::cnt = 0;
std::array<RegisterDevice*, RegisterDevice::maxDevices> RegisterDevice::devices;

Protocol::DirectRegisterInfo RegisterDevice::getInfo() {
	Protocol::DirectRegisterInfo i;
	i.num = num;
	strncpy(i.name, name, sizeof(i.name));
	i.name[sizeof(i.name) - 1] = '\0';
	strncpy(i.type, type, sizeof(i.type));
	i.type[sizeof(i.type) - 1] = '\0';
	return i;
}

RegisterDevice* RegisterDevice::getDevice(uint8_t num) {
	if(num < cnt) {
		return devices[num];
	} else {
		return nullptr;
	}
}
