#include "Hardware.hpp"

#include "Protocol.hpp"

bool HW::Init() {
	return true;
}

void HW::fillDeviceInfo(Protocol::DeviceInfo *info) {
	*info = HW::Info;
}
