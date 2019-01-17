//	Harmonical.h - Declaration of the plugin.
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

/*	To Do List
	----------
	- float2int conversion.
 */

#ifndef Harmonical_H_
#define Harmonical_H_

#include "audioeffectx.h"
#include "NoteMaster.h"

//I got this from Windows' calculator.
#ifndef PI_DEFS
#define PI_DEFS
#define PI 3.1415926535897932384626433832795
#define TWOPI 6.283185307179586476925286766559
#endif

class HarmonicalEditor3D; //to pass the vertices to...

class HarmonicalProg;

//----------------------------------------------------------------------------
class Harmonical : public AudioEffectX
{
public:
	Harmonical(audioMasterCallback audioMaster);
	~Harmonical();

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

	void setEd3D(HarmonicalEditor3D *ed) {ed3d = ed;};

	//------------------------------------------
	enum
	{
		numPrograms = 16,
		numInputs = 0,
		numOutputs = 2,
		ID = 'Hm10',
		channelID = 'Hm10',
		versionNo = 100,
		isSynthVar = true,
		canReplacingVar = true,
		canMonoVar = true,
		VU = false,

		maxNumEvents = 250,
	};
	enum
	{
		M0Val = 0,
		M0MSrc,
		M0Mod,
		M2Val,
		M2MSrc,
		M2Mod,
		M4Val,
		M4MSrc,
		M4Mod,
		M6Val,
		M6MSrc,
		M6Mod,
		M1Val,
		M3Val,
		M5Val,
		M7Val,
		LFO1Wave,
		LFO1Reset,
		LFO1Rate,
		LFO2Wave,
		LFO2Reset,
		LFO2Rate,
		Env1A,
		Env1D,
		Env1S,
		Env1R,
		Env2A,
		Env2D,
		Env2S,
		Env2R,
		FiltCutoff,
		FiltRes,
		FiltType,
		FiltMSrc,
		FiltMod,
		Level,
		BackgroundColour,
#ifdef PATCH_GEN
		GeneratePatch,
#endif

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

	void setPresets();

#ifdef PATCH_GEN
	void writeParameters();
#endif

	//------------------------------------------
    HarmonicalProg *programs;

	float samplerate, tempo;

	float params[numParams];

	//------------------------------------------
	//MIDI stuff
	VstEvents *events;
	VstMidiEvent *MIDIEvent[maxNumEvents];
	int numEvents;

	///	Used to hold indices to all the pending events.
	/*!
		This is to optimise it a bit - we don't have to search the entire
		event array to find which entries hold valid events.
	 */
	int eventNumArray[maxNumEvents];
	///	How many events are pending
	int numPendingEvents;

	long frames;
	//------------------------------------------
    
    char effectName[24];
    char productName[24];
    char vendorName[24];

	//------------------------------------------
	HarmonicalEditor3D *ed3d;

	/// Array of vertices for the object (move this onto the individual voices).
	Vertex vertices[NUM_VERTICES];

	///	Handles all the voices, and allocation stuff.
	NoteMaster *noteMaster;
};

//---------------------------------------------------------------------------
class HarmonicalProg
{
  friend class Harmonical;

  public:
	HarmonicalProg();
	~HarmonicalProg() {};
  private:
	float params[Harmonical::numParams];
	char name[24];
};

#endif
