//	Voice.h - Single voice.
//	--------------------------------------------------------------------------
//	Copyright (c) 2005 Niall Moody
//	
//	Permission is hereby granted, free of charge, to any person obtaining a
//	copy of this software and associated documentation files (the "Software"),
//	to deal in the Software without restriction, including without limitation
//	the rights to use, copy, modify, merge, publish, distribute, sublicense,
//	and/or sell copies of the Software, and to permit persons to whom the
//	Software is furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//	THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//	DEALINGS IN THE SOFTWARE.
//	--------------------------------------------------------------------------

#ifndef VOICE_H_
#define VOICE_H_

#include "twofloats.h"
#include "LFO.h"
#include "ADSR.h"

#include <cmath>

#define NUM_LOOPS 32
#define NUM_LOOPS_F 32.0f
#define NUM_LOOPS_INV (1.0f/32.0f)
#define NUM_VERTICES (NUM_LOOPS*NUM_LOOPS)
#define FILTER_MAX 7500.0f	//Maximum frequency the filter will go to.

//----------------------------------------------------------------------------
///	Simple data structure to hold vertex data.
typedef struct _vertex
{
	float x;
	float y;
	float z;

	void set(float _x, float _y, float _z)
	{
		x = _x;
		y = _y;
		z = _z;
	};

	_vertex operator=(_vertex op2)
	{
		x = op2.x;
		y = op2.y;
		z = op2.z;
		return *this;
	};

	_vertex operator+(_vertex op2)
	{
		_vertex temp;

		temp.x = x + op2.x;
		temp.y = y + op2.y;
		temp.z = z + op2.z;

		return temp;
	};
} Vertex;

///	Contains a single voice of the synth, and handles all the sound generation stuff.
/*!
	There will be NUM_VOICES of these, all handled by a single NoteMaster
	object.
 */
class Voice
{
  public:
	///	Cannot have any parameters, because it will be initialised in an array in NoteMaster's constructor.
	Voice();
	~Voice();

	///	Returns the current sample.
	twofloats getSample();

	///	Handles a Note On message.
	void onNoteOn(unsigned char note, float velocity);
	///	Handles a Note Off message.
	void onNoteOff();
	///	Handles a CC message (for MIDI controller support).
	void onCC(unsigned char cc, float value);
	///	Handles a pitch-bend message.
	void onPitchBend(float value);

	///	Sets the samplerate.
	void setSamplerate(float value);
	///	Sets a parameter.
	void setParameter(long index, float value);

	///	Used by NoteMaster to find out if the voice is still active, or has finished.
#ifdef WIN32
	__forceinline bool getIsActive() {return isActive;};
#else
	inline bool getIsActive() {return isActive;};
#endif

	///	Called from an ADSRAttack envelope section to tell us to move to the Decay section.
	/*!
		Need env as a parameter because we've got 2 envelopes, so we need to
		distinguish which one we're supposed to be incrementing.
	 */
	void incEnv_D(ADSRSection *env);
	///	Called from an ADSRDecay envelope section to tell us to move to the Sustain section.
	/*!
		\sa Voice::incEnv_D
	 */
	void incEnv_S(ADSRSection *env);
	///	Called from an ADSRRelease envelope section to tell us the voice should no longer be active.
	void setInactive() {isActive = false; env1Attack->setLevelVal(0.0f); env1 = env1Attack;};

	///	Called from NoteMaster, to update the gui with the latest vertices.
	Vertex *getVertices() {return vertices;};
	///	Called from NoteMaster, to update the voice's gui object according to the amp env's value.
	float getEnvVal() {return (env1Val*velocity);};
  private:
	///	Fast alternative to sinf(), using a lookup table.
#ifdef WIN32
	__forceinline float niallSin(float val);
#else
	inline float niallSin(float val);
#endif
	///	Fast alternative to cosf(), using a lookup table.
#ifdef WIN32
	__forceinline float niallCos(float val);
#else
	inline float niallCos(float val);
#endif

	///	Interpolation method (linear for now, but probably want something a bit smoother).
/*#ifdef WIN32
	__forceinline float interp(float index, float val1, float val2)
#else
	inline float interp(float index, float val1, float val2)
#endif
	{
		float tempf;
		tempf = index - static_cast<float>(static_cast<int>(index));
		return (tempf*val2)+((1.0f-tempf)*val1);
	};*/
	///	3rd order spline interpolation (nicer, but more cpu-intensive).
#ifdef WIN32
	__forceinline float ThirdInterp(const float x,
#else
	inline float ThirdInterp(const float x,
#endif
							 const float L1,
							 const float L0,
							 const float H0,
							 const float H1)
	{
		return
		L0 +
		.5f*
		x*(H0-L1 +
		   x*(H0 + L0*(-2) + L1 +
			  x*( (H0 - L0)*9 + (L1 - H1)*3 +
				 x*((L0 - H0)*15 + (H1 -  L1)*5 +
					x*((H0 - L0)*6 + (L1 - H1)*2 )))));
	}
	///	Used to calculate the filter's current f value (does this really need to be a separate function?).
#ifdef WIN32
	__forceinline void calcF(float val)
#else
	inline void calcF(float val)
#endif
	{
		f = 2.0f*sinf((PI*val)/samplerate);
	};

	///	The current pitch of the note, in Hz (including pitch bends).
	float frequency;
	///	The current pitch of the note, in Hz.
	float baseFrequency;
	///	The current samplerate.
	float samplerate;
	///	Whether or not the voice is currently active.
	bool isActive;
	///	If we've received a Note Off, we probably want to wait for our envelope's release before setting isActive to false.
	bool noteIsOff;
	///	The current velocity value.
	float velocity;

	//--------
	///	Number of samples to skip between each update of the modulation sources.
	int controlRateSamples;
	///	Used to determine when to update the modulation sources.
	int controlCount;

	//--------
	///	Current LFO pointer for LFO1.
	LFOBase *lfo1;
	///	Sine LFO pointer for LFO1.
	LFOSine *lfo1Sine;
	///	Sawtooth LFO pointer for LFO1.
	LFOSaw *lfo1Saw;
	///	Ramp LFO pointer for LFO1.
	LFORamp *lfo1Ramp;
	///	Square LFO pointer for LFO1.
	LFOSquare *lfo1Square;
	///	Sample & Hold LFO pointer for LFO1.
	LFOSH *lfo1SH;
	///	The current value of LFO1.
	float lfo1Val;

	///	Current LFO pointer for LFO2.
	LFOBase *lfo2;
	///	Sine LFO pointer for LFO2.
	LFOSine *lfo2Sine;
	///	Sawtooth LFO pointer for LFO2.
	LFOSaw *lfo2Saw;
	///	Ramp LFO pointer for LFO2.
	LFORamp *lfo2Ramp;
	///	Square LFO pointer for LFO2.
	LFOSquare *lfo2Square;
	///	Sample & Hold LFO pointer for LFO2.
	LFOSH *lfo2SH;
	///	The current value of LFO2.
	float lfo2Val;

	//--------
	///	Current ADSR pointer for envelope1.
	ADSRSection *env1;
	///	ADSR attack section pointer for envelope1.
	ADSRAttack *env1Attack;
	///	ADSR decay section pointer for envelope1.
	ADSRDecay *env1Decay;
	///	ADSR sustain section pointer for envelope1.
	ADSRSustain *env1Sustain;
	///	ADSR release section pointer for envelope1.
	ADSRRelease *env1Release;
	///	The current value of envelope1.
	float env1Val;

	///	Current ADSR pointer for envelope2.
	ADSRSection *env2;
	///	ADSR attack section pointer for envelope2.
	ADSRAttack *env2Attack;
	///	ADSR decay section pointer for envelope2.
	ADSRDecay *env2Decay;
	///	ADSR sustain section pointer for envelope2.
	ADSRSustain *env2Sustain;
	///	ADSR release section pointer for envelope2.
	ADSRRelease *env2Release;
	///	The current value of envelope1.
	float env2Val;

	//--------
	///	Whether the LFO should be reset when a MIDI Note On message is received.
	bool lfo1Reset;
	///	Whether the LFO should be reset when a MIDI Note On message is received.
	bool lfo2Reset;

	//--------
	///	Array of vertices representing the base sphere.
	Vertex sphereVertices[NUM_VERTICES];
	///	The array of vertices containing the current shape of this voice's object.
	Vertex vertices[NUM_VERTICES];
	///	Array of the u-values for the sphere, which are used in the spherical harmonics equation.
	float u[NUM_VERTICES];
	///	Array of the v-values for the sphere, which are used in the spherical harmonics equation.
	float v[NUM_VERTICES];
	///	An array of Vertex pointers, which are set to point to each waveform segment of the object's vertices array.
	/*!
		These pointers are used to speed up the calculation of the various
		waveforms that make up a single sound.  Each segment of the vertices
		array (i.e. NUM_LOOPS samples) constitutes a waveform.  This synth
		therefore uses a kind of additive synthesis, with each segment
		contributing a waveform to the output sound.  The speed the waveforms
		are read through at depends on the frequency of the input MIDI note.
		The centre segment (no. 16) represents the base frequency, while all
		the others are relative to it.
	 */
	Vertex *waveforms[NUM_LOOPS];
	///	Array of indices to the waveform tables.
	float indices[NUM_LOOPS];
	///	Array of increments for the waveform tables.
	float increments[NUM_LOOPS];

	//--------
	///	Array of the 8 parameters you use to control spherical harmonics.
	float m[8];
	///	Because the odd numbers are supposed to be integers (keep it 8 so it's less confusing, hopefully).
	int mi[8];
	///	Number of samples to skip between updates of the 3d object.
	int updateSamples;
	///	Counter to tell when to update the 3d object.
	int updateCount;
	///	Enum containing all the possible modulation sources.
	typedef enum
	{
		mNone = 0,
		mEnv1,
		mEnv2,
		mLFO1,
		mLFO2,
		mEnv1inv,
		mEnv2inv,
		mLFO1inv,
		mLFO2inv
	} modSources;
	///	Where m[0] is being modulated from.
	modSources m0Src;
	///	Where m[2] is being modulated from.
	modSources m2Src;
	///	Where m[4] is being modulated from.
	modSources m4Src;
	///	Where m[6] is being modulated from.
	modSources m6Src;
	///	Depth of modulation of m[0].
	float m0Depth;
	///	Depth of modulation of m[2].
	float m2Depth;
	///	Depth of modulation of m[4].
	float m4Depth;
	///	Depth of modulation of m[6].
	float m6Depth;

	//--------
	///	Enum to represent how the filter's configured.
	typedef enum
	{
		fOff,
		fLP,
		fBP,
		fHP
	} filterTypes;
	///	Current filter type.
	filterTypes filtType;
	///	Filter cutoff.
	float cutOff;	//in Hz;
	///	Filter's resonance.
	float q;		//0->1
	///	Variable used in the filter calculation.
	float f;
	///	Various filter type outputs (also variables used in the filter calculation.
	twofloats low, high, band, notch;
	///	Where the filter's cutoff is being modulated from.
	modSources filtMSrc;
	///	Depth of the filter's cutoff modulation.
	float filtMDepth;

	//--------
	///	Output level.
	float level;

	//--------
	///	Sine table (256 because then we can use an unsigned char to index it, and it'll just wrap around).
	float sine[256];
};

#endif
