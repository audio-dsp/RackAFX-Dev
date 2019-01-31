/*
 * Moog.cpp
 *
 */
#pragma pack(1)
#include "Moog.h"
#include <cmath>

Moog::Moog() :
	oscil1_freq(0), oscil1_amp(1.0), oscil2_freq(0), oscil2_amp(1.0),
			noise_amp(1.0), adder(3), contour_amount(0.0),cutoff_freq(400),filter_quality(1.0), adder_aux(2),mult_aux(3), mult_aux2(3), master_amp(1.0), input_freq(
					440), oscil1_range(8), oscil1FreqOffset(0),
			oscil2_range(8), oscil2FreqOffset(0) {

	/*
	 * Osciladores e ruído
	 */
	oscil1.setFrequencyInput(&oscil1_freq);
	oscil1.setAmplitudeInput(&oscil1_amp);
	oscil1.setSlave(&oscil2);

	oscil2.setFrequencyInput(&oscil2_freq);
	oscil2.setAmplitudeInput(&oscil2_amp);

	noise.setAmplitudeInput(&noise_amp);

	/*
	 * Somador
	 */
	adder.setInput(0, &oscil1);
	adder.setInput(1, &oscil2);
	adder.setInput(2, &noise);



	/*
	 * Configuração do filtro
	 */
	mult_aux.setInput(0,&contour_amount);
	mult_aux.setInput(1,&filter_env);
	mult_aux.setInput(2,&cutoff_freq);

	adder_aux.setInput(0,&mult_aux);
	adder_aux.setInput(1,&cutoff_freq);

	filter.setQualityInput(&filter_quality);
	filter.setInputSignal(&adder);
	filter.setFrequencyInput(&adder_aux);

	/*
	 * Configuração do trecho final
	 */

	mult_aux2.setInput(0, &filter);
	mult_aux2.setInput(1, &env);
	mult_aux2.setInput(2, &master_amp);

	updateOscil1();
	updateOscil2();
}

void Moog::setInputFreq(float freq) {
	this->input_freq = freq;
	oscil1.resetBlock();
	oscil2.resetBlock();
	updateOscil1();
	updateOscil2();
	env.resetBlock();
	filter_env.resetBlock();
}

void Moog::setMasterAmp(float amp) {
	master_amp.setNumber(amp);
}

float Moog::getMasterAmp() {
	return master_amp.getNumber();
}

float Moog::getNextValue() {
	return mult_aux2.getNextValue();
}

void Moog::release() {
	env.setPhase(RELEASE);
}
/*
 * Oscil1
 */

void Moog::setOscil1Amp(float amp) {
	oscil1_amp.setNumber(amp);
}

float Moog::getOscil1Amp() {
	return oscil1_amp.getNumber();
}

void Moog::setOscil1Waveform(wavetype_t type) {
	oscil1.setWavetable(type);
}

wavetype_t Moog::getOscil1Waveform() {
	return oscil1.getWavetype();
}

void Moog::setOscil1Range(int range) {
	oscil1_range = range;
	updateOscil1();
}

int Moog::getOscil1Range() {
	return oscil1_range;
}

void Moog::setOscil1Frequency(float freq) {
	oscil1FreqOffset = freq;
	updateOscil1();
}

float Moog::getOscil1Frequency() {
	return oscil1FreqOffset;
}

void Moog::setOscil1ON(bool ON) {
	oscil1.setON(ON);
}

bool Moog::getOscil1ON() {
	return oscil1.getON();
}

void Moog::setSyncON(bool ON){
	oscil1.setSyncON(ON);
}

bool Moog::getSyncON() {
	return oscil1.getSyncON();
}

/*
 * Oscil2
 */

void Moog::setOscil2Amp(float amp) {
	oscil2_amp.setNumber(amp);
}

float Moog::getOscil2Amp() {
	return oscil2_amp.getNumber();
}

void Moog::setOscil2Waveform(wavetype_t type) {
	oscil2.setWavetable(type);
}

wavetype_t Moog::getOscil2Waveform() {
	return oscil2.getWavetype();
}

void Moog::setOscil2Range(int range) {
	oscil2_range = range;
	updateOscil2();
}

int Moog::getOscil2Range() {
	return oscil2_range;
}

void Moog::setOscil2Frequency(float freq) {
	oscil2FreqOffset = freq;
	updateOscil2();
}

float Moog::getOscil2Frequency() {
	return oscil2FreqOffset;
}

void Moog::setOscil2ON(bool ON) {
	oscil2.setON(ON);
}

bool Moog::getOscil2ON() {
	return oscil2.getON();
}

/*
 * Noise
 */

void Moog::setNoiseON(bool ON){
	noise.setON(ON);
}

bool Moog::getNoiseON() {
	return noise.getON();
}

void Moog::setNoiseAmp(float amp){
	noise_amp.setNumber(amp);
}

float Moog::getNoiseAmp() {
	return noise_amp.getNumber();
}

void Moog::setNoiseType(noisetype_t type) {
	noise.setType(type);
}

noisetype_t Moog::getNoiseType() {
	return noise.getType();
}

/*
 * Filtro
 */

void Moog::setFilterCutoffFreq(float freq){
	cutoff_freq.setNumber(freq);
}

float Moog::getfilterCutoffFreq() {
	return cutoff_freq.getNumber();
}

void Moog::setContourAmount(float amount) {
	contour_amount.setNumber(amount);
}

float Moog::getContourAmount() {
	return contour_amount.getNumber();
}

void Moog::setFilterQuality(float q) {
	return filter_quality.setNumber(q);
}

float Moog::getFilterQuality() {
	return filter_quality.getNumber();
}

void Moog::setFilterAttack(float attack){
	filter_env.setAttack(attack);
}

float Moog::getFilterAttack() {
	return filter_env.getAttack();
}

void Moog::setFilterDecay(float decay){
	filter_env.setDecay(decay);
	filter_env.setRelease(decay);
}

float Moog::getFilterDecay() {
	return filter_env.getDecay();
}

void Moog::setFilterSustain(float sustain){
	filter_env.setSustain(sustain);
}

float Moog::getFilterSustain() {
	return filter_env.getSustain();
}

void Moog::setFilterSustainLevel(float amp) {
	filter_env.setSustain_amp(amp);
}

float Moog::getFilterSustainLevel() {
	return filter_env.getSustain_amp();
}

void Moog::setFilterON(bool ON){
	filter.setON(ON);
}

bool Moog::getFilterON() {
	return filter.getON();
}

/*
 * Envoltória final
 */
void Moog::setAttack(float attack){
	env.setAttack(attack);
}

float Moog::getAttack() {
	return env.getAttack();
}

void Moog::setDecay(float decay){
	env.setDecay(decay);
	env.setRelease(decay);
}

float Moog::getDecay() {
	return env.getDecay();
}

void Moog::setSustain(float sustain){
	env.setSustain(sustain);
}

float Moog::getSustain() {
	return env.getSustain();
}

void Moog::setReleaseON(bool ON){
	env.setReleaseON(ON);
}

bool Moog::getReleaseON() {
	return env.getReleaseON();
}

void Moog::setEnvON(bool ON){
	env.setON(ON);
}

bool Moog::getEnvON() {
	return env.getON();
}

void Moog::setSustainLevel(float value) {
	env.setSustain_amp(value);
}

float Moog::getSustainLevel() {
	return env.getSustain_amp();
}

/*
 * Aux
 */
void Moog::updateOscil1() {
	oscil1_freq.setNumber(input_freq * ((float) oscil1_range / 8.0f) * pow(2.0f,
			oscil1FreqOffset / 12));
}

void Moog::updateOscil2() {
	oscil2_freq.setNumber(input_freq * ((float) oscil2_range / 8.0f) * pow(2.0f,
			oscil2FreqOffset / 12));
}
