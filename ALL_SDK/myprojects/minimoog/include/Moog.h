/*
 * Moog.h
 *
 */
#pragma pack(1)
#ifndef MOOG_H
#define MOOG_H

#include "basic_blocks.h"

class Moog {
public:
	Moog();
	/*
	 *
	 * Controles gerais
	 *
	 */
	void setInputFreq(float freq);
	void release();
	float getInputFreq();
	void setMasterAmp(float amp);
	float getMasterAmp();
	float getNextValue();

	/*
	 * Controles do Oscil1
	 */
	void setOscil1Amp(float amp);
	float getOscil1Amp();
	void setOscil1Waveform(wavetype_t type);
	wavetype_t getOscil1Waveform();
	void setOscil1Range(int range);
	int getOscil1Range();
	void setOscil1Frequency(float freq);
	float getOscil1Frequency();
	void setOscil1ON(bool ON);
	bool getOscil1ON();

	void setSyncON(bool ON);
	bool getSyncON();

	/*
	 * Controles do Oscil2
	 */
	void setOscil2Amp(float amp);
	float getOscil2Amp();
	void setOscil2Waveform(wavetype_t type);
	wavetype_t getOscil2Waveform();
	void setOscil2Range(int range);
	int getOscil2Range();
	void setOscil2Frequency(float freq);
	float getOscil2Frequency();
	void setOscil2ON(bool ON);
	bool getOscil2ON();

	/*
	 * Controles do Ruído
	 */
	void setNoiseAmp(float amp);
	float getNoiseAmp();
	void setNoiseType(noisetype_t type);
	noisetype_t getNoiseType();
	void setNoiseON(bool ON);
	bool getNoiseON();

	/*
	 * Controles do filtro
	 */
	void setFilterCutoffFreq(float freq);
	float getfilterCutoffFreq();
	void setContourAmount(float amount);
	float getContourAmount();
	void setFilterQuality(float q);
	float getFilterQuality();
	void setFilterAttack(float attack);
	float getFilterAttack();
	void setFilterDecay(float decay);
	float getFilterDecay();
	void setFilterSustain(float sustain);
	float getFilterSustain();
	void setFilterSustainLevel(float amp);
	float getFilterSustainLevel();
	void setFilterON(bool ON);
	bool getFilterON();

	/*
	 * Controles da envoltória
	 */
	void setAttack(float attack);
	float getAttack();
	void setDecay(float decay);
	float getDecay();
	void setSustain(float sustain);
	float getSustain();
	void setSustainLevel(float amp);
	float getSustainLevel();
	void setReleaseON(bool ON);
	bool getReleaseON();
	void setEnvON(bool ON);
	bool getEnvON();

private:
	/*
	 * Os blocos que compõem o sintetizador
	 */
	Oscil oscil1;
	Number oscil1_freq;
	Number oscil1_amp;
	Oscil oscil2;
	Number oscil2_freq;
	Number oscil2_amp;

	Noise noise;
	Number noise_amp;

	Adder adder;

	Filter filter;
	Number contour_amount;
	Number cutoff_freq;
	Number filter_quality;
	Adder adder_aux;
	Multiplier mult_aux;
	ADSR filter_env;
	ADSR env;

	Multiplier mult_aux2;
	Number master_amp;

	/*
	 * Knobs
	 *
	 */
	float input_freq;
	int oscil1_range;
	float oscil1FreqOffset;
	int oscil2_range;
	float oscil2FreqOffset;

	/*
	 * Aux
	 */
	void updateOscil1();
	void updateOscil2();

	char name[21];
};

#endif /* MOOG_H_ */
