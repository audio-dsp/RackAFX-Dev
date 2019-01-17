//	VstPlugin.h - Declaration of the plugin class.
//	----------------------------------------------------------------------------
//	Copyright (c) 2006-2007 Niall Moody
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
//	----------------------------------------------------------------------------

#ifndef VSTPLUGIN_H_
#define VSTPLUGIN_H_

#include "LibraryLoader.h"
#include "ModulationManager.h"
#include "ParameterManager.h"
#include "HelperStuff.h"

#include "audioeffectx.h"
#include <string>

struct PluginProgram;
class EffectBase;
class ModulationSource;

//------------------------------------------------------------------------------
///	VST plugin class.
class VstPlugin : public AudioEffectX,
				  public ParameterCallback
{
public:
	///	Constructor.
	/*!
		\param audioMaster This is a callback function used by AudioEffect to
		communicate with the host.  Subclasses of AudioEffect/AudioEffectX
		should not ever need to make use of it directly.
	 */
	VstPlugin(audioMasterCallback audioMaster);
	///	Destructor.
	~VstPlugin();

	///	Processes a block of audio, accumulating.
	/*!
		\param inputs Pointer to an array of an array of audio samples
		containing  the audio input to the plugin (well, it won't necessarily
		be an actual, contiguous array, but it's simpler to view it that way).
		\param outputs Pointer to an array of an array of audio samples which
		will contain the audio output of the plugin (again, won't necessarily
		be a contiguous array...).
		\param sampleFrames The number of samples to process for this block.
	 */
	void process(float **inputs, float **outputs, VstInt32 sampleFrames);
	///	Processes a block of audio, replacing.
	/*!
		\sa process()
	 */
	void processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames);
	///	Called by the host to inform the plugin of certain events.
	/*!
		\param events Pointer to the events.

		The main events will be MIDI messages.
	 */
	VstInt32 processEvents(VstEvents* events);
	///	Called when audio processing begins.
	/*!
		Use this to set up buffers etc.
	 */
	void resume();
	///	Called when audio processing stops.
	void suspend();
	///	Called when the block size changes.
	void setBlockSize(VstInt32 blockSize);

	///	So we get informed when the routing parameters change.
	void parameterChanged(VstInt32 index, float val);
	///	Returns the current value of the indexed parameter.
	float getValue(VstInt32 index);
	///	The textual display of the indexed parameter's current value.
	std::string getTextValue(VstInt32 index);

	///	Called to set the plugin's current program.
	/*!
		\param program Index of the program to make current.
	 */
	void setProgram(VstInt32 program);
	///	Called to set the name of the current program.
	void setProgramName(char *name);
	///	Returns the name of the current program.
	void getProgramName(char *name);
	///	Returns the name of the current program.
	/*!
		\param category If your programs are arranged into categories?
		\param index Index of the program whose name the host wants to query.
		\param text String to put the return name into.

		\return True if successful, false otherwise?

		The aim of this method is to allow hosts to get the names for each
		program in a plugin, without having to call setProgram() first.
	 */
	bool getProgramNameIndexed(VstInt32 category, VstInt32 index, char* text);
	///	Copies the current program to the destination program.
	bool copyProgram(VstInt32 destination);

	///	Called to set the plugin's parameters.
	void setParameter(VstInt32 index, float value);
	///	Returns the value of the indexed parameter.
	float getParameter(VstInt32 index);
	///	Returns the label (units) of the indexed parameter.
	void getParameterLabel(VstInt32 index, char *label);
	///	Returns the value of the indexed parameter as text.
	void getParameterDisplay(VstInt32 index, char *text);
	///	Returns the name of the indexed parameter.
	void getParameterName(VstInt32 index, char *text);

	///	Called by the host to determine the plugin's capabilities.
	/*!
		\return 1=can do, -1=cannot do, 0=don't know.
	 */
	VstInt32 canDo(char* text);
	///	Returns the value of the plugin's VU meter?
	float getVu();
	///	Returns the name of the plugin.
	bool getEffectName(char* name);
	///	Returns the name of the plugin author.
	bool getVendorString(char* text);
	///	Returns the name of the plugin.
	bool getProductString(char* text);
	///	Returns the plugin's version.
	VstInt32 getVendorVersion();
	///	Returns the plugin's category.
	VstPlugCategory getPlugCategory();
	///	Returns certain information about the plugin's inputs.
	/*!
		This is used to assign names to the plugin's inputs.
	 */
	bool getInputProperties(VstInt32 index, VstPinProperties* properties);
	///	Returns certain information about the plugin's outputs.
	/*!
		\sa getInputProperties()
	 */
	bool getOutputProperties(VstInt32 index, VstPinProperties* properties);
	///	Stupid name method...
	VstInt32 getGetTailSize();

	///	Returns a pointer to the plugin's ModulationManager.
	strictinline ModulationManager *getModulationManager() {return &modManager;};
	///	Returns a pointer to the plugin's ParameterManager.
	strictinline ParameterManager *getParameterManager() {return &paramManager;};
	///	Returns the LibraryLoader used to load NiallsPVOCLib.
	LibraryLoader *getLibraryLoader() {return &libLoader;};

	///	Returns the current actual position of the PhysModKnob from the indexed mod source.
	float getKnobPosition(int modSource) const;
	///	Returns whether or not we're in host sync mode.
	bool isSynced() const {return parameters[HostSync] > 0.5f;};
	///	Returns the current tempo.
	float getTempo() const {return tempo;};

	//------------------------------------------
	///	Enum enumerating the lfo parameters.
	enum
	{
		Type = 0,
		TypeKick,
		LFONumSteps,
		LFOPreset,
		LFOFreq,
		LFOSmooth,
		LFOFreqRange,
		LFOStep0,
		LFOStep31 = LFOStep0+31,
		EnvAttack,
		EnvHold,
		EnvDecay,
		EnvPoint0y,
		EnvPoint1x,
		EnvPoint1y,
		EnvPoint2x,
		EnvPoint2y,
		EnvPoint3x,
		EnvPoint3y,
		EnvPoint4x,
		EnvPoint4y,
		EnvPoint5y,
		KnobMass,
		KnobSpring,
		KnobVal,

		NumModParameters
	};

	///	Enum for enumerating the plugin's parameters.
	enum
	{
		DelayDelayLMW1,
		DelayDelayLMW2,
		DelayDelayLMW3,
		DelayDelayL,
		DelayDelayRMW1,
		DelayDelayRMW2,
		DelayDelayRMW3,
		DelayDelayR,
		DelayShortTimes,
		DelayFeedback,
		DelayMix,
		DelayLevel,

		GranDensityMW1,
		GranDensityMW2,
		GranDensityMW3,
		GranDensity,
		GranDurationMW1,
		GranDurationMW2,
		GranDurationMW3,
		GranDuration,
		GranPitchMW1,
		GranPitchMW2,
		GranPitchMW3,
		GranPitch,
		GranLevel,

		TransValMW1,
		TransValMW2,
		TransValMW3,
		TransVal,
		TransLevel,

		ExagValMW1,
		ExagValMW2,
		ExagValMW3,
		ExagVal,
		ExagLevel,

		AccumGlissMW1,
		AccumGlissMW2,
		AccumGlissMW3,
		AccumGliss,
		AccumDecayMW1,
		AccumDecayMW2,
		AccumDecayMW3,
		AccumDecay,
		AccumLevel,

		ReverseDirectionMW1,
		ReverseDirectionMW2,
		ReverseDirectionMW3,
		ReverseDirection,
		ReverseLevel,

		ModSelect,

		ModBlue,
		B1,
		B2,
		B3,
		B4,
		B5,
		B6,
		B7,
		B8,
		B9,
		B10,
		B11,
		B12,
		B13,
		B14,
		B15,
		B16,
		B17,
		B18,
		B19,
		B20,
		B21,
		B22,
		B23,
		B24,
		B25,
		B26,
		B27,
		B28,
		B29,
		B30,
		B31,
		B32,
		B33,
		B34,
		B35,
		B36,
		B37,
		B38,
		B39,
		B40,
		B41,
		B42,
		B43,
		B44,
		B45,
		B46,
		B47,
		B48,
		B49,
		B50,
		B51,
		B52,
		B53,
		B54,
		ModRed,
		R1,
		R2,
		R3,
		R4,
		R5,
		R6,
		R7,
		R8,
		R9,
		R10,
		R11,
		R12,
		R13,
		R14,
		R15,
		R16,
		R17,
		R18,
		R19,
		R20,
		R21,
		R22,
		R23,
		R24,
		R25,
		R26,
		R27,
		R28,
		R29,
		R30,
		R31,
		R32,
		R33,
		R34,
		R35,
		R36,
		R37,
		R38,
		R39,
		R40,
		R41,
		R42,
		R43,
		R44,
		R45,
		R46,
		R47,
		R48,
		R49,
		R50,
		R51,
		R52,
		R53,
		R54,
		ModGreen,
		G1,
		G2,
		G3,
		G4,
		G5,
		G6,
		G7,
		G8,
		G9,
		G10,
		G11,
		G12,
		G13,
		G14,
		G15,
		G16,
		G17,
		G18,
		G19,
		G20,
		G21,
		G22,
		G23,
		G24,
		G25,
		G26,
		G27,
		G28,
		G29,
		G30,
		G31,
		G32,
		G33,
		G34,
		G35,
		G36,
		G37,
		G38,
		G39,
		G40,
		G41,
		G42,
		G43,
		G44,
		G45,
		G46,
		G47,
		G48,
		G49,
		G50,
		G51,
		G52,
		G53,
		G54,
		/*
		For some bizarre reason, the following doesn't work for me in MSVC v6:

		ModGreen = ModRed + NumModParameters,
		ModBlue = ModGreen + NumModParameters,*/

		Route1,
		Route2,
		Route3,
		Route4,
		Route5,
		Route6,

		Mix,
		LevelMW1,
		LevelMW2,
		LevelMW3,
		Level,

		HostSync,

		DLMx,
		DLMy,
		DRMx,
		DRMy,
		GDex,
		GDey,
		GDrx,
		GDry,
		GPix,
		GPiy,
		TVax,
		TVay,
		EVax,
		EVay,
		AGlx,
		AGly,
		ADex,
		ADey,
		RDix,
		RDiy,
		OLex,
		OLey,

#ifdef PRESETSAVER
		PresetSaver,
#endif

		NumParameters
	};
  private:
	///	Called for every sample, to dispatch MIDI events appropriately.
	strictinline void processMIDI(VstInt32 pos);
	///	Called when a MIDI Note On message is received.
	strictinline void MIDI_NoteOn(int ch, int note, int val, int delta);
	///	Called when a MIDI Note Off message is received.
	strictinline void MIDI_NoteOff(int ch, int note, int val, int delta);
	///	Called when a MIDI Aftertouch message is received.
	strictinline void MIDI_PolyAftertouch(int ch, int note, int val, int delta);
	///	Called when a MIDI CC message is received.
	strictinline void MIDI_CC(int ch, int num, int val, int delta);
	///	Called when a MIDI Program Change message is received.
	strictinline void MIDI_ProgramChange(int ch, int val, int delta);
	///	Called when a MIDI Aftertouch message is received.
	strictinline void MIDI_ChannelAftertouch(int ch, int val, int delta);
	///	Called when a MIDI Pitchbend message is received.
	strictinline void MIDI_PitchBend(int ch, int x1, int x2, int delta);

	///	Helper method to set the presets.
	void setPresets();

	//-----------------------------------------
	///	Various constants.
	enum
	{
		NumPrograms = 16,	///<Number of programs this plugin has.
		VersionNumber = 100,///<The current version of the plugin.
		MaxNumEvents = 250	///<The maximum number of events in our MIDI queue.
	};

	//------------------------------------------
	///	Array of our plugin's programs.
    PluginProgram *programs;

	///	The current samplerate.
	float samplerate;

	///	Array of our plugin's parameters.
	float parameters[NumParameters];

	//------------------------------------------
	//MIDI stuff
	///	VstEvents pointer for outputting events to the host.
	VstEvents *tempEvents;
	///	Our MIDI event queue.
	VstMidiEvent *midiEvent[MaxNumEvents];
	///	The number of events in our event queue.
	int numEvents;

	///	Sorted array holding the indices of pending events (in midiEvent), for speed.
	int eventNumArray[MaxNumEvents];
	///	Number of events in eventNumArray (same as numEvents?).
	int numPendingEvents;

	///	Number of samples in the current processing block.
	VstInt32 frames;

	//-----------------------------------------
	///	Name of the plugin.
	std::string effectName;
	///	Name of the plugin's author.
    std::string vendorName;

	//-----------------------------------------

	///	ModulationManager for the plugin.
	ModulationManager modManager;
	///	ParameterManager for the plugin.
	ParameterManager paramManager;
	///	Indices to the routing sliders.
	int routing[6];
	///	Indices to the mod source type parameters.
	int modSourceType[3];

	///	Our ModulationSources.
	ModulationSource *modSources[3];

	///	Pointers to all the effects.
	EffectBase *effects[6];
	///	Array containing the order in which the effects should be processed.
	/*!
		i.e. effectsOrder[0] will contain the index (in effects) of the first
		effect to be processed, and so on...
	 */
	int effectsOrder[6];
	///	Array containing the routing variable for each effect.
	/*!
		This is a value 0->6, with 0 meaning the effect is bypassed.
	 */
	int effectsRouting[6];
	///	Array containing all the inputs for each effect.
	/*!
		i.e. effectsInputs[0] contains all the inputs for effects[0].  -1
		means no input, 6 means the plugin's input.

		The sixth entry (effectsInputs[6]) is for the output of the plugin,
		and contains all the effects inputs to be output.
	 */
	int effectsInputs[7][6];

	///	Temporary buffer for summing the outputs of the various effects.
	float *tempBuffer[2];
	///	Temporary buffer for storing the input to the various effects.
	float *tempInput[2];

	///	Index for the mix parameter,
	int mixIndex;
	///	Index for the output level parameter.
	int outputIndex;
	///	Index for the output level mod wheel 1 parameter.
	int outputMW1;
	///	Index for the output level mod wheel 2 parameter.
	int outputMW2;
	///	Index for the output level mod wheel 3 parameter.
	int outputMW3;

	///	For the dc filter on the output.
	float dcSpeedLeft;
	///	For the dc filter on the output.
	float dcSpeedRight;
	///	For the dc filter on the output.
	float dcPosLeft;
	///	For the dc filter on the output.
	float dcPosRight;

	///	Delay compensation buffers.
	/*!
		Because the PVOC effects each impose a 1024-sample delay, we need to
		call setInitialDelay() to allow the host to compensate.  The problem
		is that the PVOC effects won't necessarily be active all the time,
		so we have to compensate ourselves by introducing a delay to the
		output when they're bypassed.
	 */
	//float delayCompensation[2][3072];
	float *delayCompensation2[2];
	///	Current size of the delayCompensation buffer.
	/*!
		This will change depending on how many PVOC effects are active, and
		whether they're connected in parallel or serial, etc.

		\sa delayCompensation
	 */
	int delayCompSize;
	///	Current delayCompensation write pointer.
	/*!
		The current read pointer is calculated by the following:
		\code
		readPointer = (delayCompWrite-delayCompSize);
		if(readPointer < 0)
			readPointer += 3072;
		\endcode
	 */
	int delayCompWrite;
	///	Buffers used to delay the input for the Mix control, to match the o/p.
	//float inputCompensation[2][3072];
	///	Current write position for inputCompensation.
	//int inputCompWrite;

	///	The current maximum block size.
	int currentBlockSize;

	///	The current tempo.
	float tempo;
	///	The last ppq value.
	float lastPpq;
	///	Whether we're currently playing or not.
	bool currentlyPlaying;

	///	LibraryLoader used to load NiallsPVOCLib.
	LibraryLoader libLoader;
};

//------------------------------------------------------------------------------
///	Simple struct representing a program of the plugin.
struct PluginProgram
{
  public:
	///	Constructor.
	PluginProgram():
	name("empty")
	{
		for(VstInt32 i=0;i<VstPlugin::NumParameters;++i)
			parameters[i] = 0.0f;
	};

	///	The parameters of the plugin.
	float parameters[VstPlugin::NumParameters];
	///	The name of this program.
	std::string name;
};

#endif
