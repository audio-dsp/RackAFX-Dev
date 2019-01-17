/*
 * Waveform.cpp
 *
 */
#pragma pack(1)
#include "../include/basic_blocks.h"
#include <cmath>

vector<float> * Waveforms::trig = NULL;
vector<float> * Waveforms::saw = NULL;
vector<float> * Waveforms::sawtrig = NULL;
vector<float> * Waveforms::square = NULL;
vector<float> * Waveforms::widerect = NULL;
vector<float> * Waveforms::narrowrect = NULL;

vector<float> * Waveforms::getWaveform(wavetype_t wave) {
	switch (wave) {
		case TRIG:
			return trig;
		case SAWTRIG:
			return trig;
		case SAW:
			return saw;
		case SQUARE:
			return square;
		case WIDERECT:
			return widerect;
		case NARROWRECT:
			return narrowrect;
		default:
			return NULL;
	}
}

static float triangular_wave(float x) {
	float aux;
	aux = fmod(x, 1.0f);
	if (aux < 0.5f){
		return 4.0f*aux - 1.0f;
	}
	return -4.0f*aux + 3.0f;
}

static float square_wave(float x) {
	float aux; 
	aux = fmod(x, 1.0f);
	if (aux < 0.5)
		return 1.0;
	return -1.0;
}

static float narrow_wave(float x) {
	float aux;
	aux = fmod(x, 1.0f);
	if (aux < 0.2) {
		return 1.0;
	}
	return -1.0;
}

static float wide_wave(float x) {
	float aux;
	aux = fmod(x, 1.0f);
	if (aux < 0.7) {
		return 1.0;
	}
	return -1.0;
}

static float sawtooth_wave(float x) {
	return x - floor(x);
}

void Waveforms::initializeWaves(int length, int extraPoints) {
	int i;
	float aux;
	trig = new vector<float> (length + extraPoints);
	sawtrig = new vector<float> (length + extraPoints);
	saw = new vector<float> (length + extraPoints);
	square = new vector<float> (length + extraPoints);
	widerect = new vector<float> (length + extraPoints);
	narrowrect = new vector<float> (length + extraPoints);

	for (i = 0; i < length + extraPoints; i++) {
		aux =((float)i)/length;
		(*trig)[i] = triangular_wave(aux);
		(*square)[i] = square_wave(aux);
		(*saw)[i] = sawtooth_wave(aux);
		(*narrowrect)[i] = narrow_wave(aux);
		(*widerect)[i] = wide_wave(aux);
	}
}

void Waveforms::deleteWaves() {
	delete trig;
	delete sawtrig;
	delete saw;
	delete square;
	delete widerect;
	delete narrowrect;
}
