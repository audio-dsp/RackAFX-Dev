//	VstPlugin.cpp - Definition of the plugin class.
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

#include "VstPlugin.h"
#include "Editor.h"

#include <cmath>

using namespace std;

//-------------------------------------------------------------------------------------------------------
AudioEffect *createEffectInstance(audioMasterCallback audioMaster)
{
	return new VstPlugin(audioMaster);
}

//----------------------------------------------------------------------------
VstPlugin::VstPlugin(audioMasterCallback audioMaster):
AudioEffectX(audioMaster, numPrograms, numParameters),
programs(0),
samplerate(44100.0f),
tempEvents(0),
numEvents(0),
numPendingEvents(0),
frames(0),
effectName("SoulForce"),
vendorName("ndc Plugs"),
coeff(0.5f),
wave(0.0f),
env(0.0f),
footEnv(0.0f)
{
	int i;

	//Initialise parameters.
	for(i=0;i<numParameters;++i)
		parameters[i] = 0.0f;

	//Setup event queue.
	for(i=0;i<maxNumEvents;++i)
	{
		midiEvent[i] = new VstMidiEvent;

		midiEvent[i]->type = 1;
		midiEvent[i]->midiData[3] = 0;
		midiEvent[i]->reserved1 = 99;
		midiEvent[i]->deltaFrames = -99;
		midiEvent[i]->noteLength = 0;
		midiEvent[i]->noteOffset = 0;

		eventNumArray[i] = -1;
	}
	tempEvents = new VstEvents;
	tempEvents->numEvents = 1;
	tempEvents->events[0] = (VstEvent *)midiEvent[0];

	//Setup programs here.
	programs = new PluginProgram[numPrograms];

	setProgram(8);
	setParameter(Shape, 0.3f);
	setParameter(Feedback, 0.5f);
	setParameter(Source, 1.0f);
	setParameter(Footswitch, 1.0f);
	setProgramName("Soul Force");

	setProgram(7);
	setParameter(Shape, 0.3f);
	setParameter(Feedback, 0.5f);
	setParameter(Source, 0.0f);
	setParameter(Footswitch, 1.0f);
	setProgramName("You and Only You");

	setProgram(6);
	setParameter(Shape, 0.0f);
	setParameter(Feedback, 1.0f);
	setParameter(Source, 1.0f);
	setParameter(Footswitch, 1.0f);
	setProgramName("Past is Past");

	setProgram(5);
	setParameter(Shape, 0.5f);
	setParameter(Feedback, 1.0f);
	setParameter(Source, 1.0f);
	setParameter(Footswitch, 1.0f);
	setProgramName("Do Yourself a Favour");

	setProgram(4);
	setParameter(Shape, 0.0f);
	setParameter(Feedback, 1.0f);
	setParameter(Source, 0.0f);
	setParameter(Footswitch, 1.0f);
	setProgramName("Tumble to the Power");

	setProgram(3);
	setParameter(Shape, 0.5f);
	setParameter(Feedback, 1.0f);
	setParameter(Source, 0.0f);
	setParameter(Footswitch, 1.0f);
	setProgramName("Guerilla Love");

	setProgram(2);
	setParameter(Shape, 1.0f);
	setParameter(Feedback, 0.0f);
	setParameter(Source, 0.0f);
	setParameter(Footswitch, 1.0f);
	setProgramName("Looking for the World");

	setProgram(1);
	setParameter(Shape, 0.4f);
	setParameter(Feedback, 0.0f);
	setParameter(Source, 0.0f);
	setParameter(Footswitch, 1.0f);
	setProgramName("Stay Down");

	setProgram(0);
	setParameter(Shape, 0.5f);
	setParameter(Feedback, 0.0f);
	setParameter(Source, 0.0f);
	setParameter(Footswitch, 1.0f);
	setProgramName("Default");

	for(i=9;i<numPrograms;++i)
		copyProgram(i);

	//Set various constants.
    setNumInputs(2);
    setNumOutputs(2);
    canProcessReplacing(true);
    isSynth(false);
    setUniqueID('SlFr');

	//Construct editor here.
	editor = new Editor(this);
}

//----------------------------------------------------------------------------
VstPlugin::~VstPlugin()
{
	int i;

	//Delete event queue.
	for(i=0;i<maxNumEvents;++i)
	{
		if(midiEvent[i])
			delete midiEvent[i];
	}
	if(tempEvents)
		delete tempEvents;

	//Delete programs.
	if(programs)
		delete [] programs;
}

//----------------------------------------------------------------------------
void VstPlugin::process(float **inputs, float **outputs, VstInt32 sampleFrames)
{
	VstInt32 i;

	frames = sampleFrames;

	//Calculate audio.
	for(i=0;i<frames;++i)
	{
		//Process MIDI events,if there are any.
		if(numEvents>0)
			processMIDI(i);

		outputs[0][i] += inputs[0][i];
		outputs[1][i] += inputs[1][i];
	}
	//If there are events remaining in the queue, update their delta values.
	if(numPendingEvents > 0)
	{
		for(i=0;i<numPendingEvents;++i)
			midiEvent[eventNumArray[i]]->deltaFrames -= frames;
	}
}

//----------------------------------------------------------------------------
void VstPlugin::processReplacing(float **inputs,
								 float **outputs,
								 VstInt32 sampleFrames)
{
	VstInt32 i;
	float tempf;
	float inLeft, inRight;

	frames = sampleFrames;

	//Calculate audio.
	for(i=0;i<frames;++i)
	{
		//Process MIDI events,if there are any.
		if(numEvents>0)
			processMIDI(i);

		//For footEnv, later.
		inLeft = inputs[0][i];
		inRight = inputs[1][i];

		//Update coeff, if necessary.
		if(parameters[Feedback] > 0.0f)
		{
			float tempf2;
			if(parameters[Source] > 0.5f)
			{
				tempf2 = (1.0f-parameters[Feedback]) * parameters[Shape];
				tempf2 += parameters[Feedback] * wave;
				coeff = (1.0f-(1.0f/((tempf2 * 0.99f)+0.005f)));
			}
			else
			{
				if(fabs(inLeft) > env)
					env = fabs(inLeft);
				else if(env > 0.0f)
				{
					env -= 0.001f;
					if(env < 0.0f)
						env = 0.0f;
				}

				tempf2 = (1.0f-parameters[Feedback]) * parameters[Shape];
				tempf2 += parameters[Feedback] * env;
				coeff = (1.0f-(1.0f/((tempf2 * 0.99f)+0.005f)));
			}
		}

		tempf = inLeft;
		if(tempf > 0.0f)
			tempf = tempf/(tempf+(coeff*(tempf-1.0f)));
		else if(tempf == 0.0f)
			tempf = 0.0f;
		else
		{
			tempf = fabs(tempf);
			//tempf += 1.0f;
			tempf = 1.0f - (tempf/(tempf+(coeff*(tempf-1.0f))));
			tempf -= 1.0f;
		}
		if(parameters[Shape] < 0.5f)
			tempf *= ((0.5f-parameters[Shape])*16.0f)+1.0f;
		if(parameters[Source] > 0.5f)
			tempf *= 1.0f + (parameters[Feedback] * 2.0f);
		outputs[0][i] = tempf;

		tempf = inRight;
		if(tempf >= 0.0f)
			tempf = tempf/(tempf+(coeff*(tempf-1.0f)));
		else if(tempf == 0.0f)
			tempf = 0.0f;
		else
		{
			tempf = fabs(tempf);
			//tempf += 1.0f;
			tempf = 1.0f - (tempf/(tempf+(coeff*(tempf-1.0f))));
			tempf -= 1.0f;
		}
		if(parameters[Shape] < 0.5f)
			tempf *= ((0.5f-parameters[Shape])*16.0f)+1.0f;
		if(parameters[Source] > 0.5f)
			tempf *= 1.0f + (parameters[Feedback] * 2.0f);
		outputs[1][i] = tempf;

		outputs[0][i] = tanh(outputs[0][i]) * 0.9f;
		outputs[1][i] = tanh(outputs[1][i]) * 0.9f;
		wave = outputs[0][i];

		//Handle footswitch.
		outputs[0][i] *= footEnv;
		outputs[0][i] += (1.0f-footEnv) * inLeft;
		outputs[1][i] *= footEnv;
		outputs[1][i] += (1.0f-footEnv) * inRight;

		if((parameters[Footswitch] > 0.5f) && (footEnv < 1.0f))
			footEnv += 0.01f;
		else if((parameters[Footswitch] < 0.5f) && (footEnv > 0.0f))
			footEnv -= 0.01f;
	}
	//If there are events remaining in the queue, update their delta values.
	if(numPendingEvents > 0)
	{
		for(i=0;i<numPendingEvents;++i)
			midiEvent[eventNumArray[i]]->deltaFrames -= frames;
	}
}

//----------------------------------------------------------------------------
VstInt32 VstPlugin::processEvents(VstEvents *events)
{
	int i, j, k;
	VstMidiEvent *event;

	//Go through each event in events.
	for(i=0;i<events->numEvents;++i)
	{
		//Only interested in MIDI events.
		if(events->events[i]->type == kVstMidiType)
		{
			event = (VstMidiEvent *)events->events[i];
			j = -1;

			//Find a space for it in the midiEvent queue.
			for(k=1;k<maxNumEvents;++k)
			{
				if(midiEvent[k]->deltaFrames == -99)
				{
					eventNumArray[numPendingEvents] = k;
					++numPendingEvents;

					j = k;
					break;
				}
			}
			//Add it to the queue if there's still room.
			if((j > 0)&&(numEvents < maxNumEvents))
			{
				numEvents++;
				midiEvent[j]->midiData[0] =		event->midiData[0];
				midiEvent[j]->midiData[1] =		event->midiData[1];
				midiEvent[j]->midiData[2] =		event->midiData[2];
				midiEvent[j]->midiData[3] =		event->midiData[3];
				midiEvent[j]->type =			event->type;
				midiEvent[j]->byteSize =		event->byteSize;
				midiEvent[j]->deltaFrames =		event->deltaFrames;
				midiEvent[j]->flags =			event->flags;
				midiEvent[j]->noteLength =		event->noteLength;
				midiEvent[j]->noteOffset =		event->noteOffset;
				midiEvent[j]->detune =			event->detune;
				midiEvent[j]->noteOffVelocity = event->noteOffVelocity;
				midiEvent[j]->reserved1 =		99;
				midiEvent[j]->reserved2 =		event->reserved2;
			}
		}
	}

	return 1;
}

//----------------------------------------------------------------------------
void VstPlugin::resume()
{
	//Let the host know we want to receive MIDI events.
	AudioEffectX::resume();

	//Get samplerate.
	samplerate = getSampleRate();
	if(samplerate <= 11025.0f)
		samplerate = 44100.0f;
}

//----------------------------------------------------------------------------
void VstPlugin::suspend()
{
    
}

//----------------------------------------------------------------------------
void VstPlugin::setProgram(VstInt32 program)
{
	int i;

	curProgram = program;

	for(i=0;i<numParameters;++i)
		setParameter(i, programs[curProgram].parameters[i]);
}

//----------------------------------------------------------------------------
void VstPlugin::setProgramName(char *name)
{
	programs[curProgram].name = name;
}


//----------------------------------------------------------------------------
void VstPlugin::getProgramName(char *name)
{
	strcpy(name, programs[curProgram].name.c_str());
}

//----------------------------------------------------------------------------
bool VstPlugin::getProgramNameIndexed(VstInt32 category, VstInt32 index, char* text)
{
	bool retval = false;

    if(index < numPrograms)
    {
		strcpy(text, programs[index].name.c_str());
		retval = true;
    }

	return retval;
}

//----------------------------------------------------------------------------
bool VstPlugin::copyProgram(VstInt32 destination)
{
	bool retval = false;

    if(destination < numPrograms)
    {
		programs[destination] = programs[curProgram];
        retval = true;
    }

    return retval;
}

//----------------------------------------------------------------------------
void VstPlugin::setParameter(VstInt32 index, float value)
{
	parameters[index] = value;
	programs[curProgram].parameters[index] = parameters[index];

	if(index == Shape)
		coeff = (1.0f-(1.0f/((value * 0.99f)+0.005f)));
	else if((index == Feedback) && (value == 0.0f))
		coeff = (1.0f-(1.0f/((parameters[Shape] * 0.99f)+0.005f)));

	if(editor)
	{
		((AEffGUIEditor *)editor)->setParameter(index, value);
	}
}

//----------------------------------------------------------------------------
float VstPlugin::getParameter(VstInt32 index)
{
	return parameters[index];
}

//----------------------------------------------------------------------------
void VstPlugin::getParameterLabel(VstInt32 index, char *label)
{
	strcpy(label, " ");
}

//----------------------------------------------------------------------------
void VstPlugin::getParameterDisplay(VstInt32 index, char *text)
{
	float2string(parameters[index], text, 5);
}

//----------------------------------------------------------------------------
void VstPlugin::getParameterName(VstInt32 index, char *label)
{
	char tempch[8];

	switch(index)
	{
		case Shape:
			sprintf(tempch, "Shape");
			break;
		case Feedback:
			sprintf(tempch, "FBack");
			break;
		case Source:
			sprintf(tempch, "Source");
			break;
		case Footswitch:
			sprintf(tempch, "Foot");
			break;
	}
	strcpy(label, tempch);
}

//----------------------------------------------------------------------------
VstInt32 VstPlugin::canDo(char *text)
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
float VstPlugin::getVu()
{
	return 0.0f;
}

//----------------------------------------------------------------------------
bool VstPlugin::getEffectName(char* name)
{
    strcpy(name, effectName.c_str());

    return true;
}

//----------------------------------------------------------------------------
bool VstPlugin::getVendorString(char* text)
{
    strcpy(text, vendorName.c_str());

    return true;
}

//----------------------------------------------------------------------------
bool VstPlugin::getProductString(char* text)
{
    strcpy(text, effectName.c_str());

    return true;
}

//----------------------------------------------------------------------------
VstInt32 VstPlugin::getVendorVersion()
{
    return versionNumber;
}

//----------------------------------------------------------------------------
VstPlugCategory VstPlugin::getPlugCategory()
{
    return(kPlugCategEffect);
}

//----------------------------------------------------------------------------
bool VstPlugin::getInputProperties(VstInt32 index, VstPinProperties* properties)
{
	bool retval = false;

	if(index == 0)
	{
		sprintf(properties->label, "%s Left Input 1", effectName.c_str());
		properties->flags = kVstPinIsStereo|kVstPinIsActive;
		retval = true;
	}
	else if(index == 1)
	{
		sprintf(properties->label, "%s Right Input 1", effectName.c_str());
		properties->flags = kVstPinIsStereo|kVstPinIsActive;
		retval = true;
	}

	return retval;
}

//----------------------------------------------------------------------------
bool VstPlugin::getOutputProperties(VstInt32 index, VstPinProperties* properties)
{
	bool retval = false;

	if(index == 0)
	{
		sprintf(properties->label, "%s Left Output 1", effectName.c_str());
		properties->flags = kVstPinIsStereo|kVstPinIsActive;
		retval = true;
	}
	else if(index == 1)
	{
		sprintf(properties->label, "%s Right Output 1", effectName.c_str());
		properties->flags = kVstPinIsStereo|kVstPinIsActive;
		retval = true;
	}

	return retval;
}

//----------------------------------------------------------------------------
VstInt32 VstPlugin::getGetTailSize()
{
	return 1; //1=no tail, 0=don't know, everything else=tail size
}

//----------------------------------------------------------------------------
void VstPlugin::processMIDI(VstInt32 pos)
{
	int data1, data2;
	int status, ch, delta;
	int note;
	int i, j;

	for(i=0;i<numPendingEvents;++i)
	{
		if((midiEvent[eventNumArray[i]]->deltaFrames%frames) == pos)
		{
			//--pass on/act on event--
			delta = 0; //because we're at pos frames into the buffer...
			ch = (midiEvent[eventNumArray[i]]->midiData[0] & 0x0F);
			status = (midiEvent[eventNumArray[i]]->midiData[0] & 0xF0);
			data1 = (midiEvent[eventNumArray[i]]->midiData[1] & 0x7F);
			data2 = (midiEvent[eventNumArray[i]]->midiData[2] & 0x7F);

			note = data1;

			switch(status)
			{
				case 0x90:
					if(data2 > 0)
						MIDI_NoteOn(ch, data1, data2, delta);
					else
						MIDI_NoteOff(ch, data1, data2, delta);
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
			midiEvent[eventNumArray[i]]->deltaFrames = -99;
			--numEvents;

			//--reset EventNumArray--
			for(j=(i+1);j<numPendingEvents;++j)
				eventNumArray[(j-1)] = eventNumArray[j];
			--numPendingEvents;
			//break;
		}
	}
}

//----------------------------------------------------------------------------
void VstPlugin::MIDI_NoteOn(int ch, int note, int val, int delta)
{
	midiEvent[0]->midiData[0] = 0x90 + ch;
	midiEvent[0]->midiData[1] = note;
	midiEvent[0]->midiData[2] = val;
	midiEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(tempEvents);
}

//----------------------------------------------------------------------------
void VstPlugin::MIDI_NoteOff(int ch, int note, int val, int delta)
{
	midiEvent[0]->midiData[0] = 0x80 + ch;
	midiEvent[0]->midiData[1] = note;
	midiEvent[0]->midiData[2] = val;
	midiEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(tempEvents);
}

//----------------------------------------------------------------------------
void VstPlugin::MIDI_PolyAftertouch(int ch, int note, int val, int delta)
{
	midiEvent[0]->midiData[0] = 0xA0 + ch;
	midiEvent[0]->midiData[1] = note;
	midiEvent[0]->midiData[2] = val;
	midiEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(tempEvents);
}

//----------------------------------------------------------------------------
void VstPlugin::MIDI_CC(int ch, int num, int val, int delta)
{
	float tempf;

	tempf = static_cast<float>(val)/127.0f;	// CC data

	midiEvent[0]->midiData[0] = 0xB0 + ch;
	midiEvent[0]->midiData[1] = num;
	midiEvent[0]->midiData[2] = val;
	midiEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(tempEvents);
}

//----------------------------------------------------------------------------
void VstPlugin::MIDI_ProgramChange(int ch, int val, int delta)
{
	if(val < numPrograms)
		setProgram(val);

	midiEvent[0]->midiData[0] = 0xD0 + ch;
	midiEvent[0]->midiData[1] = val;
	midiEvent[0]->midiData[2] = 0;
	midiEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(tempEvents);
}

//----------------------------------------------------------------------------
void VstPlugin::MIDI_ChannelAftertouch(int ch, int val, int delta)
{
	midiEvent[0]->midiData[0] = 0xD0 + ch;
	midiEvent[0]->midiData[1] = val;
	midiEvent[0]->midiData[2] = 0;
	midiEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(tempEvents);
}

//----------------------------------------------------------------------------
void VstPlugin::MIDI_PitchBend(int ch, int x1, int x2, int delta)
{
	midiEvent[0]->midiData[0] = 0xE0 + ch;
	midiEvent[0]->midiData[1] = x1;
	midiEvent[0]->midiData[2] = x2;
	midiEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(tempEvents);
}
