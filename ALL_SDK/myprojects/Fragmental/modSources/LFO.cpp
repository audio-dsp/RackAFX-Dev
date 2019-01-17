//	LFO.cpp - An LFO made up of up to 32 steps.
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

#include "LFO.h"
#include "../VstPlugin.h"

#include <sstream>
#include <cmath>
#include <iomanip>

using namespace std;

//------------------------------------------------------------------------------
LFO::LFO(VstPlugin *plugin):
ModType(plugin),
numSteps(32),
freq(0.5f),
smoothing(0.0f),
freqRange(false),
pos(0.0f),
samplerate(44100.0f),
storedFreq(0.5f),
plug(plugin)
{
	int i;

	paramManager = plugin->getParameterManager();

	paramIds[NumSteps] = paramManager->registerParameter(this, "LFO st", " ");
	paramIds[Preset] = paramManager->registerParameter(this, "LFO pr", " ");
	paramIds[Freq] = paramManager->registerParameter(this, "LFO fq", "Hz");
	paramIds[Smooth] = paramManager->registerParameter(this, "LFO sm", " ");
	paramIds[FreqRange] = paramManager->registerParameter(this, "LFO fr", " ");
	for(i=Step0;i<NumParameters;++i)
		paramIds[i] = paramManager->registerParameter(this, "LFO st", " ");

	for(i=0;i<32;++i)
		steps[i] = 0.5f;

	expCoeff = 1.0f - (1.0f/0.1f);
}

//------------------------------------------------------------------------------
LFO::~LFO()
{
	
}

//------------------------------------------------------------------------------
void LFO::getBlock(float *block,
				   VstInt32 blockSize,
				   VstInt32 barStart,
				   float ppq)
{
	VstInt32 i;
	int tempint;
	int x0, x1, x2, x3;
	float interp, nonInterp;
	double increment = (1.0/(double)samplerate) * (double)freq;
	double tempo = (double)plug->getTempo()/60.0;
	bool thisBlock = true;
	double beatValue = (double)getBeatValue(storedFreq);

	if(plug->isSynced() && (!freqRange))
	{
		increment = (1.0/(double)samplerate) * (1.0/(beatValue/tempo));

		//Make sure we allow the LFO to run its full length, if it's set to
		//run for longer than one bar.
		if(beatValue > ppq)
		{
			if((1.0-(pos + (increment*(double)(blockSize-1)))) <
			   (beatValue-(double)ppq)/beatValue)
			{
				thisBlock = false;
			}
		}
	}

	for(i=0;i<blockSize;++i)
	{
		pos += increment;
		if(pos > 1.0)
			pos = fmod(pos, 1.0);

		//Sync to bar start if necesary.
		if(thisBlock && plug->isSynced())
		{
			--barStart;

			if(barStart == 0)
				pos = 0.0f;
		}

		tempint = static_cast<int>(pos * numSteps);
		x0 = tempint-1;
		if(x0 < 0)
			x0 = numSteps-1;
		x1 = tempint;
		x2 = tempint+1;
		if(x2 > numSteps-1)
			x2 = 0;
		x3 = tempint+2;
		if(x3 > numSteps-1)
			x3 %= numSteps;
		nonInterp = steps[tempint];
		interp = bSplineInterp(((pos*(float)numSteps)-(float)tempint),
							   steps[x0],
							   steps[x1],
							   steps[x2],
							   steps[x3]);

		block[i] = ((1.0f-smoothing)*nonInterp) + (smoothing*interp);
	}
}

//------------------------------------------------------------------------------
void LFO::parameterChanged(VstInt32 index, float val)
{
	int i;

	if(index == paramIds[NumSteps])
	{
		int tempint = static_cast<int>(val);

		switch(tempint)
		{
			case 0:
				numSteps = 32;
				break;
			case 1:
				numSteps = 16;
				break;
			case 2:
				numSteps = 12;
				break;
			case 3:
				numSteps = 8;
				break;
			case 4:
				numSteps = 6;
				break;
			case 5:
				numSteps = 4;
				break;
			case 6:
				numSteps = 3;
				break;
			case 7:
				numSteps = 2;
				break;
		}
	}
	else if(index == paramIds[Freq])
	{
		storedFreq = val;
		if(freqRange)
			freq = (fakeExp(val) * 20000.0f)+20.0f;
		else
			freq = (fakeExp(val) * 19.9999f)+0.0001f;
	}
	else if(index == paramIds[Smooth])
		smoothing = val;
	else if(index == paramIds[FreqRange])
	{
		if(val < 0.5f)
		{
			if(freqRange)
			{
				//float tempf = (freq - 20.0f)/20000.0f;
				freq = (fakeExp(storedFreq) * 19.9999f)+0.0001f;
			}
			freqRange = false;
		}
		else
		{
			if(!freqRange)
			{
				//float tempf = (freq - 0.0001f)/19.9999f;
				freq = (fakeExp(storedFreq) * 20000.0f)+20.0f;
			}
			freqRange = true;
		}
	}
	else if(index <= paramIds[Step31])
	{
		for(i=Step0;i<NumParameters;++i)
		{
			if(index == paramIds[i])
			{
				steps[i-Step0] = val;
				break;
			}
		}
	}
}

//------------------------------------------------------------------------------
void LFO::setSamplerate(float rate)
{
	samplerate = rate;

	//This gets called in resume, so it makes sense to reset pos here.
	pos = 0.0f;
}

//------------------------------------------------------------------------------
float LFO::getValue(VstInt32 index)
{
	int i;
	float retval = 0.0f;

	if(index == paramIds[NumSteps])
	{
		//retval = static_cast<float>(numSteps)/32.0f;
		//retval = static_cast<float>(numSteps);
		switch(numSteps)
		{
			case 32:
				retval = 0.0f;
				break;
			case 16:
				retval = 1.0f;
				break;
			case 12:
				retval = 2.0f;
				break;
			case 8:
				retval = 3.0f;
				break;
			case 6:
				retval = 4.0f;
				break;
			case 4:
				retval = 5.0f;
				break;
			case 3:
				retval = 6.0f;
				break;
			case 2:
				retval = 7.0f;
				break;
		}
	}
	else if(index == paramIds[Freq])
	{
		/*if(freqRange)
			retval = (freq - 20.0f)/20000.0f;
		else
			retval = (freq - 0.0001f)/19.9999f;*/
		retval = storedFreq;
	}
	else if(index == paramIds[Smooth])
		retval = smoothing;
	else if(index == paramIds[FreqRange])
	{
		if(freqRange)
			retval = 1.0f;
		else
			retval = 0.0f;
	}
	else if(index <= paramIds[Step31])
	{
		for(i=Step0;i<NumParameters;++i)
		{
			if(index == paramIds[i])
			{
				retval = steps[i-Step0];
				break;
			}
		}
	}

	return retval;
}

//------------------------------------------------------------------------------
string LFO::getTextValue(VstInt32 index)
{
	int i;
	stringstream tempConverter;

	if(index == paramIds[NumSteps])
		tempConverter << numSteps;
	else if(index == paramIds[Freq])
	{
		if(plug->isSynced() && (!freqRange))
			tempConverter << setprecision(2) << getBeatValue(storedFreq);
		else if(!freqRange)
		{
			if(freq < 1.0f)
				tempConverter << setprecision(2) << freq;
			else if(freq < 10.0f)
				tempConverter << setprecision(3) << freq;
			else
				tempConverter << setprecision(4) << freq;
		}
		else
		{
			if(freq < 100.0f)
				tempConverter << setprecision(3) << freq;
			else
				tempConverter << setprecision(5) << freq;
		}
	}
	else if(index == paramIds[Smooth])
		tempConverter << smoothing;
	else if(index == paramIds[FreqRange])
	{
		if(freqRange)
			tempConverter << "audio";
		else
			tempConverter << "low";
	}
	else if(index <= paramIds[Step31])
	{
		for(i=Step0;i<NumParameters;++i)
			tempConverter << steps[i-Step0];
	}

	return tempConverter.str();
}
