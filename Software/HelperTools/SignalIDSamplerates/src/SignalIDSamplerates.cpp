//============================================================================
// Name        : SignalIDSamplerates.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <vector>
#include <cstdlib>
#include <limits>
using namespace std;

constexpr int32_t freqRange = 20000000;
constexpr int32_t freqStep = 4000;
constexpr int32_t FPGA_CLK = 102400000;
constexpr int32_t IF_freq = 250000;
constexpr int32_t minAliasDistance = 100000;

constexpr uint8_t minPrescaler = 112;
constexpr uint8_t maxPrescaler = 180;
constexpr uint8_t maxPrescalers = 5;

int bestDistance;
vector<int> bestPrescalers;

int Alias(int f, int f_s) {
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

int DistanceToIF(double f, double f_s) {
	auto alias = Alias(f, f_s);
	return abs(int(alias - IF_freq));
}

int farthestDistanceForFrequency(double f, vector<int> &prescalers) {
	int highestDistance = numeric_limits<int>::min();

	for(auto p : prescalers) {
		double f_s = (double) FPGA_CLK / p;
		auto dist = DistanceToIF(f, f_s);
		if(dist > highestDistance) {
			highestDistance = dist;
		}
	}
	return highestDistance;
}

void checkPrescalers(vector<int> &prescalers) {
//	cout << "Checking [" << prescalers[0];
//	for (auto i = prescalers.begin() + 1; i != prescalers.end(); i++) {
//		cout << ',' << *i;
//	}
//	cout << "]... ";
	int lowestDistance = numeric_limits<int>::max();
	for(int freq = minAliasDistance + freqStep;freq < freqRange; freq += freqStep) {
		int dist = farthestDistanceForFrequency(IF_freq + freq, prescalers);
		if(dist < lowestDistance) {
			lowestDistance = dist;
		}
		if(freq > 2*IF_freq + minAliasDistance) {
			int dist = farthestDistanceForFrequency(IF_freq - freq, prescalers);
			if(dist < lowestDistance) {
				lowestDistance = dist;
			}
		}
		if(lowestDistance < bestDistance) {
			break;
		}
	}
//	cout << lowestDistance << endl;
	if(lowestDistance > bestDistance) {
		bestDistance = lowestDistance;
		bestPrescalers = prescalers;
	}
}

void recursivePrescalersTest(vector<int> &prescalers, int depth) {
	if(prescalers.size() >= depth) {
		checkPrescalers(prescalers);
	} else {
		prescalers.push_back(0);
		int start = minPrescaler;
		if(prescalers.size() > 1) {
			start = prescalers[prescalers.size() - 2] + 1;
		}
		for (int i = start; i <= maxPrescaler; i++) {
			prescalers.back() = i;
			recursivePrescalersTest(prescalers, depth);
		}
		prescalers.pop_back();
	}
}

int main() {
	vector<int> prescalers = {112};
	for(int d = 2;d<=maxPrescalers;d++) {
		bestDistance = 0;
		recursivePrescalersTest(prescalers, d);
		cout << "Depth " << d <<": best distance: " << bestDistance << " Prescalers: [" << bestPrescalers[0];
		for (auto i = bestPrescalers.begin() + 1; i != bestPrescalers.end(); i++) {
			cout << ',' << *i;
		}
		cout << "]" << endl;
	}
	return 0;
}
