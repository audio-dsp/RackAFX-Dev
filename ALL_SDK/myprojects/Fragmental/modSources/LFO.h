//	LFO.h - An LFO made up of up to 32 steps.
//	----------------------------------------------------------------------------
//	Copyright (c) 2006-2007 Niall Moody
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
//	----------------------------------------------------------------------------

#ifndef LFO_H_
#define LFO_H_

#include "ModType.h"
#include "../ParameterManager.h"
#include "../HelperStuff.h"

///	An LFO made up of up to 32 steps.
class LFO : public ModType,
			public ParameterCallback
{
  public:
	///	Constructor.
	LFO(VstPlugin *plugin);
	///	Destructor.
	~LFO();

	///	Updates the LFO's value.
	void getBlock(float *block,
				  VstInt32 blockSize,
				  VstInt32 barStart,
				  float ppq);

	///	To set the various parameters.
	void parameterChanged(VstInt32 index, float val);
	///	Informs us of the current samplerate.
	void setSamplerate(float rate);

	///	Returns the current value of the indexed parameter.
	float getValue(VstInt32 index);
	///	Returns a textual version of the indexed parameter's value.
	std::string getTextValue(VstInt32 index);
  private:
	///	Helper b-spline interpolation method.
	strictinline float bSplineInterp(float val,
									 float x0,
									 float x1,
									 float x2,
									 float x3)
	{
		// 4-point, 3rd-order B-spline (x-form)
		float ym1py1 = x0+x2;
		float c0 = (1.0f/6.0f) * ym1py1 + (2.0f/3.0f) * x1;
		float c1 = (1.0f/2.0f) * (x2-x0);
		float c2 = (1.0f/2.0f) * ym1py1 - x1;
		float c3 = (1.0f/2.0f) * (x1-x2) + (1.0f/6.0f) * (x3-x0);
		return ((c3*val+c2)*val+c1)*val+c0;
	};

	///	A cheap, faked exp curve.
	/*!
		Only works on a 0->1 range.
	 */
	inline float fakeExp(float val)
	{
		return val/(val + (expCoeff*(val-1.0f)));
	};

	///	IDs for our parameters.
	enum
	{
		NumSteps,
		Preset,
		Freq,
		Smooth,
		FreqRange,
		Step0,
		Step31 = Step0+31,

		NumParameters
	};

	///	The ParameterManager for this plugin.
	ParameterManager *paramManager;
	///	The ParameterManager IDs for all our parameters.
	VstInt32 paramIds[NumParameters];

	///	The 32 steps.
	float steps[32];
	///	The number of active steps.
	int numSteps;
	///	The frequency of the LFO.
	float freq;
	///	The amount of smoothing of the LFO.
	float smoothing;
	///	Whether the LFO's running at audio-rate speeds or not.
	bool freqRange;

	///	The current position within the waveform.
	double pos;
	///	The current samplerate.
	float samplerate;

	///	Coefficient for exponential curve.
	float expCoeff;

	///	For getValue().
	float storedFreq;

	///	Pointer to the plugin.
	VstPlugin *plug;
};

#endif
