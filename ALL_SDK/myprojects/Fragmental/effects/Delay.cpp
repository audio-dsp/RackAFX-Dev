//	Delay.cpp - Simple delay effect.
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

#include "Delay.h"
#include "../VstPlugin.h"
#include <cmath>
#include <sstream>

using namespace std;

//------------------------------------------------------------------------------
Delay::Delay(VstPlugin *plugin):
EffectBase(plugin),
modManager(0),
bufferSize(88200),
writePointer(0),
currentDelayL(0.0f),
currentDelayR(0.0f)
{
	int i;

	buffer[0] = new float[bufferSize];
	buffer[1] = new float[bufferSize];

	for(i=0;i<bufferSize;++i)
	{
		buffer[0][i] = 0.0f;
		buffer[1][i] = 0.0f;
	}

	paramManager = plugin->getParameterManager();
	paramIds[DelayLMW1] = paramManager->registerParameter(this, "DelayLMW1", " ");
	paramIds[DelayLMW2] = paramManager->registerParameter(this, "DelayLMW2", " ");
	paramIds[DelayLMW3] = paramManager->registerParameter(this, "DelayLMW3", " ");
	paramIds[DelayL] = paramManager->registerParameter(this, "DelayL", "s");
	paramIds[DelayRMW1] = paramManager->registerParameter(this, "DelayRMW1", " ");
	paramIds[DelayRMW2] = paramManager->registerParameter(this, "DelayRMW2", " ");
	paramIds[DelayRMW3] = paramManager->registerParameter(this, "DelayRMW3", " ");
	paramIds[DelayR] = paramManager->registerParameter(this, "DelayR", "s");
	paramIds[ShortTimes] = paramManager->registerParameter(this, "ShortTimes", " ");
	paramIds[Feedback] = paramManager->registerParameter(this, "Feedback", " ");
	paramIds[Mix] = paramManager->registerParameter(this, "D. Mix", " ");
	paramIds[Level] = paramManager->registerParameter(this, "D. Level", " ");

	modManager = plugin->getModulationManager();
}

//------------------------------------------------------------------------------
Delay::~Delay()
{
	delete [] buffer[0];
	delete [] buffer[1];
}

//------------------------------------------------------------------------------
float **Delay::getBlock(float **input, VstInt32 blockSize)
{
	VstInt32 i;
	float tempf0, tempf1;
	float readIndex;
	float readFrac;
	VstInt64 indexInt;
	VstInt64 readNext;
	float tempDelay;
	float bufferSizeF = static_cast<float>(bufferSize);
	float *modBuffers[3];
	float modAmts[2]; //The amount by which the 2 delay parameters are
					  //modulated.
	float leftDelay, rightDelay; //The up-to-date delay parameter values.
	float tempo = getTempo()/60.0f; //Converted to beats-per-second.
	bool syncMode = getSyncMode();
	float samplerate = getSamplerate();

	//Get modBuffers.
	for(i=0;i<3;++i)
		modBuffers[i] = (*modManager)[i];

	//Calculate audio.
	for(i=0;i<blockSize;++i)
	{
		//Update modAmts.
		modAmts[0] = fakeExp(parameters[DelayLMW1]) +
					 fakeExp(parameters[DelayLMW2]) +
					 fakeExp(parameters[DelayLMW3]);
		modAmts[1] = fakeExp(parameters[DelayRMW1]) +
					 fakeExp(parameters[DelayRMW2]) +
					 fakeExp(parameters[DelayRMW3]);

		//Update left and rightDelays.
		if(modAmts[0] > 0.0f)
		{
			leftDelay = parameters[DelayL] * (1.0f-modAmts[0]);
			leftDelay += parameters[DelayLMW1] * modBuffers[0][i];
			leftDelay += parameters[DelayLMW2] * modBuffers[1][i];
			leftDelay += parameters[DelayLMW3] * modBuffers[2][i];
		}
		else
			leftDelay = parameters[DelayL];
		if(modAmts[1] > 0.0f)
		{
			rightDelay = parameters[DelayR] * (1.0f-modAmts[1]);
			rightDelay += parameters[DelayRMW1] * modBuffers[0][i];
			rightDelay += parameters[DelayRMW2] * modBuffers[1][i];
			rightDelay += parameters[DelayRMW3] * modBuffers[2][i];
		}
		else
			rightDelay = parameters[DelayR];

		if(leftDelay <= 0.0f)
			leftDelay = 0.000001f;
		if(rightDelay <= 0.0f)
			rightDelay = 0.000001f;

		//Convert to beats if necessary.
		if((syncMode) && (parameters[ShortTimes] < 0.5f))
		{
			leftDelay = getBeatValue(leftDelay)/tempo;
			leftDelay *= samplerate;
			if(leftDelay >= bufferSizeF)
				leftDelay = bufferSizeF;
			rightDelay = getBeatValue(rightDelay)/tempo;
			rightDelay *= samplerate;
			if(rightDelay >= bufferSizeF)
				rightDelay = bufferSizeF;
		}

		//Update currentDelay.
		if(syncMode && (parameters[ShortTimes] < 0.5f))
			tempDelay = leftDelay;
		else if(parameters[ShortTimes] < 0.5f)
			tempDelay = leftDelay * bufferSizeF;
		else
			tempDelay = leftDelay * 0.01f * bufferSizeF;

		tempf0 = (tempDelay - currentDelayL) * (1.0f/1024.0f);
		if(tempf0 > 2.0f)
			tempf0 = 2.0f;
		else if(tempf0 < -2.0f)
			tempf0 = -2.0f;
		currentDelayL += tempf0;

		//Get current sample.
		readIndex = static_cast<float>(writePointer) - currentDelayL;
		while(readIndex < 0.0f)
			readIndex += bufferSizeF;
		if(readIndex > bufferSizeF)
			readIndex = 0.0f;
		indexInt = static_cast<int>(readIndex);
		readNext = (indexInt+1) % bufferSize;
		readFrac = readIndex - static_cast<float>(indexInt);
		tempf0 = buffer[0][indexInt] * (1.0f-readFrac);
		tempf0 += buffer[0][readNext] * readFrac;

		//Update currentDelay.
		if(syncMode && (parameters[ShortTimes] < 0.5f))
			tempDelay = rightDelay;
		else if(parameters[ShortTimes] < 0.5f)
			tempDelay = rightDelay * bufferSizeF;
		else
			tempDelay = rightDelay * 0.01f * bufferSizeF;

		tempf1 = (tempDelay - currentDelayR) * (1.0f/1024.0f);
		if(tempf1 > 2.0f)
			tempf1 = 2.0f;
		else if(tempf1 < -2.0f)
			tempf1 = -2.0f;
		currentDelayR += tempf1;

		//Get current sample.
		readIndex = static_cast<float>(writePointer) - currentDelayR;
		while(readIndex < 0.0f)
			readIndex += bufferSizeF;
		if(readIndex > bufferSizeF)
			readIndex = 0.0f;
		indexInt = static_cast<int>(readIndex);
		readNext = (indexInt+1) % bufferSize;
		readFrac = readIndex - static_cast<float>(indexInt);
		tempf1 = buffer[1][indexInt] * (1.0f-readFrac);
		tempf1 += buffer[1][readNext] * readFrac;

		//Update buffer.
		//buffer[0][writePointer] = tanh(input[0][i] + (tempf0 * (parameters[Feedback]*0.31415f)) * 3.1415f);
		//buffer[1][writePointer] = tanh(input[1][i] + (tempf1 * (parameters[Feedback]*0.31415f)) * 3.1415f);
		buffer[0][writePointer] = tanh(input[0][i] + (tempf0 * (parameters[Feedback]*0.35f)) * 3.1415f);
		buffer[0][writePointer] += DenormalFix;
		buffer[0][writePointer] -= DenormalFix;
		buffer[1][writePointer] = tanh(input[1][i] + (tempf1 * (parameters[Feedback]*0.35f)) * 3.1415f);
		buffer[1][writePointer] += DenormalFix;
		buffer[1][writePointer] -= DenormalFix;
		++writePointer;
		writePointer %= bufferSize;

		//Write outputs.
		tempBlock[0][i] = ((input[0][i] * (1.0f-parameters[Mix])) +
						   (tempf0 * parameters[Mix])) *
						  parameters[Level] * 2.0f;
		tempBlock[1][i] = ((input[1][i] * (1.0f-parameters[Mix])) +
						   (tempf1 * parameters[Mix])) *
						  parameters[Level] * 2.0f;

		//Sometimes the delay can go crazy for a single sample when modulated.
		//Unfortunately this tends to screw up any phase vocoders or filters
		//that come after it, so we need to limit it.  Hopefully +/-4 is a
		//reasonable limit...
		if(tempBlock[0][i] > 4.0f)
			tempBlock[0][i] = 4.0f;
		else if(tempBlock[0][i] < -4.0f)
			tempBlock[0][i] = -4.0f;

		if(tempBlock[1][i] > 4.0f)
			tempBlock[1][i] = 4.0f;
		else if(tempBlock[1][i] < -4.0f)
			tempBlock[1][i] = -4.0f;
	}

	return tempBlock;
}

//------------------------------------------------------------------------------
void Delay::samplerateChanged()
{
	int i;

	delete [] buffer[0];
	delete [] buffer[1];

	bufferSize = static_cast<VstInt64>(getSamplerate()*2.0f);

	buffer[0] = new float[bufferSize];
	buffer[1] = new float[bufferSize];

	for(i=0;i<bufferSize;++i)
	{
		buffer[0][i] = 0.0f;
		buffer[1][i] = 0.0f;
	}
}

//------------------------------------------------------------------------------
void Delay::parameterChanged(VstInt32 index, float val)
{
	if(index == paramIds[DelayL])
		parameters[DelayL] = val;
	else if(index == paramIds[DelayLMW1])
		parameters[DelayLMW1] = val;
	else if(index == paramIds[DelayLMW2])
		parameters[DelayLMW2] = val;
	else if(index == paramIds[DelayLMW3])
		parameters[DelayLMW3] = val;
	else if(index == paramIds[DelayR])
		parameters[DelayR] = val;
	else if(index == paramIds[DelayRMW1])
		parameters[DelayRMW1] = val;
	else if(index == paramIds[DelayRMW2])
		parameters[DelayRMW2] = val;
	else if(index == paramIds[DelayRMW3])
		parameters[DelayRMW3] = val;
	else if(index == paramIds[ShortTimes])
		parameters[ShortTimes] = val;
	else if(index == paramIds[Feedback])
		parameters[Feedback] = val;
	else if(index == paramIds[Mix])
		parameters[Mix] = val;
	else if(index == paramIds[Level])
		parameters[Level] = val;
}

//------------------------------------------------------------------------------
float Delay::getValue(VstInt32 index)
{
	float retval = 0.0f;

	if(index == paramIds[DelayLMW1])
		retval = parameters[DelayLMW1];
	else if(index == paramIds[DelayLMW2])
		retval = parameters[DelayLMW2];
	else if(index == paramIds[DelayLMW3])
		retval = parameters[DelayLMW3];
	else if(index == paramIds[DelayL])
		retval = parameters[DelayL];
	else if(index == paramIds[DelayRMW1])
		retval = parameters[DelayRMW1];
	else if(index == paramIds[DelayRMW2])
		retval = parameters[DelayRMW2];
	else if(index == paramIds[DelayRMW3])
		retval = parameters[DelayRMW3];
	else if(index == paramIds[DelayR])
		retval = parameters[DelayR];
	else if(index == paramIds[ShortTimes])
		retval = parameters[ShortTimes];
	else if(index == paramIds[Feedback])
		retval = parameters[Feedback];
	else if(index == paramIds[Mix])
		retval = parameters[Mix];
	else if(index == paramIds[Level])
		retval = parameters[Level];

	return retval;
}

//------------------------------------------------------------------------------
string Delay::getTextValue(VstInt32 index)
{
	ostringstream tempstr;

	if(index == paramIds[DelayLMW1])
		tempstr << parameters[DelayLMW1];
	else if(index == paramIds[DelayLMW2])
		tempstr << parameters[DelayLMW2];
	else if(index == paramIds[DelayLMW3])
		tempstr << parameters[DelayLMW3];
	else if(index == paramIds[DelayL])
	{
		if(parameters[ShortTimes] < 0.5f)
		{
			tempstr.precision(2);
			if(getSyncMode())
				tempstr << getBeatValue(parameters[DelayL]);
			else
				tempstr << (parameters[DelayL] * 2.0f);
		}
		else
			tempstr << (parameters[DelayL] * 0.01f);
	}
	else if(index == paramIds[DelayRMW1])
		tempstr << parameters[DelayRMW1];
	else if(index == paramIds[DelayRMW2])
		tempstr << parameters[DelayRMW2];
	else if(index == paramIds[DelayRMW3])
		tempstr << parameters[DelayRMW3];
	else if(index == paramIds[DelayR])
	{
		if(parameters[ShortTimes] < 0.5f)
		{
			tempstr.precision(2);
			if(getSyncMode())
				tempstr << getBeatValue(parameters[DelayR]);
			else
				tempstr << (parameters[DelayR] * 2.0f);
		}
		else
			tempstr << (parameters[DelayR] * 0.01f);
	}
	else if(index == paramIds[ShortTimes])
		tempstr << parameters[ShortTimes];
	else if(index == paramIds[Feedback])
		tempstr << parameters[Feedback];
	else if(index == paramIds[Mix])
		tempstr << parameters[Mix];
	else if(index == paramIds[Level])
		tempstr << (parameters[Level] * 2.0f);

	return tempstr.str();
}
