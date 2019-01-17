//	CycleShifter.h - Declaration of the plugin.
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

#ifndef CYCLESHIFTER_H_
#define CYCLESHIFTER_H_

#ifndef __AudioEffectX__
#include "audioeffectx.h"
#endif

#define NUM_EVENTS_MAX 250

typedef struct
{
    float left;
    float right;
} twofloats;

//#define TWO_PI (2.0*3.1415926535897932384626433832795) //<I got this from Windows' calculator.

//#define BUFFER_SIZE 1764 // - equivalent to a maximum cycle of 25Hz w/a 44100Hz samplerate...
#define BUFFER_SIZE 11025 //not sure why the above value clicked for drums, but this seems to fix it...

enum
{
    kNumPrograms = 1,
    kNumInputs = 2,
    kNumOutputs = 2,
    kID = 'CSft',
    kChannelID = 'CSft',
    kVersionNo = 101,
    kIsSynth = false,
    kCanReplacing = true,
    kCanMono = true,
    kVU = false
};

enum
{
	kNewCycleVolume,
	kInputVolume,

    kNumParams
};

class CycleShifter;

class CycleShifterProg
{
friend class CycleShifter;
public:
	CycleShifterProg();
	~CycleShifterProg() {}
private:
	float fNewCycleVolume,
		  fInputVolume;

	char name[24];
};

class CycleShifter : public AudioEffectX
{
public:
	CycleShifter(audioMasterCallback audioMaster);
	~CycleShifter();
	
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

	long canDo (char* text);
	float getVu();
	bool getEffectName(char* name);
	bool getVendorString(char* text);
	bool getProductString(char* text);
	long getVendorVersion();
	VstPlugCategory getPlugCategory();
	bool getInputProperties(long index, VstPinProperties* properties);
	bool getOutputProperties(long index, VstPinProperties* properties);
	long getGetTailSize();

	bool getBarStart();
	void onoff2string(float value, char *text);

  protected:
	void processMIDI(long pos);
	void MIDI_NoteOn(int ch, int note, int val, int delta);
	void MIDI_NoteOff(int ch, int note, int val, int delta);
	void MIDI_PolyAftertouch(int ch, int note, int val, int delta);
	void MIDI_CC(int ch, int num, int val, int delta);
	void MIDI_ProgramChange(int ch, int val, int delta);
	void MIDI_ChannelAftertouch(int ch, int val, int delta);
	void MIDI_PitchBend(int ch, int x1, int x2, int delta);
 
private:
    CycleShifterProg *programs;

	float samplerate, tempo;

	float fNewCycleVolume,
		  fInputVolume;

	float CycleBuffer[BUFFER_SIZE];	//buffer to store the cycle in
	int OutIndex;					//index for playing the buffer
	int InCount;					//counts how many samples we've recorded
	bool ReadWrite;					//0=read, 1=write

	float EnvOld;					//last (input) sample - used to determine zero-crossings

//------------------------------------------
	//MIDI stuff
	VstEvents *Events;
	VstMidiEvent *MIDIEvent[NUM_EVENTS_MAX];
	int NumEvents;

	int EventNumArray[NUM_EVENTS_MAX];
	int NumPendingEvents;

	long Frames;
//------------------------------------------

	int checkTooSoon, checkTempo;
	bool checkBSZero;
    
    char kEffectName[24];
    char kProduct[24];
    char kVendor[24];
};

#endif
