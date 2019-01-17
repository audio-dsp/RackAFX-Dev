//	Reverse.cpp - Reversinator effect.
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

#include "Reverse.h"
#include "../VstPlugin.h"
#include <cmath>
#include <sstream>

using namespace std;

//-----------------------------------------------------------------------------
Reverse::Reverse(VstPlugin *plugin):
EffectBase(plugin),
bufferSize(88200),
cosBuffer(0),
writePointer(0),
readPointer1(44100),
readPointer2(88199),
readOffset(0)
{
	int i;

	buffer[0] = new float[bufferSize];
	buffer[1] = new float[bufferSize];
	cosBuffer = new float[bufferSize];

	for(i=0;i<bufferSize;++i)
	{
		buffer[0][i] = 0.0f;
		buffer[1][i] = 0.0f;
		cosBuffer[i] = cosf(((float)i/(float)bufferSize) * 4.0f * Pi);
		cosBuffer[i] += 1.0f;
		cosBuffer[i] *= 0.5f;
	}

	paramManager = plugin->getParameterManager();
	paramIds[DirectionMW1] = paramManager->registerParameter(this, "DirectionMW1", " ");
	paramIds[DirectionMW2] = paramManager->registerParameter(this, "DirectionMW2", " ");
	paramIds[DirectionMW3] = paramManager->registerParameter(this, "DirectionMW3", " ");
	paramIds[Direction] = paramManager->registerParameter(this, "Direction", " ");
	paramIds[Level] = paramManager->registerParameter(this, "Level", " ");

	modManager = plugin->getModulationManager();
}

//-----------------------------------------------------------------------------
Reverse::~Reverse()
{
	delete [] buffer[0];
	delete [] buffer[1];
	delete [] cosBuffer;
}

//-----------------------------------------------------------------------------
float **Reverse::getBlock(float **input, VstInt32 blockSize)
{
	VstInt32 i;
	float *modBuffers[3];
	float modAmt; //The amount by which the direction parameter is modulated.
	float direction; //The up-to-date parameter values.
	float lastDir = parameters[Direction]; //The previous direction value.

	critSec.enter();

	//Get modBuffers.
	for(i=0;i<3;++i)
		modBuffers[i] = (*modManager)[i];

	for(i=0;i<blockSize;++i)
	{
		//Update modAmts.
		modAmt = parameters[DirectionMW1] +
				 parameters[DirectionMW2] +
				 parameters[DirectionMW3];

		//Update Direction.
		if(modAmt > 0.0f)
		{
			direction = parameters[Direction] * (1.0f-modAmt);
			direction += parameters[DirectionMW1] * modBuffers[0][i];
			direction += parameters[DirectionMW2] * modBuffers[1][i];
			direction += parameters[DirectionMW3] * modBuffers[2][i];
		}
		else
			direction = parameters[Direction];

		//Update pointers etc.
		if((direction >= 0.5f) && (lastDir < 0.5f))
		{
			readOffset = writePointer - readPointer2;
			if(readOffset < 0)
				readOffset += bufferSize;
			readPointer1 = writePointer - readOffset - (bufferSize/2);
			while(readPointer1 < 0)
				readPointer1 += bufferSize;

			readPointer2 = writePointer - readOffset - 1;
			while(readPointer2 < 0)
				readPointer2 += bufferSize;
		}
		lastDir = direction;

		buffer[0][writePointer] = input[0][i];
		buffer[1][writePointer] = input[1][i];
		++writePointer;
		writePointer %= bufferSize;
		/*if(writePointer >= bufferSize)
			writePointer = 0;*/

		if(direction < 0.5f)
		{
			tempBlock[0][i] = input[0][i] * parameters[Level] * 2.0f;
			tempBlock[1][i] = input[1][i] * parameters[Level] * 2.0f;
		}
		else
		{
			tempBlock[0][i] = buffer[0][readPointer1] * cosBuffer[readPointer1];
			tempBlock[0][i] += buffer[0][readPointer2] * (1.0f - cosBuffer[readPointer2]);
			tempBlock[0][i] *= parameters[Level] * 2.0f;
			tempBlock[1][i] = buffer[1][readPointer1] * cosBuffer[readPointer1];
			tempBlock[1][i] += buffer[1][readPointer2] * (1.0f - cosBuffer[readPointer2]);
			tempBlock[1][i] *= parameters[Level] * 2.0f;

			--readPointer1;
			if(readPointer1 < 0)
				readPointer1 += bufferSize;

			--readPointer2;
			if(readPointer2 < 0)
				readPointer2 += bufferSize;
		}
	}

	critSec.leave();

	return tempBlock;
}

//-----------------------------------------------------------------------------
void Reverse::parameterChanged(VstInt32 index, float val)
{
	if(index == paramIds[DirectionMW1])
		parameters[DirectionMW1] = val;
	else if(index == paramIds[DirectionMW2])
		parameters[DirectionMW2] = val;
	else if(index == paramIds[DirectionMW3])
		parameters[DirectionMW3] = val;
	else if(index == paramIds[Direction])
	{
		if((val >= 0.5f) && (parameters[Direction] < 0.5f))
		{
			critSec.enter();

			//readOffset = abs(writePointer - readPointer2);
			readOffset = writePointer - readPointer2;
			if(readOffset < 0)
				readOffset += bufferSize;
			/*readPointer1 = (writePointer + (bufferSize/2)) % bufferSize;
			readPointer2 = (writePointer-1) < 0 ?
						     readPointer2 = (bufferSize-1) :
						     readPointer2 = (writePointer-1);*/
			readPointer1 = writePointer - readOffset - (bufferSize/2);
			while(readPointer1 < 0)
				readPointer1 += bufferSize;

			readPointer2 = writePointer - readOffset - 1;
			while(readPointer2 < 0)
				readPointer2 += bufferSize;

			critSec.leave();
		}
		parameters[Direction] = val;
	}
	else if(index == paramIds[Level])
		parameters[Level] = val;
}

//-----------------------------------------------------------------------------
float Reverse::getValue(VstInt32 index)
{
	float retval = 0.0f;

	if(index == paramIds[DirectionMW1])
		retval = parameters[DirectionMW1];
	else if(index == paramIds[DirectionMW2])
		retval = parameters[DirectionMW2];
	else if(index == paramIds[DirectionMW3])
		retval = parameters[DirectionMW3];
	else if(index == paramIds[Direction])
		retval = parameters[Direction];
	else if(index == paramIds[Level])
		retval = parameters[Level];

	return retval;
}

//-----------------------------------------------------------------------------
string Reverse::getTextValue(VstInt32 index)
{
	ostringstream tempstr;

	if(index == paramIds[DirectionMW1])
		tempstr << parameters[DirectionMW1];
	else if(index == paramIds[DirectionMW2])
		tempstr << parameters[DirectionMW2];
	else if(index == paramIds[DirectionMW3])
		tempstr << parameters[DirectionMW3];
	else if(index == paramIds[Direction])
	{
		if(parameters[Direction] < 0.5f)
			tempstr << "backward";
		else
			tempstr << "forward";
	}
	else if(index == paramIds[Level])
		tempstr << (parameters[Level] * 2.0f);

	return tempstr.str();
}

//-----------------------------------------------------------------------------
void Reverse::samplerateChanged()
{
	int i;

	delete [] buffer[0];
	delete [] buffer[1];
	delete [] cosBuffer;

	bufferSize = static_cast<VstInt64>(getSamplerate()*2.0f);

	buffer[0] = new float[bufferSize];
	buffer[1] = new float[bufferSize];
	cosBuffer = new float[bufferSize];

	for(i=0;i<bufferSize;++i)
	{
		buffer[0][i] = 0.0f;
		buffer[1][i] = 0.0f;
		cosBuffer[i] = cosf(((float)i/(float)bufferSize) * 4.0f * Pi);
		cosBuffer[i] += 1.0f;
		cosBuffer[i] *= 0.5f;
	}
}
