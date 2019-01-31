#include "audioeffectx.h"
#include "Moog.h"

#define NUM_PARAMS 25
enum {
	oscil1Waveform = 0,
	oscil1Amp,
	oscil1Range,
	oscil1Freq,
	oscil1ON,
	oscil2Waveform,
	oscil2Amp,
	oscil2Range,
	oscil2Freq,
	oscil2ON,
	oscilSync,
	noiseType,
	noiseAmp,
	noiseON,
	filterCutoff,
	filterContourAmount,
	filterQuality,
	filterON,
	filterAttack,
	filterDecay,
	filterSustain,
	envAttack,
	envDecay,
	envSustain,
	masterAmp,
};

class MoogPlugin : public AudioEffectX {
public:
	MoogPlugin(audioMasterCallback audioMaster);
	~MoogPlugin();
	void processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames);
	double freqTable[128];
	VstInt32 processEvents (VstEvents* ev);
	
	/*
	 * Parâmetros
	 *
	 */
	virtual void setParameter (VstInt32 index, float value);
	virtual float getParameter (VstInt32 index);
	virtual void getParameterLabel (VstInt32 index, char* label);
	virtual void getParameterDisplay (VstInt32 index, char* text);
	virtual void getParameterName (VstInt32 index, char* text);
private:
	VstInt32 currentNote;
	VstInt32 currentVelocity;
	VstInt32 currentDelta;
	bool noteIsOn;
	void noteOn (VstInt32 note, VstInt32 velocity, VstInt32 delta);
	void noteOff ();
	Moog * moog;


};