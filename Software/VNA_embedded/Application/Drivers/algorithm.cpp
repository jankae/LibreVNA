#include "algorithm.hpp"

#include "stm.hpp"
#include <cmath>

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
