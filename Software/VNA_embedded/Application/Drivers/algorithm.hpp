#pragma once

#include <stdint.h>

namespace Algorithm {

using RationalApproximation = struct _rationalapproximation {
	uint32_t num;
	uint32_t denom;
};

RationalApproximation BestRationalApproximation(RationalApproximation ratio, uint32_t max_denom);
RationalApproximation BestRationalApproximation(float ratio, uint32_t max_denom);

}
