#include "Util.hpp"

uint32_t Util::Alias(int64_t f, uint32_t f_s) {
	// move into f_s range
	f %= f_s;
	if (f < 0) {
		f += f_s;
	}
	// fold at half the samplerate
	if(f >= f_s / 2) {
		f = f_s - f;
	}
	return f;
}
