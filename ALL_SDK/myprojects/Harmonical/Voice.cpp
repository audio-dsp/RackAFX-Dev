//	Voice.cpp - Single voice.
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

#include "Voice.h"
#include "MIDI2Freq.h"
#include "Harmonical.h"

//Taken from tobybear's post here: http://www.musicdsp.org/archive.php?classid=5#133
#ifdef WIN32
__forceinline float fastpower(float f,int n)
#else
inline float fastpower(float f,int n)
#endif
{
	long *lp, l;
	lp = (long *)(&f);
	l = *lp;
	l -= 0x3F800000l;
	l <<= (n-1);
	l += 0x3F800000l;
	*lp = l;
	return f;
}
#ifdef WIN32
__forceinline  float n_tanh(float inval)
#else
inline float n_tanh(float inval)
#endif
{
	///Does expf make any difference to exp?
	return -(1.0f-(float)expf(2.0f*inval))/(1.0f+(float)expf(2.0f*inval));
}

#ifdef WIN32
__forceinline int float2int(float val)
{
	int result;
	_asm {
		fld dword ptr [val]
		fistp result
	}
	return result;
}
#elif MACX
//const double _double2fixmagic = 68719476736.0*1.5;
inline int float2int(float val)
{
	/*	Unfortunately, this assembly doesn't actually gain us any clock
		cycles... :(

	register int retval;
	asm {
		fctiwz fp0, fp1
		stfd fp0, -16(sp)
		lwz retval, -12(sp)
	}
	return retval;

	//--------
	//	And neither does this little trick...
	val= val + _double2fixmagic;
	return ((long *)&val)[1] >> 16;*/

	return static_cast<int>(val);
}
#endif

#ifdef WIN32
__forceinline float interp(float index, float val1, float val2)
#else
inline float interp(float index, float val1, float val2)
#endif
{
	float tempf;
	tempf = index - static_cast<float>(float2int(index));
	return (tempf*val2)+((1.0f-tempf)*val1);
};

//----------------------------------------------------------------------------
Voice::Voice()
{
	int i, j, k;

	frequency = 440.0f;
	baseFrequency = 440.0f;
	samplerate = 44100.0f;
	isActive = false;
	noteIsOff = false;

	controlRateSamples = 5;
	controlCount = 0;

	// /controlRateSamples because we don't need to update the modulation sources for every audio sample.
	lfo1Sine = new LFOSine(samplerate/static_cast<float>(controlRateSamples));
	lfo1Saw = new LFOSaw(samplerate/static_cast<float>(controlRateSamples));
	lfo1Ramp = new LFORamp(samplerate/static_cast<float>(controlRateSamples));
	lfo1Square = new LFOSquare(samplerate/static_cast<float>(controlRateSamples));
	lfo1SH = new LFOSH(samplerate/static_cast<float>(controlRateSamples));
	lfo1 = lfo1Sine;
	lfo1Val = 0.0f;

	lfo2Sine = new LFOSine(samplerate/static_cast<float>(controlRateSamples));
	lfo2Saw = new LFOSaw(samplerate/static_cast<float>(controlRateSamples));
	lfo2Ramp = new LFORamp(samplerate/static_cast<float>(controlRateSamples));
	lfo2Square = new LFOSquare(samplerate/static_cast<float>(controlRateSamples));
	lfo2SH = new LFOSH(samplerate/static_cast<float>(controlRateSamples));
	lfo2 = lfo2Sine;
	lfo2Val = 0.0f;

	env1Attack = new ADSRAttack(samplerate/static_cast<float>(controlRateSamples),
								this,
								true);
	env1Decay = new ADSRDecay(samplerate/static_cast<float>(controlRateSamples),
							  this,
							  true);
	env1Sustain = new ADSRSustain(samplerate/static_cast<float>(controlRateSamples),
								  this,
								  true);
	env1Release = new ADSRRelease(samplerate/static_cast<float>(controlRateSamples),
								  this,
								  true);
	env1 = env1Attack;
	env1Val = 0.0f;

	env2Attack = new ADSRAttack(samplerate/static_cast<float>(controlRateSamples),
								this,
								false);
	env2Decay = new ADSRDecay(samplerate/static_cast<float>(controlRateSamples),
							  this,
							  false);
	env2Sustain = new ADSRSustain(samplerate/static_cast<float>(controlRateSamples),
								  this,
								  false);
	env2Release = new ADSRRelease(samplerate/static_cast<float>(controlRateSamples),
								  this,
								  false);
	env2 = env2Attack;
	env2Val = 0.0f;

	//--------
	lfo1Reset = false;
	lfo2Reset = false;

	//--------
	k = 0;
	//Set the vertices to display a sphere.
	for(i=0;i<NUM_LOOPS;i++)
	{
		waveforms[i] = &(vertices[k]);
		indices[i] = 0.0f;
		increments[i] = 1.0f;
		for(j=0;j<NUM_LOOPS;j++)
		{
			v[k] = ((float)i/(float)NUM_LOOPS) * (float)TWOPI;
			u[k] = ((float)j/(float)NUM_LOOPS) * (float)TWOPI;
			sphereVertices[k].set(cosf(u[k])*cosf(v[k]),
								  cosf(u[k])*sinf(v[k]),
								  sinf(u[k]));
			vertices[k] = sphereVertices[k];
			k++;
		}
	}

	//--------
	m[0] = 4.0f;
	mi[1] = 1;
	m[2] = 4.0f;
	mi[3] = 1;
	m[4] = 4.0f;
	mi[5] = 1;
	m[6] = 4.0f;
	mi[7] = 1;
	updateSamples = 480; //It's the vertices getting updated that causes the slow-down, so this has to be a fairly large number.
	updateCount = 119; //So it gets updated as soon as the voice starts making noise.
	m0Src = mNone;
	m2Src = mNone;
	m4Src = mNone;
	m6Src = mNone;
	m0Depth = 0.0f;
	m2Depth = 0.0f;
	m4Depth = 0.0f;
	m6Depth = 0.0f;

	//--------
	cutOff = 440.0f;
	q = 0.0f;
	low = 0.0f;
	high = 0.0f;
	band = 0.0f;
	notch = 0.0f;
	calcF(cutOff);

	//--------
	level = 1.0f;

	//--------
	for(i=0;i<256;i++)
	{
		sine[i] = sinf(((float)i/255.0f)*TWOPI);
	}
}

//----------------------------------------------------------------------------
Voice::~Voice()
{
	delete lfo1Sine;
	delete lfo1Saw;
	delete lfo1Ramp;
	delete lfo1Square;
	delete lfo1SH;
	delete lfo2Sine;
	delete lfo2Saw;
	delete lfo2Ramp;
	delete lfo2Square;
	delete lfo2SH;

	delete env1Attack;
	delete env1Decay;
	delete env1Sustain;
	delete env1Release;
	delete env2Attack;
	delete env2Decay;
	delete env2Sustain;
	delete env2Release;
}

//----------------------------------------------------------------------------
twofloats Voice::getSample()
{
	int i;
	int tempint, tempint2, tempint3, tempint4;
	float radius;
	twofloats retval;
	float mvals[4];
	float cutoffval;
	float ampval;
	float tempf;

	controlCount++;
	//Update modulation sources.
	if(controlCount >= controlRateSamples)
	{
		//Man, I love polymorphism...
		env1Val = env1->getSample();
		env2Val = env2->getSample();
		lfo1Val = lfo1->getSample();
		lfo2Val = lfo2->getSample();
		controlCount = 0;
	}	

	updateCount++;
	if(updateCount >= updateSamples)
	{
		mvals[0] = m[0];
		mvals[1] = m[2];
		mvals[2] = m[4];
		mvals[3] = m[6];
		//Calculate modulation (is there any way to replace these switch statements?).
		switch(m0Src)
		{
			case mEnv1:
				mvals[0] *= (env1Val*TWOPI);
				mvals[0] = (mvals[0]*m0Depth)+(m[0]*(1.0f-m0Depth));
				break;
			case mEnv2:
				mvals[0] *=(env2Val*TWOPI);
				mvals[0] = (mvals[0]*m0Depth)+(m[0]*(1.0f-m0Depth));
				break;
			case mLFO1:
				mvals[0] *= (((lfo1Val+1.0f)*0.5f)*TWOPI);
				mvals[0] = (mvals[0]*m0Depth)+(m[0]*(1.0f-m0Depth));
				break;
			case mLFO2:
				mvals[0] *= (((lfo2Val+1.0f)*0.5f)*TWOPI);
				mvals[0] = (mvals[0]*m0Depth)+(m[0]*(1.0f-m0Depth));
				break;
			case mEnv1inv:
				mvals[0] *= ((1.0f-env1Val)*TWOPI);
				mvals[0] = (mvals[0]*m0Depth)+(m[0]*(1.0f-m0Depth));
				break;
			case mEnv2inv:
				mvals[0] *= ((1.0f-env2Val)*TWOPI);
				mvals[0] = (mvals[0]*m0Depth)+(m[0]*(1.0f-m0Depth));
				break;
			case mLFO1inv:
				mvals[0] *= ((1.0f-((lfo1Val+1.0f)*0.5f))*TWOPI);
				mvals[0] = (mvals[0]*m0Depth)+(m[0]*(1.0f-m0Depth));
				break;
			case mLFO2inv:
				mvals[0] *= ((1.0f-((lfo2Val+1.0f)*0.5f))*TWOPI);
				mvals[0] = (mvals[0]*m0Depth)+(m[0]*(1.0f-m0Depth));
				break;
		}
		switch(m2Src)
		{
			case mEnv1:
				mvals[1] *= (env1Val*TWOPI);
				mvals[1] = (mvals[1]*m2Depth)+(m[2]*(1.0f-m2Depth));
				break;
			case mEnv2:
				mvals[1] *= (env2Val*TWOPI);
				mvals[1] = (mvals[1]*m2Depth)+(m[2]*(1.0f-m2Depth));
				break;
			case mLFO1:
				mvals[1] *= (((lfo1Val+1.0f)*0.5f)*TWOPI);
				mvals[1] = (mvals[1]*m2Depth)+(m[2]*(1.0f-m2Depth));
				break;
			case mLFO2:
				mvals[1] *= (((lfo2Val+1.0f)*0.5f)*TWOPI);
				mvals[1] = (mvals[1]*m2Depth)+(m[2]*(1.0f-m2Depth));
				break;
			case mEnv1inv:
				mvals[1] *= ((1.0f-env1Val)*TWOPI);
				mvals[1] = (mvals[1]*m2Depth)+(m[2]*(1.0f-m2Depth));
				break;
			case mEnv2inv:
				mvals[1] *= ((1.0f-env2Val)*TWOPI);
				mvals[1] = (mvals[1]*m2Depth)+(m[2]*(1.0f-m2Depth));
				break;
			case mLFO1inv:
				mvals[1] *= ((1.0f-((lfo1Val+1.0f)*0.5f))*TWOPI);
				mvals[1] = (mvals[1]*m2Depth)+(m[2]*(1.0f-m2Depth));
				break;
			case mLFO2inv:
				mvals[1] *= ((1.0f-((lfo2Val+1.0f)*0.5f))*TWOPI);
				mvals[1] = (mvals[1]*m2Depth)+(m[2]*(1.0f-m2Depth));
				break;
		}
		switch(m4Src)
		{
			case mEnv1:
				mvals[2] *= (env1Val*TWOPI);
				mvals[2] = (mvals[2]*m4Depth)+(m[4]*(1.0f-m4Depth));
				break;
			case mEnv2:
				mvals[2] *= (env2Val*TWOPI);
				mvals[2] = (mvals[2]*m4Depth)+(m[4]*(1.0f-m4Depth));
				break;
			case mLFO1:
				mvals[2] *= (((lfo1Val+1.0f)*0.5f)*TWOPI);
				mvals[2] = (mvals[2]*m4Depth)+(m[4]*(1.0f-m4Depth));
				break;
			case mLFO2:
				mvals[2] *= (((lfo2Val+1.0f)*0.5f)*TWOPI);
				mvals[2] = (mvals[2]*m4Depth)+(m[4]*(1.0f-m4Depth));
				break;
			case mEnv1inv:
				mvals[2] *= ((1.0f-env1Val)*TWOPI);
				mvals[2] = (mvals[2]*m4Depth)+(m[4]*(1.0f-m4Depth));
				break;
			case mEnv2inv:
				mvals[2] *= ((1.0f-env2Val)*TWOPI);
				mvals[2] = (mvals[2]*m4Depth)+(m[4]*(1.0f-m4Depth));
				break;
			case mLFO1inv:
				mvals[2] *= ((1.0f-((lfo1Val+1.0f)*0.5f))*TWOPI);
				mvals[2] = (mvals[2]*m4Depth)+(m[4]*(1.0f-m4Depth));
				break;
			case mLFO2inv:
				mvals[2] *= ((1.0f-((lfo2Val+1.0f)*0.5f))*TWOPI);
				mvals[2] = (mvals[2]*m4Depth)+(m[4]*(1.0f-m4Depth));
				break;
		}
		switch(m6Src)
		{
			case mEnv1:
				mvals[3] *= (env1Val*TWOPI);
				mvals[3] = (mvals[3]*m6Depth)+(m[6]*(1.0f-m6Depth));
				break;
			case mEnv2:
				mvals[3] *= (env2Val*TWOPI);
				mvals[3] = (mvals[3]*m6Depth)+(m[6]*(1.0f-m6Depth));
				break;
			case mLFO1:
				mvals[3] *= (((lfo1Val+1.0f)*0.5f)*TWOPI);
				mvals[3] = (mvals[3]*m6Depth)+(m[6]*(1.0f-m6Depth));
				break;
			case mLFO2:
				mvals[3] *= (((lfo2Val+1.0f)*0.5f)*TWOPI);
				mvals[3] = (mvals[3]*m6Depth)+(m[6]*(1.0f-m6Depth));
				break;
			case mEnv1inv:
				mvals[3] *= ((1.0f-env1Val)*TWOPI);
				mvals[3] = (mvals[3]*m6Depth)+(m[6]*(1.0f-m6Depth));
				break;
			case mEnv2inv:
				mvals[3] *= ((1.0f-env2Val)*TWOPI);
				mvals[3] = (mvals[3]*m6Depth)+(m[6]*(1.0f-m6Depth));
				break;
			case mLFO1inv:
				mvals[3] *= ((1.0f-((lfo1Val+1.0f)*0.5f))*TWOPI);
				mvals[3] = (mvals[3]*m6Depth)+(m[6]*(1.0f-m6Depth));
				break;
			case mLFO2inv:
				mvals[3] *= ((1.0f-((lfo2Val+1.0f)*0.5f))*TWOPI);
				mvals[3] = (mvals[3]*m6Depth)+(m[6]*(1.0f-m6Depth));
				break;
		}

		for(i=0;i<NUM_VERTICES;i++)
		{
			radius = 0.0f;
			radius += fastpower(niallSin(mvals[0]*u[i]), mi[1]);
			radius += fastpower(niallCos(mvals[1]*u[i]), mi[3]);
			radius += fastpower(niallSin(mvals[2]*v[i]), mi[5]);
			radius += fastpower(niallCos(mvals[3]*v[i]), mi[7]);
			/*radius += pow(sinf(mvals[0]*u[i]), m[1]); //This is the clock-cycles-no-object option...
			radius += pow(cosf(mvals[1]*u[i]), m[3]);
			radius += pow(sinf(mvals[2]*v[i]), m[5]);
			radius += pow(cosf(mvals[3]*v[i]), m[7]);*/
			vertices[i].set(radius*sphereVertices[i].x,
							radius*sphereVertices[i].y,
							radius*sphereVertices[i].z);
		}
		updateCount = 0;
	}

	//Generate current sample from vertices array.
	for(i=1;i<(NUM_LOOPS-1);i++) //Very top & bottom verts are all zero anyway. (huh?)
	{
		tempint = float2int(indices[i]);
		tempint2 = tempint+1;
		if(tempint2 >= NUM_LOOPS)
			tempint2 = 0;
		retval += interp(indices[i],
						 waveforms[i][tempint].x,
						 waveforms[i][tempint2].x) * NUM_LOOPS_INV; //need to scale, or it'll be far too loud
						 //I'm not entirely sure why, but NUM_LOOPS_INV actually gains me a couple of clock cycles on my macmini...
						 //Dunno why (1.0f/NUM_LOOPS_F) isn't automatically converted by the compiler...

		//This is the faster, aliasing-like-hell version.
		//retval += waveforms[i][float2int(indices[i])].x * (1.0f/(float)NUM_LOOPS);

		indices[i] += increments[i];
		if(indices[i] >= NUM_LOOPS_F)
		{
			//Can we do better than fmod, without too much cpu?
			indices[i] = (float)fmod(indices[i], NUM_LOOPS_F);
		}
	}

	//Apply filter (if anyone reading this is interested, this is the same filter in Buffer Synth 2).
	if(filtType != fOff)
	{
		cutoffval = cutOff;
		switch(filtMSrc)
		{
			case mEnv1:
				cutoffval *= env1Val;
				cutoffval = (cutoffval*filtMDepth)+(cutOff*(1.0f-filtMDepth));
				break;
			case mEnv2:
				cutoffval *= env2Val;
				cutoffval = (cutoffval*filtMDepth)+(cutOff*(1.0f-filtMDepth));
				break;
			case mLFO1:
				cutoffval *= ((lfo1Val+1.0f)*0.5f);
				cutoffval = (cutoffval*filtMDepth)+(cutOff*(1.0f-filtMDepth));
				break;
			case mLFO2:
				cutoffval *= ((lfo2Val+1.0f)*0.5f);
				cutoffval = (cutoffval*filtMDepth)+(cutOff*(1.0f-filtMDepth));
				break;
			case mEnv1inv:
				cutoffval *= (1.0f-env1Val);
				cutoffval = (cutoffval*filtMDepth)+(cutOff*(1.0f-filtMDepth));
				break;
			case mEnv2inv:
				cutoffval *= (1.0f-env2Val);
				cutoffval = (cutoffval*filtMDepth)+(cutOff*(1.0f-filtMDepth));
				break;
			case mLFO1inv:
				cutoffval *= (1.0f-((lfo1Val+1.0f)*0.5f));
				cutoffval = (cutoffval*filtMDepth)+(cutOff*(1.0f-filtMDepth));
				break;
			case mLFO2inv:
				cutoffval *= (1.0f-((lfo2Val+1.0f)*0.5f));
				cutoffval = (cutoffval*filtMDepth)+(cutOff*(1.0f-filtMDepth));
				break;
		}

		//if(cutoffval != cutOff) //I'm not entirely sure why, but the square wave LFO doesn't work with this in...
		{
			if(cutoffval <= 0.0f)
				cutoffval = 0.5f;
			calcF(cutoffval);
		}

		//Actual filter calculations.
		low = low + (band * f);
		high.left = (retval.left * q) - low.left - (band.left*q);
		high.right = (retval.right * q) - low.right - (band.right*q);
		band = (high * f) + band;
		notch = high + low;

		switch(filtType)
		{
			case fHP:
				retval = high;
				break;
			case fBP:
				retval = band;
				break;
			case fLP:
				retval = low;
				break;
		}

		//Now we amplify it w/a tanh distortion, according to what value q is set to.
		tempf = ((1.0f-q));
		ampval = n_tanh(tempf);

		tempf = n_tanh(retval.left*(1.0f+(ampval*16.0f)));
		retval.left = tempf;
		tempf = n_tanh(retval.right*(1.0f+(ampval*16.0f)));
		retval.right = tempf;
	}

	retval *= velocity;
	retval *= env1Val;

	return (retval*level);
}

//----------------------------------------------------------------------------
void Voice::onNoteOn(unsigned char note, float velocity)
{
	int i;

	if(!isActive)
	{
		isActive = true;

		baseFrequency = MIDI2Freq[note];
		frequency = baseFrequency;
		this->velocity = velocity;
		noteIsOff = false;

		// 4+1 = 5 => update modulation sources for the first sample after the Note On message has been received
		controlCount = 4;

		if(lfo1Reset)
			lfo1->setPhase(0.0f);
		if(lfo2Reset)
			lfo2->setPhase(0.0f);

		//reset the attack sections of the envelopes to have index = 0
		env1Attack->setLevelVal(0.0f);
		env2Attack->setLevelVal(0.0f);
		//reset the envelopes to their attack sections
		env1 = env1Attack;
		env2 = env2Attack;

		for(i=0;i<NUM_LOOPS;i++)
		{
			indices[i] = 0.0f;
		}
		increments[16] = frequency * ((float)NUM_LOOPS/samplerate);
		increments[15] = increments[16] * 2.0f;
		increments[17] = increments[16] * 3.0f;
		increments[14] = increments[16] * 4.0f;
		increments[18] = increments[16] * 5.0f;
		increments[13] = increments[16] * 6.0f;
		increments[19] = increments[16] * 7.0f;
		increments[12] = increments[16] * 8.0f;
		increments[20] = increments[16] * 9.0f;
		increments[11] = increments[16] * 10.0f;
		increments[21] = increments[16] * 11.0f;
		increments[10] = increments[16] * 12.0f;
		increments[22] = increments[16] * 13.0f;
		increments[9] = increments[16] * 14.0f;
		increments[23] = increments[16] * 15.0f;
		increments[8] = increments[16] * 16.0f;
		increments[24] = increments[16] * 17.0f;
		increments[7] = increments[16] * 18.0f;
		increments[25] = increments[16] * 19.0f;
		increments[6] = increments[16] * 20.0f;
		increments[26] = increments[16] * 21.0f;
		increments[5] = increments[16] * 22.0f;
		increments[27] = increments[16] * 23.0f;
		increments[4] = increments[16] * 22.0f;
		increments[28] = increments[16] * 23.0f;
		increments[3] = increments[16] * 24.0f;
		increments[29] = increments[16] * 25.0f;
		increments[2] = increments[16] * 26.0f;
		increments[30] = increments[16] * 27.0f;
		increments[1] = increments[16] * 26.0f;
		increments[31] = increments[16] * 27.0f;
		increments[0] = increments[16] * 28.0f;
	}
}

//----------------------------------------------------------------------------
void Voice::onNoteOff()
{
	if(isActive)
	{
		noteIsOff = true;

		env1Release->setLevelVal(env1->getLevelVal());
		env1 = env1Release;
		env2Release->setLevelVal(env2->getLevelVal());
		env2 = env2Release;
	}
}

//----------------------------------------------------------------------------
void Voice::onCC(unsigned char cc, float value)
{
	
}

//----------------------------------------------------------------------------
void Voice::onPitchBend(float value)
{
	//We're assuming value is going to be between 0.25 and 4 here...
	frequency = baseFrequency * value;
}

//----------------------------------------------------------------------------
void Voice::setSamplerate(float value)
{
	samplerate = value;

	lfo1Sine->setSamplerate(samplerate/static_cast<float>(controlRateSamples));
	lfo1Saw->setSamplerate(samplerate/static_cast<float>(controlRateSamples));
	lfo1Ramp->setSamplerate(samplerate/static_cast<float>(controlRateSamples));
	lfo1Square->setSamplerate(samplerate/static_cast<float>(controlRateSamples));
	lfo1SH->setSamplerate(samplerate/static_cast<float>(controlRateSamples));

	lfo2Sine->setSamplerate(samplerate/static_cast<float>(controlRateSamples));
	lfo2Saw->setSamplerate(samplerate/static_cast<float>(controlRateSamples));
	lfo2Ramp->setSamplerate(samplerate/static_cast<float>(controlRateSamples));
	lfo2Square->setSamplerate(samplerate/static_cast<float>(controlRateSamples));
	lfo2SH->setSamplerate(samplerate/static_cast<float>(controlRateSamples));

	env1Attack->setSamplerate(samplerate/static_cast<float>(controlRateSamples));
	env1Decay->setSamplerate(samplerate/static_cast<float>(controlRateSamples));
	env1Sustain->setSamplerate(samplerate/static_cast<float>(controlRateSamples));
	env1Release->setSamplerate(samplerate/static_cast<float>(controlRateSamples));

	env2Attack->setSamplerate(samplerate/static_cast<float>(controlRateSamples));
	env2Decay->setSamplerate(samplerate/static_cast<float>(controlRateSamples));
	env2Sustain->setSamplerate(samplerate/static_cast<float>(controlRateSamples));
	env2Release->setSamplerate(samplerate/static_cast<float>(controlRateSamples));

	calcF(cutOff);
}

//----------------------------------------------------------------------------
void Voice::setParameter(long index, float value)
{
	const float numWaves = 5.0f;
	float tempf;

	switch(index)
	{
		//--------
		case Harmonical::M0Val:
			m[0] = 8.0f * value;
			break;
		case Harmonical::M0MSrc:
			if(value < (1.0f/9.0f))
				m0Src = mNone;
			else if(value < (2.0f/9.0f))
				m0Src = mEnv1;
			else if(value < (3.0f/9.0f))
				m0Src = mEnv2;
			else if(value < (4.0f/9.0f))
				m0Src = mLFO1;
			else if(value < (5.0f/9.0f))
				m0Src = mLFO2;
			else if(value < (6.0f/9.0f))
				m0Src = mEnv1inv;
			else if(value < (7.0f/9.0f))
				m0Src = mEnv2inv;
			else if(value < (8.0f/9.0f))
				m0Src = mLFO1inv;
			else
				m0Src = mLFO2inv;
			break;
		case Harmonical::M0Mod:
			m0Depth = value;
			break;
		case Harmonical::M2Val:
			m[2] = 8.0f * value;
			break;
		case Harmonical::M2MSrc:
			if(value < (1.0f/9.0f))
				m2Src = mNone;
			else if(value < (2.0f/9.0f))
				m2Src = mEnv1;
			else if(value < (3.0f/9.0f))
				m2Src = mEnv2;
			else if(value < (4.0f/9.0f))
				m2Src = mLFO1;
			else if(value < (5.0f/9.0f))
				m2Src = mLFO2;
			else if(value < (6.0f/9.0f))
				m2Src = mEnv1inv;
			else if(value < (7.0f/9.0f))
				m2Src = mEnv2inv;
			else if(value < (8.0f/9.0f))
				m2Src = mLFO1inv;
			else
				m2Src = mLFO2inv;
			break;
		case Harmonical::M2Mod:
			m2Depth = value;
			break;
		case Harmonical::M4Val:
			m[4] = 8.0f * value;
			break;
		case Harmonical::M4MSrc:
			if(value < (1.0f/9.0f))
				m4Src = mNone;
			else if(value < (2.0f/9.0f))
				m4Src = mEnv1;
			else if(value < (3.0f/9.0f))
				m4Src = mEnv2;
			else if(value < (4.0f/9.0f))
				m4Src = mLFO1;
			else if(value < (5.0f/9.0f))
				m4Src = mLFO2;
			else if(value < (6.0f/9.0f))
				m4Src = mEnv1inv;
			else if(value < (7.0f/9.0f))
				m4Src = mEnv2inv;
			else if(value < (8.0f/9.0f))
				m4Src = mLFO1inv;
			else
				m4Src = mLFO2inv;
			break;
		case Harmonical::M4Mod:
			m4Depth = value;
			break;
		case Harmonical::M6Val:
			m[6] = 8.0f * value;
			break;
		case Harmonical::M6MSrc:
			if(value < (1.0f/9.0f))
				m6Src = mNone;
			else if(value < (2.0f/9.0f))
				m6Src = mEnv1;
			else if(value < (3.0f/9.0f))
				m6Src = mEnv2;
			else if(value < (4.0f/9.0f))
				m6Src = mLFO1;
			else if(value < (5.0f/9.0f))
				m6Src = mLFO2;
			else if(value < (6.0f/9.0f))
				m6Src = mEnv1inv;
			else if(value < (7.0f/9.0f))
				m6Src = mEnv2inv;
			else if(value < (8.0f/9.0f))
				m6Src = mLFO1inv;
			else
				m6Src = mLFO2inv;
			break;
		case Harmonical::M6Mod:
			m6Depth = value;
			break;
		case Harmonical::M1Val:
			mi[1] = (static_cast<int>(9.0f * value));
			break;
		case Harmonical::M3Val:
			mi[3] = (static_cast<int>(9.0f * value));
			break;
		case Harmonical::M5Val:
			mi[5] = (static_cast<int>(9.0f * value));
			break;
		case Harmonical::M7Val:
			mi[7] = (static_cast<int>(9.0f * value));
			break;
		//--------
		case Harmonical::LFO1Wave:
			tempf = lfo1->getRate();
			if(value < 1.0f/numWaves)
				lfo1 = lfo1Sine;
			else if(value < 2.0f/numWaves)
				lfo1 = lfo1Saw;
			else if(value < 3.0f/numWaves)
				lfo1 = lfo1Ramp;
			else if(value < 4.0f/numWaves)
				lfo1 = lfo1Square;
			else
				lfo1 = lfo1SH;
			lfo1->setRate(tempf);
			break;
		case Harmonical::LFO1Reset:
			if(value < 0.5f)
				lfo1Reset = false;
			else
				lfo1Reset = true;
			break;
		case Harmonical::LFO1Rate:
			lfo1->setRate(value);
			break;
		case Harmonical::LFO2Wave:
			tempf = lfo2->getRate();
			if(value < 1.0f/numWaves)
				lfo2 = lfo2Sine;
			else if(value < 2.0f/numWaves)
				lfo2 = lfo2Saw;
			else if(value < 3.0f/numWaves)
				lfo2 = lfo2Ramp;
			else if(value < 4.0f/numWaves)
				lfo2 = lfo2Square;
			else
				lfo2 = lfo2SH;
			lfo2->setRate(tempf);
			break;
		case Harmonical::LFO2Reset:
			if(value < 0.5f)
				lfo2Reset = false;
			else
				lfo2Reset = true;
			break;
		case Harmonical::LFO2Rate:
			lfo2->setRate(value);
			break;
		//--------
		case Harmonical::Env1A:
			env1Attack->setTime(value);
			break;
		case Harmonical::Env1D:
			env1Decay->setTime(value);
			break;
		case Harmonical::Env1S:
			env1Decay->setLevelVal(value);
			env1Sustain->setLevelVal(value);
			env1Release->setLevelVal(value);
			break;
		case Harmonical::Env1R:
			env1Release->setTime(value);
			break;
		case Harmonical::Env2A:
			env2Attack->setTime(value);
			break;
		case Harmonical::Env2D:
			env2Decay->setTime(value);
			break;
		case Harmonical::Env2S:
			env2Decay->setLevelVal(value);
			env2Sustain->setLevelVal(value);
			env2Release->setLevelVal(value);
			break;
		case Harmonical::Env2R:
			env2Release->setTime(value);
			break;
		//--------
		case Harmonical::FiltType:
			if(value < 0.25f)
				filtType = fOff;
			else if(value < 0.5f)
				filtType = fLP;
			else if(value < 0.75f)
				filtType = fBP;
			else
				filtType = fHP;
			break;
		case Harmonical::FiltCutoff:
			cutOff = value * FILTER_MAX;
			calcF(cutOff);
			break;
		case Harmonical::FiltRes:
			q = 1.0f-value;
			break;
		case Harmonical::FiltMSrc:
			if(value < (1.0f/9.0f))
				filtMSrc = mNone;
			else if(value < (2.0f/9.0f))
				filtMSrc = mEnv1;
			else if(value < (3.0f/9.0f))
				filtMSrc = mEnv2;
			else if(value < (4.0f/9.0f))
				filtMSrc = mLFO1;
			else if(value < (5.0f/9.0f))
				filtMSrc = mLFO2;
			else if(value < (6.0f/9.0f))
				filtMSrc = mEnv1inv;
			else if(value < (7.0f/9.0f))
				filtMSrc = mEnv2inv;
			else if(value < (8.0f/9.0f))
				filtMSrc = mLFO1inv;
			else
				filtMSrc = mLFO2inv;
			break;
		case Harmonical::FiltMod:
			filtMDepth = value;
			break;
		//--------
		case Harmonical::Level:
			level = value * 2.0f;
			break;
	}
}

//----------------------------------------------------------------------------
void Voice::incEnv_D(ADSRSection *env)
{
	if(env == env1)
	{
		//This line's really just to make sure env1Decay's index is at 0.
		env1Decay->setLevelVal(env1Sustain->getLevelVal());
		env1 = env1Decay;
	}
	else
	{
		env2Decay->setLevelVal(env1Sustain->getLevelVal());
		env2 = env2Decay;
	}
}

//----------------------------------------------------------------------------
void Voice::incEnv_S(ADSRSection *env)
{
	if(env == env1)
		env1 = env1Sustain;
	else
		env2 = env2Sustain;
}

//----------------------------------------------------------------------------
float Voice::niallSin(float val)
{
	float index_f;
	unsigned char index_uc, index_uc2;

	index_f = (255.0f/TWOPI)*val;
	index_uc = static_cast<unsigned char>(index_f);
	index_uc2 = index_uc+1;

	return interp(index_f, sine[index_uc], sine[index_uc2]);
}

//----------------------------------------------------------------------------
float Voice::niallCos(float val)
{
	float index_f;
	unsigned char index_uc, index_uc2;

	index_f = (255.0f/TWOPI)*(val+(PI/2.0f));
	index_uc = static_cast<unsigned char>(index_f);
	index_uc2 = index_uc+1;

	return interp(index_f, sine[index_uc], sine[index_uc2]);
}
