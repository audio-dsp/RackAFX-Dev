/*
 * Noise.cpp
 *
 */

#include "../include/basic_blocks.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <stdio.h>

static float b0;
static float b1;
static float b2;
static float b3;
static float b4;
static float b5;
static float b6;
Noise::Noise() :
	BasicBlock(1) {
	setType(WHITE);
	b0 = b1 = b2 = b3 = b4 = b5 = b6 = 0.0;
}

static float generateWhiteNoise() {
	return (2.0f * rand() / RAND_MAX) - 1.0f;
}

static float generatePinkNoise()
{
	float white = (2.0f * rand() / RAND_MAX) - 1.0f;
	float pink;
	
	
	b0 = 0.99886f * b0 + white * 0.0555179f;
	b1 = 0.99332f * b1 + white * 0.0750759f;
	b2 = 0.96900f * b2 + white * 0.1538520f;
	b3 = 0.86650f * b3 + white * 0.3104856f;
	b4 = 0.55000f * b4 + white * 0.5329522f;
	b5 = -0.7616f * b5 - white * 0.0168980f;
	pink = b0 + b1 + b2 + b3 + b4 + b5 + b6 + white * 0.5362f;
	b6 = white * 0.115926f;
	return pink / 10.0f;

}

void Noise::setAmplitudeInput(BasicBlock * block) {
	(this->inputs)[0] = block;
}


void Noise::setType(noisetype_t type) {
	this->noise_type = type;
	srand(time(NULL));
	if (type == WHITE) {
		this->noiseGenerator = generateWhiteNoise;
		
	} else {
		this->noiseGenerator = generatePinkNoise;
	}
}

noisetype_t Noise::getType() {
	return this->noise_type;
}

float Noise::getNextValue() {
	if (!this->ON)
		return 0.0;
	float amp = inputs[0]->getNextValue();
	return amp * this->noiseGenerator();
}
