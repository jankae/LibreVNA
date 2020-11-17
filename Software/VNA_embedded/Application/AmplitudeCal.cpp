#include "AmplitudeCal.hpp"
#include <cstring>
#include "Communication.h"
#include "HW_HAL.hpp"

#define LOG_LEVEL	LOG_LEVEL_INFO
#define LOG_MODULE	"CAL"
#include "Log.h"


// increment when Calibration struct format changed. If a wrong version is found in flash, it will revert to default values
static constexpr uint16_t version = 0x0000;

using CorrectionTable = struct {
	uint8_t usedPoints;
	uint32_t freq[AmplitudeCal::maxPoints]; // LSB = 10Hz
	int16_t port1Correction[AmplitudeCal::maxPoints]; // LSB = 0.01db
	int16_t port2Correction[AmplitudeCal::maxPoints]; // LSB = 0.01db
};

using Calibration = struct _calibration {
	uint16_t version;
	CorrectionTable Source;
	CorrectionTable Receiver;
};

static Calibration cal;

static_assert(sizeof(cal) <= AmplitudeCal::flash_size, "Reserved flash size is too small");

bool AmplitudeCal::Load() {
	HWHAL::flash.read(flash_address, sizeof(cal), &cal);
	if(cal.version != version) {
		LOG_WARN("Invalid version in flash, expected %u, got %u", version, cal.version);
		SetDefault();
		return false;
	} else {
		LOG_INFO("Loaded from flash");
		return true;
	}
}

bool AmplitudeCal::Save() {
	if(!HWHAL::flash.eraseRange(flash_address, flash_size)) {
		return false;
	}
	uint32_t write_size = sizeof(cal);
	if(write_size % Flash::PageSize != 0) {
		// round up to next page
		write_size += Flash::PageSize - write_size % Flash::PageSize;
	}
	return HWHAL::flash.write(flash_address, write_size, &cal);
}

void AmplitudeCal::SetDefault() {
	memset(&cal, 0, sizeof(cal));
	cal.version = version;
	cal.Source.usedPoints = 1;
	cal.Source.freq[0] = 100000000;
	cal.Receiver.usedPoints = 1;
	cal.Receiver.freq[0] = 100000000;
	LOG_INFO("Set to default");
}

static AmplitudeCal::Correction InterpolateCorrection(const CorrectionTable& table, uint64_t freq) {
	// adjust LSB to match table
	freq /= 10;
	AmplitudeCal::Correction ret;
	// find first valid index that is higher than the given frequency
	uint8_t i = 0;
	for (; i < table.usedPoints; i++) {
		if (table.freq[i] >= freq) {
			break;
		}
	}
	if (i == 0) {
		// no previous index, nothing to interpolate
		ret.port1 = table.port1Correction[0];
		ret.port2 = table.port2Correction[0];
	} else if (i >= table.usedPoints) {
		// went beyond last point, nothing to interpolate
		ret.port1 = table.port1Correction[table.usedPoints - 1];
		ret.port2 = table.port2Correction[table.usedPoints - 1];
	} else {
		// frequency is between i and i-1, interpolate
		float alpha = (float) (freq - table.freq[i - 1]) / (table.freq[i] - table.freq[i - 1]);
		ret.port1 = table.port1Correction[i - 1] * (1.0f - alpha) + table.port1Correction[i] * alpha;
		ret.port2 = table.port2Correction[i - 1] * (1.0f - alpha) + table.port2Correction[i] * alpha;
	}
	return ret;
}

AmplitudeCal::Correction AmplitudeCal::SourceCorrection(uint64_t freq) {
	return InterpolateCorrection(cal.Source, freq);
}

AmplitudeCal::Correction AmplitudeCal::ReceiverCorrection(uint64_t freq) {
	return InterpolateCorrection(cal.Receiver, freq);
}

static void SendCorrectionTable(const CorrectionTable& table, Protocol::PacketType type) {
	for(uint8_t i=0;i<table.usedPoints;i++) {
		// assemble packet
		Protocol::PacketInfo p;
		p.type = type;
		p.amplitudePoint.totalPoints = table.usedPoints;
		p.amplitudePoint.pointNum = i;
		p.amplitudePoint.freq = table.freq[i];
		p.amplitudePoint.port1 = table.port1Correction[i];
		p.amplitudePoint.port2 = table.port2Correction[i];
		Communication::Send(p);
	}
}

void AmplitudeCal::SendSource() {
	SendCorrectionTable(cal.Source, Protocol::PacketType::SourceCalPoint);
}

void AmplitudeCal::SendReceiver() {
	SendCorrectionTable(cal.Receiver, Protocol::PacketType::ReceiverCalPoint);
}

static void addPoint(CorrectionTable& table, const Protocol::AmplitudeCorrectionPoint& p) {
	if(p.pointNum >= AmplitudeCal::maxPoints) {
		// ignore out-of-bounds point
		return;
	}
	table.freq[p.pointNum] = p.freq;
	table.port1Correction[p.pointNum] = p.port1;
	table.port2Correction[p.pointNum] = p.port2;
	if(p.pointNum == p.totalPoints - 1) {
		// this was the last point, update used points and save
		table.usedPoints = p.totalPoints;
		LOG_INFO("Last point received, saving to flash");
		AmplitudeCal::Save();
	}
}

void AmplitudeCal::AddSourcePoint(const Protocol::AmplitudeCorrectionPoint& p) {
	addPoint(cal.Source, p);
}

void AmplitudeCal::AddReceiverPoint(const Protocol::AmplitudeCorrectionPoint& p) {
	addPoint(cal.Receiver, p);
}
