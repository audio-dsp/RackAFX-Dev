//	Step.cpp - Simple class representing a single step.
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

#include "Step.h"

//----------------------------------------------------------------------------
Step::Step():
samplerate(44100.0f),
bpm(120.0f),
noteLength(0.25f),
noteValue(64),
volume(1.0f),
index(0),
eventHandler(0),
isRunning(false),
id(0)
{
	
}

//----------------------------------------------------------------------------
Step::Step(long id):
samplerate(44100.0f),
bpm(120.0f),
noteLength(1.0f),
noteValue(64),
volume(1.0f),
index(0),
eventHandler(0),
isRunning(false)
{
	this->id = id;
}

//----------------------------------------------------------------------------
Step::~Step()
{
	if(isRunning)
		eventHandler->stepStopEvent(id);
}

//----------------------------------------------------------------------------
void Step::setId(long val)
{
	id = val;
}

//----------------------------------------------------------------------------
void Step::registerEventHandler(StepEventHandler *handler)
{
	eventHandler = handler;
}

//----------------------------------------------------------------------------
void Step::activate(long offset)
{
	if(isRunning)
		eventHandler->stepStopEvent(id);

	if(offset > 0)
		index = offset;
	else
		index = 0;
	isRunning = true;

	if(eventHandler)
		eventHandler->stepStartEvent(id);
}

//----------------------------------------------------------------------------
void Step::deactivate()
{
	if(isRunning)
		eventHandler->stepStopEvent(id);

	isRunning = false;
}

//----------------------------------------------------------------------------
void Step::setSamplerate(float val)
{
	samplerate = val;
}

//----------------------------------------------------------------------------
void Step::setBpm(float val)
{
	bpm = val;
}

//----------------------------------------------------------------------------
void Step::setNoteLength(float val)
{
	noteLength = val;
}

//----------------------------------------------------------------------------
void Step::setNoteValue(unsigned char val)
{
	/*eventHandler->stepStopEvent(id);
	if(isRunning)
	{
		//eventHandler->stepStopEvent(id);
		noteValue = val;
		eventHandler->stepStartEvent(id);
	}
	else
		noteValue = val;*/
	newNoteValue = val;
}

//----------------------------------------------------------------------------
void Step::setVolume(float val)
{
	volume = val;
}

//----------------------------------------------------------------------------
bool Step::tick()
{
	long tempint;

	/*if(noteValue != newNoteValue)
	{
		eventHandler->stepStopEvent(id);
		//if(isRunning)
		//{
			noteValue = newNoteValue;
			eventHandler->stepStartEvent(id);
		//}
		//else
		//	noteValue = newNoteValue;
	}*/

	++index;

	tempint = static_cast<long>(noteLength * (samplerate/(bpm/60.0f)));
	if(index >= tempint)
	{
		isRunning = false;
		if(eventHandler)
			eventHandler->stepStopEvent(id);

		return false;
	}

	return true;
}

//----------------------------------------------------------------------------
MidiMessage Step::getMIDI(bool onOff)
{
	MidiMessage mess = new unsigned char[3];

	if(onOff)
	{
		mess[0] = 0x90;
		mess[1] = noteValue;
		mess[2] = static_cast<unsigned char>(volume * 127.0f);
	}
	else
	{
		mess[0] = 0x80;
		mess[1] = noteValue;
		mess[2] = 0;
	}

	return mess;
}
