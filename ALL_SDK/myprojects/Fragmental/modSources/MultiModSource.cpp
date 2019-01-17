//	MultiModSource.cpp - ModulationSource which can have multiple
//						 characteristics.
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

#include "MultiModSource.h"
#include "LFO.h"
#include "EnvelopeFollower.h"
#include "PhysModKnob.h"

//-----------------------------------------------------------------------------
MultiModSource::MultiModSource(VstPlugin *plugin):
ModulationSource(plugin),
modIndex(0)
{
	modTypes[0] = new LFO(plugin);
	modTypes[1] = new EnvelopeFollower(plugin);
	modTypes[2] = new PhysModKnob(plugin);
}

//-----------------------------------------------------------------------------
MultiModSource::~MultiModSource()
{
	for(int i=0;i<3;++i)
		delete modTypes[i];
}

//-----------------------------------------------------------------------------
void MultiModSource::changeModType(unsigned int val)
{
	if(val < 3)
		modIndex = val;
	else
		modIndex = 2;
}

//-----------------------------------------------------------------------------
void MultiModSource::generateBlock(VstInt32 blockSize,
								   VstInt32 barStart,
								   float ppq)
{
	modTypes[modIndex]->getBlock(tempBlock, blockSize, barStart, ppq);
}

//-----------------------------------------------------------------------------
void MultiModSource::setBlockSize(VstInt32 newSize)
{
	
}

//-----------------------------------------------------------------------------
void MultiModSource::setSamplerate(float newRate)
{
	for(int i=0;i<3;++i)
		modTypes[i]->setSamplerate(newRate);
}

//-----------------------------------------------------------------------------
float MultiModSource::getKnobPos() const
{
	PhysModKnob *knob = dynamic_cast<PhysModKnob *>(modTypes[2]);

	if(knob)
		return knob->getKnobPos();
	else
		return 0.0f;
}
