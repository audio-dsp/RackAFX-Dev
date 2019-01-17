//	Harmonical.cpp - Definition of the plugin.
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


#include <stdio.h>
#include <math.h>

#include "MainEditor.h"

#include "Harmonical.h"

extern bool oome;

//----------------------------------------------------------------------------
//Program Constructor - set default values of the program
//----------------------------------------------------------------------------
HarmonicalProg::HarmonicalProg()
{
	int i;

	for(i=0;i<Harmonical::numParams;i++)
		params[i] = 0.0f;

	strcpy(name, "Init");
}

//----------------------------------------------------------------------------
//Plugin Constructor - create programs, set certain variables
//----------------------------------------------------------------------------
Harmonical::Harmonical(audioMasterCallback audioMaster):
AudioEffectX(audioMaster, numPrograms, numParams)
{
	int i;

	programs = 0;
	ed3d = 0;
	noteMaster = 0;

	for(i=0;i<numParams;i++)
		params[i] = 0.0f;

	for(i=0;i<maxNumEvents;i++)
	{
		MIDIEvent[i] = new VstMidiEvent;

		MIDIEvent[i]->type = 1;
		MIDIEvent[i]->midiData[3] = 0;
		MIDIEvent[i]->reserved1 = 99;
		MIDIEvent[i]->deltaFrames = -99;
		MIDIEvent[i]->noteLength = 0;
		MIDIEvent[i]->noteOffset = 0;

		eventNumArray[i] = -1;
	}
	numPendingEvents = 0;
	events = new VstEvents;
	events->numEvents = 1;
	events->events[0] = (VstEvent *)MIDIEvent[0];
	numEvents = 0;

	frames = 0;

	tempo = 125.0f;
	samplerate = getSampleRate();
	if(samplerate <= 11025.0f)
		samplerate = 44100.0f;

	noteMaster = new NoteMaster(samplerate);

    programs = new HarmonicalProg[numPrograms];

	setPresets();

	strcpy(effectName, "Harmonical");
	strcpy(productName, "ndc Plugs Harmonical");
	strcpy(vendorName, "ndc Plugs");
	
	hasVu(VU);
    setNumInputs(numInputs);
    setNumOutputs(numOutputs);
    canMono(canMonoVar);
    canProcessReplacing(canReplacingVar);
    isSynth(isSynthVar);
    setUniqueID(ID);

	editor = new MainEditor(this);
	if(!editor)
		oome = true;

	noteMaster->setEditor(ed3d);

	//Move this to the individual voices...
	int j, k;
	float u[NUM_VERTICES];
	float v[NUM_VERTICES];
	k = 0;
	for(i=0;i<NUM_LOOPS;i++)
	{
		for(j=0;j<NUM_LOOPS;j++)
		{
			v[k] = ((float)i/NUM_LOOPS) * TWOPI;
			u[k] = ((float)j/NUM_LOOPS) * TWOPI;
			vertices[k].set(cosf(v[k])*cosf(u[k]),
							cosf(v[k])*sinf(u[k]),
							sinf(v[k]));
			k++;
		}
	}

	//ed3d is set from the editor itself
	if(ed3d)
	{
		for(i=0;i<NUM_VOICES;i++)
		{
			ed3d->setVertices(vertices, i, 0.0f);
		}
	}
}

//----------------------------------------------------------------------------
//Plugin Destructor - destroy programs, destroy buffers if you've got them
//----------------------------------------------------------------------------
Harmonical::~Harmonical()
{
	int i;

	if(programs)
		delete [] programs;

	for(i=0;i<maxNumEvents;i++)
	{
		if(MIDIEvent[i])
			delete MIDIEvent[i];
	}
	if(events)
		delete events;
}

//----------------------------------------------------------------------------
//Output = input + previous output.
//----------------------------------------------------------------------------
void Harmonical::process(float **inputs, float **outputs, long sampleFrames)
{
	long i;
	twofloats tempf;

	frames = sampleFrames;

	for(i=0;i<frames;i++)
	{
		if((numEvents>0))
		{
			processMIDI(i);
		}

		tempf = noteMaster->getSample();
		outputs[0][i] += tempf.left;
		outputs[1][i] += tempf.right;
	}
	if(numPendingEvents > 0)
	{
		for(i=0;i<numPendingEvents;i++)
		{
			MIDIEvent[eventNumArray[i]]->deltaFrames -= frames;
		}
	}
}

//----------------------------------------------------------------------------
//Output replaces old output.
//----------------------------------------------------------------------------
void Harmonical::processReplacing(float **inputs, float **outputs, long sampleFrames)
{
	long i;
	twofloats tempf;

	frames = sampleFrames;

	for(i=0;i<frames;i++)
	{
		if((numEvents>0))
		{
			processMIDI(i);
		}

		tempf = noteMaster->getSample();
		outputs[0][i] = tempf.left;
		outputs[1][i] = tempf.right;
	}
	if(numPendingEvents > 0)
	{
		for(i=0;i<numPendingEvents;i++)
		{
			MIDIEvent[eventNumArray[i]]->deltaFrames -= frames;
		}
	}
}

//----------------------------------------------------------------------------
//Here I just pass all the appropriate messages onto my MIDI queue (MIDIEvent).
//----------------------------------------------------------------------------
long Harmonical::processEvents(VstEvents *ev)
{
	int i, j, k;
	VstMidiEvent *event;

	for(i=0;i<ev->numEvents;i++)
	{
		if(ev->events[i]->type == kVstMidiType)
		{
			event = (VstMidiEvent *)ev->events[i];
			j = -1;

			for(k=1;k<maxNumEvents;k++)
			{
				if(MIDIEvent[k]->deltaFrames == -99)
				{
					eventNumArray[numPendingEvents] = k;
					numPendingEvents++;

					j = k;
					break;
				}
			}
			if((j > 0)&&(numEvents < maxNumEvents))
			{
				numEvents++;
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
void Harmonical::resume()
{
    wantEvents(); // important for all plugins that receive MIDI! 

	tempo = (float)((float)tempoAt(0)*0.0001f);
	if(tempo <= 0.0f)
		tempo = 120.0f;
	samplerate = getSampleRate();
	if(samplerate <= 11025.0f)
		samplerate = 44100.0f;

	noteMaster->setSamplerate(samplerate);

	setProgram(curProgram);
}

//----------------------------------------------------------------------------
void Harmonical::suspend()
{
    // buffers are normally cleared/initialized here
}

//----------------------------------------------------------------------------
void Harmonical::setProgram(long program)
{
	int i;

    HarmonicalProg *prog = &programs[program];

	curProgram = program;

	for(i=0;i<numParams;i++)
		setParameter(i, prog->params[i]);
}

//----------------------------------------------------------------------------
void Harmonical::setProgramName(char *name)
{
	strcpy(programs[curProgram].name, name);
}


//----------------------------------------------------------------------------
void Harmonical::getProgramName(char *name)
{
	if(!strcmp(programs[curProgram].name, "Init"))
		sprintf(name, "%s %d", programs[curProgram].name, curProgram + 1);
	else
		strcpy(name, programs[curProgram].name);
}

//----------------------------------------------------------------------------
bool Harmonical::getProgramNameIndexed(long category, long index, char* text)
{
    return false;
    if(index<numPrograms)
    {
      strcpy(text,programs[index].name);
      return true;
    }
}

//----------------------------------------------------------------------------
bool Harmonical::copyProgram(long destination)
{
	bool returnCode = false;
    if(destination < numPrograms)
    {
		programs[destination] = programs[curProgram];
        returnCode = true;
    }
    return returnCode;
}

//----------------------------------------------------------------------------
void Harmonical::setParameter(long index, float value)
{
	HarmonicalProg *prog = &programs[curProgram];

	params[index] = value;
	prog->params[index] = value;

	noteMaster->setParameter(index, value);

#ifdef PATCH_GEN
	if(value > 0.5f)
		writeParameters();
#endif

	if(editor)
	{
		//editor->postUpdate();
		((MainEditor *)editor)->setParameter(index, value);
	}
}

//----------------------------------------------------------------------------
float Harmonical::getParameter(long index)
{
	return params[index];
}

//----------------------------------------------------------------------------
void Harmonical::getParameterLabel(long index, char *label)
{
	strcpy(label, " ");
}

//----------------------------------------------------------------------------
//gets values for parameter display - see 'AudioEffect.hpp' for others (float2string,
//Hz2string etc.)
//----------------------------------------------------------------------------
void Harmonical::getParameterDisplay(long index, char *text)
{
	float2string(params[index], text);
}

//----------------------------------------------------------------------------
void Harmonical::getParameterName(long index, char *label)
{
	char tempch[8];

	sprintf(tempch, "p%d", index);
	strcpy(label, tempch);
}

//----------------------------------------------------------------------------
//Tells the host what the plugin can do: 1=can do, -1=cannot do, 0=don't know.
//----------------------------------------------------------------------------
long Harmonical::canDo(char *text)
{
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
float Harmonical::getVu()
{
	return 0.0f;
}

//----------------------------------------------------------------------------
bool Harmonical::getEffectName(char* name)
{
    strcpy(name,effectName);
    return true;
}

//----------------------------------------------------------------------------
bool Harmonical::getVendorString(char* text)
{
    strcpy(text, vendorName);
    return true;
}

//----------------------------------------------------------------------------
bool Harmonical::getProductString(char* text)
{
    strcpy(text, productName);
    return true;
}

//----------------------------------------------------------------------------
long Harmonical::getVendorVersion()
{
    return versionNo;
}

//----------------------------------------------------------------------------
VstPlugCategory Harmonical::getPlugCategory()
{
    return(kPlugCategEffect);
}

//----------------------------------------------------------------------------
bool Harmonical::getInputProperties(long index, VstPinProperties* properties)
{
	bool returnCode = false;
	int temp = 1;
	if(index == 0)
	{
		sprintf(properties->label, "%s Left Input %d", effectName, temp);
		properties->flags = kVstPinIsStereo|kVstPinIsActive;
		returnCode = true;
	}
	else if(index == 1)
	{
		sprintf(properties->label, "%s Right Input %d", effectName, temp);
		properties->flags = kVstPinIsStereo|kVstPinIsActive;
		returnCode = true;
	}
	return returnCode;
}

//----------------------------------------------------------------------------
bool Harmonical::getOutputProperties(long index, VstPinProperties* properties)
{
	bool returnCode = false;
	int temp = 1;
	if(index == 0)
	{
		sprintf(properties->label, "%s Left Output %d", effectName, temp);
		properties->flags = kVstPinIsStereo|kVstPinIsActive;
		returnCode = true;
	}
	else if(index == 1)
	{
		sprintf(properties->label, "%s Right Output %d", effectName, temp);
		properties->flags = kVstPinIsStereo|kVstPinIsActive;
		returnCode = true;
	}
	return returnCode;
}

//----------------------------------------------------------------------------
long Harmonical::getGetTailSize()
{
	return 1; //1=no tail, 0=don't know, everything else=tail size
}

//----------------------------------------------------------------------------
void Harmonical::onoff2string(float value, char *text)
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
void Harmonical::processMIDI(long pos)
{
	int data1, data2;
	int status, ch, delta;
	int note;
	int i, j;

	for(i=0;i<numPendingEvents;i++)
	{
		if(((MIDIEvent[eventNumArray[i]]->deltaFrames) == pos)||
		   ((MIDIEvent[eventNumArray[i]]->deltaFrames) < 0))
		{
			//--pass on/act on event--
			delta = 0; //because we're at pos frames into the buffer...
			ch = (MIDIEvent[eventNumArray[i]]->midiData[0] & 0x0F);
			status = (MIDIEvent[eventNumArray[i]]->midiData[0] & 0xF0);
			data1 = (MIDIEvent[eventNumArray[i]]->midiData[1] & 0x7F);
			data2 = (MIDIEvent[eventNumArray[i]]->midiData[2] & 0x7F);
			//delta = MIDIEvent[eventNumArray[i]]->deltaFrames;	//huh?

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
			MIDIEvent[eventNumArray[i]]->deltaFrames = -99;
			--numEvents;

			//--reset EventNumArray--
			for(j=(i+1);j<numPendingEvents;j++)
			{
				eventNumArray[(j-1)] = eventNumArray[j];
			}
			--numPendingEvents;
			//break;
		}
	}
}

//----------------------------------------------------------------------------
//If we were using MIDI for something, we'd add it into these methods, rather
//than just passing the data on like a MIDI through.
//----------------------------------------------------------------------------
void Harmonical::MIDI_NoteOn(int ch, int note, int val, int delta)
{
	MIDIEvent[0]->midiData[0] = 0x90 + ch;
	MIDIEvent[0]->midiData[1] = note;
	MIDIEvent[0]->midiData[2] = val;
	MIDIEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(events);

	noteMaster->onNoteOn(note, (static_cast<float>(val)/127.0f));
}

//----------------------------------------------------------------------------
void Harmonical::MIDI_NoteOff(int ch, int note, int val, int delta)
{
	MIDIEvent[0]->midiData[0] = 0x80 + ch;
	MIDIEvent[0]->midiData[1] = note;
	MIDIEvent[0]->midiData[2] = val;
	MIDIEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(events);

	noteMaster->onNoteOff(note);
}

//----------------------------------------------------------------------------
void Harmonical::MIDI_PolyAftertouch(int ch, int note, int val, int delta)
{
	MIDIEvent[0]->midiData[0] = 0xA0 + ch;
	MIDIEvent[0]->midiData[1] = note;
	MIDIEvent[0]->midiData[2] = val;
	MIDIEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(events);
}

//----------------------------------------------------------------------------
void Harmonical::MIDI_CC(int ch, int num, int val, int delta)
{
	float tempf;

	tempf = static_cast<float>(val)/127.0f;	// CC data

	MIDIEvent[0]->midiData[0] = 0xB0 + ch;
	MIDIEvent[0]->midiData[1] = num;
	MIDIEvent[0]->midiData[2] = val;
	MIDIEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(events);

	noteMaster->onCC(num, tempf);
}

//----------------------------------------------------------------------------
//You don't have to do this, but I like to...
//----------------------------------------------------------------------------
void Harmonical::MIDI_ProgramChange(int ch, int val, int delta)
{
	if(val < numPrograms)
		setProgram(val);

	MIDIEvent[0]->midiData[0] = 0xD0 + ch;
	MIDIEvent[0]->midiData[1] = val;
	MIDIEvent[0]->midiData[2] = 0;
	MIDIEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(events);
}

//----------------------------------------------------------------------------
void Harmonical::MIDI_ChannelAftertouch(int ch, int val, int delta)
{
	MIDIEvent[0]->midiData[0] = 0xD0 + ch;
	MIDIEvent[0]->midiData[1] = val;
	MIDIEvent[0]->midiData[2] = 0;
	MIDIEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(events);
}

//----------------------------------------------------------------------------
void Harmonical::MIDI_PitchBend(int ch, int x1, int x2, int delta)
{
	float pbend, tempf;

	MIDIEvent[0]->midiData[0] = 0xE0 + ch;
	MIDIEvent[0]->midiData[1] = x1;
	MIDIEvent[0]->midiData[2] = x2;
	MIDIEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(events);

	tempf = (float)(x2 * 128);         //MSB
    pbend = ((tempf + (float)x1)/16384.0f);//LSB - pitchbend data (2 bytes worth)

	noteMaster->onPitchBend(pbend);
}

//----------------------------------------------------------------------------
#ifdef PATCH_GEN
void Harmonical::writeParameters()
{
	int i;
	FILE *fp;

	fp = fopen("D:\\patch.txt", "w");
	if(!fp)
		return;

	for(i=0;i<(numParams-1);i++)
		fprintf(fp, "	setParameter(%d, %f);\n", i, params[i]);

	fclose(fp);
}
#endif
