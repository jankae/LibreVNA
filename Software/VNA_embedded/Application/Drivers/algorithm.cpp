#include "algorithm.hpp"

#include "stm.hpp"
#include <cmath>
#include <algorithm>

Algorithm::RationalApproximation Algorithm::BestRationalApproximation(float ratio, uint32_t max_denom) {
	RationalApproximation result;
	uint32_t a = 0, b = 1, c = 1, d = 1;
	while (b + d <= max_denom) {
		auto mediant = (float) (a + c) / (b + d);
		if (ratio == mediant) {
			if (b + d <= max_denom) {
				result.num = a + c;
				result.denom = b + d;
				return result;
			} else if (d > b) {
				result.num = c;
				result.denom = d;
				return result;
			} else {
				result.num = a;
				result.denom = b;
				return result;
			}
		} else if (ratio > mediant) {
			a = a + c;
			b = b + d;
		} else {
			c = a + c;
			d = b + d;
		}
	}
	// check which of the two is the better solution
	float dev_ab = (float) a / b - ratio;
	float dev_cd = (float) c / d - ratio;
	if(fabs(dev_cd) < fabs(dev_ab)) {
		result.num = c;
		result.denom = d;
	} else {
		result.num = a;
		result.denom = b;
	}
	return result;
}

uint32_t gcd(uint32_t u, uint32_t v) {
	if(u==0) {
		return v;
	} else if(v==0) {
		return u;
	}

	uint8_t i = __builtin_ctz(u);
	u >>= i;
	uint8_t j = __builtin_ctz(v);
	v >>= j;

	uint8_t k = i < j ? i : j;
	while(true) {
		if(u > v) {
			std::swap(u, v);
		}
		v -= u;
		if(v==0) {
			return u << k;
		}
		v >>= __builtin_ctz(v);
	}
}

Algorithm::RationalApproximation Algorithm::BestRationalApproximation(
		RationalApproximation ratio, uint32_t max_denom) {
	if(ratio.denom <= max_denom) {
		// nothing to do, we can just return the ratio as it is
		return ratio;
	}
	// Try to simplify the ratio.
	// Find greatest common divider
	uint32_t div = gcd(ratio.num, ratio.denom);
	ratio.num /= div;
	ratio.denom /= div;
	if(ratio.denom <= max_denom) {
		// small enough now, can use as is
		return ratio;
	}
	// no good, we need to approximate
	return Algorithm::BestRationalApproximation((float) ratio.num / ratio.denom, max_denom);
}
