//	Delay.h - Simple delay effect.
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

#ifndef DELAY_H_
#define DELAY_H_

#include "../EffectBase.h"
#include "../ParameterManager.h"
#include "../ModulationManager.h"

///	Simple delay effect.
class Delay : public EffectBase,
			  public ParameterCallback
{
  public:
	///	Constructor.
	Delay(VstPlugin *plugin);
	///	Destructor.
	~Delay();

	///	Returns a pointer to a block of audio for the current process() block.
	float **getBlock(float **input, VstInt32 blockSize);

	///	So we get informed when one of our parameters changes.
	void parameterChanged(VstInt32 index, float val);
	///	Returns the current value of the indexed parameter.
	float getValue(VstInt32 index);
	///	The textual display of the indexed parameter's current value.
	std::string getTextValue(VstInt32 index);
  private:
	///	So we can resize the delay buffer accordingly.
	void samplerateChanged();
	///	A cheap, faked exp curve.
	inline float fakeExp(float val)
	{
		//return val/(val + (-9.0f*(val-1.0f)));
		return val/(val + ((1.0f - (1.0f/0.05f))*(val-1.0f)));
	}

	///	constants for the paramIds variables.
	enum
	{
		DelayLMW1,
		DelayLMW2,
		DelayLMW3,
		DelayL,
		DelayRMW1,
		DelayRMW2,
		DelayRMW3,
		DelayR,
		ShortTimes,
		Feedback,
		Mix,
		Level,

		NumParams
	};

	///	Pointer to the ParameterManager for this plugin instance.
	ParameterManager *paramManager;
	///	The ParameterManager IDs for all our parameters.
	VstInt32 paramIds[NumParams];
	///	The parameter values.
	float parameters[NumParams];

	///	Pointer to the ModulationManager for this plugin instance.
	ModulationManager *modManager;

	///	Size of the delay buffer.
	VstInt64 bufferSize;
	///	The delay buffer.
	float *buffer[2];
	///	Write pointer.
	VstInt64 writePointer;
	///	The current delay value (left).
	float currentDelayL;
	///	The current delay value (right).
	float currentDelayR;
};

#endif
