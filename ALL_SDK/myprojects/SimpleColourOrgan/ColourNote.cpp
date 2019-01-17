//	ColourNote.cpp - A single 'note' of colour.
//	--------------------------------------------------------------------------
//	Copyright (c) 2006 Niall Moody
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

#include "ColourNote.h"

//----------------------------------------------------------------------------
ColourNote::ColourNote():
attackTime(1.0f),
releaseTime(1.0f),
env(0.0f),
attackRelease(false),
isActive(false),
midiNote(0),
velocity(0.0f)
{
	
}

//----------------------------------------------------------------------------
ColourNote::~ColourNote()
{
	
}

//----------------------------------------------------------------------------
void ColourNote::setAttack(float val)
{
	attackTime = (val * 63.99f) + 0.01f;
}

//----------------------------------------------------------------------------
void ColourNote::setRelease(float val)
{
	releaseTime = (val * 63.99f) + 0.01f;
}

//----------------------------------------------------------------------------
void ColourNote::setSilentColour(const Colour& val)
{
	silentColour = val;
}

//----------------------------------------------------------------------------
void ColourNote::setColour(Colour * const newColours)
{
	playingColour = newColours[midiNote%12];
	playingColour *= velocity;
}

//----------------------------------------------------------------------------
void ColourNote::setMidiNote(int val)
{
	midiNote = val;
}

//----------------------------------------------------------------------------
void ColourNote::noteOn(const Colour& val, float vel)
{
	env = 0.0f;
	attackRelease = false;
	playingColour = ((1.0f - vel) * silentColour) + (vel * val);
	velocity = vel;
	isActive = true;
}

//----------------------------------------------------------------------------
void ColourNote::noteOff()
{
	attackRelease = true;
}

//----------------------------------------------------------------------------
Colour ColourNote::tick()
{
	Colour retval;

	if(isActive)
	{
		//Calculate correct colour.
		retval = ((1.0f - env) * silentColour) + (env * playingColour);

		//Calculate envelope.
		if((!attackRelease) && (env < 1.0f))
		{
			//Increment env.
			env += (1.0f/attackTime); //Assuming 10ms 'samplerate'.
			if(env > 1.0f)
				env = 1.0f;
		}
		else if((attackRelease) && (env > 0.0f))
		{
			//Decrement env.
			env -= (1.0f/releaseTime); //Assuming 10ms 'samplerate'.
			if(env <= 0.0f)
				isActive = false;
		}
	}

	return retval;
}
