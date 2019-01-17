//	NoteMaster.cpp - Coordinates the voices.
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

#include "NoteMaster.h"
#include "HarmonicalEditor3D.h"

//----------------------------------------------------------------------------
NoteMaster::NoteMaster(float samplerate):
guiCount(0),
ed(0)
{
	int i;

	this->samplerate = samplerate;

	voices = 0;
	voices = new Voice[NUM_VOICES];
	for(i=0;i<NUM_VOICES;i++)
	{
		voiceIsActive[i] = false;
		notes[i] = 255; //Only 128 notes in MIDI, so 255 is used to distinguish non-active notes.
		voices[i].setSamplerate(samplerate);
	}
}

//----------------------------------------------------------------------------
NoteMaster::~NoteMaster()
{
	if(voices)
		delete [] voices;
}

//----------------------------------------------------------------------------
twofloats NoteMaster::getSample()
{
	int i;
	twofloats retval;

	for(i=0;i<NUM_VOICES;i++)
	{
		if(voiceIsActive[i])
		{
			if(voices[i].getIsActive()) //is this too many if statements?
				retval += voices[i].getSample();
			else
				voiceIsActive[i] = false;
		}
	}

	guiCount++;
	if(guiCount > 64)
	{
		guiCount = 0;
		if(ed)
		{
			for(i=0;i<NUM_VOICES;i++)
			{
				if(voices[i].getIsActive())
					ed->setVertices(voices[i].getVertices(), i, voices[i].getEnvVal());
			}
		}
	}

	return retval;
}

//----------------------------------------------------------------------------
void NoteMaster::onNoteOn(unsigned char note, float velocity)
{
	int i;

	for(i=0;i<NUM_VOICES;i++)
	{
		if(!voiceIsActive[i])
		{
			voices[i].onNoteOn(note, velocity);
			notes[i] = note;
			voiceIsActive[i] = true;
			ed->noteOn(i);
			return;
		}
	}

	//If we get here, all 16 voices are already being used.
	//Should probably do something about that...
}

//----------------------------------------------------------------------------
void NoteMaster::onNoteOff(unsigned char note)
{
	int i;

	for(i=0;i<NUM_VOICES;i++)
	{
		if(notes[i]==note) //&&isActive[i]
		{
			voices[i].onNoteOff();
			notes[i] = 255;
			ed->noteOff(i);
			//break;
		}
	}
}

//----------------------------------------------------------------------------
void NoteMaster::onCC(unsigned char cc, float value)
{
	int i;

	for(i=0;i<NUM_VOICES;i++)
		voices[i].onCC(cc, value);
}

//----------------------------------------------------------------------------
void NoteMaster::onPitchBend(float value)
{
	int i;

	for(i=0;i<NUM_VOICES;i++)
		voices[i].onPitchBend(value);
}

//----------------------------------------------------------------------------
void NoteMaster::setSamplerate(float value)
{
	int i;

	samplerate = value;

	for(i=0;i<NUM_VOICES;i++)
		voices[i].setSamplerate(value);
}

//----------------------------------------------------------------------------
void NoteMaster::setParameter(long index, float value)
{
	int i;

	for(i=0;i<NUM_VOICES;i++)
		voices[i].setParameter(index, value);
}

//----------------------------------------------------------------------------
void NoteMaster::setEditor(HarmonicalEditor3D *ed)
{
	this->ed = ed;
}
