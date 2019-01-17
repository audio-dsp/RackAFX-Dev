//	AmplitudeImposer.cpp - Definition of the plugin.
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
#include <math.h>

#include "AEffEditor.hpp"

#include "AmplitudeImposer.h"
#include "AmplitudeImposerEditor.h"

extern bool oome;

//----------------------------------------------------------------------------
//Program Constructor - set default values of the program
//----------------------------------------------------------------------------
AmplitudeImposerProg::AmplitudeImposerProg()
{
	fDepth = 1.0f;
	fThreshold = 0.5f;

	strcpy(name, "Init");
}

//----------------------------------------------------------------------------
//Plugin Constructor - create programs, set certain variables
//----------------------------------------------------------------------------
AmplitudeImposer::AmplitudeImposer(audioMasterCallback audioMaster)
	: AudioEffectX(audioMaster, kNumPrograms, kNumParams)
{
	programs = 0;

	fDepth = 1.0f;
	fThreshold = 0.5f;

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

    programs = new AmplitudeImposerProg[numPrograms];
	
	if(programs)
	{
		setProgram(0);
		setParameter(kDepth, 1.0f);
		setParameter(kThreshold, 0.5f);
	    setProgramName("Amplitude Imposer");
	}
		
	strcpy(kEffectName, "Amplitude Imposer");
	strcpy(kProduct, "ndc Plugs Amp Imposer");
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

	ampEnvelope_l = 0.0f;
	ampEnvelope_r = 0.0f;
	envDecay = 0.0001f;
	audioEnvelope_l = 0.0f;
	audioEnvelope_r = 0.0f;

	//----------------

	editor = new AmplitudeImposerEditor(this);
	if(!editor)
		oome = true;
 }

//----------------------------------------------------------------------------
//Plugin Destructor - destroy programs, destroy buffers if you've got them
//----------------------------------------------------------------------------
AmplitudeImposer::~AmplitudeImposer()
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

//----------------------------------------------------------------------------
//Borrowed this from Toby Bear's Delphi template - it maybe adds a bit to cpu
//usage, but it makes things a bit simpler...
//----------------------------------------------------------------------------
twofloats AmplitudeImposer::DoProcess(twofloats a)
{
	return a;
}

//----------------------------------------------------------------------------
//Shouldn't need to change this...
//----------------------------------------------------------------------------
void AmplitudeImposer::process(float **inputs, float **outputs, long sampleFrames)
{
	unsigned long i;
    float *in1  =  inputs[0];
    float *in2  =  inputs[1];
    float *in3  =  inputs[2];
    float *in4  =  inputs[3];
    float *out1 = outputs[0];
    float *out2 = outputs[1];
	twofloats tempf;
	twofloats tempin;

	Frames = sampleFrames;

	for(i=0;i<static_cast<unsigned long>(sampleFrames);i++)
	{
		if((NumEvents>0))
		{
			processMIDI(i);
		}

		//calculate envelope from 1st two inputs
		if(fabs(in1[i]) > ampEnvelope_l)
			ampEnvelope_l = fabs(in1[i]);
		else if(fabs(in1[i]) < ampEnvelope_l)
			ampEnvelope_l -= envDecay;
		if(fabs(in2[i]) > ampEnvelope_r)
			ampEnvelope_r = fabs(in2[i]);
		else if(fabs(in2[i]) < ampEnvelope_r)
			ampEnvelope_r -= envDecay;

		//calculate envelope from 2nd two inputs
		if(fabs(in3[i]) > audioEnvelope_l)
			audioEnvelope_l = fabs(in3[i]);
		else if(fabs(in3[i]) < audioEnvelope_l)
			audioEnvelope_l -= envDecay;
		if(fabs(in4[i]) > audioEnvelope_r)
			audioEnvelope_r = fabs(in4[i]);
		else if(fabs(in4[i]) < audioEnvelope_r)
			audioEnvelope_r -= envDecay;

		//make sure we're not multiplying by a negative number
		if(ampEnvelope_l < 0.0f)
			ampEnvelope_l = 0.0f;
		if(ampEnvelope_r < 0.0f)
			ampEnvelope_r = 0.0f;
		if(audioEnvelope_l < 0.0f)
			audioEnvelope_l = 0.0f;
		if(audioEnvelope_r < 0.0f)
			audioEnvelope_r = 0.0f;

		//work out whether we need to multiply audio input
		if(audioEnvelope_l > fThreshold)
			tempin.left = in3[i];
		else
		{
			if(audioEnvelope_l > 0.001f)
				tempin.left = in3[i] * (fThreshold/audioEnvelope_l);
			else
				tempin.left = in3[i] * (fThreshold/0.001f); //so it'll decay away smoothly
		}
		if(audioEnvelope_r > fThreshold)
			tempin.right = in4[i];
		else
		{
			if(audioEnvelope_r > 0.001f)
				tempin.right = in4[i] * (fThreshold/audioEnvelope_r);
			else
				tempin.right = in4[i] * (fThreshold/0.001f);
		}

		//calculate output
		tempf.left = tempin.left * ampEnvelope_l;
		tempf.left *= fDepth;
		tempf.left = tempf.left + ((1.0f-fDepth)*tempin.left);
		tempf.right = tempin.right * ampEnvelope_r;
		tempf.right *= fDepth;
		tempf.right = tempf.right + ((1.0f-fDepth)*tempin.right);
        
		out1[i] += tempf.left;
		out2[i] += tempf.right;
	}
	if(NumPendingEvents > 0)
	{
		for(i=0;i<static_cast<unsigned long>(NumPendingEvents);i++)
		{
			MIDIEvent[EventNumArray[i]]->deltaFrames -= sampleFrames;
		}
	}
}

//----------------------------------------------------------------------------
//Shouldn't need to change this...
//----------------------------------------------------------------------------
void AmplitudeImposer::processReplacing(float **inputs, float **outputs, long sampleFrames)
{
	unsigned long i;
    float *in1  =  inputs[0];
    float *in2  =  inputs[1];
    float *in3  =  inputs[2];
    float *in4  =  inputs[3];
    float *out1 = outputs[0];
    float *out2 = outputs[1];
	twofloats tempf;
	twofloats tempin;

	Frames = sampleFrames;

	for(i=0;i<static_cast<unsigned long>(sampleFrames);i++)
	{
		if((NumEvents>0))
		{
			processMIDI(i);
		}

		//calculate envelope from 1st two inputs
		if(fabs(in1[i]) > ampEnvelope_l)
			ampEnvelope_l = fabs(in1[i]);
		else if(fabs(in1[i]) < ampEnvelope_l)
			ampEnvelope_l -= envDecay;
		if(fabs(in2[i]) > ampEnvelope_r)
			ampEnvelope_r = fabs(in2[i]);
		else if(fabs(in2[i]) < ampEnvelope_r)
			ampEnvelope_r -= envDecay;

		//calculate envelope from 2nd two inputs
		if(fabs(in3[i]) > audioEnvelope_l)
			audioEnvelope_l = fabs(in3[i]);
		else if(fabs(in3[i]) < audioEnvelope_l)
			audioEnvelope_l -= envDecay;
		if(fabs(in4[i]) > audioEnvelope_r)
			audioEnvelope_r = fabs(in4[i]);
		else if(fabs(in4[i]) < audioEnvelope_r)
			audioEnvelope_r -= envDecay;

		//make sure we're not multiplying by a negative number
		if(ampEnvelope_l < 0.0f)
			ampEnvelope_l = 0.0f;
		if(ampEnvelope_r < 0.0f)
			ampEnvelope_r = 0.0f;
		if(audioEnvelope_l < 0.0f)
			audioEnvelope_l = 0.0f;
		if(audioEnvelope_r < 0.0f)
			audioEnvelope_r = 0.0f;

		//work out whether we need to multiply audio input
		if(audioEnvelope_l > fThreshold)
			tempin.left = in3[i];
		else
		{
			if(audioEnvelope_l > 0.001f)
				tempin.left = in3[i] * (fThreshold/audioEnvelope_l);
			else
				tempin.left = in3[i] * (fThreshold/0.001f); //so it'll decay away smoothly
		}
		if(audioEnvelope_r > fThreshold)
			tempin.right = in4[i];
		else
		{
			if(audioEnvelope_r > 0.001f)
				tempin.right = in4[i] * (fThreshold/audioEnvelope_r);
			else
				tempin.right = in4[i] * (fThreshold/0.001f);
		}

		//calculate output
		tempf.left = tempin.left * ampEnvelope_l;
		tempf.left *= fDepth;
		tempf.left = tempf.left + ((1.0f-fDepth)*tempin.left);
		tempf.right = tempin.right * ampEnvelope_r;
		tempf.right *= fDepth;
		tempf.right = tempf.right + ((1.0f-fDepth)*tempin.right);
        
		out1[i] = tempf.left;
		out2[i] = tempf.right;
	}
	if(NumPendingEvents > 0)
	{
		for(i=0;i<static_cast<unsigned long>(NumPendingEvents);i++)
		{
			MIDIEvent[EventNumArray[i]]->deltaFrames -= sampleFrames;
		}
	}
}

//----------------------------------------------------------------------------
//Here I just pass all the appropriate messages onto my MIDI queue (MIDIEvent).
//----------------------------------------------------------------------------
long AmplitudeImposer::processEvents(VstEvents *ev)
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

//----------------------------------------------------------------------------
//A general function to get the bar start position from the host (not used in this plugin).
//----------------------------------------------------------------------------
bool AmplitudeImposer::getBarStart()
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

//----------------------------------------------------------------------------
void AmplitudeImposer::resume()
{
    wantEvents(); // important for all plugins that receive MIDI! 

	tempo = (float)((float)tempoAt(0)*0.0001f);
	if(tempo <= 0.0f)
		tempo = 120.0f;
	samplerate = getSampleRate();
	if(samplerate <= 11025.0f)
		samplerate = 44100.0f;
}

//----------------------------------------------------------------------------
void AmplitudeImposer::suspend()
{
    // buffers are normally cleared/initialized here
}

//----------------------------------------------------------------------------
void AmplitudeImposer::setProgram(long program)
{
    AmplitudeImposerProg *ap = &programs[program];

	curProgram = program;

	setParameter(kDepth, ap->fDepth);
	setParameter(kThreshold, ap->fThreshold);
}

//----------------------------------------------------------------------------
void AmplitudeImposer::setProgramName(char *name)
{
	strcpy(programs[curProgram].name, name);
}


//----------------------------------------------------------------------------
void AmplitudeImposer::getProgramName(char *name)
{
	if(!strcmp(programs[curProgram].name, "Init"))
		sprintf(name, "%s %d", programs[curProgram].name, curProgram + 1);
	else
		strcpy(name, programs[curProgram].name);
}

//----------------------------------------------------------------------------
bool AmplitudeImposer::getProgramNameIndexed(long category, long index, char* text)
{
    return false;
    if(index<kNumPrograms)
    {
      strcpy(text,programs[index].name);
      return true;
    }
}

//----------------------------------------------------------------------------
bool AmplitudeImposer::copyProgram(long destination)
{
	bool returnCode = false;
    if(destination < kNumPrograms)
    {
		programs[destination] = programs[curProgram];
        returnCode = true;
    }
    return returnCode;
}

//----------------------------------------------------------------------------
void AmplitudeImposer::setParameter(long index, float value)
{
	AmplitudeImposerProg * ap = &programs[curProgram];

	switch(index)
	{
		case kDepth:
			fDepth = ap->fDepth = value;
			break;
		case kThreshold:
			fThreshold = ap->fThreshold = value;
			break;
	}

	if(editor)
	{
		editor->postUpdate();
		((AEffGUIEditor*)editor)->setParameter(index, value);
	}
}

//----------------------------------------------------------------------------
float AmplitudeImposer::getParameter(long index)
{
	float v = 0;

	switch(index)
	{
		case kDepth:
			v = fDepth;
			break;
		case kThreshold:
			v = fThreshold;
			break;
	}
	return v;
}

//----------------------------------------------------------------------------
void AmplitudeImposer::getParameterLabel(long index, char *label)
{
	switch(index)
	{
		case kDepth:
			strcpy(label, "   dB   ");
			break;
		case kThreshold:
			strcpy(label, "   dB   ");
			break;
	}
}

//----------------------------------------------------------------------------
//gets values for parameter display - see 'AudioEffect.hpp' for others (float2string,
//Hz2string etc.)
//----------------------------------------------------------------------------
void AmplitudeImposer::getParameterDisplay(long index, char *text)
{
	switch(index)
	{
		case kDepth:
			dB2string(fDepth, text);
			break;
		case kThreshold:
			dB2string(fThreshold, text);
			break;
	}
}

//----------------------------------------------------------------------------
void AmplitudeImposer::getParameterName(long index, char *label)
{
	switch(index)
	{
		case kDepth:
			strcpy(label, " Depth  ");
			break;
		case kThreshold:
			strcpy(label, " Thresh ");
			break;
	}
}

//----------------------------------------------------------------------------
//tells the host what the plugin can do: 1=can do, -1=cannot do, 0=don't know
long AmplitudeImposer::canDo(char *text)
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

//----------------------------------------------------------------------------
//don't know how this works - I've never used it
//----------------------------------------------------------------------------
float AmplitudeImposer::getVu()
{
	return 0.0f;
}

//----------------------------------------------------------------------------
bool AmplitudeImposer::getEffectName(char* name)
{
    strcpy(name,kEffectName);
    return true;
}

//----------------------------------------------------------------------------
bool AmplitudeImposer::getVendorString(char* text)
{
    strcpy(text, kVendor);
    return true;
}

//----------------------------------------------------------------------------
bool AmplitudeImposer::getProductString(char* text)
{
    strcpy(text, kProduct);
    return true;
}

//----------------------------------------------------------------------------
long AmplitudeImposer::getVendorVersion()
{
    return kVersionNo;
}

//----------------------------------------------------------------------------
VstPlugCategory AmplitudeImposer::getPlugCategory()
{
    return(kPlugCategEffect);
}

//----------------------------------------------------------------------------
bool AmplitudeImposer::getInputProperties(long index, VstPinProperties* properties)
{
	bool returnCode = false;
	if(index == 0)
	{
		sprintf(properties->label, "%s Left Input (Amp Env)", kEffectName);
		properties->flags = kVstPinIsStereo|kVstPinIsActive;
		returnCode = true;
	}
	else if(index == 1)
	{
		sprintf(properties->label, "%s Right Input (Amp Env)", kEffectName);
		properties->flags = kVstPinIsStereo|kVstPinIsActive;
		returnCode = true;
	}
	else if(index == 2)
	{
		sprintf(properties->label, "%s Left Input (Audio)", kEffectName);
		properties->flags = kVstPinIsStereo|kVstPinIsActive;
		returnCode = true;
	}
	else if(index == 3)
	{
		sprintf(properties->label, "%s Right Input (Audio)", kEffectName);
		properties->flags = kVstPinIsStereo|kVstPinIsActive;
		returnCode = true;
	}
	return returnCode;
}

//----------------------------------------------------------------------------
bool AmplitudeImposer::getOutputProperties(long index, VstPinProperties* properties)
{
	bool returnCode = false;
	int temp = 1;
	if(index == 0)
	{
		sprintf(properties->label, "%s Left Output %d", kEffectName, temp);
		properties->flags = kVstPinIsStereo|kVstPinIsActive;
		returnCode = true;
	}
	else if(index == 1)
	{
		sprintf(properties->label, "%s Right Output %d", kEffectName, temp);
		properties->flags = kVstPinIsStereo|kVstPinIsActive;
		returnCode = true;
	}
	return returnCode;
}

//----------------------------------------------------------------------------
long AmplitudeImposer::getGetTailSize()
{
	return 1; //1=no tail, 0=don't know, everything else=tail size
}

//----------------------------------------------------------------------------
void AmplitudeImposer::onoff2string(float value, char *text)
{
	if(value < 0.5f)
		strcpy(text, "   Off  ");
	else
		strcpy(text, "   On   ");
}

//----------------------------------------------------------------------------
//Called from process/Replacing to dispatch the appropriate MIDI events for the 'pos'
//frame in the current audio buffer.
//----------------------------------------------------------------------------
void AmplitudeImposer::processMIDI(long pos)
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

//----------------------------------------------------------------------------
//If we were using MIDI for something, we'd add it into these methods, rather
//than just passing the data on like a MIDI through.
//----------------------------------------------------------------------------
void AmplitudeImposer::MIDI_NoteOn(int ch, int note, int val, int delta)
{
	MIDIEvent[0]->midiData[0] = 0x90 + ch;
	MIDIEvent[0]->midiData[1] = note;
	MIDIEvent[0]->midiData[2] = val;
	MIDIEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(Events);
}

//----------------------------------------------------------------------------
void AmplitudeImposer::MIDI_NoteOff(int ch, int note, int val, int delta)
{
	MIDIEvent[0]->midiData[0] = 0x80 + ch;
	MIDIEvent[0]->midiData[1] = note;
	MIDIEvent[0]->midiData[2] = val;
	MIDIEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(Events);
}

//----------------------------------------------------------------------------
void AmplitudeImposer::MIDI_PolyAftertouch(int ch, int note, int val, int delta)
{
	MIDIEvent[0]->midiData[0] = 0xA0 + ch;
	MIDIEvent[0]->midiData[1] = note;
	MIDIEvent[0]->midiData[2] = val;
	MIDIEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(Events);
}

//----------------------------------------------------------------------------
void AmplitudeImposer::MIDI_CC(int ch, int num, int val, int delta)
{
	float temp;

	temp = val/127.0f;	// CC data

	MIDIEvent[0]->midiData[0] = 0xB0 + ch;
	MIDIEvent[0]->midiData[1] = num;
	MIDIEvent[0]->midiData[2] = val;
	MIDIEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(Events);
}

//----------------------------------------------------------------------------
//You don't have to do this, but I like to...
//----------------------------------------------------------------------------
void AmplitudeImposer::MIDI_ProgramChange(int ch, int val, int delta)
{
	if(val < kNumPrograms)
		setProgram(val);

	MIDIEvent[0]->midiData[0] = 0xD0 + ch;
	MIDIEvent[0]->midiData[1] = val;
	MIDIEvent[0]->midiData[2] = 0;
	MIDIEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(Events);
}

//----------------------------------------------------------------------------
void AmplitudeImposer::MIDI_ChannelAftertouch(int ch, int val, int delta)
{
	MIDIEvent[0]->midiData[0] = 0xD0 + ch;
	MIDIEvent[0]->midiData[1] = val;
	MIDIEvent[0]->midiData[2] = 0;
	MIDIEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(Events);
}

//----------------------------------------------------------------------------
void AmplitudeImposer::MIDI_PitchBend(int ch, int x1, int x2, int delta)
{
	MIDIEvent[0]->midiData[0] = 0xE0 + ch;
	MIDIEvent[0]->midiData[1] = x1;
	MIDIEvent[0]->midiData[2] = x2;
	MIDIEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(Events);
}