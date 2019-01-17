//	LFO.cpp - LFO classes, so we can use polymorphism to avoid big switch-case
//			  statements.
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

#include "LFO.h"
#include <cmath>
#include <cstdlib>
#include <time.h>

//----------------------------------------------------------------------------
LFOBase::LFOBase(float samplerate):
increment(1.0f),
index(0.0f)
{
	this->samplerate = samplerate;
}

void LFOBase::setSamplerate(float val)
{
	float tempfreq;

	//We use tempfreq to re-calculate the increment for the new samplerate.
	tempfreq = (increment*samplerate)/static_cast<float>(TWOPI);
	//Set the new samplerate...
	samplerate = val;
	//...and reset the increment.
	setRate(tempfreq);
}

void LFOBase::setRate(float val)
{
	increment = (val/samplerate)*static_cast<float>(TWOPI);
}

void LFOBase::setPhase(float val)
{
	index = static_cast<float>(fmod(static_cast<double>(val), TWOPI));
}

float LFOBase::getRate()
{
	return (increment*samplerate)/static_cast<float>(TWOPI);
}

//----------------------------------------------------------------------------
LFOSine::LFOSine(float samplerate):
LFOBase(samplerate)
{
	
}

float LFOSine::getSample()
{
	float retval = sinf(index);

	index += increment;
	if(index > static_cast<float>(TWOPI))
		index = static_cast<float>(fmod(static_cast<double>(index), TWOPI));

	return retval;
}

//----------------------------------------------------------------------------
LFOSaw::LFOSaw(float samplerate):
LFOBase(samplerate)
{
	
}

float LFOSaw::getSample()
{
	float retval;

	retval = 1.0f - (index*static_cast<float>(INV_TWOPI));
	retval *= 2.0f;
	retval -= 1.0f;

	index += increment;
	if(index > static_cast<float>(TWOPI))
		index = static_cast<float>(fmod(static_cast<double>(index), TWOPI));

	return retval;
}

//----------------------------------------------------------------------------
LFORamp::LFORamp(float samplerate):
LFOBase(samplerate)
{
	
}

float LFORamp::getSample()
{
	float retval;

	retval = (index*static_cast<float>(INV_TWOPI));
	retval *= 2.0f;
	retval -= 1.0f;

	index += increment;
	if(index > static_cast<float>(TWOPI))
		index = static_cast<float>(fmod(static_cast<double>(index), TWOPI));

	return retval;
}

//----------------------------------------------------------------------------
LFOSquare::LFOSquare(float samplerate):
LFOBase(samplerate)
{
	
}

float LFOSquare::getSample()
{
	float retval;

	if(index < PI)
		retval = 1.0f;
	else
		retval = -1.0f;

	index += increment;
	if(index > static_cast<float>(TWOPI))
		index = static_cast<float>(fmod(static_cast<double>(index), TWOPI));

	return retval;
}

//----------------------------------------------------------------------------
LFOSH::LFOSH(float samplerate):
LFOBase(samplerate),
lastval(0.0f)
{
	//So we (hopefully) don't always start at the same value...
	srand(time(NULL));
}

float LFOSH::getSample()
{
	// hmm... is this what I mean?
	if((index >= PI)&&((index-increment) < PI))
		lastval = -static_cast<float>(rand())/static_cast<float>(RAND_MAX);

	index += increment;
	if(index > static_cast<float>(TWOPI))
	{
		index = static_cast<float>(fmod(static_cast<double>(index), TWOPI));
		lastval = static_cast<float>(rand())/static_cast<float>(RAND_MAX);
	}

	return lastval;
}