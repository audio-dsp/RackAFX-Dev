//	CycleShifter.cpp - Definition of the plugin.
//	--------------------------------------------------------------------------
//	Copyright (c) 2004 Niall Moody
//	
//	Permission is hereby granted, free of charge, to any person obtaining a
//	copy of this software and associated documentation files (the "Software"),
//	to deal in the Software without restriction, including without limitation
//	the rights to use, copy, modify, merge, publish, distribute, sublicense,
//	and/or sell copies of the Software, and to permit persons to whom the
//	Software is furnished to do so, subject to the following conditions:

//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.

//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//	THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//	DEALINGS IN THE SOFTWARE.
//	--------------------------------------------------------------------------


#include <stdio.h>

#include "AEffEditor.hpp"
#include "CycleShifter.h"

#ifndef CYCLESHIFTEREDITOR_H_
#include "CycleShifterEditor.h"
#endif

#include <math.h>

extern bool oome;

//-----------------------------------------------------------------------------------------
//Program Constructor - set default values of the program
//-----------------------------------------------------------------------------------------
CycleShifterProg::CycleShifterProg ()
{
	fNewCycleVolume = 1.0f;
	fInputVolume = 1.0f;

	strcpy (name, "Init");
}

//-----------------------------------------------------------------------------------------
//Plugin Constructor - create programs, set certain variables
//-----------------------------------------------------------------------------------------
CycleShifter::CycleShifter(audioMasterCallback audioMaster)
	: AudioEffectX(audioMaster, kNumPrograms, kNumParams)
{
	programs = 0;

	fNewCycleVolume = 1.0f;
	fInputVolume = 1.0f;

	for(int i=0;i<NUM_EVENTS_MAX;i++)
	{
		MIDIEvent[i] = new VstMidiEvent;

		MIDIEvent[i]->type = 1;
		MIDIEvent[i]->midiData[3] = 0;
		MIDIEvent[i]->reserved1 = 99;
		MIDIEvent[i]->deltaFrames = -99;
		MIDIEvent[i]->noteLength = 0;
		MIDIEvent[i]->noteOffset = 0;

		EventNumArray[i] = -1;
	}
	NumPendingEvents = 0;
	Events = new VstEvents;
	Events->numEvents = 1;
	Events->events[0] = (VstEvent *)MIDIEvent[0];
	NumEvents = 0;

	Frames = 0;

    programs = new CycleShifterProg[numPrograms];
	
	if(programs)
	{
		setProgram(0);
		setParameter(kNewCycleVolume, 1.0f);
		setParameter(kInputVolume, 1.0f);
	    setProgramName("Cycle Shifter");
	}
		
	strcpy(kEffectName, "Cycle Shifter");
	strcpy(kProduct, "ndc Plugs CycleShifter");
	strcpy(kVendor, "ndc Plugs");
	
	hasVu(kVU);
    setNumInputs(kNumInputs);
    setNumOutputs(kNumOutputs);
    canMono(kCanMono);
    canProcessReplacing(kCanReplacing);
    isSynth(kIsSynth);
    setUniqueID(kID);

	tempo = 125.0f;
	samplerate = getSampleRate();
	if(samplerate <= 11025.0f)
		samplerate = 44100.0f;

	checkTooSoon = 0;
	checkTempo = 1024;
	checkBSZero = false;

	for(i=0;i<BUFFER_SIZE;i++)
	{
		CycleBuffer[i] = 0.0f;
	}
	OutIndex = 0;
	InCount = 0;
	ReadWrite = false;
	EnvOld = 0.0f;

	editor = new CycleShifterEditor(this);
	if (!editor)
		oome = true;
 }

//-----------------------------------------------------------------------------------------
//Plugin Destructor - destroy programs, destroy buffers if you've got them
//-----------------------------------------------------------------------------------------
CycleShifter::~CycleShifter()
{
	int i;

	if(programs)
		delete [] programs;

	for(i=0;i<NUM_EVENTS_MAX;i++)
	{
		if(MIDIEvent[i])
			delete MIDIEvent[i];
	}
	if(Events)
		delete Events;
}

//-----------------------------------------------------------------------------------------
//Borrowed this from Toby Bear's Delphi template - it maybe adds a bit to cpu
//usage, but it makes things simpler...
//-----------------------------------------------------------------------------------------
twofloats CycleShifter::DoProcess(twofloats a)
{
	float tempval; //we're essentially going to have mono operation, so only 1 temp variable...
	float outval;

	tempval = a.left;
	if(!ReadWrite) //if we're in read mode
	{
		if(InCount == 0) //if we're waiting for the start of a new cycle to read
		{
			if((EnvOld < 0.0f)&&(tempval >= 0.0f)) //as soon as the input goes past 0 we start reading
			{
				CycleBuffer[InCount] = tempval;
				InCount++;
			}
		}
		else if(!((EnvOld < 0.0f)&&(tempval >= 0.0f))) //if we've not reached the end of the cycle yet
		{
			CycleBuffer[InCount] = tempval;
			InCount++;
			if(InCount >= BUFFER_SIZE) //if we've reached the end of the buffer
			{
				/* ok - I've taken this out because it didn't work, and caused clicks...
				for(i=InCount;i<BUFFER_SIZE;i++) //short fade out for neatness
				{
					CycleBuffer[i] = 1.0f - ((float)(i-(BUFFER_SIZE-16))/16.0f);
					CycleBuffer[i] *= BUFFER_SIZE - 16; //oops, that was a stupid mistake...
				}*/
				InCount = BUFFER_SIZE;
				ReadWrite = true; //we're in write mode now
			}
		}
		else //we've reached the end of the cycle
		{
			CycleBuffer[InCount] = 0.0f;
			InCount++;
			ReadWrite = true;
		}
		a.left *= fInputVolume;
		a.right = a.left; //to avoid weird stereo effects, 'cause we're in mono
	}
	else //we're in write mode
	{
		outval = (a.left*fInputVolume) + (CycleBuffer[OutIndex]*fNewCycleVolume);
		OutIndex++;
		if(OutIndex == InCount) //we've reached the end of our stored cycle
		{
			InCount = 0;
			OutIndex = 0;
			ReadWrite = false;
		}
		a.left = outval;
		a.right = outval;
	}

	EnvOld = tempval;

    return a;
}

//-----------------------------------------------------------------------------------------
//Shouldn't need to change this...
//-----------------------------------------------------------------------------------------
void CycleShifter::process(float **inputs, float **outputs, long sampleFrames)
{
	int i;

    float *in1  =  inputs[0];
    float *in2  =  inputs[1];
    float *out1 = outputs[0];
    float *out2 = outputs[1];
	twofloats a;

	Frames = sampleFrames;

	for(i=0;i<sampleFrames;i++)
	{
		if((NumEvents>0))
		{
			processMIDI(i);
		}

		a.left = (*in1++);
		a.right = (*in2++);

		a = DoProcess(a);
        
		(*out1++) += a.left;
		(*out2++) += a.right;
	}
	if(NumPendingEvents > 0)
	{
		for(i=0;i<NumPendingEvents;i++)
		{
			MIDIEvent[EventNumArray[i]]->deltaFrames -= sampleFrames;
		}
	}
}

//-----------------------------------------------------------------------------------------
//Shouldn't need to change this...
//-----------------------------------------------------------------------------------------
void CycleShifter::processReplacing(float **inputs, float **outputs, long sampleFrames)
{
	int i;

    float *in1  =  inputs[0];
    float *in2  =  inputs[1];
    float *out1 = outputs[0];
    float *out2 = outputs[1];
	twofloats a;

	Frames = sampleFrames;

	for(i=0;i<sampleFrames;i++)
	{
		if((NumEvents>0))
		{
			processMIDI(i);
		}

		a.left = (*in1++);
		a.right = (*in2++);

		a = DoProcess(a);
        
		(*out1++) = a.left;
		(*out2++) = a.right;
	}
	if(NumPendingEvents > 0)
	{
		for(i=0;i<NumPendingEvents;i++)
		{
			MIDIEvent[EventNumArray[i]]->deltaFrames -= sampleFrames;
		}
	}
}

//-----------------------------------------------------------------------------------------
//Here I just pass all the appropriate messages onto my MIDI queue (MIDIEvent).
//-----------------------------------------------------------------------------------------
long CycleShifter::processEvents(VstEvents *ev)
{
	int i, j, k;
	VstMidiEvent *event;

	for(i=0;i<ev->numEvents;i++)
	{
		if(ev->events[i]->type == kVstMidiType)
		{
			event = (VstMidiEvent *)ev->events[i];
			j = -1;

			for(k=1;k<NUM_EVENTS_MAX;k++)
			{
				if(MIDIEvent[k]->deltaFrames == -99)
				{
					EventNumArray[NumPendingEvents] = k;
					NumPendingEvents++;

					j = k;
					break;
				}
			}
			if((j > 0)&&(NumEvents < NUM_EVENTS_MAX))
			{
				NumEvents++;
				MIDIEvent[j]->midiData[0] =		event->midiData[0];
				MIDIEvent[j]->midiData[1] =		event->midiData[1];
				MIDIEvent[j]->midiData[2] =		event->midiData[2];
				MIDIEvent[j]->midiData[3] =		event->midiData[3];
				MIDIEvent[j]->type =			event->type;
				MIDIEvent[j]->byteSize =		event->byteSize;
				MIDIEvent[j]->deltaFrames =		event->deltaFrames;
				MIDIEvent[j]->flags =			event->flags;
				MIDIEvent[j]->noteLength =		event->noteLength;
				MIDIEvent[j]->noteOffset =		event->noteOffset;
				MIDIEvent[j]->detune =			event->detune;
				MIDIEvent[j]->noteOffVelocity = event->noteOffVelocity;
				MIDIEvent[j]->reserved1 =		99;
				MIDIEvent[j]->reserved2 =		event->reserved2;
			}
		}
	}
	return 1;
}

//-----------------------------------------------------------------------------------------
//A general function to get the bar start position from the host (not used in this plugin).
//-----------------------------------------------------------------------------------------
bool CycleShifter::getBarStart()
{
	unsigned long bs, ppq;

	if(checkTooSoon > 0)
	{
		checkTooSoon--;
		return false;
	}

	VstTimeInfo	*TimeInfo = getTimeInfo(kVstPpqPosValid|kVstBarsValid);
	if(!TimeInfo)
		return false;

	bs = (unsigned long)((double)TimeInfo->barStartPos*100.0);
	ppq = (unsigned long)((double)TimeInfo->ppqPos*100.0);

	if((TimeInfo->flags&kVstPpqPosValid) != kVstPpqPosValid)
		return false;
	if((TimeInfo->flags&kVstBarsValid) != kVstBarsValid)
		return false;

	if(bs != 0)
	{
		checkBSZero = false;
		if(ppq == bs)
		{
			checkTooSoon = 2048;
			return true;
		}
		else
			return false;
	}
	else if(checkBSZero == false)
	{
		checkBSZero = true;
		if(ppq == 0)
		{
			checkTooSoon = 2048;
			return true;
		}
		else
			return false;
	}
	else
		return false;
}

//-----------------------------------------------------------------------------------------
void CycleShifter::resume()
{
    wantEvents(); // important for all plugins that receive MIDI! 

	tempo = (float)((float)tempoAt(0)*0.0001f);
	if(tempo <= 0.0f)
		tempo = 125.0f;
	samplerate = getSampleRate();
	if(samplerate <= 11025.0f)
		samplerate = 44100.0f;
}

//-----------------------------------------------------------------------------------------
void CycleShifter::suspend()
{
    // buffers are normally cleared/initialized here
}

//-----------------------------------------------------------------------------------------
void CycleShifter::setProgram(long program)
{
    CycleShifterProg *ap = &programs[program];

	curProgram = program;

	setParameter(kNewCycleVolume, ap->fNewCycleVolume);
	setParameter(kInputVolume, ap->fInputVolume);
}

//-----------------------------------------------------------------------------------------
void CycleShifter::setProgramName (char *name)
{
	strcpy (programs[curProgram].name, name);
}


//-----------------------------------------------------------------------------------------
void CycleShifter::getProgramName(char *name)
{
	if (!strcmp (programs[curProgram].name, "Init"))
		sprintf (name, "%s %d", programs[curProgram].name, curProgram + 1);
	else
		strcpy (name, programs[curProgram].name);
}

//-----------------------------------------------------------------------------------------
bool CycleShifter::getProgramNameIndexed (long category, long index, char* text)
{
    return false;
    if (index<kNumPrograms)
    {
      strcpy(text,programs[index].name);
      return true;
    }
}

//-----------------------------------------------------------------------------------------
bool CycleShifter::copyProgram (long destination)
{
	bool returnCode = false;
    if (destination < kNumPrograms)
    {        programs[destination] = programs[curProgram];
        returnCode = true;
    }
    return (returnCode);
}

//-----------------------------------------------------------------------------------------
void CycleShifter::setParameter(long index, float value)
{
	CycleShifterProg * ap = &programs[curProgram];

	switch (index)
	{
		case kNewCycleVolume :
			fNewCycleVolume = ap->fNewCycleVolume = value;
			break;
		case kInputVolume :
			fInputVolume = ap->fInputVolume = value;
			break;
	}

	if(editor)
	{
		editor->postUpdate();
		((AEffGUIEditor*)editor)->setParameter(index, value);
	}
}

//-----------------------------------------------------------------------------------------
float CycleShifter::getParameter(long index)
{
	float v = 0;

	switch (index)
	{
		case kNewCycleVolume :    v = fNewCycleVolume; break;
		case kInputVolume :    v = fInputVolume; break;
	}
	return v;
}

//-----------------------------------------------------------------------------------------
void CycleShifter::getParameterLabel(long index, char *label)
{
	switch (index)
	{
		case kNewCycleVolume :      strcpy (label, "   dB   ");	break;
	}
}

//-----------------------------------------------------------------------------------------
//gets values for parameter display - see 'AudioEffect.hpp' for others (float2string,
//Hz2string etc.)
//-----------------------------------------------------------------------------------------
void CycleShifter::getParameterDisplay(long index, char *text)
{
	switch (index)
	{
		case kNewCycleVolume : dB2string(fNewCycleVolume, text); break;
		case kInputVolume : dB2string(fInputVolume, text); break;
	}
}

//-----------------------------------------------------------------------------------------
void CycleShifter::getParameterName(long index, char *label)
{
	switch (index)
	{
		case kNewCycleVolume : strcpy (label, "ncvolume"); break;
		case kInputVolume : strcpy (label, "ipvolume"); break;
	}
}

//-----------------------------------------------------------------------------------------
//tells the host what the plugin can do: 1=can do, -1=cannot do, 0=don't know
long CycleShifter::canDo(char *text)
{
    //return -1;

	if(!strcmp(text, "sendVstEvents")) return 1;
	if(!strcmp(text, "sendVstMidiEvent")) return 1; //because the plugin acts as a MIDI through
	if(!strcmp(text, "sendVstTimeInfo")) return -1;
	if(!strcmp(text, "receiveVstEvents")) return 1;
	if(!strcmp(text, "receiveVstMidiEvent")) return 1;
	if(!strcmp(text, "receiveVstTimeInfo")) return 1;
	if(!strcmp(text, "offline")) return -1;
	if(!strcmp(text, "plugAsChannelInsert")) return -1;
	if(!strcmp(text, "plugAsSend")) return -1;
	if(!strcmp(text, "mixDryWet")) return -1;
	if(!strcmp(text, "noRealTime")) return -1;
	if(!strcmp(text, "multipass")) return -1;
	if(!strcmp(text, "metapass")) return -1;
	if(!strcmp(text, "1in1out")) return -1;
	if(!strcmp(text, "1in2out")) return -1;
	if(!strcmp(text, "2in1out")) return -1;
	if(!strcmp(text, "2in2out")) return -1;
	if(!strcmp(text, "2in4out")) return -1;
	if(!strcmp(text, "4in2out")) return -1;
	if(!strcmp(text, "4in4out")) return -1;
	if(!strcmp(text, "4in8out")) return -1;					// 4:2 matrix to surround bus
	if(!strcmp(text, "8in4out")) return -1;					// surround bus to 4:2 matrix
	if(!strcmp(text, "8in8out")) return -1;
	if(!strcmp(text, "midiProgramNames")) return -1;
	if(!strcmp(text, "conformsToWindowRules") ) return -1;	// mac: doesn't mess with grafport. general: may want
															// to call sizeWindow (). if you want to use sizeWindow (),
															// you must return true (1) in canDo ("conformsToWindowRules")
	if(!strcmp(text, "bypass")) return -1;

	return -1;
}

//-----------------------------------------------------------------------------------------
//don't know how this works - I've never used it
//-----------------------------------------------------------------------------------------
float CycleShifter::getVu()
{
	return 0.0f;
}

//-----------------------------------------------------------------------------------------
bool CycleShifter::getEffectName (char* name)
{
    strcpy(name,kEffectName);
    return true;
}

//-----------------------------------------------------------------------------------------
bool CycleShifter::getVendorString (char* text)
{
    strcpy(text, kVendor);
    return true;
}

//-----------------------------------------------------------------------------------------
bool CycleShifter::getProductString (char* text)
{
    strcpy(text, kProduct);
    return true;
}

//-----------------------------------------------------------------------------------------
long CycleShifter::getVendorVersion ()
{
    return kVersionNo;
}

//-----------------------------------------------------------------------------------------
VstPlugCategory CycleShifter::getPlugCategory()
{
    return (kPlugCategEffect);
}

//-----------------------------------------------------------------------------------------
bool CycleShifter::getInputProperties(long index, VstPinProperties* properties)
{
	bool returnCode = false;
	int temp = 1;
	if(index == 0)
	{
		sprintf(properties->label, "%s Left Input %d", kEffectName, temp);
		properties->flags = kVstPinIsStereo|kVstPinIsActive;
		returnCode = true;
	}
	else if(index == 1)
	{
		sprintf(properties->label, "%s Right Input %d", kEffectName, temp);
		properties->flags = kVstPinIsStereo|kVstPinIsActive;
		returnCode = true;
	}
	return returnCode;
}

//-----------------------------------------------------------------------------------------
bool CycleShifter::getOutputProperties(long index, VstPinProperties* properties)
{
	bool returnCode = false;
	int temp = 1;
	if (index == 0)
	{
		sprintf(properties->label, "%s Left Output %d", kEffectName, temp);
		properties->flags = kVstPinIsStereo|kVstPinIsActive;
		returnCode = true;
	}
	else 
	if (index == 1)
	{
		sprintf(properties->label, "%s Right Output %d", kEffectName, temp);
		properties->flags = kVstPinIsStereo|kVstPinIsActive;
		returnCode = true;
	}
	return (returnCode);
}

//-----------------------------------------------------------------------------------------
long CycleShifter::getGetTailSize()
{
	return 1; //1=no tail, 0=don't know, everything else=tail size
}

//-----------------------------------------------------------------------------------------
void CycleShifter::onoff2string(float value, char *text)
{
	if (value < 0.5)
		strcpy(text, "   Off  ");
	else
		strcpy(text, "   On   ");
}

//-----------------------------------------------------------------------------------------
//Called from process/Replacing to dispatch the appropriate MIDI events for the 'pos'
//frame in the current audio buffer.
//-----------------------------------------------------------------------------------------
void CycleShifter::processMIDI(long pos)
{
	int data1, data2;
	int status, ch, delta;
	int note;
	int i, j;

	for(i=0;i<NumPendingEvents;i++)
	{
		if((MIDIEvent[EventNumArray[i]]->deltaFrames%Frames) == pos)
		{
			//--pass on/act on event--
			delta = 0; //because we're at pos frames into the buffer...
			ch = (MIDIEvent[EventNumArray[i]]->midiData[0] & 0x0F);
			status = (MIDIEvent[EventNumArray[i]]->midiData[0] & 0xF0);
			data1 = (MIDIEvent[EventNumArray[i]]->midiData[1] & 0x7F);
			data2 = (MIDIEvent[EventNumArray[i]]->midiData[2] & 0x7F);
			delta = MIDIEvent[EventNumArray[i]]->deltaFrames;	//huh?

			note = data1;

			switch(status)
			{
				case 0x90:
					if(data2 > 0)
					{
						MIDI_NoteOn(ch, data1, data2, delta);
					}
					else
					{
						MIDI_NoteOff(ch, data1, data2, delta);
					}
					break;
				case 0x80:
					MIDI_NoteOff(ch, data1, data2, delta);
					break;
				case 0xA0:
					MIDI_PolyAftertouch(ch, data1, data2, delta);
					break;
				case 0xB0:
					MIDI_CC(ch, data1, data2, delta);
					break;
				case 0xC0:
					MIDI_ProgramChange(ch, data1, delta);
					break;
				case 0xD0:
					MIDI_ChannelAftertouch(ch, data1, delta);
					break;
				case 0xE0:
					MIDI_PitchBend(ch, data1, data2, delta);
					break;
			}
			MIDIEvent[EventNumArray[i]]->deltaFrames = -99;
			--NumEvents;

			//--reset EventNumArray--
			for(j=(i+1);j<NumPendingEvents;j++)
			{
				EventNumArray[(j-1)] = EventNumArray[j];
			}
			--NumPendingEvents;
			//break;
		}
	}
}

//-----------------------------------------------------------------------------------------
//If we were using MIDI for something, we'd add it into these methods, rather than just
//passing the data on like a MIDI through.
//-----------------------------------------------------------------------------------------
void CycleShifter::MIDI_NoteOn(int ch, int note, int val, int delta)
{
	MIDIEvent[0]->midiData[0] = 0x90 + ch;
	MIDIEvent[0]->midiData[1] = note;
	MIDIEvent[0]->midiData[2] = val;
	MIDIEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(Events);
}

//-----------------------------------------------------------------------------------------
void CycleShifter::MIDI_NoteOff(int ch, int note, int val, int delta)
{
	MIDIEvent[0]->midiData[0] = 0x80 + ch;
	MIDIEvent[0]->midiData[1] = note;
	MIDIEvent[0]->midiData[2] = val;
	MIDIEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(Events);
}

//-----------------------------------------------------------------------------------------
void CycleShifter::MIDI_PolyAftertouch(int ch, int note, int val, int delta)
{
	MIDIEvent[0]->midiData[0] = 0xA0 + ch;
	MIDIEvent[0]->midiData[1] = note;
	MIDIEvent[0]->midiData[2] = val;
	MIDIEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(Events);
}

//-----------------------------------------------------------------------------------------
void CycleShifter::MIDI_CC(int ch, int num, int val, int delta)
{
	float temp;

	temp = val/127.0f;	// CC data
	if(num == 1)		//mod wheel
		setParameter(kNewCycleVolume, temp);

	MIDIEvent[0]->midiData[0] = 0xB0 + ch;
	MIDIEvent[0]->midiData[1] = num;
	MIDIEvent[0]->midiData[2] = val;
	MIDIEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(Events);
}

//-----------------------------------------------------------------------------------------
//You don't have to do this, but I like to...
//-----------------------------------------------------------------------------------------
void CycleShifter::MIDI_ProgramChange(int ch, int val, int delta)
{
	if(val < kNumPrograms)
		setProgram(val);

	MIDIEvent[0]->midiData[0] = 0xD0 + ch;
	MIDIEvent[0]->midiData[1] = val;
	MIDIEvent[0]->midiData[2] = 0;
	MIDIEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(Events);
}

//-----------------------------------------------------------------------------------------
void CycleShifter::MIDI_ChannelAftertouch(int ch, int val, int delta)
{
	MIDIEvent[0]->midiData[0] = 0xD0 + ch;
	MIDIEvent[0]->midiData[1] = val;
	MIDIEvent[0]->midiData[2] = 0;
	MIDIEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(Events);
}

//-----------------------------------------------------------------------------------------
void CycleShifter::MIDI_PitchBend(int ch, int x1, int x2, int delta)
{
	MIDIEvent[0]->midiData[0] = 0xE0 + ch;
	MIDIEvent[0]->midiData[1] = x1;
	MIDIEvent[0]->midiData[2] = x2;
	MIDIEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(Events);
}