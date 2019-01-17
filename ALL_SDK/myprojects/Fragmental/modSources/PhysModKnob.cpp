//	PhysModKnob.cpp - A knob which acts as a damped mass-spring.
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

#include "PhysModKnob.h"
#include "../VstPlugin.h"

#include <sstream>

using namespace std;

//-----------------------------------------------------------------------------
PhysModKnob::PhysModKnob(VstPlugin *plugin):
ModType(plugin),
intendedPos(0.5f),
mass(0.01f),
damping(0.0f),
stiffness(10.0f),
stiffnessAndMass(1.0f),
samplePeriod(1.0f/44100.0f),
amplitude(0.5f)
{
	paramManager = plugin->getParameterManager();

	paramIds[Mass] = paramManager->registerParameter(this, "Knob m", "g");
	paramIds[Damping] = paramManager->registerParameter(this, "Knob d", " ");
	paramIds[Position] = paramManager->registerParameter(this, "Knob p", " ");
}

//-----------------------------------------------------------------------------
PhysModKnob::~PhysModKnob()
{
	
}

//-----------------------------------------------------------------------------
void PhysModKnob::getBlock(float *block,
						   VstInt32 blockSize,
						   VstInt32 barStart,
						   float ppq)
{
	VstInt32 i;

	for(i=0;i<blockSize;++i)
	{
		integrate(samplePeriod);
		block[i] = ((knobPos.x*amplitude)+intendedPos);
		if(block[i] > 1.0f)
			block[i] = 1.0f;
		else if(block[i] < 0.0f)
			block[i] = 0.0f;
	}
}

//-----------------------------------------------------------------------------
void PhysModKnob::parameterChanged(VstInt32 index, float val)
{
	if(index == paramIds[Position])
	{
		float lastPos = intendedPos;

		//First make sure the output gets scaled and shifted correctly.
		/*if((1.0f-val) > val)
			amplitude = val;
		else
			amplitude = 1.0f-val;*/
		amplitude = 0.5f;
		intendedPos = val;

		//Now move the knob itself.
		//knobPos.x += intendedPos-lastPos;
		knobPos.x += lastPos-intendedPos;
	}
	else if(index == paramIds[Mass])
	{
		mass = (val * 9.99f)+0.01f;

		stiffnessAndMass = stiffness/mass;
	}
	else if(index == paramIds[Damping])
		damping = val * 10.0f;
}

//-----------------------------------------------------------------------------
void PhysModKnob::setSamplerate(float rate)
{
	samplePeriod = 1.0f/rate;
}

//-----------------------------------------------------------------------------
float PhysModKnob::getValue(VstInt32 index)
{
	float retval;

	if(index == paramIds[Position])
		retval = intendedPos;
	else if(index == paramIds[Mass])
		retval = (mass-0.01f) * (1.0f/9.99f);
	else if(index == paramIds[Damping])
		retval = damping * 0.1f;

	return retval;
}

//-----------------------------------------------------------------------------
string PhysModKnob::getTextValue(VstInt32 index)
{
	stringstream tempConverter;

	if(index == paramIds[Position])
		tempConverter << intendedPos;
	else if(index == paramIds[Mass])
		tempConverter << mass;
	else if(index == paramIds[Damping])
		tempConverter << damping;

	return tempConverter.str();
}
