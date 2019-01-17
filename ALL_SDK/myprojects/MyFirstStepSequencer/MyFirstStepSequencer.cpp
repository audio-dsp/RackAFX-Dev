//	MyFirstStepSequencer.cpp - Definition of the plugin.
//					     (remember: bits to alter are marked: //**)
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

#include "MyFirstStepSequencer.h"
#include "MFSSGui.h"
#include "Round.h"
#include <time.h>
#include <cstdio>

using namespace std;

extern bool oome;

//----------------------------------------------------------------------------
//Program Constructor - set default values of the program
//----------------------------------------------------------------------------
MyFirstStepSequencerProg::MyFirstStepSequencerProg()
{
	int i;

	for(i=0;i<MyFirstStepSequencer::numParams;++i)
		params[i] = 0.0f;

	strcpy(name, "Init");
}

//----------------------------------------------------------------------------
//Plugin Constructor - create programs, set certain variables
//----------------------------------------------------------------------------
MyFirstStepSequencer::MyFirstStepSequencer(audioMasterCallback audioMaster):
AudioEffectX(audioMaster, numPrograms, numParams),
numSteps(16),
currentDelta(0),
barStartSamples(0),
currentlyPlaying(false),
lastBypass(0.0f),
rightClick(false)
{
	int i;

	srand(time(0));

	programs = 0;

	for(i=0;i<12;++i)
		allowedNotes[i] = true;

	for(i=0;i<numParams;++i)
		params[i] = 0.0f;

	for(i=0;i<maxNumEvents;++i)
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

	for(i=0;i<numSteps;++i)
	{
		steps[i].setId(i);
		steps[i].registerEventHandler(this);

		//Just in case...
		steps[i].setSamplerate(samplerate);
		steps[i].setBpm(tempo);
	}
	stepIndex = 0;

    programs = new MyFirstStepSequencerProg[numPrograms];

	/*if(programs)
	{
		setProgram(0);
		for(i=0;i<numParams;++i)
			setParameter(i, 0.0f);
		for(i=Step1Volume;i<Octave;++i)
			setParameter(i, 1.0f);
		setParameter(Octave, (4.0f/9.0f));
		setParameter(Shuffle, 0.0f);
		setParameter(EightStepsMode, 0.0f);
		setParameter(Bypass, 0.0f);
		for(i=ScaleStart; i<Random;++i)
			setParameter(i, 1.0f);
		setParameter(Random, 0.0f);
	    setProgramName("My First Step Sequencer");
	}*/
	setupPresets();

	strcpy(effectName, "My First Step Sequencer");
	strcpy(productName, "ndc Plugs Sequencer");
	strcpy(vendorName, "ndc Plugs");
	
	hasVu(VU);
    setNumInputs(numInputs);
    setNumOutputs(numOutputs);
    canMono(canMonoVar);
    canProcessReplacing(canReplacingVar);
    isSynth(isSynthVar);
    setUniqueID(ID);

	tempo = 125.0f;
	samplerate = getSampleRate();
	if(samplerate <= 11025.0f)
		samplerate = 44100.0f;

	editor = new MFSSGui(this);
	if(!editor)
		oome = true;
}

//----------------------------------------------------------------------------
//Plugin Destructor - destroy programs, destroy buffers if you've got them
//----------------------------------------------------------------------------
MyFirstStepSequencer::~MyFirstStepSequencer()
{
	int i;

	if(programs)
		delete [] programs;

	for(i=0;i<maxNumEvents;++i)
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
void MyFirstStepSequencer::process(float **inputs, float **outputs, long sampleFrames)
{
	long i;
	unsigned char j;
	VstTimeInfo	*timeInfo;
	double currentPPQ;
	double quartersPerBar;
	double remainingTime;
	long numSamplesInOneStep;
	long numSamplesInOneBar;
	long newStepOffset;
	bool resetHere = false;
	long resetAt;
	float newTempo;
	MidiMessage tempMess;

	frames = sampleFrames;
	timeInfo = getTimeInfo(kVstPpqPosValid |
						   kVstBarsValid |
						   kVstTimeSigValid |
						   kVstTempoValid);
	newTempo = static_cast<float>(timeInfo->tempo);
	if(tempo != newTempo)
	{
		tempo = newTempo;

		for(i=0;i<numSteps;++i)
			steps[i].setBpm(tempo);
	}

	if(timeInfo)
	{
		//If we're not currently playing, but we're supposed to be.
		if((!currentlyPlaying) && (timeInfo->flags & kVstTransportPlaying))
		{
			//If these flags aren't valid, we're stuffed...
			if((timeInfo->flags & kVstPpqPosValid)&&
			   (timeInfo->flags & kVstBarsValid) &&
			   (timeInfo->flags & kVstTimeSigValid))
			{
				currentlyPlaying = true;

				//Calculate barStartSamples (from Steps' offset position).
				currentPPQ = timeInfo->ppqPos - timeInfo->barStartPos;
				quartersPerBar = ((1.0/timeInfo->timeSigDenominator) * 4.0) *
								  timeInfo->timeSigNumerator;
				remainingTime = (60.0/static_cast<double>(tempo)) *
								(quartersPerBar - currentPPQ);
				barStartSamples = static_cast<long>(remainingTime *
								  samplerate);
				//barStartSamples -= blockSize; //Because this is for the Steps, which are offset to ensure we always send the correct delta frames to the host...

				//Work out which Step should be active.
				/*//********
				stepIndex = static_cast<long>(currentPPQ * 4.0);
				if(stepIndex > 15) //In case we're not in 4/4 (although we're probably screwed by this point anyway...).
					stepIndex = 0;
				//*********/
				stepIndex = getCorrectStep(currentPPQ);

				//Activate it with the correct offset.
				numSamplesInOneStep = static_cast<long>(0.25f *
									  (samplerate / (tempo/60.0f)));
				numSamplesInOneBar = static_cast<long>(4.0f *
									 (samplerate / (tempo/60.0f)));
				newStepOffset = (numSamplesInOneBar - barStartSamples);
				newStepOffset -= (stepIndex * numSamplesInOneStep);
				steps[stepIndex].activate(newStepOffset);
			}
		}
		else
		{
			//Check if the host's stopped, currentlyPlaying = false;
			if(!(timeInfo->flags & kVstTransportPlaying))
			{
				currentlyPlaying = false;
				for(i=0;i<numSteps;++i)
					steps[i].deactivate();
			}
			else //Update barStartSamples.
			{
				//If these flags aren't valid, we can't check.
				if((timeInfo->flags & kVstPpqPosValid)&&
				   (timeInfo->flags & kVstBarsValid) &&
				   (timeInfo->flags & kVstTimeSigValid))
				{
					//Calculate barStartSamples.
					currentPPQ = timeInfo->ppqPos - timeInfo->barStartPos;
					quartersPerBar = ((1.0/timeInfo->timeSigDenominator) * 4.0) *
									  timeInfo->timeSigNumerator;
					remainingTime = (60.0/static_cast<double>(tempo)) *
									(quartersPerBar - currentPPQ);
					barStartSamples = static_cast<long>(remainingTime *
									  samplerate);
					//barStartSamples -= blockSize;

					//If there should be a bar start w/in this block.
					if(barStartSamples < frames)
					{
						//Indicate that we need to reset to the 1st step,
						//resetAt samples from now.
						resetHere = true;
						resetAt = barStartSamples;
					}
				}
			}
		}
	}

	for(i=0;i<frames;++i)
	{
		if((numEvents>0))
			processMIDI(i);

		currentDelta = i;
		//barStartSamples--;

		//Make sure note values are updated if the user's changed them.
		for(j=0;j<16;++j)
			steps[j].updateNoteValue();
		//Make sure we're in bypass mode if the user's switched it.
		if(lastBypass != params[Bypass])
		{
			lastBypass = params[Bypass];
			if(lastBypass > 0.5f)
			{
				tempMess = steps[stepIndex].getMIDI(false);

				MIDIEvent[0]->midiData[0] = tempMess[0];
				MIDIEvent[0]->midiData[1] = tempMess[1];
				MIDIEvent[0]->midiData[2] = tempMess[2];
				MIDIEvent[0]->deltaFrames = currentDelta + blockSize;
				sendVstEventsToHost(events);
			}
		}

		if(currentlyPlaying)
		{
			//i.e. if we've reached the end of this step...
			if(!steps[stepIndex].tick())
			{
				stepIndex++;
				if(stepIndex > (numSteps-1))
					stepIndex = 0;

				steps[stepIndex].activate();
			}

			//i.e. when the bar start occurs, we reset to the first step,
			//just in case something strange's happened.
			if(resetHere)
			{
				resetAt--;
				if(resetAt == 0)
				{
					steps[stepIndex].deactivate();

					stepIndex = 0;
					steps[stepIndex].activate();

					resetHere = false;
				}
			}
		}

		outputs[0][i] += inputs[0][i];
		outputs[1][i] += inputs[1][i];
	}
	if(numPendingEvents > 0)
	{
		for(i=0;i<numPendingEvents;++i)
			MIDIEvent[eventNumArray[i]]->deltaFrames -= frames;
	}
	currentDelta = 0;
}

//----------------------------------------------------------------------------
//Output replaces old output.
//----------------------------------------------------------------------------
void MyFirstStepSequencer::processReplacing(float **inputs, float **outputs, long sampleFrames)
{
	long i;
	unsigned char j;
	VstTimeInfo	*timeInfo;
	double currentPPQ;
	double quartersPerBar;
	double remainingTime;
	long numSamplesInOneStep;
	long numSamplesInOneBar;
	long newStepOffset;
	bool resetHere = false;
	long resetAt;
	float newTempo;
	MidiMessage tempMess;

	frames = sampleFrames;
	timeInfo = getTimeInfo(kVstPpqPosValid |
						   kVstBarsValid |
						   kVstTimeSigValid |
						   kVstTempoValid);
	newTempo = static_cast<float>(timeInfo->tempo);
	if(tempo != newTempo)
	{
		tempo = newTempo;

		for(i=0;i<numSteps;++i)
			steps[i].setBpm(tempo);
	}

	if(timeInfo)
	{
		//If we're not currently playing, but we're supposed to be.
		if((!currentlyPlaying) && (timeInfo->flags & kVstTransportPlaying))
		{
			//If these flags aren't valid, we're stuffed...
			if((timeInfo->flags & kVstPpqPosValid)&&
			   (timeInfo->flags & kVstBarsValid) &&
			   (timeInfo->flags & kVstTimeSigValid))
			{
				currentlyPlaying = true;

				//Calculate barStartSamples (from Steps' offset position).
				currentPPQ = timeInfo->ppqPos - timeInfo->barStartPos;
				quartersPerBar = ((1.0/timeInfo->timeSigDenominator) * 4.0) *
								  timeInfo->timeSigNumerator;
				remainingTime = (60.0/static_cast<double>(tempo)) *
								(quartersPerBar - currentPPQ);
				barStartSamples = static_cast<long>(remainingTime *
								  samplerate);
				//barStartSamples -= blockSize; //Because this is for the Steps, which are offset to ensure we always send the correct delta frames to the host...

				//Work out which Step should be active.
				/*//********
				stepIndex = static_cast<long>(currentPPQ * 4.0);
				if(stepIndex > 15) //In case we're not in 4/4 (although we're probably screwed by this point anyway...).
					stepIndex = 0;
				//*********/
				stepIndex = getCorrectStep(currentPPQ);

				//Activate it with the correct offset.
				numSamplesInOneStep = static_cast<long>(0.25f *
									  (samplerate / (tempo/60.0f)));
				numSamplesInOneBar = static_cast<long>(4.0f *
									 (samplerate / (tempo/60.0f)));
				newStepOffset = (numSamplesInOneBar - barStartSamples);
				newStepOffset -= (stepIndex * numSamplesInOneStep);
				steps[stepIndex].activate(newStepOffset);
			}
		}
		else
		{
			//Check if the host's stopped, currentlyPlaying = false;
			if(!(timeInfo->flags & kVstTransportPlaying))
			{
				currentlyPlaying = false;
				for(i=0;i<numSteps;++i)
					steps[i].deactivate();
			}
			else //Update barStartSamples.
			{
				//If these flags aren't valid, we can't check.
				if((timeInfo->flags & kVstPpqPosValid)&&
				   (timeInfo->flags & kVstBarsValid) &&
				   (timeInfo->flags & kVstTimeSigValid))
				{
					//Calculate barStartSamples.
					currentPPQ = timeInfo->ppqPos - timeInfo->barStartPos;
					quartersPerBar = ((1.0/timeInfo->timeSigDenominator) * 4.0) *
									  timeInfo->timeSigNumerator;
					remainingTime = (60.0/static_cast<double>(tempo)) *
									(quartersPerBar - currentPPQ);
					barStartSamples = static_cast<long>(remainingTime *
									  samplerate);
					//barStartSamples -= blockSize;

					//If there should be a bar start w/in this block.
					if(barStartSamples < frames)
					{
						//Indicate that we need to reset to the 1st step,
						//resetAt samples from now.
						resetHere = true;
						resetAt = barStartSamples;
					}
				}
			}
		}
	}

	for(i=0;i<frames;++i)
	{
		if((numEvents>0))
			processMIDI(i);

		currentDelta = i;
		//barStartSamples--;

		//Make sure note values are updated if the user's changed them.
		for(j=0;j<16;++j)
			steps[j].updateNoteValue();
		//Make sure we're in bypass mode if the user's switched it.
		if(lastBypass != params[Bypass])
		{
			lastBypass = params[Bypass];
			if(lastBypass > 0.5f)
			{
				tempMess = steps[stepIndex].getMIDI(false);

				MIDIEvent[0]->midiData[0] = tempMess[0];
				MIDIEvent[0]->midiData[1] = tempMess[1];
				MIDIEvent[0]->midiData[2] = tempMess[2];
				MIDIEvent[0]->deltaFrames = currentDelta + blockSize;
				sendVstEventsToHost(events);
			}
		}

		if(currentlyPlaying)
		{
			//i.e. if we've reached the end of this step...
			if(!steps[stepIndex].tick())
			{
				stepIndex++;
				if(stepIndex > (numSteps-1))
					stepIndex = 0;

				steps[stepIndex].activate();
			}

			//i.e. when the bar start occurs, we reset to the first step,
			//just in case something strange's happened.
			if(resetHere)
			{
				resetAt--;
				if(resetAt == 0)
				{
					steps[stepIndex].deactivate();

					stepIndex = 0;
					steps[stepIndex].activate();

					resetHere = false;
				}
			}
		}

		outputs[0][i] = inputs[0][i];
		outputs[1][i] = inputs[1][i];
	}
	if(numPendingEvents > 0)
	{
		for(i=0;i<numPendingEvents;++i)
			MIDIEvent[eventNumArray[i]]->deltaFrames -= frames;
	}
	currentDelta = 0;
}

//----------------------------------------------------------------------------
void MyFirstStepSequencer::stepStartEvent(long id)
{
	MidiMessage tempMess;

	tempMess = steps[id].getMIDI(true);

	MIDIEvent[0]->midiData[0] = tempMess[0];
	MIDIEvent[0]->midiData[1] = tempMess[1];
	MIDIEvent[0]->midiData[2] = tempMess[2];
	MIDIEvent[0]->deltaFrames = currentDelta/* + blockSize*/;
	if(params[Bypass] < 0.5f)
		sendVstEventsToHost(events);

	delete tempMess;
}

//----------------------------------------------------------------------------
void MyFirstStepSequencer::stepStopEvent(long id)
{
	MidiMessage tempMess;

	tempMess = steps[id].getMIDI(false);

	MIDIEvent[0]->midiData[0] = tempMess[0];
	MIDIEvent[0]->midiData[1] = tempMess[1];
	MIDIEvent[0]->midiData[2] = tempMess[2];
	MIDIEvent[0]->deltaFrames = currentDelta/* + blockSize*/;
	if(params[Bypass] < 0.5f)
		sendVstEventsToHost(events);

	delete tempMess;
}

//----------------------------------------------------------------------------
//Here I just pass all the appropriate messages onto my MIDI queue (MIDIEvent).
//----------------------------------------------------------------------------
long MyFirstStepSequencer::processEvents(VstEvents *ev)
{
	int i, j, k;
	VstMidiEvent *event;

	for(i=0;i<ev->numEvents;++i)
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
void MyFirstStepSequencer::resume()
{
    wantEvents(); // important for all plugins that receive MIDI! 

	tempo = static_cast<float>(tempoAt(0)) * 0.0001f;
	if(tempo <= 0.0f)
		tempo = 120.0f;
	samplerate = getSampleRate();
	if(samplerate <= 11025.0f)
		samplerate = 44100.0f;

	for(int i=0;i<numSteps;++i)
	{
		steps[i].setSamplerate(samplerate);
		steps[i].setBpm(tempo);
	}
}

//----------------------------------------------------------------------------
void MyFirstStepSequencer::suspend()
{
    // buffers are normally cleared/initialized here
}

//----------------------------------------------------------------------------
void MyFirstStepSequencer::setProgram(long program)
{
	int i;

    MyFirstStepSequencerProg *prog = &programs[program];

	curProgram = program;

	for(i=0;i<numParams;++i)
		setParameter(i, prog->params[i]);
}

//----------------------------------------------------------------------------
void MyFirstStepSequencer::setProgramName(char *name)
{
	strcpy(programs[curProgram].name, name);
}


//----------------------------------------------------------------------------
void MyFirstStepSequencer::getProgramName(char *name)
{
	if(!strcmp(programs[curProgram].name, "Init"))
		sprintf(name, "%s %d", programs[curProgram].name, curProgram + 1);
	else
		strcpy(name, programs[curProgram].name);
}

//----------------------------------------------------------------------------
bool MyFirstStepSequencer::getProgramNameIndexed(long category, long index, char* text)
{
    return false;
    if(index<numPrograms)
    {
      strcpy(text,programs[index].name);
      return true;
    }
}

//----------------------------------------------------------------------------
bool MyFirstStepSequencer::copyProgram(long destination)
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
void MyFirstStepSequencer::setParameter(long index, float value)
{
	int i;
	int tempint;
	bool tempb;
	float tempf;

	MyFirstStepSequencerProg *prog = &programs[curProgram];

	if(index < Step1Volume)
	{
		params[index] = prog->params[index] = getAllowedValue(value);
		tempint = static_cast<int>(round(getAllowedValue(value) * 11.0f));
		tempint += static_cast<int>(round((params[Octave] * 9.0f) * 12.0f));
		steps[index].setNoteValue(static_cast<unsigned char>(tempint));

		if(editor)
		{
			editor->postUpdate();
			((AEffGUIEditor*)editor)->setParameter(index, params[index]);
		}
		return;
	}
	else if(index < Octave)
	{
		params[index] = prog->params[index] = value;
		steps[(index-Step1Volume)].setVolume(value);
	}
	else if(index == Octave)
	{
		for(i=0;i<numSteps;++i)
		{
			tempint = steps[i].getNoteValue();
			tempint -= static_cast<int>(round((params[Octave] * 9.0f) * 12.0f));
			tempint += static_cast<int>(round((value * 9.0f) * 12.0f));
			steps[i].setNoteValue(static_cast<unsigned char>(tempint));
		}
		params[Octave] = prog->params[Octave] = value;
	}
	else if(index == Shuffle)
	{
		params[index] = prog->params[index] = value;

		if(params[EightStepsMode] < 0.5f)
		{
			if(value < 0.5f) //Straight.
			{
				for(i=0;i<16;++i)
					steps[i].setNoteLength(0.25f);
			}
			else //Standard shuffle pattern?
			{
				tempb = false;
				for(i=0;i<16;++i)
				{
					if(!tempb)
					{
						steps[i].setNoteLength(0.33f);
						tempb = true;
					}
					else
					{
						steps[i].setNoteLength(0.17f);
						tempb = false;
					}
				}
			}
		}
		else
		{
			if(params[Shuffle] < 0.5f) //Straight.
			{
				for(i=0;i<16;++i)
					steps[i].setNoteLength(0.5f);
			}
			else //Standard shuffle pattern?
			{
				tempb = false;
				for(i=0;i<16;++i)
				{
					if(!tempb)
					{
						steps[i].setNoteLength(0.66f);
						tempb = true;
					}
					else
					{
						steps[i].setNoteLength(0.34f);
						tempb = false;
					}
				}
			}
			numSteps = 8;
		}
	}
	else if(index == EightStepsMode)
	{
		params[index] = prog->params[index] = value;

		if(value < 0.5f)
		{
			if(params[Shuffle] < 0.5f) //Straight.
			{
				for(i=0;i<16;++i)
					steps[i].setNoteLength(0.25f);
			}
			else //Standard shuffle pattern?
			{
				tempb = false;
				for(i=0;i<16;++i)
				{
					if(!tempb)
					{
						steps[i].setNoteLength(0.33f);
						tempb = true;
					}
					else
					{
						steps[i].setNoteLength(0.17f);
						tempb = false;
					}
				}
			}
			numSteps = 16;
		}
		else
		{
			if(params[Shuffle] < 0.5f) //Straight.
			{
				for(i=0;i<16;++i)
					steps[i].setNoteLength(0.5f);
			}
			else //Standard shuffle pattern?
			{
				tempb = false;
				for(i=0;i<16;++i)
				{
					if(!tempb)
					{
						steps[i].setNoteLength(0.66f);
						tempb = true;
					}
					else
					{
						steps[i].setNoteLength(0.34f);
						tempb = false;
					}
				}
			}
			numSteps = 8;
		}
	}
	else if(index == Bypass)
	{
		params[index] = prog->params[index] = value;

		/*if(value > 0.5f)
		{
			tempMess = steps[stepIndex].getMIDI(false);

			MIDIEvent[0]->midiData[0] = tempMess[0];
			MIDIEvent[0]->midiData[1] = tempMess[1];
			MIDIEvent[0]->midiData[2] = tempMess[2];
			MIDIEvent[0]->deltaFrames = currentDelta + blockSize;
			sendVstEventsToHost(events);
		}*/
	}
	else if(index < Random) //Scale forcing.
	{
		params[index] = prog->params[index] = value;

		tempint = 0;
		for(i=0;i<12;++i)
		{
			if(!allowedNotes[i])
				++tempint;
		}
		//So that there's always one possible note.
		if((tempint < 11)||(value > 0.5f))
		{
			//Have to take into account the fact that the keyboard control
			//numbers it's keys: 0->6 = white keys, 7-11 = black keys.
			switch(index)
			{
				case ScaleStart:
					allowedNotes[0] = (value > 0.5f);
					break;
				case ScalePlaceHolder7:
					allowedNotes[1] = (value > 0.5f);
					break;
				case ScalePlaceHolder1:
					allowedNotes[2] = (value > 0.5f);
					break;
				case ScalePlaceHolder8:
					allowedNotes[3] = (value > 0.5f);
					break;
				case ScalePlaceHolder2:
					allowedNotes[4] = (value > 0.5f);
					break;
				case ScalePlaceHolder3:
					allowedNotes[5] = (value > 0.5f);
					break;
				case ScalePlaceHolder9:
					allowedNotes[6] = (value > 0.5f);
					break;
				case ScalePlaceHolder4:
					allowedNotes[7] = (value > 0.5f);
					break;
				case ScalePlaceHolder10:
					allowedNotes[8] = (value > 0.5f);
					break;
				case ScalePlaceHolder5:
					allowedNotes[9] = (value > 0.5f);
					break;
				case ScalePlaceHolder11:
					allowedNotes[10] = (value > 0.5f);
					break;
				case ScalePlaceHolder6:
					allowedNotes[11] = (value > 0.5f);
					break;
			}
			for(i=0;i<16;++i)
				setParameter(i, getParameter(i));
		}
	}
	else if(index == Random)
	{
#ifndef ALTERNATIVE_OPERATION
		if((params[Random] < 0.5f) && (value >= 0.5f))
		{
			for(i=0;i<16;++i)
			{
				tempf = static_cast<float>(rand())/
						static_cast<float>(RAND_MAX);
				setParameter(i, tempf);
			}
		}
#elif RANDOM_VELOCITIES
		if((params[Random] < 0.5f) && (value >= 0.5f))
		{
			for(i=0;i<16;++i)
			{
				tempf = static_cast<float>(rand())/
						static_cast<float>(RAND_MAX);
				if(tempf < 0.1f)
					tempf = 0.0f;
				setParameter((i+Step1Volume), tempf);
			}
		}
#elif RANDOM_ALL
		if((params[Random] < 0.5f) && (value >= 0.5f))
		{
			for(i=0;i<16;++i)
			{
				tempf = static_cast<float>(rand())/
						static_cast<float>(RAND_MAX);
				setParameter(i, tempf);
				tempf = static_cast<float>(rand())/
						static_cast<float>(RAND_MAX);
				if(tempf < 0.1f)
					tempf = 0.0f;
				setParameter((i+Step1Volume), tempf);
			}
		}
#endif
		params[Random] = value;
	}

	if(editor)
	{
		editor->postUpdate();
		((AEffGUIEditor*)editor)->setParameter(index, value);
	}
}

//----------------------------------------------------------------------------
float MyFirstStepSequencer::getParameter(long index)
{
	return params[index];
}

//----------------------------------------------------------------------------
void MyFirstStepSequencer::getParameterLabel(long index, char *label)
{
	strcpy(label, " ");
}

//----------------------------------------------------------------------------
//gets values for parameter display - see 'AudioEffect.hpp' for others (float2string,
//Hz2string etc.)
//----------------------------------------------------------------------------
void MyFirstStepSequencer::getParameterDisplay(long index, char *text)
{
	float2string(params[index], text);
}

//----------------------------------------------------------------------------
void MyFirstStepSequencer::getParameterName(long index, char *label)
{
	char tempch[8];

	if(index < Step1Volume)
			sprintf(tempch, "Note %d", (index+1));
	else if(index < Octave)
		sprintf(tempch, "Vol %d", (index-Step16Note));
	else if(index == Octave)
		sprintf(tempch, "Octave");
	else if(index == Shuffle)
		sprintf(tempch, "Shuffle");
	else if(index == EightStepsMode)
		//sprintf(tempch, "8 Steps ");
		sprintf(tempch, "8 steps");
	else if(index == Bypass)
		sprintf(tempch, "Bypass");
	else if(index < Random)
		sprintf(tempch, "SNote%d", (index - ScaleStart));
	else if(index == Random)
		sprintf(tempch, "Random");

	strcpy(label, tempch);
}

//----------------------------------------------------------------------------
//Tells the host what the plugin can do: 1=can do, -1=cannot do, 0=don't know.
//----------------------------------------------------------------------------
long MyFirstStepSequencer::canDo(char *text)
{
	if(!strcmp(text, "sendVstEvents")) return 1;
	if(!strcmp(text, "sendVstMidiEvent")) return 1;
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
float MyFirstStepSequencer::getVu()
{
	return 0.0f;
}

//----------------------------------------------------------------------------
bool MyFirstStepSequencer::getEffectName(char* name)
{
    strcpy(name,effectName);
    return true;
}

//----------------------------------------------------------------------------
bool MyFirstStepSequencer::getVendorString(char* text)
{
    strcpy(text, vendorName);
    return true;
}

//----------------------------------------------------------------------------
bool MyFirstStepSequencer::getProductString(char* text)
{
    strcpy(text, productName);
    return true;
}

//----------------------------------------------------------------------------
long MyFirstStepSequencer::getVendorVersion()
{
    return versionNo;
}

//----------------------------------------------------------------------------
VstPlugCategory MyFirstStepSequencer::getPlugCategory()
{
    return(kPlugCategEffect);
}

//----------------------------------------------------------------------------
bool MyFirstStepSequencer::getInputProperties(long index, VstPinProperties* properties)
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
bool MyFirstStepSequencer::getOutputProperties(long index, VstPinProperties* properties)
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
long MyFirstStepSequencer::getGetTailSize()
{
	return 1; //1=no tail, 0=don't know, everything else=tail size
}

//----------------------------------------------------------------------------
void MyFirstStepSequencer::onoff2string(float value, char *text)
{
	if(value < 0.5f)
		strcpy(text, "   Off  ");
	else
		strcpy(text, "   On   ");
}

//----------------------------------------------------------------------------
void MyFirstStepSequencer::rightClickNoteUpdate()
{
	rightClick = true;
}

//----------------------------------------------------------------------------
//Called from process/Replacing to dispatch the appropriate MIDI events for the 'pos'
//frame in the current audio buffer.
//----------------------------------------------------------------------------
void MyFirstStepSequencer::processMIDI(long pos)
{
	int data1, data2;
	int status, ch, delta;
	int note;
	int i, j;

	for(i=0;i<numPendingEvents;++i)
	{
		if(((MIDIEvent[eventNumArray[i]]->deltaFrames%frames) == pos)||
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
			for(j=(i+1);j<numPendingEvents;++j)
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
void MyFirstStepSequencer::MIDI_NoteOn(int ch, int note, int val, int delta)
{
	MIDIEvent[0]->midiData[0] = 0x90 + ch;
	MIDIEvent[0]->midiData[1] = note;
	MIDIEvent[0]->midiData[2] = val;
	MIDIEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(events);
}

//----------------------------------------------------------------------------
void MyFirstStepSequencer::MIDI_NoteOff(int ch, int note, int val, int delta)
{
	MIDIEvent[0]->midiData[0] = 0x80 + ch;
	MIDIEvent[0]->midiData[1] = note;
	MIDIEvent[0]->midiData[2] = val;
	MIDIEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(events);
}

//----------------------------------------------------------------------------
void MyFirstStepSequencer::MIDI_PolyAftertouch(int ch, int note, int val, int delta)
{
	MIDIEvent[0]->midiData[0] = 0xA0 + ch;
	MIDIEvent[0]->midiData[1] = note;
	MIDIEvent[0]->midiData[2] = val;
	MIDIEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(events);
}

//----------------------------------------------------------------------------
void MyFirstStepSequencer::MIDI_CC(int ch, int num, int val, int delta)
{
	float tempf;

	tempf = static_cast<float>(val)/127.0f;	// CC data

	MIDIEvent[0]->midiData[0] = 0xB0 + ch;
	MIDIEvent[0]->midiData[1] = num;
	MIDIEvent[0]->midiData[2] = val;
	MIDIEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(events);
}

//----------------------------------------------------------------------------
//You don't have to do this, but I like to...
//----------------------------------------------------------------------------
void MyFirstStepSequencer::MIDI_ProgramChange(int ch, int val, int delta)
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
void MyFirstStepSequencer::MIDI_ChannelAftertouch(int ch, int val, int delta)
{
	MIDIEvent[0]->midiData[0] = 0xD0 + ch;
	MIDIEvent[0]->midiData[1] = val;
	MIDIEvent[0]->midiData[2] = 0;
	MIDIEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(events);
}

//----------------------------------------------------------------------------
void MyFirstStepSequencer::MIDI_PitchBend(int ch, int x1, int x2, int delta)
{
	MIDIEvent[0]->midiData[0] = 0xE0 + ch;
	MIDIEvent[0]->midiData[1] = x1;
	MIDIEvent[0]->midiData[2] = x2;
	MIDIEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(events);
}

//----------------------------------------------------------------------------
int MyFirstStepSequencer::getCorrectStep(double currentPPQ)
{
	int retval = 15;
	int i;
	double runningCount = 0.0;
	double ppq = currentPPQ/* * 4.0*/;

	for(i=0;i<numSteps;++i)
	{
		runningCount += static_cast<double>(steps[i].getNoteLength());
		if(ppq < runningCount)
		{
			retval = (i-1);
			break;
		}
	}

	if(retval < 0)
		retval = 0;

	//Uh oh...  If we get here, it's probably an error?
	return retval;
}

//----------------------------------------------------------------------------
void MyFirstStepSequencer::setupPresets()
{
	int i, j;

	for(j=15;j>3;--j)
	{
		setProgram(j);
		for(i=ScaleStart; i<Random;++i)
			setParameter(i, 1.0f);
		for(i=0;i<Step1Volume;++i)
			setParameter(i, 0.0f);
		for(i=Step1Volume;i<Octave;++i)
			setParameter(i, 1.0f);
		setParameter(Octave, (4.0f/9.0f));
		setParameter(Shuffle, 0.0f);
		setParameter(EightStepsMode, 0.0f);
		setParameter(Bypass, 0.0f);
		setParameter(Random, 0.0f);
		setProgramName("(empty)");
	}

	setProgram(3);
	for(i=ScaleStart; i<Random;++i)
		setParameter(i, 1.0f);
	for(i=0;i<Step1Volume;++i)
		setParameter(i, 0.0f);
	for(i=Step1Volume;i<Octave;++i)
		setParameter(i, 1.0f);
	setParameter(Octave, (4.0f/9.0f));
	setParameter(Shuffle, 1.0f);
	setParameter(EightStepsMode, 1.0f);
	setParameter(Bypass, 0.0f);
	setParameter(Random, 0.0f);
	setProgramName("8 step shuffle");

	setProgram(2);
	for(i=ScaleStart; i<Random;++i)
		setParameter(i, 1.0f);
	for(i=0;i<Step1Volume;++i)
		setParameter(i, 0.0f);
	for(i=Step1Volume;i<Octave;++i)
		setParameter(i, 1.0f);
	setParameter(Octave, (4.0f/9.0f));
	setParameter(Shuffle, 0.0f);
	setParameter(EightStepsMode, 1.0f);
	setParameter(Bypass, 0.0f);
	setParameter(Random, 0.0f);
	setProgramName("Default 8 steps");

	setProgram(1);
	for(i=ScaleStart; i<Random;++i)
		setParameter(i, 1.0f);
	for(i=0;i<Step1Volume;++i)
		setParameter(i, 0.0f);
	for(i=Step1Volume;i<Octave;++i)
		setParameter(i, 1.0f);
	setParameter(Octave, (4.0f/9.0f));
	setParameter(Shuffle, 1.0f);
	setParameter(EightStepsMode, 0.0f);
	setParameter(Bypass, 0.0f);
	setParameter(Random, 0.0f);
	setProgramName("16 step shuffle");

	setProgram(0);
	for(i=ScaleStart; i<Random;++i)
		setParameter(i, 1.0f);
	for(i=0;i<Step1Volume;++i)
		setParameter(i, 0.0f);
	for(i=Step1Volume;i<Octave;++i)
		setParameter(i, 1.0f);
	setParameter(Octave, (4.0f/9.0f));
	setParameter(Shuffle, 0.0f);
	setParameter(EightStepsMode, 0.0f);
	setParameter(Bypass, 0.0f);
	setParameter(Random, 0.0f);
	setProgramName("Default 16 steps");
}

//----------------------------------------------------------------------------
float MyFirstStepSequencer::getAllowedValue(float val)
{
	float retval;
	int tempint;

	tempint = static_cast<int>(round(val * 11.0f));

	if(!rightClick)
	{
		while(!allowedNotes[tempint])
		{
			++tempint;
			if(tempint > 11)
				tempint = 0;
		}
	}
	else
	{
		while(!allowedNotes[tempint])
		{
			--tempint;
			if(tempint < 0)
				tempint = 11;
		}
		rightClick = false;
	}

	retval = static_cast<float>(tempint)/11.0f;

	return retval;
}
