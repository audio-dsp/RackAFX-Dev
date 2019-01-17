//	TempoSyncReverser.h - Declaration of the plugin.
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

#ifndef TEMPOSYNCREVERSER_H_
#define TEMPOSYNCREVERSER_H_

#ifndef __AudioEffectX__
#include "audioeffectx.h"
#endif

typedef struct
{
    float left;
    float right;
} twofloats;

//#define TWO_PI (2.0*3.1415926535897932384626433832795) //<I got this from Windows' calculator.

#define BUFFER_SIZE 88200 //i.e. can handle -4 notes at 120bpm & higher
						  //(but not any slower - don't want to make this
						  // any higher as it will already take up 176.4kB)
						  //(assuming a samplerate of 44100Hz)

enum
{
    kNumPrograms = 1,
    kNumInputs = 2,
    kNumOutputs = 2,
    kID = 'TSRv',
    kChannelID = 'TSR2',
    kVersionNo = 100,
    kIsSynth = false,
    kCanReplacing = true,
    kCanMono = true,
    kVU = false
};

enum
{
#ifdef WIN32
	kReverse,
#else
        kMacReverse,
#endif
	kNoteVal,
	kQuantise,
	kMIDINote,
	kMIDICC,

    kNumParams
};

class TempoSyncReverser;

class TempoSyncReverserProg
{
friend class TempoSyncReverser;
public:
	TempoSyncReverserProg();
	~TempoSyncReverserProg() {}
private:
	float fReverse,
		  fNoteVal,
		  fQuantise,
		  fMIDINote,
		  fMIDICC;

	char name[24];
};

class TempoSyncReverser : public AudioEffectX
{
public:
	TempoSyncReverser(audioMasterCallback audioMaster);
	~TempoSyncReverser();
	
	twofloats DoProcess(twofloats a);
	void process(float **inputs, float **outputs, long sampleFrames);
	void processReplacing(float **inputs, float **outputs, long sampleFrames);
	long processEvents(VstEvents* ev);
	void resume();
	void suspend();

	void setProgram(long program);
	void setProgramName(char *name);
	void getProgramName(char *name);
	bool getProgramNameIndexed(long category, long index, char* text);
	bool copyProgram(long destination);
	
	void setParameter(long index, float value);
	float getParameter(long index);
	void getParameterLabel(long index, char *label);
	void getParameterDisplay(long index, char *text);
	void getParameterName(long index, char *text);

	long canDo(char* text);
	float getVu();
	bool getEffectName(char* name);
	bool getVendorString(char* text);
	bool getProductString(char* text);
	long getVendorVersion();
	VstPlugCategory getPlugCategory();
	bool getInputProperties(long index, VstPinProperties* properties);
	bool getOutputProperties(long index, VstPinProperties* properties);
	long getGetTailSize();

	void onoff2string(float value, char *text);
	void noteval2string(float value, char *text);
	void note2string(float value, char *text);
  private:
	void processMIDI(long pos);
	void MIDI_NoteOn(int ch, int note, int val, int delta);
	void MIDI_NoteOff(int ch, int note, int val, int delta);
	void MIDI_PolyAftertouch(int ch, int note, int val, int delta);
	void MIDI_CC(int ch, int num, int val, int delta);
	void MIDI_ProgramChange(int ch, int val, int delta);
	void MIDI_ChannelAftertouch(int ch, int val, int delta);
	void MIDI_PitchBend(int ch, int x1, int x2, int delta);

	enum
	{
		maxNumEvents = 250	///<The maximum number of events in our MIDI queue.
	};

    TempoSyncReverserProg *programs;

	float samplerate, tempo;

	float fReverse,
		  fNoteVal,
		  fQuantise,
		  fMIDINote,
		  fMIDICC;

	float bufferL[BUFFER_SIZE];
	float bufferR[BUFFER_SIZE];
	unsigned long bufferIndex;
	unsigned long backCount;
	bool countBackNow;
	unsigned long countBackTo;

	float justFinished;

	//Quantisation Stuff
	unsigned long quantCount;
	unsigned long triggerPoint;
	bool quantiseNow;

	//MIDI Parameters
	int midiNote;
	int midiCC;

//------------------------------------------
	//MIDI stuff
	///	VstEvents pointer for outputting events to the host.
	VstEvents *tempEvents;
	///	Our MIDI event queue.
	VstMidiEvent *midiEvent[maxNumEvents];
	///	The number of events in our event queue.
	int numEvents;

	///	Sorted array holding the indices of pending events (in midiEvent), for speed.
	int eventNumArray[maxNumEvents];
	///	Number of events in eventNumArray (same as numEvents?).
	unsigned int numPendingEvents;

	///	Number of samples in the current processing block.
	unsigned int frames;
//------------------------------------------

	int checkTooSoon, checkTempo;
	bool checkBSZero;
    
    char kEffectName[24];
    char kProduct[24];
    char kVendor[24];
};

#endif
