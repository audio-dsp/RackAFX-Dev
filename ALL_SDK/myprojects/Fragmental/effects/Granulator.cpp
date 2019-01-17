//	Granulator.cpp - Granulator effect.
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

#include "Granulator.h"
#include "../VstPlugin.h"

#include <cassert>
#include <cmath>
#include <sstream>

using namespace std;

//------------------------------------------------------------------------------
Granulator::Granulator(VstPlugin *plugin):
EffectBase(plugin),
readPosition(0),
increment(1.0f),
grainSize(4410),
nextOnset(0),
ampScale(1.0f),
lastDensity(0.0f)
{
	int i;

	//Setup delayLine.
	delayLine = new float *[2];
	for(i=0;i<2;++i)
		delayLine[i] = new float[DelayLineSize];
	for(i=0;i<DelayLineSize;++i)
	{
		delayLine[0][i] = 0.0f;
		delayLine[1][i] = 0.0f;
	}

	//Update grains with the delay line.
	for(i=0;i<MaxGrains;++i)
		grains[i].setDelayLine(delayLine, DelayLineSize);

	paramManager = plugin->getParameterManager();
	paramIds[DensityMW1] = paramManager->registerParameter(this, "DensityMW1", " ");
	paramIds[DensityMW2] = paramManager->registerParameter(this, "DensityMW2", " ");
	paramIds[DensityMW3] = paramManager->registerParameter(this, "DensityMW3", " ");
	paramIds[Density] = paramManager->registerParameter(this, "Density", " ");
	paramIds[DurationMW1] = paramManager->registerParameter(this, "DurationMW1", " ");
	paramIds[DurationMW2] = paramManager->registerParameter(this, "DurationMW2", " ");
	paramIds[DurationMW3] = paramManager->registerParameter(this, "DurationMW3", " ");
	paramIds[Duration] = paramManager->registerParameter(this, "Duration", "s");
	paramIds[PitchMW1] = paramManager->registerParameter(this, "PitchMW1", " ");
	paramIds[PitchMW2] = paramManager->registerParameter(this, "PitchMW2", " ");
	paramIds[PitchMW3] = paramManager->registerParameter(this, "PitchMW3", " ");
	paramIds[Pitch] = paramManager->registerParameter(this, "Pitch", " ");
	paramIds[Level] = paramManager->registerParameter(this, "Level", " ");

	modManager = plugin->getModulationManager();
}

//------------------------------------------------------------------------------
Granulator::~Granulator()
{
	int i;

	//Delete delayLine.
	for(i=0;i<2;++i)
		delete [] delayLine[i];
	delete [] delayLine;
}

//------------------------------------------------------------------------------
float **Granulator::getBlock(float **input, VstInt32 blockSize)
{
	VstInt32 i, j;
	float tempf;
	float *tempOut[2];
	int numGrains = 0;
	bool skipGrains[MaxGrains];
	int grainStarts[MaxGrains];
	int grainBufferStarts[MaxGrains];
	float invNumGrains;
	float samplerate = getSamplerate();
	float *modBuffers[3];
	float modAmts[3]; //The amount by which the 2 delay parameters are
					  //modulated.
	float density, duration, pitch; //The up-to-date parameter values.
	int tempOnset;
	float tempo = getTempo()/60.0f;

	//Get modBuffers.
	for(i=0;i<3;++i)
		modBuffers[i] = (*modManager)[i];

	//Calculate number of active grains.
	for(i=0;i<MaxGrains;++i)
	{
		if(grains[i].getIsActive())
			++numGrains;
		skipGrains[i] = false;
		grainStarts[i] = -1;
		grainBufferStarts[i] = -1;
	}

	//Calculate grain scheduling stuff.
	for(i=0;i<blockSize;++i)
	{
		//Update modAmts.
		modAmts[0] = parameters[DensityMW1] +
					 parameters[DensityMW2] +
					 parameters[DensityMW3];
		modAmts[1] = parameters[DurationMW1] +
					 parameters[DurationMW2] +
					 parameters[DurationMW3];
		modAmts[2] = parameters[PitchMW1] +
					 parameters[PitchMW2] +
					 parameters[PitchMW3];

		//Update Pitch.
		if(modAmts[2] > 0.0f)
		{
			pitch = parameters[Pitch] * (1.0f-modAmts[2]);
			pitch += parameters[PitchMW1] * modBuffers[0][i];
			pitch += parameters[PitchMW2] * modBuffers[1][i];
			pitch += parameters[PitchMW3] * modBuffers[2][i];

			if(pitch < 0.5f)
				increment = (pitch * 1.5f)+0.25f; //0.25->1.0
			else if(pitch > 0.5f)
				increment = ((pitch - 0.5f)*6.0f)+1.0f; //1.0->4.0
			else
				increment = 1.0f;
		}
		else
		{
			pitch = parameters[Pitch];

			if(pitch < 0.5f)
				increment = (pitch * 1.5f)+0.25f; //0.25->1.0
			else if(pitch > 0.5f)
				increment = ((pitch - 0.5f)*6.0f)+1.0f; //1.0->4.0
			else
				increment = 1.0f;
		}

		//Update Duration.
		if(modAmts[1] > 0.0f)
		{
			duration = parameters[Duration] * (1.0f-modAmts[1]);
			duration += parameters[DurationMW1] * modBuffers[0][i];
			duration += parameters[DurationMW2] * modBuffers[1][i];
			duration += parameters[DurationMW3] * modBuffers[2][i];

			//if(!getSyncMode())
			{
				if(duration < 0.001f)
					tempf = 0.001f;
				else
					tempf = duration;
				tempf = tanh((tempf*Pi)-Pi)+1.0f;
				grainSize = static_cast<int>(tempf * DelayLineSize * 0.5f);
			}
			/*else
			{
				tempf = getBeatValue(duration);

				grainSize = static_cast<int>((tempf/tempo)*getSamplerate());
			}*/
		}
		else
		{
			duration = parameters[Duration];

			//if(!getSyncMode())
			{
				if(duration < 0.001f)
					tempf = 0.001f;
				else
					tempf = duration;
				tempf = tanh((tempf*Pi)-Pi)+1.0f;
				grainSize = static_cast<int>(tempf * DelayLineSize * 0.5f);
			}
			/*else
			{
				tempf = getBeatValue(parameters[Duration]);

				grainSize = static_cast<int>((tempf/tempo)*getSamplerate());
			}*/
		}

		//Update Density.
		if(modAmts[0] > 0.0f)
		{
			density = parameters[Density] * (1.0f-modAmts[0]);
			density += parameters[DensityMW1] * modBuffers[0][i];
			density += parameters[DensityMW2] * modBuffers[1][i];
			density += parameters[DensityMW3] * modBuffers[2][i];

			if(density < 0.001f)
				tempf = 0.001f;
			else
				tempf = density;
			tempf = tanh((tempf*Pi)-Pi)+1.0f;
			invDensity = 1.0f/(tempf * 128.0f);

			//Update nextOnset if necessary.
			tempf = static_cast<float>(rand())/static_cast<float>(RAND_MAX);
			tempOnset = static_cast<int>((-logf(tempf)*invDensity) * samplerate);
			if(nextOnset > tempOnset)
				nextOnset = tempOnset;
		}
		else if(lastDensity != parameters[Density])
		{
			density = parameters[Density];

			if(density < 0.001f)
				tempf = 0.001f;
			else
				tempf = density;
			tempf = tanh((tempf*Pi)-Pi)+1.0f;
			invDensity = 1.0f/(tempf * 128.0f);

			//Update nextOnset if necessary.
			tempf = static_cast<float>(rand())/static_cast<float>(RAND_MAX);
			tempOnset = static_cast<int>((-logf(tempf)*invDensity) * samplerate);
			if(nextOnset > tempOnset)
				nextOnset = tempOnset;
		}
		lastDensity = density;

		//Write input to delay line.
		++readPosition;
		readPosition %= DelayLineSize;
		assert(readPosition < DelayLineSize);
		delayLine[0][readPosition] = input[0][i];
		delayLine[1][readPosition] = input[1][i];

		//Now clear outputs.
		tempBlock[0][i] = 0.0f;
		tempBlock[1][i] = 0.0f;

		//Check onset.
		--nextOnset;
		if(nextOnset <= 0)
		{
			for(j=0;j<MaxGrains;++j)
			{
				if(grainStarts[j] == -1)
				{
					tempf = static_cast<float>(rand())/static_cast<float>(RAND_MAX);
					grainStarts[j] = (int)(tempf * (float)DelayLineSize);
					//grainStarts[j] = readPosition;
					grainBufferStarts[j] = i;
					break;
				}
			}

			//Update nextOnset.
			tempf = static_cast<float>(rand())/static_cast<float>(RAND_MAX);
			nextOnset = static_cast<int>((-logf(tempf)*invDensity) *
										 samplerate);
		}
	}

	//Calculate grains ready to start in this block.
	for(i=0;i<MaxGrains;++i)
	{
		if(grainStarts[i] == -1)
			break;

		for(j=0;j<MaxGrains;++j)
		{
			if(!grains[j].getIsActive())
			{
				grains[j].activate(grainStarts[i],
								   grainSize,
								   increment);
				//Calculate audio for this grain.
				tempOut[0] = (tempBlock[0] + grainBufferStarts[i]);
				tempOut[1] = (tempBlock[1] + grainBufferStarts[i]);
				++numGrains;
				grains[j].setNumGrains(numGrains);
				grains[j].getBlock(tempOut, (blockSize-grainBufferStarts[i]));

				//To avoid getBlock() getting called below.
				skipGrains[j] = true;
				break;
			}
		}
	}

	//Calculate audio.
	for(j=0;j<MaxGrains;++j)
	{
		if((grains[j].getIsActive())&&(!skipGrains[j]))
		{
			grains[j].setNumGrains(numGrains);
			grains[j].getBlock(tempBlock, blockSize);
		}
	}

	//Scale amplitude according to num active grains.
	if(numGrains > 1)
	{
		/*if(increment == 1.0f)
		{
			invNumGrains = 1.0f/static_cast<float>(numGrains);
			scaleDelay = 10000;
		}
		else if(scaleDelay > 0)
		{
			invNumGrains = 1.0f/static_cast<float>(numGrains);
			--scaleDelay;
		}
		else*/
			invNumGrains = 1.0f/sqrtf(static_cast<float>(numGrains));
	}
	else
		invNumGrains = 1.0f;
	for(i=0;i<blockSize;++i)
	{
		tempBlock[0][i] *= ampScale * 2.0f;
		tempBlock[0][i] *= parameters[Level] * 2.0f;
		tempBlock[1][i] *= ampScale * 2.0f;
		tempBlock[1][i] *= parameters[Level] * 2.0f;
		if(ampScale != invNumGrains)
			ampScale += (invNumGrains - ampScale) * 0.0625f;
	}

	return tempBlock;
}

//------------------------------------------------------------------------------
void Granulator::parameterChanged(VstInt32 index, float val)
{
	float tempf;
	int tempOnset;
	float samplerate = getSamplerate();

	if(index == paramIds[DensityMW1])
		parameters[DensityMW1] = val;
	else if(index == paramIds[DensityMW2])
		parameters[DensityMW2] = val;
	else if(index == paramIds[DensityMW3])
		parameters[DensityMW3] = val;
	else if(index == paramIds[Density])
	{
		parameters[Density] = val;

		if(parameters[Density] < 0.001f)
			tempf = 0.001f;
		else
			tempf = parameters[Density];
		tempf = tanh((tempf*Pi)-Pi)+1.0f;
		invDensity = 1.0f/(tempf * 128.0f);

		//Update nextOnset if necessary.
		tempf = static_cast<float>(rand())/static_cast<float>(RAND_MAX);
		tempOnset = static_cast<int>((-logf(tempf)*invDensity) * samplerate);
		if(nextOnset > tempOnset)
			nextOnset = tempOnset;
	}
	else if(index == paramIds[DurationMW1])
		parameters[DurationMW1] = val;
	else if(index == paramIds[DurationMW2])
		parameters[DurationMW2] = val;
	else if(index == paramIds[DurationMW3])
		parameters[DurationMW3] = val;
	else if(index == paramIds[Duration])
	{
		parameters[Duration] = val;

		if(!getSyncMode())
		{
			if(parameters[Duration] < 0.001f)
				tempf = 0.001f;
			else
				tempf = parameters[Duration];
			tempf = tanh((tempf*Pi)-Pi)+1.0f;
			grainSize = static_cast<int>(tempf * DelayLineSize * 0.5f);
		}
		else
		{
			tempf = getBeatValue(parameters[Duration]);

			grainSize = static_cast<int>((tempf/getTempo())*getSamplerate());
		}
	}
	else if(index == paramIds[PitchMW1])
		parameters[PitchMW1] = val;
	else if(index == paramIds[PitchMW2])
		parameters[PitchMW2] = val;
	else if(index == paramIds[PitchMW3])
		parameters[PitchMW3] = val;
	else if(index == paramIds[Pitch])
	{
		parameters[Pitch] = val;

		if(val < 0.5f)
			increment = (val * 1.5f)+0.25f; //0.25->1.0
		else if(val > 0.5f)
			increment = ((val - 0.5f)*6.0f)+1.0f; //1.0->4.0
		else
			increment = 1.0f;
	}
	else if(index == paramIds[Level])
		parameters[Level] = val;
}

//------------------------------------------------------------------------------
float Granulator::getValue(VstInt32 index)
{
	float retval = 0.0f;

	if(index == paramIds[DensityMW1])
		retval = parameters[DensityMW1];
	else if(index == paramIds[DensityMW2])
		retval = parameters[DensityMW2];
	else if(index == paramIds[DensityMW3])
		retval = parameters[DensityMW3];
	else if(index == paramIds[Density])
		retval = parameters[Density];
	else if(index == paramIds[DurationMW1])
		retval = parameters[DurationMW1];
	else if(index == paramIds[DurationMW2])
		retval = parameters[DurationMW2];
	else if(index == paramIds[DurationMW3])
		retval = parameters[DurationMW3];
	else if(index == paramIds[Duration])
		retval = parameters[Duration];
	else if(index == paramIds[PitchMW1])
		retval = parameters[PitchMW1];
	else if(index == paramIds[PitchMW2])
		retval = parameters[PitchMW2];
	else if(index == paramIds[PitchMW3])
		retval = parameters[PitchMW3];
	else if(index == paramIds[Pitch])
		retval = parameters[Pitch];
	else if(index == paramIds[Level])
		retval = parameters[Level];

	return retval;
}

//------------------------------------------------------------------------------
string Granulator::getTextValue(VstInt32 index)
{
	ostringstream tempstr;

	if(index == paramIds[DensityMW1])
		tempstr << parameters[DensityMW1];
	else if(index == paramIds[DensityMW2])
		tempstr << parameters[DensityMW2];
	else if(index == paramIds[DensityMW3])
		tempstr << parameters[DensityMW3];
	else if(index == paramIds[Density])
		tempstr << parameters[Density];
	else if(index == paramIds[DurationMW1])
		tempstr << parameters[DurationMW1];
	else if(index == paramIds[DurationMW2])
		tempstr << parameters[DurationMW2];
	else if(index == paramIds[DurationMW3])
		tempstr << parameters[DurationMW3];
	else if(index == paramIds[Duration])
	{
		if(!getSyncMode())
			tempstr << parameters[Duration] * 2.0f;
		else
		{
			tempstr.precision(2);
			tempstr << getBeatValue(parameters[Duration]);
		}
	}
	else if(index == paramIds[PitchMW1])
		tempstr << parameters[PitchMW1];
	else if(index == paramIds[PitchMW2])
		tempstr << parameters[PitchMW2];
	else if(index == paramIds[PitchMW3])
		tempstr << parameters[PitchMW3];
	else if(index == paramIds[Pitch])
	{
		/*if(parameters[Pitch] < 0.5f)
			tempstr << (parameters[Pitch] * 1.5f)+0.25f; //0.25->1.0
		else if(parameters[Pitch] > 0.5f)
			tempstr << ((parameters[Pitch] - 0.5f)*6.0f)+1.0f; //1.0->4.0
		else
			tempstr << 1.0f;*/
		tempstr << (parameters[Pitch] * 48.0f)-24.0f;
	}
	else if(index == paramIds[Level])
		tempstr << parameters[Level] * 2.0f;

	return tempstr.str();
}
