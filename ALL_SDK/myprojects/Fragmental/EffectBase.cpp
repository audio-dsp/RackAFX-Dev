//	EffectBase.cpp - Abstract base class for the individual effect classes.
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

#include "EffectBase.h"

//------------------------------------------------------------------------------
EffectBase::EffectBase(VstPlugin *plugin):
tempBlockSize(0),
samplerate(44100.0f),
tempo(120.0f),
syncMode(false)
{
	tempBlock[0] = 0;
	tempBlock[1] = 0;
}

//------------------------------------------------------------------------------
EffectBase::~EffectBase()
{
	if(tempBlockSize > 0)
	{
		delete [] tempBlock[0];
		delete [] tempBlock[1];
	}
}

//------------------------------------------------------------------------------
void EffectBase::setBlockSize(VstInt32 newSize)
{
	int i;

	if(tempBlockSize > 0)
	{
		delete [] tempBlock[0];
		delete [] tempBlock[1];
	}

	tempBlockSize = newSize;
	tempBlock[0] = new float[tempBlockSize];
	tempBlock[1] = new float[tempBlockSize];

	for(i=0;i<tempBlockSize;++i)
	{
		tempBlock[0][i] = 0.0f;
		tempBlock[1][i] = 0.0f;
	}
}

//------------------------------------------------------------------------------
void EffectBase::setSamplerate(float newRate)
{
	samplerate = newRate;
	samplerateChanged();
}

//------------------------------------------------------------------------------
void EffectBase::setTempo(float val)
{
	tempo = val;
}

//------------------------------------------------------------------------------
void EffectBase::setSyncMode(bool val)
{
	syncMode = val;
}
