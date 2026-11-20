// Host-side check of the LibreVNA VNA frequency plan.
//
// Compiles the firmware's own Si5351C, MAX2871 and Algorithm drivers against a stub HAL
// (see build.sh), replays VNA::Setup() and VNA::SweepHalted() for one sweep, then decodes the
// register values the drivers produced into the frequencies the hardware would really make:
//   Si5351 PLL A/B and multisynths from the I2C register file (AN619 P1/P2/P3),
//   MAX2871 outputs from the N/FRAC/M/DIVA fields the FPGA gets for each point, with the
//   Si5351 reference output that is active at that point,
//   the DFT bin from the FPGA clock, ADC prescaler and phase increment.
// The output is the true offset of the final IF from the DFT bin at every point, for the
// port 1, port 2 and reference receivers. The firmware's own estimate is printed next to it.
//
//   fwplan F_START F_STOP POINTS IFBW [log] > points.csv
//   fwplan F_START F_STOP POINTS IFBW [log] --limit=0.75
//       no CSV; exit status 1 if any point's true offset exceeds 0.75 x IFBW (see run_checks.sh)
//
// Build-time switches:
//   FW_V164          firmware 1.6.4 API and behaviour (no exact Si5351 dividers, lowband on PLL B,
//                    '>' band-switch comparisons). Default is 1.6.5 / current master.
//   PATCH_LO2_EXACT  exact divider for Si5351 PLL B (the 2nd LO); costs a long divider search.
//   PATCH_LO2_ROUND  round instead of truncate in the fast Si5351 divider path (build.sh edits
//                    the copied Si5351C.cpp).
//   LO2_TOL_DIV=n    2nd-LO retune tolerance is IFBW/n (firmware: 2).
//
// VNA.cpp is not compiled (it needs the FPGA, USB and RTOS); the parts of it that choose the
// frequencies are copied below, with the firmware line they mirror noted.

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "Si5351C.hpp"
#include "max2871.hpp"

#ifndef LO2_TOL_DIV
#define LO2_TOL_DIV 2
#endif

namespace hoststub {
uint8_t si5351_regs[256];
uint32_t tick;
}

typedef long double R;  // 64-bit mantissa: 6 GHz resolves to below 1 nHz

// ---- constants from Hardware.hpp / VNA.cpp ----
static constexpr uint32_t XTAL = 26000000;
static constexpr uint32_t PLLA = 800000000;
static constexpr uint32_t IF1 = 62000000, IF2 = 250000;
static constexpr uint32_t DefaultLO2 = IF1 - IF2;
static constexpr uint8_t LO2Multiplier = 13;
static constexpr uint32_t PLLRef = 100000000;
static constexpr uint32_t BandSwitch = 25000000;
static constexpr uint32_t FPGAClkIn = 16000000, FPGAClockrate = 102400000;
static constexpr uint32_t ADCRate = 800000;
static constexpr uint8_t ADCPrescaler = 128;
static constexpr uint16_t DFTPhaseInc = 1280;
static constexpr uint8_t altPrescaler = 112;
static constexpr uint16_t altPhaseInc = 1120;
static constexpr uint64_t maxFreq = 6000000000ULL;
static constexpr uint16_t maxPointsBetweenHalts = 40;
namespace Ch { enum { LowbandSource = 0, RefLO2 = 1, Port1LO2 = 2, Source = 3, Port2LO2 = 4, LO1 = 5, FPGA = 7 }; }

static Si5351C Si(nullptr, XTAL);
static SPI_HandleTypeDef spi;
static GPIO_TypeDef le_pin;  // latch-enable pin written by MAX2871::Write()
static MAX2871 Source(&spi, &le_pin, 1), LO1(&spi, &le_pin, 1);
static uint32_t PLLRefFreqs[] = {PLLRef, PLLRef - 1000000};
static uint8_t sourceRefIndex, LO1RefIndex;
static uint32_t last_LO2, actualBandwidth;

extern unsigned long alg_iters;            // counted in algorithm.cpp by build.sh
static unsigned long max_iters, pllb_calls;

#ifdef FW_V164
#define LOWBAND_PLL Si5351C::PLL::B
static bool setPLLB(uint32_t f) { return Si.SetPLL(Si5351C::PLL::B, f, Si5351C::PLLSource::XTAL); }
static bool setLowband(uint32_t f) { return Si.SetCLK(Ch::LowbandSource, f, LOWBAND_PLL, Si5351C::DriveStrength::mA8); }
static inline bool srcActive(uint64_t f) { return f > BandSwitch; }   // VNA.cpp 1.6.4: '>'
#else
#define LOWBAND_PLL Si5351C::PLL::A
#ifdef PATCH_LO2_EXACT
static constexpr bool LO2exact = true;
#else
static constexpr bool LO2exact = false;   // VNA.cpp 1.6.5: SetPLL(PLL::B, ..., false)
#endif
static bool setPLLB(uint32_t f) {
	unsigned long before = alg_iters;
	bool ok = Si.SetPLL(Si5351C::PLL::B, f, Si5351C::PLLSource::XTAL, LO2exact);
	if (alg_iters - before > max_iters) {
		max_iters = alg_iters - before;
	}
	pllb_calls++;
	return ok;
}
static bool setLowband(uint32_t f) { return Si.SetCLK(Ch::LowbandSource, f, LOWBAND_PLL, Si5351C::DriveStrength::mA8, false); }
static inline bool srcActive(uint64_t f) { return f >= BandSwitch; }
#endif

// ---- register decoding (ground truth) ----
static R ms_ratio(const uint8_t *r) {   // AN619: a + b/c = (P1 + 512 + P2/P3) / 128
	uint32_t P3 = ((uint32_t) r[0] << 8 | r[1]) | ((uint32_t) (r[5] & 0xF0) << 12);
	uint32_t P1 = ((uint32_t) (r[2] & 0x03) << 16) | ((uint32_t) r[3] << 8) | r[4];
	uint32_t P2 = ((uint32_t) (r[5] & 0x0F) << 16) | ((uint32_t) r[6] << 8) | r[7];
	return ((R) P1 + 512 + (R) P2 / P3) / 128;
}
static R si_pll(int pll) {
	return XTAL * ms_ratio(&hoststub::si5351_regs[26 + 8 * pll]);
}
static R si_clk(int n) {
	const uint8_t *g = hoststub::si5351_regs;
	uint8_t ctrl = g[16 + n];
	R fpll = si_pll((ctrl >> 5) & 1);
	if (n == 6 || n == 7) {
		return fpll / g[90 + (n - 6)];
	}
	const uint8_t *r = &g[42 + 8 * n];
	R div = (r[2] & 0x0C) == 0x0C ? 4 : ms_ratio(r);
	return fpll / div / (1 << ((r[2] >> 4) & 7));
}
static R max2871_out(const uint32_t *regs, R fref) {
	uint32_t N = (regs[0] & 0x7FFF8000) >> 15, F = (regs[0] & 0x7FF8) >> 3, M = (regs[1] & 0x7FF8) >> 3;
	uint32_t Rd = (regs[2] >> 14) & 0x3FF, dbl = (regs[2] >> 25) & 1, div2 = (regs[2] >> 24) & 1;
	uint32_t diva = (regs[4] >> 20) & 7;
	R pfd = fref * (1 + dbl) / (Rd * (1 + div2));
	return pfd * ((R) N + (R) F / M) / (1 << diva);
}

// ---- VNA.cpp ----
struct Settings { uint64_t f_start, f_stop; uint16_t points; uint32_t ifbw; bool log; } S;

static uint64_t pointFrequency(uint16_t i) {                       // getPointFrequency()
	if (!S.log) {
		return S.f_start + (S.f_stop - S.f_start) * i / (S.points - 1);
	}
	return S.f_start * pow(10.0, i * log10((double) S.f_stop / S.f_start) / (S.points - 1));
}

static bool setPLLFrequencies(uint64_t f) {                        // setPLLFrequencies()
	uint64_t sourceFreq = f >= BandSwitch ? f : 0;
	uint64_t LOFreq = f + IF1;
	if (srcActive(sourceFreq)) {
		Source.SetFrequency(sourceFreq);
	}
	LO1.SetFrequency(LOFreq);
	bool sw = false;
	if (srcActive(sourceFreq)) {
		uint32_t d = Source.DistanceToIntegerSpur();
		if (d > 0 && d < 3 * IF2) {
			sourceRefIndex = !sourceRefIndex;
			Source.SetReference(PLLRefFreqs[sourceRefIndex], false, 1, false);
			Source.SetFrequency(sourceFreq);
			sw = true;
		}
	}
	uint32_t d = LO1.DistanceToIntegerSpur();
	if (d > 0 && d < 3 * IF2) {
		LO1RefIndex = !LO1RefIndex;
		LO1.SetReference(PLLRefFreqs[LO1RefIndex], false, 1, false);
		LO1.SetFrequency(LOFreq);
		sw = true;
	}
	return sw;
}

static bool needs2LOshift(uint64_t f, uint32_t current2LO, uint32_t IFBW, uint32_t *new2LO) {
	uint64_t actualSource = Source.GetActualFrequency();
	uint64_t actual1LO = LO1.GetActualFrequency();
	if (f < BandSwitch) {
		actualSource = f;
	}
	uint32_t firstIF = actual1LO - actualSource;
	uint32_t finalIF = firstIF - current2LO;
	uint32_t dev = abs((int32_t) (finalIF - IF2));
	if (dev > IFBW / LO2_TOL_DIV) {
		*new2LO = firstIF - IF2;
		return true;
	}
	return false;
}

static uint32_t alias(uint32_t f, uint32_t fs) {                   // Util::Alias()
	f %= fs;
	return f > fs / 2 ? fs - f : f;
}

struct Point { uint64_t f, src_fw, lo_fw; bool lowband, halt; uint8_t sref, lref; uint32_t src[6], lo[6]; };

int main(int argc, char **argv) {
	if (argc < 5) {
		fprintf(stderr, "usage: fwplan F_START F_STOP POINTS IFBW [log] [--limit=FRACTION_OF_IFBW]\n");
		return 2;
	}
	S.f_start = strtoull(argv[1], nullptr, 10);
	S.f_stop = strtoull(argv[2], nullptr, 10);
	S.points = atoi(argv[3]);
	S.ifbw = atoi(argv[4]);
	double limit = -1;
	S.log = false;
	for (int k = 5; k < argc; k++) {
		if (!strcmp(argv[k], "log")) {
			S.log = true;
		} else if (!strncmp(argv[k], "--limit=", 8)) {
			limit = atof(argv[k] + 8);
		}
	}
	if (S.f_stop > maxFreq) {
		fprintf(stderr, "harmonic mode (above 6 GHz) is not modelled\n");
		return 2;
	}

	// HW::Init()
	Si.Init();
	Si.SetPLL(Si5351C::PLL::A, PLLA, Si5351C::PLLSource::XTAL);
	Si.SetPLL(Si5351C::PLL::B, DefaultLO2 * LO2Multiplier, Si5351C::PLLSource::XTAL);
	Si.SetCLK(Ch::Source, PLLRef, Si5351C::PLL::A, Si5351C::DriveStrength::mA8);
	Si.SetCLK(Ch::LO1, PLLRef, Si5351C::PLL::A, Si5351C::DriveStrength::mA8);
	Si.SetCLK(Ch::FPGA, FPGAClkIn, Si5351C::PLL::A, Si5351C::DriveStrength::mA8);
	Si.SetCLK(Ch::Port1LO2, DefaultLO2, Si5351C::PLL::B, Si5351C::DriveStrength::mA2);
	Si.SetCLK(Ch::Port2LO2, DefaultLO2, Si5351C::PLL::B, Si5351C::DriveStrength::mA2);
	Si.SetCLK(Ch::RefLO2, DefaultLO2, Si5351C::PLL::B, Si5351C::DriveStrength::mA2);
	Source.Init(PLLRef, false, 1, false);
	LO1.Init(PLLRef, false, 1, false);

	// VNA::Setup()
	sourceRefIndex = LO1RefIndex = 0;
	Source.SetReference(PLLRefFreqs[0], false, 1, false);
	LO1.SetReference(PLLRefFreqs[0], false, 1, false);
	uint32_t spp = ADCRate / S.ifbw;
	if (spp % 16) {
		spp += 16 - spp % 16;
	}
	actualBandwidth = ADCRate / spp;
	last_LO2 = DefaultLO2;
	setPLLB(last_LO2 * LO2Multiplier);
	std::vector<Point> pts(S.points);
	std::vector<uint32_t> belief_lo2(S.points);
	bool last_lowband = false;
	uint16_t noHalt = 0;
	for (uint16_t i = 0; i < S.points; i++) {
		Point &p = pts[i];
		p.f = pointFrequency(i);
		p.lowband = p.f < BandSwitch;
		bool halt = p.lowband || i == 0;
		if (setPLLFrequencies(p.f)) {
			halt = true;
		}
		uint32_t nl;
		if (needs2LOshift(p.f, last_LO2, actualBandwidth, &nl)) {
			halt = true;
			last_LO2 = nl;
		}
		if (last_lowband && !p.lowband) {
			halt = true;
		}
		if (!halt) {
			if (++noHalt > maxPointsBetweenHalts) {
				halt = true;
				noHalt = 0;
			}
		} else {
			noHalt = 0;
		}
		p.halt = halt;
		memcpy(p.src, Source.GetRegisters(), sizeof(p.src));
		memcpy(p.lo, LO1.GetRegisters(), sizeof(p.lo));
		belief_lo2[i] = last_LO2;
		p.src_fw = p.lowband ? p.f : Source.GetActualFrequency();
		p.lo_fw = LO1.GetActualFrequency();
		p.sref = sourceRefIndex;
		p.lref = LO1RefIndex;
		last_lowband = p.lowband;
	}
	sourceRefIndex = LO1RefIndex = 0;
	Source.SetReference(PLLRefFreqs[0], false, 1, false);
	LO1.SetReference(PLLRefFreqs[0], false, 1, false);
	Si.SetCLK(Ch::RefLO2, DefaultLO2, Si5351C::PLL::B, Si5351C::DriveStrength::mA2);

	// sweep: VNA::SweepHalted() at halted points, then the hardware truth at every point
	R worst = 0;
	uint64_t worst_f = 0;
	if (limit < 0) printf("i,f_Hz,lowband,halt,src_ref,lo1_ref,lo2_set_Hz,fw_offset_Hz,off_port1_Hz,off_port2_Hz,off_ref_Hz,adc_alt,spp\n");
	bool adcShifted = false;
	for (uint16_t i = 0; i < S.points; i++) {
		Point &p = pts[i];
		if (p.halt) {
			bool adcShift = false;
			if (p.lowband) {
				setLowband(p.f);
				uint32_t mix = (IF1 + p.f) - (IF1 - IF2);
				if ((uint32_t) abs((int32_t) alias(mix, ADCRate) - (int32_t) IF2) <= actualBandwidth * 2) {
					adcShift = true;
				}
			}
			if (i == 0) {
				sourceRefIndex = LO1RefIndex = 0;
				Source.SetReference(PLLRefFreqs[0], false, 1, false);
				LO1.SetReference(PLLRefFreqs[0], false, 1, false);
				Si.SetCLK(Ch::Source, PLLRefFreqs[0], Si5351C::PLL::A, Si5351C::DriveStrength::mA8);
				Si.SetCLK(Ch::LO1, PLLRefFreqs[0], Si5351C::PLL::A, Si5351C::DriveStrength::mA8);
				last_LO2 = DefaultLO2;
				setPLLB(last_LO2 * LO2Multiplier);
			}
			if (setPLLFrequencies(p.f)) {
				Si.SetCLK(Ch::Source, PLLRefFreqs[sourceRefIndex], Si5351C::PLL::A, Si5351C::DriveStrength::mA8);
				Si.SetCLK(Ch::LO1, PLLRefFreqs[LO1RefIndex], Si5351C::PLL::A, Si5351C::DriveStrength::mA8);
			}
			if (needs2LOshift(p.f, last_LO2, actualBandwidth, &last_LO2)) {
				setPLLB(last_LO2 * LO2Multiplier);
			}
			adcShifted = adcShift;
		}
		R src = p.lowband ? si_clk(Ch::LowbandSource) : max2871_out(p.src, si_clk(Ch::Source));
		R lo1 = max2871_out(p.lo, si_clk(Ch::LO1));
		R fpga = si_clk(Ch::FPGA) * ((R) FPGAClockrate / FPGAClkIn);
		R bin = adcShifted ? fpga / altPrescaler * altPhaseInc / 4096 : fpga / ADCPrescaler * DFTPhaseInc / 4096;
		R if1 = lo1 - src;
		if (getenv("FWPLAN_ITERS") && i == S.points - 1) {
			fprintf(stderr, "PLL B programmed %lu times, worst divider search %lu iterations\n", pllb_calls, max_iters);
		}
		R off[3] = {if1 - si_clk(Ch::Port1LO2) - bin, if1 - si_clk(Ch::Port2LO2) - bin, if1 - si_clk(Ch::RefLO2) - bin};
		for (R o : off) {
			if (fabsl(o) > worst) {
				worst = fabsl(o);
				worst_f = p.f;
			}
		}
		if (limit < 0) printf("%u,%llu,%d,%d,%d,%d,%u,%.1f,%.4Lf,%.4Lf,%.4Lf,%d,%u\n", i, (unsigned long long) p.f, p.lowband, p.halt,
				p.sref, p.lref, belief_lo2[i], (double) p.lo_fw - (double) p.src_fw - (double) belief_lo2[i] - IF2,
				off[0], off[1], off[2], adcShifted, spp);
	}
	if (limit >= 0) {
		bool fail = worst > limit * actualBandwidth;
		printf("%s  %llu-%llu Hz, %u points%s, IFBW %u Hz: worst offset %.2Lf Hz (%.2Lf x IFBW) at %llu Hz\n",
				fail ? "FAIL" : "ok  ", (unsigned long long) S.f_start, (unsigned long long) S.f_stop, S.points,
				S.log ? " log" : "", actualBandwidth, worst, worst / actualBandwidth, (unsigned long long) worst_f);
		return fail ? 1 : 0;
	}
	return 0;
}
