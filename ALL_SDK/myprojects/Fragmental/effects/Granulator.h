//	Granulator.h - Granulator effect.
//	---------------------------------------------------------------------------
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
//	---------------------------------------------------------------------------

#ifndef GRANULATOR_H_
#define GRANULATOR_H_

#include "../EffectBase.h"
#include "../ParameterManager.h"
#include "../ModulationManager.h"
#include "Grain.h"

///	Granulator effect.
class Granulator : public EffectBase,
				   public ParameterCallback
{
  public:
	///	Constructor.
	Granulator(VstPlugin *plugin);
	///	Destructor.
	~Granulator();

	///	Returns a pointer to a block of audio for the current process() block.
	float **getBlock(float **input, VstInt32 blockSize);

	///	Implement this in your subclass to be informed when a parameter
	///	changes value.
	void parameterChanged(VstInt32 index, float val);
	///	Returns the current value of the indexed parameter.
	float getValue(VstInt32 index);
	///	The textual display of the indexed parameter's current value.
	std::string getTextValue(VstInt32 index);
  private:
	///	constants for the paramIds variables.
	enum
	{
		DensityMW1,
		DensityMW2,
		DensityMW3,
		Density,
		DurationMW1,
		DurationMW2,
		DurationMW3,
		Duration,
		PitchMW1,
		PitchMW2,
		PitchMW3,
		Pitch,
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

	///	Enum containing certain relevant values.
	enum
	{
		DelayLineSize = 88200,
		MaxGrains = 32
	};

	///	Our delay line.
	float **delayLine;
	///	Our grains.
	Grain grains[MaxGrains];

	///	Current read position in the delay line.
	int readPosition;
	///	Current increment (used to pitch the grains).
	float increment;
	///	Current size of the grains.
	int grainSize;

	///	Inverse density of the grains.
	float invDensity;
	///	Number of samples to the next onset.
	int nextOnset;

	///	Used to scale the volume according to how many grains are active.
	float ampScale;

	///	Used to make sure we're not constantly updating our density when we're not being modulated.
	float lastDensity;

	///	Used to delay the amplitude scaling when the pitch parameter is moved away from 1x.
	int scaleDelay;
};

#endif
