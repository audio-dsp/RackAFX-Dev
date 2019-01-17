//	SawTest.cpp - Simple sawtooth oscillator ModType test.
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

#include "SawTest.h"

#include <cmath>

//-----------------------------------------------------------------------------
SawTest::SawTest(VstPlugin *plugin):
ModType(plugin),
index(0.0f)
{
	
}

//-----------------------------------------------------------------------------
SawTest::~SawTest()
{
	
}

//-----------------------------------------------------------------------------
void SawTest::getBlock(float *block,
					   VstInt32 blockSize,
					   VstInt32 barStart,
					   float ppq)
{
	VstInt32 i;

	for(i=0;i<blockSize;++i)
	{
		block[i] = index;

		index += 1.0f/44100.0f;
		while(index > 1.0f)
			index -= 1.0f;
	}
}
