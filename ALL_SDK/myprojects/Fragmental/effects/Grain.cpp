//	Grain.cpp - Simple class representing a single grain.
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

#include "Grain.h"
#include <cassert>
#include <cmath>

//-----------------------------------------------------------------------------
Grain::Grain():
isActive(false),
buffer(0),
bufferSize(0),
index(0.0f),
grainIndex(0.0f),
increment(1.0f),
startPosition(0),
grainSize(0),
numGrains(1.0f)
{
	
}

//-----------------------------------------------------------------------------
Grain::~Grain()
{
	
}

//-----------------------------------------------------------------------------
void Grain::setDelayLine(float **delayLine, int size)
{
	buffer = delayLine;
	bufferSize = size;
}

//-----------------------------------------------------------------------------
void Grain::activate(int startPos, int size, float inc)
{
	grainSize = size;
	increment = inc;
	//If the pitch is sped up, we want to try and avoid reading past the write
	//point in the buffer.
	if(increment > 1.0f)
	{
		startPosition = static_cast<int>(startPos - ((increment-1.0f)*
													 grainSize));
		while(startPosition < 0.0f)
			startPosition += grainSize;
	}
	else
		startPosition = startPos;
	index = 0.0f;
	grainIndex = 0.0f;
	isActive = true;
}

//-----------------------------------------------------------------------------
void Grain::getBlock(float **audioBlock, int numSamples)
{
	int i;
	int tempint;
	float sectionSize, invSectionSize;
	float amp;
	float invNumGrains;

	if(numGrains > 4.0f)
		invNumGrains = 1.0f/(numGrains*0.25f);
	else
		invNumGrains = 1.0f;

	sectionSize = static_cast<float>(grainSize)/3.0f;
	invSectionSize = 1.0f/sectionSize;
	for(i=0;i<numSamples;++i)
	{
		//Calculate envelopes.
		if(grainIndex < sectionSize)
			amp = (grainIndex * invSectionSize);
		else if(grainIndex >= grainSize)
			isActive = false;
		else if(grainIndex > (sectionSize * 2.0f))
			amp = (grainSize-grainIndex) * invSectionSize;
		else
			amp = 1.0f;

		//Generate audio.
		if(isActive)
		{
			tempint = (startPosition+static_cast<int>(index)) % bufferSize;
			audioBlock[0][i] += buffer[0][tempint] * amp;
			audioBlock[1][i] += buffer[1][tempint] * amp;

			index += increment;
			++grainIndex;
		}
	}
}
