#ifndef NOISE_H
#define NOISE_H

#include "HeightMap.h"

class ImprovedPerlinNoise {
public:
	ImprovedPerlinNoise();
	~ImprovedPerlinNoise();

	double getValue(double, double, double);
	double getValue(double, double, double, int, double);

	HeightMap convertToHeightMap(int, int, int, int, double);
	HeightMap convertToHeightMap(int, int, int, int, double, int, double);
private:
	double fade(double);
	double lerp(double , double, double);
	double grad(int, double, double, double);
	
	int p[512];
	const static int permutation[];
};

#endif

