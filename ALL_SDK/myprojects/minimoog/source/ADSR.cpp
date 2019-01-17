/*
 * ADSR.cpp
 *
 *  
 */
#pragma pack(1)

#include "../include/basic_blocks.h"
#include "../include/globals.h"
#include <cmath>


ADSR::ADSR() :
	BasicBlock(0), current_phase(ATTACK), current_phase_samples(0),
			attack(0.2f), decay(0.1f), sustain(0.2f), release(1.2f),
			attack_sharpness(1000.0f), decay_sharpness(0.5f), sustain_amp(0.9f),
			release_sharpness(0.5f), releaseON(false) {

}

ADSR::ADSR(float attack, float decay, float sustain, float release) :
	BasicBlock(0), current_phase(ATTACK), current_phase_samples(0), attack(
			attack), decay(decay), sustain(sustain), release(release),
			attack_sharpness(1000.0f), decay_sharpness(0.5f), sustain_amp(0.9f),
			release_sharpness(0.5f), releaseON(false) {

}

void ADSR::setPhase(phase_t phase) {
	current_phase = phase;
	current_phase_samples = 0;
}

void ADSR::resetBlock() {
	current_phase = ATTACK;
	current_phase_samples = 0;
}

float ADSR::attack_function() {
	return exp(-attack_sharpness/current_phase_samples + (attack_sharpness/(attack*sample_rate)));
}

float ADSR::decay_function() {
	return exp(log(sustain_amp)*current_phase_samples/(decay*sample_rate));
}

float ADSR::release_function() {
	return sustain_amp*exp(current_phase_samples*(log(0.01f) - log(sustain_amp))/(release*sample_rate));
}
float ADSR::getNextValue() {
	if (!this->ON){
		return 1.0;
	}

	float out;
	current_phase_samples++;
	switch (current_phase) {
	case ATTACK:
		out = attack_function();
		if (current_phase_samples > sample_rate * attack) {
			current_phase = DECAY;
			current_phase_samples = 0;
		}
		break;
	case DECAY:
		out = decay_function();
		if (current_phase_samples > sample_rate * decay) {
			current_phase = SUSTAIN;
			current_phase_samples = 0;
		}
		break;
	case SUSTAIN:
		out = sustain_amp;
		if (releaseON && (current_phase_samples > sample_rate * sustain)){
			current_phase = RELEASE;
			current_phase_samples = 0;
		}
		break;
	case RELEASE:
		out = release_function();
		break;
	}

	return out;
}

float ADSR::getAttack_sharpness() const {
	return attack_sharpness;
}

float ADSR::getDecay_sharpness() const {
	return decay_sharpness;
}

float ADSR::getRelease_sharpness() const {
	return release_sharpness;
}

float ADSR::getSustain_amp() const {
	return sustain_amp;
}

bool ADSR::getReleaseON() const {
	return releaseON;
}

float ADSR::getAttack() const {
	return this->attack;
}

float ADSR::getDecay() const {
	return this->decay;
}

float ADSR::getSustain () const {
	return this->sustain;
}


float ADSR::getRelease() const {
	return this->release;
}

void ADSR::setAttack(float attack) {
	this->attack = attack;
}

void ADSR::setDecay(float decay) {
	this->decay = decay;
}

void ADSR::setSustain(float sustain) {
	this->sustain = sustain;
}

void ADSR::setRelease(float release) {
	this->release = release;
}

void ADSR::setAttack_sharpness(float attack_sharpness) {
	this->attack_sharpness = attack_sharpness;
}

void ADSR::setDecay_sharpness(float decay_sharpness) {
	this->decay_sharpness = decay_sharpness;
}

void ADSR::setRelease_sharpness(float release_sharpness) {
	this->release_sharpness = release_sharpness;
}

void ADSR::setSustain_amp(float sustain_amp) {
	this->sustain_amp = sustain_amp;
}

void ADSR::setReleaseON(bool on) {
	releaseON = on;
}
