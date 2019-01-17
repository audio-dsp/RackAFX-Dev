//	TempoSyncReverser.cpp - Definition of the plugin.
//	--------------------------------------------------------------------------
//	Copyright (c) 2004-2006 Niall Moody
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

#include "TempoSyncReverser.h"
#include "TempoSyncReverserEditor.h"
#include "NoteNames.h"

extern bool oome;

//----------------------------------------------------------------------------
//Program Constructor - set default values of the program
//----------------------------------------------------------------------------
TempoSyncReverserProg::TempoSyncReverserProg()
{
	fReverse = 0.0f;
	fNoteVal = (8.0f/12.0f);
	fQuantise = 0.0f;
	fMIDINote = 0.0f;
	fMIDICC = 0.0f;

	strcpy(name, "Init");
}

//----------------------------------------------------------------------------
//Plugin Constructor - create programs, set certain variables
//----------------------------------------------------------------------------
TempoSyncReverser::TempoSyncReverser(audioMasterCallback audioMaster)
	: AudioEffectX(audioMaster, kNumPrograms, kNumParams)
{
	int i;

	programs = 0;

	fReverse = 0.0f;
	fNoteVal = (8.0f/12.0f);
	fQuantise = 0.0f;
	fMIDINote = 0.0f;
	fMIDICC = 0.0f;

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

    programs = new TempoSyncReverserProg[numPrograms];
	
	if(programs)
	{
		setProgram(0);
#ifdef WIN32
		setParameter(kReverse, 0.0f);
#else
                setParameter(kMacReverse, 0.0f);
#endif
		setParameter(kNoteVal, (8.0f/12.0f));
		setParameter(kQuantise, 0.0f);
		setParameter(kMIDINote, 0.0f);
		setParameter(kMIDICC, 0.0f);
	    setProgramName("Tempo Sync Reverser");
	}
		
	strcpy(kEffectName, "Tempo Sync Reverser");
	strcpy(kProduct, "ndc Plugs TS Reverser");
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

	//----------------
	for(i=0;i<BUFFER_SIZE;i++)
	{
		bufferL[0] = 0.0f;
		bufferR[0] = 0.0f;
	}
	bufferIndex = 0;
	backCount = 0;
	countBackNow = false;
	countBackTo = 0;

	justFinished = 0.0f;

	quantCount = 0;
	triggerPoint = 0;
	quantiseNow = false;

	midiNote = -1;
	midiCC = -1;

	editor = new TempoSyncReverserEditor(this);
	if(!editor)
		oome = true;
 }

//----------------------------------------------------------------------------
//Plugin Destructor - destroy programs, destroy buffers if you've got them
//----------------------------------------------------------------------------
TempoSyncReverser::~TempoSyncReverser()
{
	int i;

	if(programs)
		delete [] programs;

	//Delete event queue.
	for(i=0;i<maxNumEvents;++i)
	{
		if(midiEvent[i])
			delete midiEvent[i];
	}
	if(tempEvents)
		delete tempEvents;
}

//----------------------------------------------------------------------------
//Borrowed this from Toby Bear's Delphi template - it maybe adds a bit to cpu
//usage, but it makes things a bit simpler...
//----------------------------------------------------------------------------
twofloats TempoSyncReverser::DoProcess(twofloats a)
{
	twofloats tempval;
	long realIndex;

	if((fQuantise > 0.5f)&&(quantiseNow))
	{
		if(quantCount <= 0)
		{
			quantiseNow = false;
			countBackNow = true;
		}
		quantCount--;
	}

	if(!countBackNow)
	{
		if(justFinished > 0.0f)
		{
			realIndex = static_cast<long>(bufferIndex) - 1 - static_cast<long>(backCount);
			if(realIndex < 0)
				realIndex += BUFFER_SIZE;

			tempval.left = (bufferL[realIndex] * justFinished);
			tempval.left += (a.left * (1.0f-justFinished));
			tempval.right = (bufferR[realIndex] * justFinished);
			tempval.right += (a.right * (1.0f-justFinished));
			backCount++;

			justFinished -= 0.001f;
			if(justFinished <= 0.0f)
			{
				justFinished = 0.0f;
				backCount = 0;
			}
			/*tempval.left = justFinished;
			tempval.right = justFinished;*/

			return tempval;
		}
		else
		{
			bufferL[bufferIndex] = a.left;
			bufferR[bufferIndex] = a.right;
			bufferIndex++;
			bufferIndex %= BUFFER_SIZE; //keeps the index within BUFFER_SIZE

			return a;
		}
	}
	else
	{
		realIndex = static_cast<long>(bufferIndex) - 1 - static_cast<long>(backCount);
		if(realIndex < 0)
			realIndex += BUFFER_SIZE;

		tempval.left = bufferL[realIndex];
		tempval.right = bufferR[realIndex];
		backCount++;
		if(backCount == countBackTo)
		{
			countBackNow = false;
			countBackTo = 0;
			justFinished = 1.0f;
		}

		return tempval;
	}
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void TempoSyncReverser::process(float **inputs, float **outputs, long sampleFrames)
{
	unsigned long i;
	/*double bs, ppq;
	long numerator, denominator;
	double currentPPQ;
	double quartersPerBar;
	double remainingTime;
	unsigned long remainingSamples = 0;
	bool barStart = false;*/
    float *in1  =  inputs[0];
    float *in2  =  inputs[1];
    float *out1 = outputs[0];
    float *out2 = outputs[1];
	twofloats a;

	frames = sampleFrames;

	/*if(fQuantise > 0.5f)
	{
		tempo = (float)((float)tempoAt(0)*0.0001f);

		VstTimeInfo	*TimeInfo = getTimeInfo(kVstPpqPosValid|kVstBarsValid|kVstTimeSigValid);
		if(TimeInfo)
		{
			bs = TimeInfo->barStartPos;
			ppq = TimeInfo->ppqPos;
			numerator = TimeInfo->timeSigNumerator;
			denominator = TimeInfo->timeSigDenominator;

			if((TimeInfo->flags&kVstPpqPosValid) != kVstPpqPosValid)
				barStart = false;
			else if((TimeInfo->flags&kVstBarsValid) != kVstBarsValid)
				barStart = false;
			else if((TimeInfo->flags&kVstTimeSigValid) != kVstTimeSigValid)
				barStart = false;
			else
			{
				currentPPQ = ppq-bs;
				if(currentPPQ > 0.0)
				{
					quartersPerBar = ((1.0/denominator)*4.0)*numerator;
					remainingTime = (60.0/static_cast<double>(tempo))*(quartersPerBar-currentPPQ);
					remainingSamples = static_cast<unsigned long>(remainingTime * samplerate);
					if(remainingSamples < static_cast<unsigned long>(sampleFrames))
						barStart = true;
				}
				else
					barStart = true;
			}
		}
	}*/

	for(i=0;i<static_cast<unsigned long>(sampleFrames);i++)
	{
		//Process MIDI events,if there are any.
		if(numEvents>0)
			processMIDI(i);

		/*if((barStart)&&(remainingSamples == i))
		{
			quantCount = 0;
			if(quantiseNow) //just in case...
			{
				countBackNow = true;
				quantiseNow = false;
			}
		}*/

		a.left = in1[i];
		a.right = in2[i];

		a = DoProcess(a);
        
		out1[i] += a.left;
		out2[i] += a.right;
	}
	//If there are events remaining in the queue, update their delta values.
	if(numPendingEvents > 0)
	{
		for(i=0;i<numPendingEvents;++i)
			midiEvent[eventNumArray[i]]->deltaFrames -= frames;
	}
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void TempoSyncReverser::processReplacing(float **inputs, float **outputs, long sampleFrames)
{
	unsigned long i;
	/*double bs, ppq;
	long numerator, denominator;
	double currentPPQ;
	double quartersPerBar;
	double remainingTime;
	unsigned long remainingSamples = 0;
	bool barStart = false;*/
    float *in1  =  inputs[0];
    float *in2  =  inputs[1];
    float *out1 = outputs[0];
    float *out2 = outputs[1];
	twofloats a;

	frames = sampleFrames;

	/*if(fQuantise > 0.5f)
	{
		tempo = (float)((float)tempoAt(0)*0.0001f);

		VstTimeInfo	*TimeInfo = getTimeInfo(kVstPpqPosValid|kVstBarsValid|kVstTimeSigValid);
		if(TimeInfo)
		{
			bs = TimeInfo->barStartPos;
			ppq = TimeInfo->ppqPos;
			numerator = TimeInfo->timeSigNumerator;
			denominator = TimeInfo->timeSigDenominator;

			if((TimeInfo->flags&kVstPpqPosValid) != kVstPpqPosValid)
				barStart = false;
			else if((TimeInfo->flags&kVstBarsValid) != kVstBarsValid)
				barStart = false;
			else if((TimeInfo->flags&kVstTimeSigValid) != kVstTimeSigValid)
				barStart = false;
			else
			{
				currentPPQ = ppq-bs;
				if(currentPPQ > 0.0)
				{
					quartersPerBar = ((1.0/denominator)*4.0)*numerator;
					remainingTime = (60.0/static_cast<double>(tempo))*(quartersPerBar-currentPPQ);
					remainingSamples = static_cast<unsigned long>(remainingTime * samplerate);
					if(remainingSamples < static_cast<unsigned long>(sampleFrames))
						barStart = true;
				}
				else
					barStart = true;
			}
		}
	}*/

	for(i=0;i<static_cast<unsigned long>(sampleFrames);i++)
	{
		//Process MIDI events,if there are any.
		if(numEvents>0)
			processMIDI(i);

		/*if((barStart)&&(remainingSamples == i))
		{
			quantCount = 0;
			if(quantiseNow) //just in case...
			{
				countBackNow = true;
				quantiseNow = false;
			}
		}*/

		a.left = in1[i];
		a.right = in2[i];

		a = DoProcess(a);
        
		out1[i] = a.left;
		out2[i] = a.right;
	}
	//If there are events remaining in the queue, update their delta values.
	if(numPendingEvents > 0)
	{
		for(i=0;i<numPendingEvents;++i)
			midiEvent[eventNumArray[i]]->deltaFrames -= frames;
	}
}

//----------------------------------------------------------------------------
//Here I just pass all the appropriate messages onto my MIDI queue (MIDIEvent).
//----------------------------------------------------------------------------
long TempoSyncReverser::processEvents(VstEvents *events)
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
void TempoSyncReverser::resume()
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
void TempoSyncReverser::suspend()
{
    // buffers are normally cleared/initialized here
}

//----------------------------------------------------------------------------
void TempoSyncReverser::setProgram(long program)
{
    TempoSyncReverserProg *ap = &programs[program];

	curProgram = program;

#ifdef WIN32
	setParameter(kReverse, ap->fReverse);
#else
        setParameter(kMacReverse, ap->fReverse);
#endif
	setParameter(kNoteVal, ap->fNoteVal);
	setParameter(kQuantise, ap->fQuantise);
	setParameter(kMIDINote, ap->fMIDINote);
	setParameter(kMIDICC, ap->fMIDICC);
}

//----------------------------------------------------------------------------
void TempoSyncReverser::setProgramName(char *name)
{
	strcpy(programs[curProgram].name, name);
}


//----------------------------------------------------------------------------
void TempoSyncReverser::getProgramName(char *name)
{
	if(!strcmp(programs[curProgram].name, "Init"))
		sprintf(name, "%s %d", programs[curProgram].name, curProgram + 1);
	else
		strcpy(name, programs[curProgram].name);
}

//----------------------------------------------------------------------------
bool TempoSyncReverser::getProgramNameIndexed(long category, long index, char* text)
{
    return false;
    if(index<kNumPrograms)
    {
      strcpy(text,programs[index].name);
      return true;
    }
}

//----------------------------------------------------------------------------
bool TempoSyncReverser::copyProgram(long destination)
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
void TempoSyncReverser::setParameter(long index, float value)
{
	float tempf, tempf2;
	int tempint;
	double bs, ppq;
	long numerator, denominator;
	double currentPPQ;
	double quartersPerBar;
	double numQuantPerBar;
	double artificialPPQPos;
	const float numValues = 12.0f;
	TempoSyncReverserProg * ap = &programs[curProgram];

	switch(index)
	{
#ifdef WIN32
		case kReverse:
#else
                case kMacReverse:
#endif
			fReverse = ap->fReverse = value;

			if(fReverse > 0.5f)
			{
				if((!countBackNow)&&(!quantiseNow))
				{
					if(fQuantise < 0.5f)
						countBackNow = true;

					backCount = 0;
					if(fNoteVal < (1.0f/numValues))
					{
						tempf = ((1.0f/32.0f)*60.0f)/tempo;
						tempf2 = (1.0f/32.0f);
					}
					else if(fNoteVal < (2.0f/numValues))
					{
						tempf = ((1.0f/16.0f)*60.0f)/tempo;
						tempf2 = (1.0f/16.0f);
					}
					else if(fNoteVal < (3.0f/numValues))
					{
						tempf = ((1.0f/12.0f)*60.0f)/tempo;
						tempf2 = (1.0f/12.0f);
					}
					else if(fNoteVal < (4.0f/numValues))
					{
						tempf = ((1.0f/8.0f)*60.0f)/tempo;
						tempf2 = (1.0f/8.0f);
					}
					else if(fNoteVal < (5.0f/numValues))
					{
						tempf = ((1.0f/6.0f)*60.0f)/tempo;
						tempf2 = (1.0f/6.0f);
					}
					else if(fNoteVal < (6.0f/numValues))
					{
						tempf = ((1.0f/4.0f)*60.0f)/tempo;
						tempf2 = (1.0f/4.0f);
					}
					else if(fNoteVal < (7.0f/numValues))
					{
						tempf = ((1.0f/3.0f)*60.0f)/tempo;
						tempf2 = (1.0f/3.0f);
					}
					else if(fNoteVal < (8.0f/numValues))
					{
						tempf = ((1.0f/2.0f)*60.0f)/tempo;
						tempf2 = (1.0f/2.0f);
					}
					else if(fNoteVal < (9.0f/numValues))
					{
						tempf = 60.0f/tempo;
						tempf2 = 1.0f;
					}
					else if(fNoteVal < (10.0f/numValues))
					{
						tempf = 120.0f/tempo;
						tempf2 = 2.0f;
					}
					else if(fNoteVal < (11.0f/numValues))
					{
						tempf = 180.0f/tempo;
						tempf2 = 3.0f;
					}
					else
					{
						tempf = 240.0f/tempo;
						tempf2 = 4.0f;
					}

					countBackTo = static_cast<unsigned long>(tempf * samplerate);

					if(fQuantise > 0.5f)
					{
						tempo = (float)((float)tempoAt(0)*0.0001f);

						VstTimeInfo	*TimeInfo = getTimeInfo(kVstPpqPosValid|kVstBarsValid|kVstTimeSigValid);
						if(TimeInfo)
						{
							bs = TimeInfo->barStartPos;
							ppq = TimeInfo->ppqPos;
							numerator = TimeInfo->timeSigNumerator;
							denominator = TimeInfo->timeSigDenominator;

							if((TimeInfo->flags&kVstPpqPosValid) != kVstPpqPosValid)
								ppq = ppq; //not the best way of doing nothing, but I'm lazy...
							else if((TimeInfo->flags&kVstBarsValid) != kVstBarsValid)
								ppq = ppq;
							else if((TimeInfo->flags&kVstTimeSigValid) != kVstTimeSigValid)
								ppq = ppq;
							else
							{
								currentPPQ = ppq-bs;
								if(currentPPQ > 0.0)
								{
									quartersPerBar = ((1.0/denominator)*4.0)*numerator;
									numQuantPerBar = (quartersPerBar/tempf2);
									artificialPPQPos = (bs+quartersPerBar);
									while(artificialPPQPos > ppq)
										artificialPPQPos -= (quartersPerBar/numQuantPerBar);
									artificialPPQPos += (quartersPerBar/numQuantPerBar);
									quantCount = static_cast<unsigned long>(((60.0f/tempo)*(artificialPPQPos-ppq))*samplerate);
									quantiseNow = true;
								}
								else
								{
									quantiseNow = false;
									countBackNow = true;
								}
							}
						}
					}
				}
			}
			break;
		case kNoteVal:
			fNoteVal = ap->fNoteVal = value;

			if(fNoteVal < (1.0f/numValues))
				tempf = ((1.0f/32.0f)*60.0f)/tempo;
			else if(fNoteVal < (2.0f/numValues))
				tempf = ((1.0f/16.0f)*60.0f)/tempo;
			else if(fNoteVal < (3.0f/numValues))
				tempf = ((1.0f/12.0f)*60.0f)/tempo;
			else if(fNoteVal < (4.0f/numValues))
				tempf = ((1.0f/8.0f)*60.0f)/tempo;
			else if(fNoteVal < (5.0f/numValues))
				tempf = ((1.0f/6.0f)*60.0f)/tempo;
			else if(fNoteVal < (6.0f/numValues))
				tempf = ((1.0f/4.0f)*60.0f)/tempo;
			else if(fNoteVal < (7.0f/numValues))
				tempf = ((1.0f/3.0f)*60.0f)/tempo;
			else if(fNoteVal < (8.0f/numValues))
				tempf = ((1.0f/2.0f)*60.0f)/tempo;
			else if(fNoteVal < (9.0f/numValues))
				tempf = 60.0f/tempo;
			else if(fNoteVal < (10.0f/numValues))
				tempf = 120.0f/tempo;
			else if(fNoteVal < (11.0f/numValues))
				tempf = 180.0f/tempo;
			else
				tempf = 240.0f/tempo;

			triggerPoint = static_cast<unsigned long>(tempf * samplerate);
			break;
		case kQuantise:
			fQuantise = ap->fQuantise = value;
			break;
		case kMIDINote:
			fMIDINote = ap->fMIDINote = value;

			if(fMIDINote < (1.0f/127.0f))
				midiNote = -1;
			else
			{
				tempint = static_cast<int>(fMIDINote * 127.0f);
				midiNote = tempint - 1;
			}
			break;
		case kMIDICC:
			fMIDICC = ap->fMIDICC = value;

			if(fMIDICC < (1.0f/128.0f))
				midiCC = -1;
			else
			{
				tempint = static_cast<int>(fMIDICC * 128.0f);
				midiCC = tempint - 1;
			}
			break;
	}

	if(editor)
	{
		editor->postUpdate();
		((AEffGUIEditor*)editor)->setParameter(index, value);
	}
}

//----------------------------------------------------------------------------
float TempoSyncReverser::getParameter(long index)
{
	float v = 0;

	switch(index)
	{
#ifdef WIN32
		case kReverse:
#else
                case kMacReverse:
#endif
			v = fReverse;
			break;
		case kNoteVal:
			v = fNoteVal;
			break;
		case kQuantise:
			v = fQuantise;
			break;
		case kMIDINote:
			v = fMIDINote;
			break;
		case kMIDICC:
			v = fMIDICC;
			break;
	}
	return v;
}

//----------------------------------------------------------------------------
void TempoSyncReverser::getParameterLabel(long index, char *label)
{
	switch(index)
	{
#ifdef WIN32
		case kReverse:
#else
                case kMacReverse:
#endif
			strcpy(label, " ");
			break;
		case kNoteVal:
			strcpy(label, " ");
			break;
		case kQuantise:
			strcpy(label, " ");
			break;
		case kMIDINote:
			strcpy(label, " ");
			break;
		case kMIDICC:
			strcpy(label, " ");
			break;
	}
}

//----------------------------------------------------------------------------
//gets values for parameter display - see 'AudioEffect.hpp' for others (float2string,
//Hz2string etc.)
//----------------------------------------------------------------------------
void TempoSyncReverser::getParameterDisplay(long index, char *text)
{
	switch(index)
	{
#ifdef WIN32
		case kReverse:
#else
                case kMacReverse:
#endif
			onoff2string(fReverse, text);
			break;
		case kNoteVal:
			noteval2string(fNoteVal, text);
			break;
		case kQuantise:
			onoff2string(fQuantise, text);
			break;
		case kMIDINote:
			note2string(fMIDINote, text);
			break;
		case kMIDICC:
			if(fMIDICC == 0.0f)
				strcpy(text, "Off ");
			else
				long2string((static_cast<long>(127.0f*fMIDICC)-1), text);
			break;
	}
}

//----------------------------------------------------------------------------
void TempoSyncReverser::getParameterName(long index, char *label)
{
	switch(index)
	{
#ifdef WIN32
		case kReverse:
#else
                case kMacReverse:
#endif
			strcpy(label, "Reverse ");
			break;
		case kNoteVal:
			strcpy(label, "Note Val");
			break;
		case kQuantise:
			strcpy(label, "Quantise");
			break;
		case kMIDINote:
			strcpy(label, "MIDINote");
			break;
		case kMIDICC:
			strcpy(label, "MIDI CC ");
			break;
	}
}

//----------------------------------------------------------------------------
//tells the host what the plugin can do: 1=can do, -1=cannot do, 0=don't know
long TempoSyncReverser::canDo(char *text)
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
float TempoSyncReverser::getVu()
{
	return 0.0f;
}

//----------------------------------------------------------------------------
bool TempoSyncReverser::getEffectName(char* name)
{
    strcpy(name,kEffectName);
    return true;
}

//----------------------------------------------------------------------------
bool TempoSyncReverser::getVendorString(char* text)
{
    strcpy(text, kVendor);
    return true;
}

//----------------------------------------------------------------------------
bool TempoSyncReverser::getProductString(char* text)
{
    strcpy(text, kProduct);
    return true;
}

//----------------------------------------------------------------------------
long TempoSyncReverser::getVendorVersion()
{
    return kVersionNo;
}

//----------------------------------------------------------------------------
VstPlugCategory TempoSyncReverser::getPlugCategory()
{
    return(kPlugCategEffect);
}

//----------------------------------------------------------------------------
bool TempoSyncReverser::getInputProperties(long index, VstPinProperties* properties)
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

//----------------------------------------------------------------------------
bool TempoSyncReverser::getOutputProperties(long index, VstPinProperties* properties)
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
long TempoSyncReverser::getGetTailSize()
{
	return 1; //1=no tail, 0=don't know, everything else=tail size
}

//----------------------------------------------------------------------------
void TempoSyncReverser::onoff2string(float value, char *text)
{
	if(value < 0.5f)
		strcpy(text, "   Off  ");
	else
		strcpy(text, "   On   ");
}

//----------------------------------------------------------------------------
void TempoSyncReverser::noteval2string(float value, char *text)
{
	const float numValues = 12.0f;

	if(value < (1.0f/numValues))
		strcpy(text, "1/32");
	else if(value < (2.0f/numValues))
		strcpy(text, "1/16");
	else if(value < (3.0f/numValues))
		strcpy(text, "1/12");
	else if(value < (4.0f/numValues))
		strcpy(text, "1/8");
	else if(value < (5.0f/numValues))
		strcpy(text, "1/6");
	else if(value < (6.0f/numValues))
		strcpy(text, "1/4");
	else if(value < (7.0f/numValues))
		strcpy(text, "1/3");
	else if(value < (8.0f/numValues))
		strcpy(text, "1/2");
	else if(value < (9.0f/numValues))
		strcpy(text, "1");
	else if(value < (10.0f/numValues))
		strcpy(text, "2");
	else if(value < (11.0f/numValues))
		strcpy(text, "3");
	else
		strcpy(text, "4");
}

//----------------------------------------------------------------------------
void TempoSyncReverser::note2string(float value, char *text)
{
	int tempint;

	tempint = static_cast<int>(value*127);
	strcpy(text, noteNames[tempint]);
}

//----------------------------------------------------------------------------
//Called from process/Replacing to dispatch the appropriate MIDI events for the 'pos'
//frame in the current audio buffer.
//----------------------------------------------------------------------------
void TempoSyncReverser::processMIDI(long pos)
{
	int data1, data2;
	int status, ch, delta;
	int note;
	unsigned int i, j;

	for(i=0;i<numPendingEvents;++i)
	{
		if((midiEvent[eventNumArray[i]]->deltaFrames%frames) == (unsigned)pos)
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
//If we were using MIDI for something, we'd add it into these methods, rather
//than just passing the data on like a MIDI through.
//----------------------------------------------------------------------------
void TempoSyncReverser::MIDI_NoteOn(int ch, int note, int val, int delta)
{
	float tempf, tempf2;
	double bs, ppq;
	long numerator, denominator;
	double currentPPQ;
	double quartersPerBar;
	double numQuantPerBar;
	double artificialPPQPos;
	const float numValues = 12.0f;

	if((midiNote != -1)&&(note == midiNote))
	{
		if((!countBackNow)&&(!quantiseNow))
		{
			if(fQuantise < 0.5f)
				countBackNow = true;

			backCount = 0;
			if(fNoteVal < (1.0f/numValues))
			{
				tempf = ((1.0f/32.0f)*60.0f)/tempo;
				tempf2 = (1.0f/32.0f);
			}
			else if(fNoteVal < (2.0f/numValues))
			{
				tempf = ((1.0f/16.0f)*60.0f)/tempo;
				tempf2 = (1.0f/16.0f);
			}
			else if(fNoteVal < (3.0f/numValues))
			{
				tempf = ((1.0f/12.0f)*60.0f)/tempo;
				tempf2 = (1.0f/12.0f);
			}
			else if(fNoteVal < (4.0f/numValues))
			{
				tempf = ((1.0f/8.0f)*60.0f)/tempo;
				tempf2 = (1.0f/8.0f);
			}
			else if(fNoteVal < (5.0f/numValues))
			{
				tempf = ((1.0f/6.0f)*60.0f)/tempo;
				tempf2 = (1.0f/6.0f);
			}
			else if(fNoteVal < (6.0f/numValues))
			{
				tempf = ((1.0f/4.0f)*60.0f)/tempo;
				tempf2 = (1.0f/4.0f);
			}
			else if(fNoteVal < (7.0f/numValues))
			{
				tempf = ((1.0f/3.0f)*60.0f)/tempo;
				tempf2 = (1.0f/3.0f);
			}
			else if(fNoteVal < (8.0f/numValues))
			{
				tempf = ((1.0f/2.0f)*60.0f)/tempo;
				tempf2 = (1.0f/2.0f);
			}
			else if(fNoteVal < (9.0f/numValues))
			{
				tempf = 60.0f/tempo;
				tempf2 = 1.0f;
			}
			else if(fNoteVal < (10.0f/numValues))
			{
				tempf = 120.0f/tempo;
				tempf2 = 2.0f;
			}
			else if(fNoteVal < (11.0f/numValues))
			{
				tempf = 180.0f/tempo;
				tempf2 = 3.0f;
			}
			else
			{
				tempf = 240.0f/tempo;
				tempf2 = 4.0f;
			}

			countBackTo = static_cast<unsigned long>(tempf * samplerate);

			if(fQuantise > 0.5f)
			{
				tempo = (float)((float)tempoAt(0)*0.0001f);

				VstTimeInfo	*TimeInfo = getTimeInfo(kVstPpqPosValid|kVstBarsValid|kVstTimeSigValid);
				if(TimeInfo)
				{
					bs = TimeInfo->barStartPos;
					ppq = TimeInfo->ppqPos;
					numerator = TimeInfo->timeSigNumerator;
					denominator = TimeInfo->timeSigDenominator;

					if((TimeInfo->flags&kVstPpqPosValid) != kVstPpqPosValid)
						ppq = ppq; //not the best way of doing nothing, but I'm lazy...
					else if((TimeInfo->flags&kVstBarsValid) != kVstBarsValid)
						ppq = ppq;
					else if((TimeInfo->flags&kVstTimeSigValid) != kVstTimeSigValid)
						ppq = ppq;
					else
					{
						currentPPQ = ppq-bs;
						if(currentPPQ > 0.0)
						{
							quartersPerBar = ((1.0/denominator)*4.0)*numerator;
							numQuantPerBar = (quartersPerBar/tempf2);
							artificialPPQPos = (bs+quartersPerBar);
							while(artificialPPQPos > ppq)
								artificialPPQPos -= (quartersPerBar/numQuantPerBar);
							artificialPPQPos += (quartersPerBar/numQuantPerBar);
							quantCount = static_cast<unsigned long>(((60.0f/tempo)*(artificialPPQPos-ppq))*samplerate);
							quantiseNow = true;
						}
						else
						{
							quantiseNow = false;
							countBackNow = true;
						}
					}
				}
			}
		}
	}

	midiEvent[0]->midiData[0] = 0x90 + ch;
	midiEvent[0]->midiData[1] = note;
	midiEvent[0]->midiData[2] = val;
	midiEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(tempEvents);
}

//----------------------------------------------------------------------------
void TempoSyncReverser::MIDI_NoteOff(int ch, int note, int val, int delta)
{
	midiEvent[0]->midiData[0] = 0x80 + ch;
	midiEvent[0]->midiData[1] = note;
	midiEvent[0]->midiData[2] = val;
	midiEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(tempEvents);
}

//----------------------------------------------------------------------------
void TempoSyncReverser::MIDI_PolyAftertouch(int ch, int note, int val, int delta)
{
	midiEvent[0]->midiData[0] = 0xA0 + ch;
	midiEvent[0]->midiData[1] = note;
	midiEvent[0]->midiData[2] = val;
	midiEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(tempEvents);
}

//----------------------------------------------------------------------------
void TempoSyncReverser::MIDI_CC(int ch, int num, int val, int delta)
{
	float temp;
	float tempf, tempf2;
	double bs, ppq;
	long numerator, denominator;
	double currentPPQ;
	double quartersPerBar;
	double numQuantPerBar;
	double artificialPPQPos;
	const float numValues = 12.0f;

	temp = (float)val/127.0f;	// CC data

	if((midiCC != -1)&&(num == midiCC)&&(val > 63))
	{
		if((!countBackNow)&&(!quantiseNow))
		{
			if(fQuantise < 0.5f)
				countBackNow = true;

			backCount = 0;
			if(fNoteVal < (1.0f/numValues))
			{
				tempf = ((1.0f/32.0f)*60.0f)/tempo;
				tempf2 = (1.0f/32.0f);
			}
			else if(fNoteVal < (2.0f/numValues))
			{
				tempf = ((1.0f/16.0f)*60.0f)/tempo;
				tempf2 = (1.0f/16.0f);
			}
			else if(fNoteVal < (3.0f/numValues))
			{
				tempf = ((1.0f/12.0f)*60.0f)/tempo;
				tempf2 = (1.0f/12.0f);
			}
			else if(fNoteVal < (4.0f/numValues))
			{
				tempf = ((1.0f/8.0f)*60.0f)/tempo;
				tempf2 = (1.0f/8.0f);
			}
			else if(fNoteVal < (5.0f/numValues))
			{
				tempf = ((1.0f/6.0f)*60.0f)/tempo;
				tempf2 = (1.0f/6.0f);
			}
			else if(fNoteVal < (6.0f/numValues))
			{
				tempf = ((1.0f/4.0f)*60.0f)/tempo;
				tempf2 = (1.0f/4.0f);
			}
			else if(fNoteVal < (7.0f/numValues))
			{
				tempf = ((1.0f/3.0f)*60.0f)/tempo;
				tempf2 = (1.0f/3.0f);
			}
			else if(fNoteVal < (8.0f/numValues))
			{
				tempf = ((1.0f/2.0f)*60.0f)/tempo;
				tempf2 = (1.0f/2.0f);
			}
			else if(fNoteVal < (9.0f/numValues))
			{
				tempf = 60.0f/tempo;
				tempf2 = 1.0f;
			}
			else if(fNoteVal < (10.0f/numValues))
			{
				tempf = 120.0f/tempo;
				tempf2 = 2.0f;
			}
			else if(fNoteVal < (11.0f/numValues))
			{
				tempf = 180.0f/tempo;
				tempf2 = 3.0f;
			}
			else
			{
				tempf = 240.0f/tempo;
				tempf2 = 4.0f;
			}

			countBackTo = static_cast<unsigned long>(tempf * samplerate);

			if(fQuantise > 0.5f)
			{
				tempo = (float)((float)tempoAt(0)*0.0001f);

				VstTimeInfo	*TimeInfo = getTimeInfo(kVstPpqPosValid|kVstBarsValid|kVstTimeSigValid);
				if(TimeInfo)
				{
					bs = TimeInfo->barStartPos;
					ppq = TimeInfo->ppqPos;
					numerator = TimeInfo->timeSigNumerator;
					denominator = TimeInfo->timeSigDenominator;

					if((TimeInfo->flags&kVstPpqPosValid) != kVstPpqPosValid)
						ppq = ppq; //not the best way of doing nothing, but I'm lazy...
					else if((TimeInfo->flags&kVstBarsValid) != kVstBarsValid)
						ppq = ppq;
					else if((TimeInfo->flags&kVstTimeSigValid) != kVstTimeSigValid)
						ppq = ppq;
					else
					{
						currentPPQ = ppq-bs;
						if(currentPPQ > 0.0)
						{
							quartersPerBar = ((1.0/denominator)*4.0)*numerator;
							numQuantPerBar = (quartersPerBar/tempf2);
							artificialPPQPos = (bs+quartersPerBar);
							while(artificialPPQPos > ppq)
								artificialPPQPos -= (quartersPerBar/numQuantPerBar);
							artificialPPQPos += (quartersPerBar/numQuantPerBar);
							quantCount = static_cast<unsigned long>(((60.0f/tempo)*(artificialPPQPos-ppq))*samplerate);
							quantiseNow = true;
						}
						else
						{
							quantiseNow = false;
							countBackNow = true;
						}
					}
				}
			}
		}
	}

	tempf = static_cast<float>(val)/127.0f;	// CC data

	midiEvent[0]->midiData[0] = 0xB0 + ch;
	midiEvent[0]->midiData[1] = num;
	midiEvent[0]->midiData[2] = val;
	midiEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(tempEvents);
}

//----------------------------------------------------------------------------
//You don't have to do this, but I like to...
//----------------------------------------------------------------------------
void TempoSyncReverser::MIDI_ProgramChange(int ch, int val, int delta)
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
void TempoSyncReverser::MIDI_ChannelAftertouch(int ch, int val, int delta)
{
	midiEvent[0]->midiData[0] = 0xD0 + ch;
	midiEvent[0]->midiData[1] = val;
	midiEvent[0]->midiData[2] = 0;
	midiEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(tempEvents);
}

//----------------------------------------------------------------------------
void TempoSyncReverser::MIDI_PitchBend(int ch, int x1, int x2, int delta)
{
	midiEvent[0]->midiData[0] = 0xE0 + ch;
	midiEvent[0]->midiData[1] = x1;
	midiEvent[0]->midiData[2] = x2;
	midiEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(tempEvents);
}
