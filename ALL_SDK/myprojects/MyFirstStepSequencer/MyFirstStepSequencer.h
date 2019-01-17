//	MyFirstStepSequencer.h - Declaration of the plugin.
//					   (remember: bits to alter are marked: //**)
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

#ifndef MYFIRSTSTEPSEQUENCER_H_
#define MYFIRSTSTEPSEQUENCER_H_

#include "audioeffectx.h"

#include "Step.h"

//I got this from Windows' calculator.
#define PI 3.1415926535897932384626433832795

class MyFirstStepSequencerProg;

//----------------------------------------------------------------------------
class MyFirstStepSequencer : public AudioEffectX,
							 public StepEventHandler
{
public:
	MyFirstStepSequencer(audioMasterCallback audioMaster);
	~MyFirstStepSequencer();

	void process(float **inputs, float **outputs, long sampleFrames);
	void processReplacing(float **inputs, float **outputs, long sampleFrames);

	///	Called when a Step starts.
	void stepStartEvent(long id);
	///	Called when a Step stops.
	void stepStopEvent(long id);

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

	///	For communication from the gui.
	/*!
		When scale forcing is active, the plugin class can't distinguish
		between a right-click and a left-click, so right-clicking will not
		decrement the current note.  This allows us to workaround that.
	 */
	void rightClickNoteUpdate();

	//------------------------------------------
	enum //**
	{
		numPrograms = 16,
		numInputs = 2,
		numOutputs = 2,
		ID = 'MFSS',
		channelID = 'MFSS',
		versionNo = 100,
		isSynthVar = false,
		canReplacingVar = true,
		canMonoVar = true,
		VU = false,

		maxNumEvents = 250,
	};
	enum
	{
		Step1Note = 0,
		Step2Note,
		Step3Note,
		Step4Note,
		Step5Note,
		Step6Note,
		Step7Note,
		Step8Note,
		Step9Note,
		Step10Note,
		Step11Note,
		Step12Note,
		Step13Note,
		Step14Note,
		Step15Note,
		Step16Note,
		Step1Volume,
		Step2Volume,
		Step3Volume,
		Step4Volume,
		Step5Volume,
		Step6Volume,
		Step7Volume,
		Step8Volume,
		Step9Volume,
		Step10Volume,
		Step11Volume,
		Step12Volume,
		Step13Volume,
		Step14Volume,
		Step15Volume,
		Step16Volume,
		Octave,
		Shuffle,
		EightStepsMode,
		Bypass,
		ScaleStart,
		ScalePlaceHolder1,
		ScalePlaceHolder2,
		ScalePlaceHolder3,
		ScalePlaceHolder4,
		ScalePlaceHolder5,
		ScalePlaceHolder6,
		ScalePlaceHolder7,
		ScalePlaceHolder8,
		ScalePlaceHolder9,
		ScalePlaceHolder10,
		ScalePlaceHolder11,
		Random,

		numParams
	};
  private:
	void processMIDI(long pos);
	void MIDI_NoteOn(int ch, int note, int val, int delta);
	void MIDI_NoteOff(int ch, int note, int val, int delta);
	void MIDI_PolyAftertouch(int ch, int note, int val, int delta);
	void MIDI_CC(int ch, int num, int val, int delta);
	void MIDI_ProgramChange(int ch, int val, int delta);
	void MIDI_ChannelAftertouch(int ch, int val, int delta);
	void MIDI_PitchBend(int ch, int x1, int x2, int delta);

	///	We use this to calculate which step we should be at, according to the
	///	VstTimeInfo struct we got back from the host.
#ifdef WIN32
	__forceinline int getCorrectStep(double currentPPQ);
#else
	inline int getCorrectStep(double currentPPQ);
#endif

	///	Sets up the presets.
	void setupPresets();

	///	Makes sure that the user can only set one of the allowed notes.
	/*!
		Converts val into the correct value, 0->1.
	 */
	float getAllowedValue(float val);

	//------------------------------------------
    MyFirstStepSequencerProg *programs;

	float samplerate, tempo;

	float params[numParams];

	//------------------------------------------
	//MIDI stuff
	VstEvents *events;
	VstMidiEvent *MIDIEvent[maxNumEvents];
	int numEvents;

	int eventNumArray[maxNumEvents];
	int numPendingEvents;

	long frames;

	//------------------------------------------
    char effectName[24];
    char productName[24];
    char vendorName[24];

	//------------------------------------------
	///	Our array of steps.
	Step steps[16];
	///	Which step we're currently at.
	long stepIndex;
	///	The number of steps we're currently using.
	int numSteps;
	///	We need to keep track of our current position within the audio block.
	/*!
		This is because we use it to calculate when we should send a MIDI
		message in order for it to sync up with the next block's audio.
	 */
	long currentDelta;
	///	The number of samples till the next bar start (from the Steps' offset index).
	long barStartSamples;
	///	If the host's transport is playing or not.
	bool currentlyPlaying;

	///	Used to avoid threading issues.
	float lastBypass;

	///	An array of which notes are allowed.
	bool allowedNotes[12];
	///	Used when the user right-clicks a note value button.
	bool rightClick;
};

//----------------------------------------------------------------------------
class MyFirstStepSequencerProg
{
  friend class MyFirstStepSequencer;

  public:
	MyFirstStepSequencerProg();
	~MyFirstStepSequencerProg() {};
  private:
	float params[MyFirstStepSequencer::numParams];
	char name[24];
};

#endif
