#include "MoogPlugin.h"

/*
 * 
 *
 */
static wavetype_t float2wavetype(float value) {
	if (value <= 0.16) {
		return TRIG;
	}
	else if (value <= 0.32) {
		return SAWTRIG;
	}
	else if (value <= 0.48) {
		return SAW;
	}
	else if (value <= 0.64) {
		return SQUARE;
	}
	else if (value <= 0.80) {
		return WIDERECT;
	}
	return NARROWRECT;
}

static float wavetype2float(wavetype_t type) {
	switch (type) {
	case TRIG:
		return 0.0f;
	case SAWTRIG:
		return 0.32f;
	case SAW:
		return 0.48f;
	case SQUARE:
		return 0.64f;
	case WIDERECT:
		return 0.80f;
	case NARROWRECT:
		return 1.00f;
	}
}

static int float2range(float value) {
	if (value <= 0.20) {
		return 2;
	}
	else if (value <= 0.40) {
		return 4;
	}
	else if (value <= 0.60) {
		return 8;
	}
	else if (value <= 0.80) {
		return 16;
	}
	else{
		return 32;
	}
	return NARROWRECT;
}

static float range2float(int range) {
	switch (range){
	case 2:
		return 0.0f;
	case 4:
		return 0.40f;
	case 8:
		return 0.60f;
	case 16:
		return 0.80f;
	case 32:
		return 1.00f;
	}
	return 0.60f;
}

static char *wavetypeStrings[] = {"Trig","SawTrig","Saw","Square","WRect","Narrow"};


/*AudioEffect* createEffectInstance(audioMasterCallback audioMaster) {
	return new MoogPlugin(audioMaster);
}*/

MoogPlugin::MoogPlugin(audioMasterCallback audioMaster) : AudioEffectX(audioMaster, 0, NUM_PARAMS) {
	if (audioMaster) {
		setNumInputs (0);
		setNumOutputs (1);	
		canProcessReplacing ();
		isSynth ();
		setUniqueID(667);
	}
	suspend();
	double k = 1.059463094359;	// 12th root of 2
	double a = 6.875;	// a
	a *= k;	// b
	a *= k;	// bb
	a *= k;	// c, frequency of midi note 0
	for (int i = 0; i < 128; i++) {
		freqTable[i] = a;
		a *= k;
	}
	Waveforms::initializeWaves(1024,2);
	noteIsOn = false;
	currentDelta = currentNote = currentVelocity = 0;
	moog = new Moog();
	moog->setOscil1Amp(1.0f/3.0f);
	moog->setOscil2Amp(1.0f/3.0f);
	moog->setNoiseAmp(1.0f/3.0f);
	moog->setInputFreq(0);
	

	moog->setOscil2ON(false);
	moog->setNoiseON(false);
	moog->setFilterON(false);
	moog->setMasterAmp(1.0f);
	moog->setOscil2ON(false);
}

MoogPlugin::~MoogPlugin() {
	delete moog;
}

void MoogPlugin::processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames) {
	float* out1 = outputs[0];

	if (noteIsOn) {
		double baseFreq = freqTable[currentNote & 0x7f];
		
		if (currentDelta > 0)
		{
			if (currentDelta >= sampleFrames)	// future
			{
				currentDelta -= sampleFrames;
				return;
			}
			memset (out1, 0, currentDelta * sizeof (float));
			out1 += currentDelta;
			sampleFrames -= currentDelta;
			currentDelta = 0;
		}
		
	}						
	for (VstInt32 i = 0; i < sampleFrames; i++) {
		out1[i] = moog->getNextValue();
	}
}

VstInt32 MoogPlugin::processEvents (VstEvents* ev)
{
	for (VstInt32 i = 0; i < ev->numEvents; i++)
	{
		if ((ev->events[i])->type != kVstMidiType)
			continue;

		VstMidiEvent* event = (VstMidiEvent*)ev->events[i];
		char* midiData = event->midiData;
		VstInt32 status = midiData[0] & 0xf0;	// ignoring channel
		if (status == 0x90 || status == 0x80)	// we only look at notes
		{
			VstInt32 note = midiData[1] & 0x7f;
			VstInt32 velocity = midiData[2] & 0x7f;
			if (status == 0x80)
				velocity = 0;	// note off by velocity 0
			if (!velocity && (note == currentNote))
				noteOff ();
			else
				noteOn (note, velocity, event->deltaFrames);
		}
		else if (status == 0xb0)
		{
			if (midiData[1] == 0x7e || midiData[1] == 0x7b)	// all notes off
				noteOff ();
		}
		event++;
	}
	return 1;
}

void MoogPlugin::noteOn (VstInt32 note, VstInt32 velocity, VstInt32 delta) {
	currentNote = note;
	currentVelocity = velocity;
	currentDelta = delta;
	noteIsOn = true;
	moog->setInputFreq(freqTable[note]);
}

void MoogPlugin::noteOff () {
	noteIsOn = false;
	moog->release();
}

/*
 * Par�metros
 */

void MoogPlugin::setParameter(VstInt32 index, float value){
	switch (index) {
	case oscil1Waveform:
		moog->setOscil1Waveform(float2wavetype(value));
		break;
	case oscil1Amp:
		moog->setOscil1Amp(value/3.0f);
		break;
	case oscil1Range:
		moog->setOscil1Range(float2range(value));
		break;
	case oscil1Freq:
		moog->setOscil1Frequency(value*24 - 12);
		break;
	case oscil1ON:
		moog->setOscil1ON(value >= 0.5);
		break;
	case oscil2Waveform:
		moog->setOscil2Waveform(float2wavetype(value));
		break;
	case oscil2Amp:
		moog->setOscil2Amp(value/3.0f);
		break;
	case oscil2Range:
		moog->setOscil2Range(float2range(value));
		break;
	case oscil2Freq:
		moog->setOscil2Frequency(value*24 - 12);
		break;
	case oscil2ON:
		moog->setOscil2ON(value >= 0.5);
		break;
	case oscilSync:
		moog->setSyncON(value >= 0.5);
		break;
	case noiseType:
		moog->setNoiseType(value <= 0.5 ? WHITE : PINK);
		break;
	case noiseAmp:
		moog->setNoiseAmp(value/3.0f);
		break;
	case noiseON:
		moog->setNoiseON(value >= 0.5);
		break;
	case filterCutoff:
		moog->setFilterCutoffFreq(2000*value);
		break;
	case filterContourAmount:
		moog->setContourAmount(value);
		break;
	case filterQuality:
		moog->setFilterQuality(value*3.5);
		break;
	case filterON:
		moog->setFilterON(value >= 0.5);
		break;
	case filterAttack:
		moog->setFilterAttack(value);
		break;
	case filterDecay:
		moog->setFilterDecay(value);
		break;
	case filterSustain:
		moog->setFilterSustainLevel(value);
		break;
	case envAttack:
		moog->setAttack(value);
		break;
	case envDecay:
		moog->setDecay(value);
		break;
	case envSustain:
		moog->setSustainLevel(value);
		break;
	case masterAmp:
		moog->setMasterAmp(value);
		break;
	}
}

float MoogPlugin::getParameter(VstInt32 index) {
	switch (index) {
	case oscil1Waveform:
		return wavetype2float(moog->getOscil1Waveform());
	case oscil1Amp:
		return 3.0f*moog->getOscil1Amp();
	case oscil1Range:
		return range2float(moog->getOscil1Range());
	case oscil1Freq:
		return ((moog->getOscil1Frequency() + 12)/24);
		break;
	case oscil1ON:
		return moog->getOscil1ON() ? 1.0f : 0.0f;
		break;
	case oscil2Waveform:
		return wavetype2float(moog->getOscil2Waveform());
	case oscil2Amp:
		return 3.0f*moog->getOscil2Amp();
	case oscil2Range:
		return range2float(moog->getOscil2Range());
	case oscil2Freq:
		return ((moog->getOscil2Frequency() + 12)/24);
		break;
	case oscil2ON:
		return moog->getOscil2ON() ? 1.0f : 0.0f;
		break;
	case oscilSync:
		return moog->getSyncON() ? 1.0f : 0.0f;
		break;
	case noiseType:
		return moog->getNoiseType() == WHITE ? 0.0f : 1.0f;
		break;
	case noiseAmp:
		return moog->getNoiseAmp()*3.0f;
		break;
	case noiseON:
		return moog->getNoiseON() ? 1.0f : 0.0f;
		break;
	case filterCutoff:
		return moog->getfilterCutoffFreq()/2000.0f;
		break;
	case filterContourAmount:
		return moog->getContourAmount();
		break;
	case filterQuality:
		return moog->getFilterQuality()/3.5f;
		break;
	case filterON:
		return moog->getFilterON() ? 1.0f : 0.0f;
		break;
	case filterAttack:
		return moog->getFilterAttack();
		break;
	case filterDecay:
		return moog->getFilterDecay();
		break;
	case filterSustain:
		return moog->getFilterSustainLevel();
		break;
	case envAttack:
		return moog->getAttack();
		break;
	case envDecay:
		return moog->getDecay();
		break;
	case envSustain:
		return moog->getSustainLevel();
		break;
	case masterAmp:
		return moog->getMasterAmp();
		break;
	}
}

void MoogPlugin::getParameterLabel (VstInt32 index, char* label){
	switch (index) {
	case oscil1Waveform:
	case oscil2Waveform:
		vst_strncpy (label, "Shape", kVstMaxParamStrLen);
		break;
	case oscil1Amp:
	case oscil2Amp:
	case noiseAmp:
	case masterAmp:
		vst_strncpy (label, "Amp", kVstMaxParamStrLen);
		break;
	case oscil1Range:
	case oscil2Range:
	case oscil1Freq:
	case oscil2Freq:
	case oscil1ON:
	case oscil2ON:
	case oscilSync:
	case noiseType:
	case noiseON:
		break;
	case filterCutoff:
		vst_strncpy (label, "Hz", kVstMaxParamStrLen);
		break;
	case filterContourAmount:
	case filterQuality:
	case filterON:
	case filterSustain:
	case envSustain:
		break;
	case filterAttack:
	case filterDecay:
	case envAttack:
	case envDecay:
		vst_strncpy (label, "s", kVstMaxParamStrLen);
		break;
	}
}

void MoogPlugin::getParameterDisplay (VstInt32 index, char* text){
	switch (index) {
	/* 
	 * Oscil 1
	 */
	case oscil1Waveform:
		vst_strncpy (text, wavetypeStrings[moog->getOscil1Waveform()], kVstMaxParamStrLen);
		break;
	case oscil1Amp:
		float2string(moog->getOscil1Amp()*3.0f,text,kVstMaxParamStrLen);
		break;
	case oscil1Range:
		int2string(moog->getOscil1Range(),text, kVstMaxParamStrLen);
		break;
	case oscil1Freq:
		float2string(moog->getOscil1Frequency(),text,kVstMaxParamStrLen);
		break;
	case oscil1ON:
		vst_strncpy(text, moog->getOscil1ON() ? "On" : "Off", kVstMaxParamStrLen);
		break;
	/* 
	 * Oscil 2
	 */
	case oscil2Waveform:
		vst_strncpy (text, wavetypeStrings[moog->getOscil2Waveform()], kVstMaxParamStrLen);
		break;
	case oscil2Amp:
		float2string(moog->getOscil2Amp()*3.0f,text,kVstMaxParamStrLen);
		break;
	case oscil2Range:
		int2string(moog->getOscil2Range(),text, kVstMaxParamStrLen);
		break;
	case oscil2Freq:
		float2string(moog->getOscil2Frequency(),text,kVstMaxParamStrLen);
		break;
	case oscil2ON:
		vst_strncpy(text, moog->getOscil2ON() ? "On" : "Off", kVstMaxParamStrLen);
		break;
	case oscilSync:
		vst_strncpy(text, moog->getSyncON() ? "On" : "Off", kVstMaxParamStrLen);
		break;
	/* 
     * Noise
	 */
	case noiseType:
		vst_strncpy(text, moog->getNoiseType() == WHITE ? "White" : "Pink", kVstMaxParamStrLen);
		break;
	case noiseAmp:
		float2string(moog->getNoiseAmp()*3.0f,text,kVstMaxParamStrLen);
		break;
	case noiseON:
		vst_strncpy(text, moog->getNoiseON() ? "On" : "Off", kVstMaxParamStrLen);
		break;
	/*
	 * Filter
	 */
	case filterCutoff:
		float2string(moog->getfilterCutoffFreq(),text,kVstMaxParamStrLen);
		break;
	case filterContourAmount:
		float2string(moog->getContourAmount(),text,kVstMaxParamStrLen);
		break;
	case filterQuality:
		float2string(moog->getFilterQuality()/3.5f,text,kVstMaxParamStrLen);		
		break;
	case filterON:
		vst_strncpy(text, moog->getFilterON() ? "On" : "Off", kVstMaxParamStrLen);		
		break;
	case filterAttack:
		float2string(moog->getFilterAttack(), text, kVstMaxParamStrLen);
		break;
	case filterDecay:
		float2string(moog->getFilterDecay(), text, kVstMaxParamStrLen);
		break;
	case filterSustain:
		float2string(moog->getFilterSustainLevel(), text, kVstMaxParamStrLen);
		break;
	/*
	 * Env
	 */
	case envAttack:
		float2string(moog->getAttack(), text, kVstMaxParamStrLen);
		break;
	case envDecay:
		float2string(moog->getDecay(), text, kVstMaxParamStrLen);
		break;
	case envSustain:
		float2string(moog->getSustainLevel(), text, kVstMaxParamStrLen);
		break;

	case masterAmp:
		float2string(moog->getMasterAmp(), text, kVstMaxParamStrLen);
		break;
	}
}

void MoogPlugin::getParameterName (VstInt32 index, char* text){
	switch (index) {
	case oscil1Waveform:
		vst_strncpy (text, "O1Wave", kVstMaxParamStrLen);	
		break;
	case oscil1Amp:
		vst_strncpy (text, "O1Amp", kVstMaxParamStrLen);	
		break;
	case oscil1Range:
		vst_strncpy (text, "O1Range", kVstMaxParamStrLen);	
		break;
	case oscil1Freq:
		vst_strncpy (text, "O1Freq", kVstMaxParamStrLen);
		break;
	case oscil1ON:
		vst_strncpy (text, "O1Power", kVstMaxParamStrLen);
		break;
	case oscil2Waveform:
		vst_strncpy (text, "O2Wave", kVstMaxParamStrLen);	
		break;
	case oscil2Amp:
		vst_strncpy (text, "O2Amp", kVstMaxParamStrLen);	
		break;
	case oscil2Range:
		vst_strncpy (text, "O2Range", kVstMaxParamStrLen);	
		break;
	case oscil2Freq:
		vst_strncpy (text, "O2Freq", kVstMaxParamStrLen);
		break;
	case oscil2ON:
		vst_strncpy (text, "O2Power", kVstMaxParamStrLen);
		break;
	case oscilSync:
		vst_strncpy (text, "Osc Sync", kVstMaxParamStrLen);
		break;
	case noiseType:
		vst_strncpy (text, "NType", kVstMaxParamStrLen);
		break;
	case noiseAmp:
		vst_strncpy (text, "NAmp", kVstMaxParamStrLen);	
		break;
	case noiseON:
		vst_strncpy (text, "NPower", kVstMaxParamStrLen);
		break;
	case filterCutoff:
		vst_strncpy (text, "FCutoff", kVstMaxParamStrLen);
		break;
	case filterContourAmount:
		vst_strncpy (text, "FCAmount", kVstMaxParamStrLen);
		break;
	case filterQuality:
		vst_strncpy (text, "FQuality", kVstMaxParamStrLen);
		break;
	case filterON:
		vst_strncpy (text, "FPower", kVstMaxParamStrLen);
		break;
	case filterAttack:
		vst_strncpy (text, "FAttack", kVstMaxParamStrLen);
		break;
	case filterDecay:
		vst_strncpy (text, "FDecay", kVstMaxParamStrLen);
		break;
	case filterSustain:
		vst_strncpy (text, "FSLevel", kVstMaxParamStrLen);
		break;
	case envAttack:
		vst_strncpy (text, "EAttack", kVstMaxParamStrLen);
		break;
	case envDecay:
		vst_strncpy (text, "EDecay", kVstMaxParamStrLen);
		break;
	case envSustain:
		vst_strncpy (text, "ESLevel", kVstMaxParamStrLen);
		break;
	case masterAmp:
		vst_strncpy (text, "MAmp", kVstMaxParamStrLen);
		break;
	}
}

