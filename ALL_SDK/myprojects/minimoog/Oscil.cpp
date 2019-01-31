/*
 * Oscil.cpp
 *
 */

#include "basic_blocks.h"
#include "globals.h"
#include <cmath>

Oscil::Oscil() :
	BasicBlock(2), table(Waveforms::getWaveform(TRIG)), increment(0.0),
			previous_phase(0.0), slave(NULL), last_freq(0), total_samples(0),
			samples_til_reset(0), syncON(false) {

}

Oscil::Oscil(wavetype_t wave) :
	BasicBlock(2), table(Waveforms::getWaveform(wave)), increment(0.0),
			previous_phase(0.0) {

}

void Oscil::setWavetable(wavetype_t wave) {
	this->wave = wave;
	this->table = Waveforms::getWaveform(wave);
}

wavetype_t Oscil::getWavetype() {
	return this->wave;
}

void Oscil::setFrequencyInput(BasicBlock * block) {
	(this->inputs)[0] = block;
}

void Oscil::setAmplitudeInput(BasicBlock * block) {
	(this->inputs)[1] = block;
}

float Oscil::getNextValue() {
	if (!this->ON)
		return 0.0;
	float phase;
	float aux;
	float freq;
	float amp;
	int table_length;

	freq = inputs[0]->getNextValue();

	if (syncON && (slave != NULL)) {
		if (fabs(last_freq-freq) >= 0.01f) {
			last_freq = freq;
			samples_til_reset = sample_rate /  freq;
			total_samples = 1;
		} else {
			if (total_samples == samples_til_reset) {
				total_samples = 0;
				(*slave).resetBlock();
			}
			total_samples++;
		}
	}

	amp = inputs[1]->getNextValue();
	/*Assume que a tabela tem dois pontos extras no final*/
	table_length = (*table).size() - 2;
	increment = (float) table_length * freq / sample_rate;
	phase = previous_phase + increment;
	/*Pega a parte fracionária da fase*/
	aux = phase - (int) phase;

	phase = (int) phase % table_length;
	/*
	 * Soma a parte fracionária à fase. Isso só é útil no
	 * caso de utilizar interpolação
	 */
	phase += aux;
	previous_phase = phase;

	return linear_interpolation(phase) * amp;
}

float Oscil::cubic_interpolation(float phase) const {
	float x0, x1, x2, x3;
	float y, y0, y1, y2, y3;
	float frac;

	x1 = (int) phase;
	x0 = x1 - 1;
	x2 = x1 + 1;
	x3 = x1 + 2;
	frac = phase - x1;

	y1 = (*table)[x1];
	y2 = (*table)[x2];
	y3 = (*table)[x3];

	if (x1 <= 0) {
		int table_length = (*table).size() - 2;
		y0 = (*table)[table_length - 1];
	} else {
		y0 = (*table)[x1 - 1];
	}

	/*Apenas divide a fórmula em duas partes para facilitar a leitura*/
	y = -frac * (frac - 1) * (frac - 2) * y0 / 6.0f + (frac + 1) * (frac - 1)
			* (frac - 2) * y1 / 2.0f;
	y += -(frac + 1) * (frac) * (frac - 2) * y2 / 2.0f + (frac + 1) * (frac)
			* (frac - 1) * y3 / 6.0f;

	return y;
}

float Oscil::linear_interpolation(float phase) const {
	int x1, x2;
	float y1, y2, y;

	x1 = (int) phase;
	x2 = (int) phase + 1;

	y1 = (*table)[x1];
	y2 = (*table)[x2];

	y = y1 + (phase - x1) * (y2 - y1);

	return y;
}

void Oscil::resetBlock(void) {
	increment = 0.0;
	previous_phase = 0.0;
	last_freq = 0;
	total_samples = 0;
}

void Oscil::setSlave(Oscil * block) {
	slave = block;
}

void Oscil::setSyncON(bool ON) {
	syncON = ON;
}

bool Oscil::getSyncON() {
	return syncON;
}
