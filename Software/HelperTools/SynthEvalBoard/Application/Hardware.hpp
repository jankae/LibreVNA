#pragma once

#include "Protocol.hpp"

namespace HW {

static constexpr uint8_t registerDevices = 3;

static constexpr Protocol::DeviceInfo Info = {
		.ProtocolVersion = Protocol::Version,
		.FW_major = 0,
		.FW_minor = 0,
		.FW_patch = 1,
		.HW_Revision = 'A',
	    .extRefAvailable = 0,
	    .extRefInUse = 0,
	    .FPGA_configured = 0,
	    .source_locked = 0,
	    .LO1_locked = 0,
	    .ADC_overload = 0,
		.unlevel = 0,
		.temp_source = 0,
		.temp_LO1 = 0,
		.temp_MCU = 0,
		.limits_minFreq = 0,
		.limits_maxFreq = 0,
		.limits_minIFBW = 0,
		.limits_maxIFBW = 0,
		.limits_maxPoints = 0,
		.limits_cdbm_min = 0,
		.limits_cdbm_max = 0,
		.limits_minRBW = 0,
		.limits_maxRBW = 0,
		.limits_maxAmplitudePoints = 0,
		.limits_maxFreqHarmonic = 0,
		.num_directRegisterDevices = registerDevices,
};

bool Init();
void fillDeviceInfo(Protocol::DeviceInfo *info);

}
