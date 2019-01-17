//	VstPlugin.cpp - Definition of the plugin class.
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

#include "VstPlugin.h"
#include "MainEditor.h"
#include "Delay.h"
#include "Granulator.h"
#include "Exag.h"
#include "Transpose.h"
#include "Accum.h"
#include "Reverse.h"
#include "MultiModSource.h"

#include <cmath>
#include <cassert>
#include <sstream>
#include <fstream>

using namespace std;

extern void *hInstance;

//	----------------------------------------------------------------------------
AudioEffect *createEffectInstance(audioMasterCallback audioMaster)
{
	return new VstPlugin(audioMaster);
}

//	----------------------------------------------------------------------------
VstPlugin::VstPlugin(audioMasterCallback audioMaster):
AudioEffectX(audioMaster, NumPrograms, NumParameters),
programs(0),
samplerate(44100.0f),
tempEvents(0),
numEvents(0),
numPendingEvents(0),
frames(0),
effectName("Fragmental"),
vendorName("ndc Plugs"),
dcSpeedLeft(0.0f),
dcSpeedRight(0.0f),
dcPosLeft(0.0f),
dcPosRight(0.0f),
delayCompSize(3072),
delayCompWrite(0),
//inputCompWrite(0),
currentBlockSize(0),
tempo(120.0f),
lastPpq(0.0f),
currentlyPlaying(false)
{
	int i, j;

	//Load NiallsPVOCLib.
#ifdef WIN32
	stringstream tempstr;

	tempstr << LibraryLoader::getModulePath(hInstance) << "NiallsPVOCLib.dll";
	libLoader.load(tempstr.str());
#endif

	tempBuffer[0] = 0;
	tempBuffer[1] = 0;
	tempInput[0] = 0;
	tempInput[1] = 0;
	delayCompensation2[0] = 0;
	delayCompensation2[1] = 0;

	//Initialise parameters.
	for(i=0;i<NumParameters;++i)
		parameters[i] = 0.0f;

	//Intialise effects.
	effects[0] = new Delay(this);
	effects[1] = new Granulator(this);
	effects[2] = new Transpose(this);
	effects[3] = new Exag(this);
	effects[4] = new Accum(this);
	effects[5] = new Reverse(this);
	//Initialise effects processing order, routing variables.
	for(i=0;i<6;++i)
	{
		effectsOrder[i] = i;
		effectsRouting[i] = i;
	}
	//Initialise effects input lists.
	for(i=0;i<7;++i)
	{
		effectsInputs[i][0] = 6;
		for(j=1;j<6;++j)
			effectsInputs[i][j] = -1;
	}

	//Initialise delayCompensation.
	/*for(i=0;i<3072;++i)
	{
		delayCompensation[0][i] = 0.0f;
		delayCompensation[1][i] = 0.0f;
		inputCompensation[0][i] = 0.0f;
		inputCompensation[1][i] = 0.0f;
	}*/
	//setInitialDelay(3072);
	setInitialDelay(7680);

	//Register the mod source parameters etc.
	paramManager.registerParameter(this, "ModSelect", " ");
	modSourceType[0] = paramManager.registerParameter(this, "b.Type", " ");
	paramManager.registerParameter(this, "b.TypeK", " ");
	/*for(i=1;i<EnvAttack;++i)
	{
		char tempch[8];
		sprintf(tempch, "b.%d", i);
		paramManager.registerParameter(this, tempch, " ");
	}*/
	modSources[0] = new MultiModSource(this);
	modSourceType[1] = paramManager.registerParameter(this, "r.Type", " ");
	paramManager.registerParameter(this, "r.TypeK", " ");
	/*for(i=1;i<EnvAttack;++i)
	{
		char tempch[8];
		sprintf(tempch, "r.%d", i);
		paramManager.registerParameter(this, tempch, " ");
	}*/
	modSources[1] = new MultiModSource(this);
	modSourceType[2] = paramManager.registerParameter(this, "g.Type", " ");
	paramManager.registerParameter(this, "g.TypeK", " ");
	/*for(i=1;i<EnvAttack;++i)
	{
		char tempch[8];
		sprintf(tempch, "g.%d", i);
		paramManager.registerParameter(this, tempch, " ");
	}*/
	modSources[2] = new MultiModSource(this);

	//Register the routing parameters.
	routing[0] = paramManager.registerParameter(this, "Route1", " ");
	routing[1] = paramManager.registerParameter(this, "Route2", " ");
	routing[2] = paramManager.registerParameter(this, "Route3", " ");
	routing[3] = paramManager.registerParameter(this, "Route4", " ");
	routing[4] = paramManager.registerParameter(this, "Route5", " ");
	routing[5] = paramManager.registerParameter(this, "Route6", " ");
	mixIndex = paramManager.registerParameter(this, "Mix", " ");
	if(mixIndex != Mix)
		mixIndex = mixIndex;
	outputMW1 = paramManager.registerParameter(this, "LevelMW1", " ");
	outputMW2 = paramManager.registerParameter(this, "LevelMW2", " ");
	outputMW3 = paramManager.registerParameter(this, "LevelMW3", " ");
	outputIndex = paramManager.registerParameter(this, "Level", " ");
	paramManager.registerParameter(this, "Sync", " ");

	for(i=0;i<22;++i)
		paramManager.registerParameter(this, ".", " ");

#ifdef PRESETSAVER
	paramManager.registerParameter(this, "Preset", " ");
#endif

	//Setup event queue.
	for(i=0;i<MaxNumEvents;++i)
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
	programs = new PluginProgram[NumPrograms];

	setProgram(0);
	//Delay
	setParameter(DelayDelayLMW1, 0.0f);
	setParameter(DelayDelayLMW2, 0.0f);
	setParameter(DelayDelayLMW3, 0.0f);
	setParameter(DelayDelayL, 0.5f);
	setParameter(DelayDelayRMW1, 0.0f);
	setParameter(DelayDelayRMW2, 0.0f);
	setParameter(DelayDelayRMW3, 0.0f);
	setParameter(DelayDelayR, 0.5f);
	setParameter(DelayShortTimes, 0.0f);
	setParameter(DelayFeedback, 0.0f);
	setParameter(DelayMix, 0.5f);
	setParameter(DelayLevel, 0.5f);
	//Granulator
	setParameter(GranDensityMW1, 0.0f);
	setParameter(GranDensityMW2, 0.0f);
	setParameter(GranDensityMW3, 0.0f);
	setParameter(GranDensity, 0.5f);
	setParameter(GranDurationMW1, 0.0f);
	setParameter(GranDurationMW2, 0.0f);
	setParameter(GranDurationMW3, 0.0f);
	setParameter(GranDuration, 0.5f);
	setParameter(GranPitchMW1, 0.0f);
	setParameter(GranPitchMW2, 0.0f);
	setParameter(GranPitchMW3, 0.0f);
	setParameter(GranPitch, 0.5f);
	setParameter(GranLevel, 0.5f);
	//Trans
	setParameter(TransValMW1, 0.0f);
	setParameter(TransValMW2, 0.0f);
	setParameter(TransValMW3, 0.0f);
	setParameter(TransVal, 0.5f);
	setParameter(TransLevel, 0.5f);
	//Exag
	setParameter(ExagValMW1, 0.0f);
	setParameter(ExagValMW2, 0.0f);
	setParameter(ExagValMW3, 0.0f);
	setParameter(ExagVal, 0.5f);
	setParameter(ExagLevel, 0.5f);
	//Accum
	setParameter(AccumGlissMW1, 0.0f);
	setParameter(AccumGlissMW2, 0.0f);
	setParameter(AccumGlissMW3, 0.0f);
	setParameter(AccumGliss, 0.5f);
	setParameter(AccumDecayMW1, 0.0f);
	setParameter(AccumDecayMW2, 0.0f);
	setParameter(AccumDecayMW3, 0.0f);
	setParameter(AccumDecay, 0.0f);
	setParameter(AccumLevel, 0.5f);
	//Reverse
	setParameter(ReverseDirectionMW1, 0.0f);
	setParameter(ReverseDirectionMW2, 0.0f);
	setParameter(ReverseDirectionMW3, 0.0f);
	setParameter(ReverseDirection, 1.0f);
	setParameter(ReverseLevel, 0.5f);

	//LFO Stuff.
	setParameter(ModSelect, 0.0f);

	setParameter(ModRed+Type, 0.0f);
	setParameter(ModRed+LFONumSteps, 0.0f);
	setParameter(ModRed+LFOPreset, 0.0f);
	setParameter(ModRed+LFOFreq, 0.1f);
	setParameter(ModRed+LFOSmooth, 0.0f);
	setParameter(ModRed+LFOFreqRange, 0.0f);
	for(i=(ModRed+LFOStep0);i<(ModRed+EnvAttack);++i)
		setParameter(i, 0.5f);
	setParameter(ModRed+LFOStep0, 0.0f);
	setParameter(ModRed+EnvAttack, 0.0f);
	setParameter(ModRed+EnvHold, 0.0f);
	setParameter(ModRed+EnvDecay, 0.5f);
	setParameter(ModRed+EnvPoint0y, 0.0f);
	setParameter(ModRed+EnvPoint1x, (1.0f/5.0f));
	setParameter(ModRed+EnvPoint1y, (1.0f/5.0f));
	setParameter(ModRed+EnvPoint2x, (2.0f/5.0f));
	setParameter(ModRed+EnvPoint2y, (2.0f/5.0f));
	setParameter(ModRed+EnvPoint3x, (3.0f/5.0f));
	setParameter(ModRed+EnvPoint3y, (3.0f/5.0f));
	setParameter(ModRed+EnvPoint4x, (4.0f/5.0f));
	setParameter(ModRed+EnvPoint4y, (4.0f/5.0f));
	setParameter(ModRed+EnvPoint5y, 1.0f);
	setParameter(ModRed+KnobVal, 0.0f);
	setParameter(ModRed+KnobMass, 0.0f);
	setParameter(ModRed+KnobSpring, 0.1f);

	setParameter(ModGreen+Type, 0.0f);
	setParameter(ModGreen+LFONumSteps, 0.0f);
	setParameter(ModGreen+LFOPreset, 0.0f);
	setParameter(ModGreen+LFOFreq, 0.1f);
	setParameter(ModGreen+LFOSmooth, 0.0f);
	setParameter(ModGreen+LFOFreqRange, 0.0f);
	for(i=(ModGreen+LFOStep0);i<(ModGreen+EnvAttack);++i)
		setParameter(i, 0.5f);
	setParameter(ModGreen+EnvAttack, 0.0f);
	setParameter(ModGreen+EnvHold, 0.0f);
	setParameter(ModGreen+EnvDecay, 0.5f);
	setParameter(ModGreen+EnvPoint0y, 0.0f);
	setParameter(ModGreen+EnvPoint1x, (1.0f/5.0f));
	setParameter(ModGreen+EnvPoint1y, (1.0f/5.0f));
	setParameter(ModGreen+EnvPoint2x, (2.0f/5.0f));
	setParameter(ModGreen+EnvPoint2y, (2.0f/5.0f));
	setParameter(ModGreen+EnvPoint3x, (3.0f/5.0f));
	setParameter(ModGreen+EnvPoint3y, (3.0f/5.0f));
	setParameter(ModGreen+EnvPoint4x, (4.0f/5.0f));
	setParameter(ModGreen+EnvPoint4y, (4.0f/5.0f));
	setParameter(ModGreen+EnvPoint5y, 1.0f);
	setParameter(ModGreen+KnobVal, 0.0f);
	setParameter(ModGreen+KnobMass, 0.0f);
	setParameter(ModGreen+KnobSpring, 0.1f);

	setParameter(ModBlue+Type, 0.0f);
	setParameter(ModBlue+LFONumSteps, 0.0f);
	setParameter(ModBlue+LFOPreset, 0.0f);
	setParameter(ModBlue+LFOFreqRange, 0.0f);
	setParameter(ModBlue+LFOFreq, 0.5f);
	setParameter(ModBlue+LFOSmooth, 0.0f);
	for(i=(ModBlue+LFOStep0);i<(ModBlue+EnvAttack);++i)
		setParameter(i, 0.5f);
	setParameter(ModBlue+EnvAttack, 0.0f);
	setParameter(ModBlue+EnvHold, 0.0f);
	setParameter(ModBlue+EnvDecay, 0.5f);
	setParameter(ModBlue+EnvPoint0y, 0.0f);
	setParameter(ModBlue+EnvPoint1x, (1.0f/5.0f));
	setParameter(ModBlue+EnvPoint1y, (1.0f/5.0f));
	setParameter(ModBlue+EnvPoint2x, (2.0f/5.0f));
	setParameter(ModBlue+EnvPoint2y, (2.0f/5.0f));
	setParameter(ModBlue+EnvPoint3x, (3.0f/5.0f));
	setParameter(ModBlue+EnvPoint3y, (3.0f/5.0f));
	setParameter(ModBlue+EnvPoint4x, (4.0f/5.0f));
	setParameter(ModBlue+EnvPoint4y, (4.0f/5.0f));
	setParameter(ModBlue+EnvPoint5y, 1.0f);
	setParameter(ModBlue+KnobVal, 0.0f);
	setParameter(ModBlue+KnobMass, 0.0f);
	setParameter(ModBlue+KnobSpring, 0.1f);

	//Routing.
	setParameter(Route1, 0.25f);
	setParameter(Route2, 0.0f);
	setParameter(Route3, 0.0f);
	setParameter(Route4, 0.0f);
	setParameter(Route5, 0.0f);
	setParameter(Route6, 0.0f);

	//Mix, output.
	setParameter(Mix, 1.0f);
	setParameter(LevelMW1, 0.0f);
	setParameter(LevelMW2, 0.0f);
	setParameter(LevelMW3, 0.0f);
	setParameter(Level, 0.5f);

	setParameter(HostSync, 0.0f);

	setParameter(DLMx, 0.2f);
	setParameter(DLMy, 0.2f);
	setParameter(DRMx, 0.2f);
	setParameter(DRMy, 0.2f);
	setParameter(GDex, 0.2f);
	setParameter(GDey, 0.2f);
	setParameter(GDrx, 0.2f);
	setParameter(GDry, 0.2f);
	setParameter(GPix, 0.2f);
	setParameter(GPiy, 0.2f);
	setParameter(TVax, 0.2f);
	setParameter(TVay, 0.2f);
	setParameter(EVax, 0.2f);
	setParameter(EVay, 0.2f);
	setParameter(AGlx, 0.2f);
	setParameter(AGly, 0.2f);
	setParameter(ADex, 0.2f);
	setParameter(ADey, 0.2f);
	setParameter(RDix, 0.2f);
	setParameter(RDiy, 0.2f);
	setParameter(OLex, 0.2f);
	setParameter(OLey, 0.2f);

#ifdef PRESETSAVER
	setParameter(PresetSaver, 0.0f);
#endif

	for(i=1;i<NumPrograms;++i)
		copyProgram(i);
	setPresets();
	setProgram(0);

	//Set various constants.
    setNumInputs(2);
    setNumOutputs(2);
    canProcessReplacing(true);
    isSynth(false);
    setUniqueID('Frag');

	//Construct editor here.
	editor = new MainEditor(this);
}

//	----------------------------------------------------------------------------
VstPlugin::~VstPlugin()
{
	int i;

	//Delete event queue.
	for(i=0;i<MaxNumEvents;++i)
	{
		if(midiEvent[i])
			delete midiEvent[i];
	}
	if(tempEvents)
		delete tempEvents;

	//Delete programs.
	if(programs)
		delete [] programs;

	//Delete effects.
	for(i=0;i<6;++i)
		delete effects[i];

	//Delete temporary buffers.
	if(tempBuffer[0])
		delete [] tempBuffer[0];
	if(tempBuffer[1])
		delete [] tempBuffer[1];
	if(tempInput[0])
		delete [] tempInput[0];
	if(tempInput[1])
		delete [] tempInput[1];
	if(delayCompensation2[0])
		delete [] delayCompensation2[0];
	if(delayCompensation2[1])
		delete [] delayCompensation2[1];
}

//	----------------------------------------------------------------------------
void VstPlugin::process(float **inputs, float **outputs, VstInt32 sampleFrames)
{
	VstInt32 i, j, k;
	int tempValue;
	float tempf;
	
	float *modBuffers[3];
	float modAmt; //The amount by which the level parameter are modulated.
	float level; //The up-to-date level parameter value.

	//The starting point in the delayCompensation2 buffer for this block.
	int delayCompStart = 0;

	//Outputs of the 5 effects.
	float **tempBlock[6];

	//Timing information structure.
	VstTimeInfo *timeInfo;
	double currentPPQ;
	double quartersPerBar;
	double remainingTime;
	double quartersPerSecond;
	double delayTime = 7680.0/static_cast<double>(samplerate);
	VstInt32 barStartSamples = -1;

	int numEffectsSkipped = 0;

	assert(tempBuffer[0] != 0);
	assert(tempBuffer[1] != 0);
	assert(tempInput[0] != 0);
	assert(tempInput[1] != 0);

	frames = sampleFrames;

	//Get timing information.
	timeInfo = getTimeInfo(kVstPpqPosValid |
						   kVstTempoValid |
						   kVstBarsValid |
						   kVstTimeSigValid);
	if(timeInfo)
	{
		//Tempo stuff.
		tempo = timeInfo->tempo;

		for(i=0;i<6;++i)
			effects[i]->setTempo(timeInfo->tempo);
		for(i=0;i<3;++i)
			modSources[i]->setTempo(timeInfo->tempo);

		//Bar start stuff.
		if(timeInfo->flags & kVstTransportPlaying)
		{
			//If these flags aren't valid, we're stuffed...
			if((timeInfo->flags & kVstPpqPosValid)&&
			   (timeInfo->flags & kVstBarsValid) &&
			   (timeInfo->flags & kVstTimeSigValid))
			{
				quartersPerSecond = tempo/60.0;
				currentPPQ = timeInfo->ppqPos - timeInfo->barStartPos;
				quartersPerBar = ((1.0/timeInfo->timeSigDenominator) * 4.0) *
								  timeInfo->timeSigNumerator;
				remainingTime = (60.0/static_cast<double>(tempo)) *
								(quartersPerBar - currentPPQ);
				barStartSamples = static_cast<long>(remainingTime *
								  samplerate);

				//Let LFOs reset their position if we've just started playing.
				/*if((timeInfo->flags&kVstTransportChanged) &&
				   (timeInfo->flags&kVstTransportPlaying))*/
				if((!currentlyPlaying) && (currentPPQ != lastPpq))
				{
					currentlyPlaying = true;
					for(i=0;i<3;++i)
						modSources[i]->setSamplerate(samplerate);
				}
				else if(currentlyPlaying && (currentPPQ == lastPpq))
					currentlyPlaying = false;

				lastPpq = currentPPQ;
			}
		}
	}

	//Update effectsRouting, reset effectsOrder.
	for(i=0;i<6;++i)
	{
		effectsRouting[i] = static_cast<int>(parameters[routing[i]] * 6.0f);
		effectsOrder[i] = i;
	}

	//Update delayCompensation info.
	{
		int tempint1 = -1;
		int numPVOCActive = 0;

		//Calculate correct delayCompensation value.
		for(i=2;i<5;++i)
		{
			if(effectsRouting[i] > 0)
				++numPVOCActive;
		}

		switch(numPVOCActive)
		{
			case 0:
				//delayCompSize = 3072;
				delayCompSize = 7680;
				break;
			case 1:
				//delayCompSize = 2048;
				delayCompSize = 5120;
				break;
			case 2:
				for(i=2;i<5;++i)
				{
					if((effectsRouting[i] > 0) && (tempint1 == -1))
						tempint1 = effectsRouting[i];
					else if(effectsRouting[i] == tempint1)
						//delayCompSize = 2048;
						delayCompSize = 5120;
					else
						//delayCompSize = 1024;
						delayCompSize = 2560;
				}
				break;
			case 3:
				if(effectsRouting[2] == effectsRouting[3] == effectsRouting[4])
					//delayCompSize = 2048;
					delayCompSize = 5120;
				else if((effectsRouting[2] == effectsRouting[3]) ||
						(effectsRouting[2] == effectsRouting[4]) ||
						(effectsRouting[3] == effectsRouting[4]))
					//delayCompSize = 1024;
					delayCompSize = 2560;
				else
					delayCompSize = 0;
				break;
		}

		delayCompStart = delayCompWrite - delayCompSize;
		if(delayCompStart < 0)
			delayCompStart += currentBlockSize;

		for(i=0;i<sampleFrames;++i)
		{
			delayCompensation2[0][delayCompWrite] = inputs[0][i];
			delayCompensation2[1][delayCompWrite] = inputs[1][i];

			//Make sure to compensate for the delay introduced by the PVOC
			//effects (and conversely compensated by the host).
			++delayCompWrite;
			if(delayCompWrite >= currentBlockSize)
				delayCompWrite = 0;

			/*tempReadPointer = delayCompWrite - delayCompSize;
			if(tempReadPointer < 0)
				tempReadPointer += 3072;*/
		}
	}

	//Set ModulationManager's copy of the input audio.
	modManager.setInputAudio(delayCompensation2);

	//Get modBuffers.
	for(i=0;i<3;++i)
		modBuffers[i] = modManager[i];

	//Update mod sources.
	for(i=0;i<3;++i)
	{
		modSources[i]->generateBlock(sampleFrames,
									 barStartSamples,
									 (float)quartersPerBar);
	}


	//Calculate effectsOrder (insertion sort).
	//This is a bit complicated, but what it's doing is using the
	//effectsRouting *values* to sort the effectsRouting *indices* into
	//effectsOrder, so that the effects get processed in the correct order.

	//e.g. what we're interested in here is the *Sorted Routing Index* values:
	//			Unsorted				|				Sorted
	//Routing Index	|	Routing Value	|	Routing Index	| Routing Value
	//		0		|		1			|		4			|		0
	//		1		|		4			|		0			|		1
	//		2		|		4			|		3			|		2
	//		3		|		2			|		1			|		4
	//		4		|		0			|		2			|		4
	for(i=1;i<6;++i)
	{
		tempValue = effectsRouting[i];

		for(j=i-1;
			(j >= 0) && (effectsRouting[effectsOrder[j]] > tempValue);
			--j)
		{
			effectsOrder[j+1] = effectsOrder[j];
		}
		effectsOrder[j+1] = i;
	}

	//Calculate effectsInputs.
	for(i=0;i<6;++i)
	{
		//For readability...
		int currentEffect = effectsOrder[i];

		//Handle disabled effects.
		if(effectsRouting[currentEffect] == 0)
		{
			for(j=0;j<6;++j)
				effectsInputs[currentEffect][j] = -1;
		}
		//Handle Effects only connected to the plugin input.
		else if(effectsRouting[currentEffect] == 1)
		{
			effectsInputs[currentEffect][0] = 6;
			for(j=1;j<6;++j)
				effectsInputs[currentEffect][j] = -1;
		}
		//Handle all other effects.
		else
		{
			int currentInput = 0;
			tempValue = 0;

			//First set all effectsInputs to -1.
			for(j=0;j<6;++j)
				effectsInputs[currentEffect][j] = -1;

			//Go back through previous effects, look for closest prior
			//effects.
			/*for(j=(i-1);j>=0;--j)
			{
				if(effectsRouting[effectsOrder[j]] == effectsRouting[effectsOrder[i]])
					continue;
				else if(effectsRouting[effectsOrder[j]] == 0)
					continue;
				else if(tempValue == -1)
				{
					tempValue = effectsRouting[effectsOrder[j]];
					effectsInputs[currentEffect][currentInput] = effectsOrder[j];
					++currentInput;
				}
				else if(effectsOrder[j] == tempValue)
				{
					effectsInputs[currentEffect][currentInput] = effectsOrder[j];
					++currentInput;
				}
				/*if(effectsRouting[effectsOrder[j]] == effectsRouting[effectsOrder[i]])
					continue;
				else if(j == (i-1))
				{
					tempValue = effectsRouting[effectsOrder[j]];
					if(tempValue == 0)
					{
						effectsInputs[currentEffect][currentInput] = 6;
						break;
					}
					else
						effectsInputs[currentEffect][currentInput] = effectsOrder[j];
					++currentInput;
				}
				else if(effectsRouting[effectsOrder[j]] == tempValue)
				{
					effectsInputs[currentEffect][currentInput] = effectsOrder[j];
					++currentInput;
				}
				else
					break;*//*
			}
			if(tempValue == -1)
				effectsInputs[currentEffect][currentInput] = 6;*/
			for(j=0;j<6;++j)
			{
				if((effectsRouting[effectsOrder[j]] > tempValue) &&
				   (effectsRouting[effectsOrder[j]] < effectsRouting[effectsOrder[i]]))
				   tempValue = effectsRouting[effectsOrder[j]];
			}
			if(tempValue > 0)
			{
				for(j=0;j<6;++j)
				{
					if(effectsRouting[effectsOrder[j]] == tempValue)
					{
						effectsInputs[currentEffect][currentInput] = effectsOrder[j];
						++currentInput;
					}
				}
			}
			else
				effectsInputs[currentEffect][currentInput] = 6;
		}
	}

	//Calculate effectsInputs[6] (i.e. which effects output to the plugin's
	//output.
	{
		int currentInput = 0;
		tempValue = 0;

		//First set all effectsInputs to -1.
		for(j=0;j<6;++j)
			effectsInputs[6][j] = -1;

		//Go back through previous effects, look for closest prior
		//effects.
		for(j=(i-1);j>=0;--j)
		{
			if((j == (i-1)) && (effectsRouting[effectsOrder[j]] > 0))
			{
				tempValue = effectsRouting[effectsOrder[j]];
				effectsInputs[6][currentInput] = effectsOrder[j];
				++currentInput;
			}
			else if((effectsRouting[effectsOrder[j]] == tempValue) &&
					(effectsRouting[effectsOrder[j]] > 0))
			{
				effectsInputs[6][currentInput] = effectsOrder[j];
				++currentInput;
			}
			else
				break;
		}
	}

	//Calculate effects' audio.
	for(i=0;i<6;++i)
	{
		//For readability...
		int currentEffect = effectsOrder[i];

		//Skip if this effect is switched off.
		if(effectsRouting[currentEffect] == 0)
		{
			++numEffectsSkipped;
			continue;
		}

		//Clear tempInput.
		for(j=0;j<sampleFrames;++j)
		{
			tempInput[0][j] = 0.0f;
			tempInput[1][j] = 0.0f;
		}

		//Calculate input audio for this effect.
		for(j=0;j<6;++j)
		{
			//Skip if this input is switched off.
			if(effectsInputs[currentEffect][j] < 0)
				continue;

			//If it's getting its input from the plugin's input.
			if(effectsInputs[currentEffect][j] == 6)
			{
				int l = delayCompStart;
				//Add inputs to tempInput.
				for(k=0;k<sampleFrames;++k)
				{
					/*tempInput[0][k] += inputs[0][k];
					tempInput[1][k] += inputs[1][k];*/
					tempInput[0][k] += delayCompensation2[0][l];
					tempInput[1][k] += delayCompensation2[1][l];
					++l;
					if(l >= currentBlockSize)
						l = 0;
				}
			}
			//Its input is from another effect.
			else if(effectsInputs[currentEffect][j] < 6)
			{
				//For readability...
				int inputIndex = effectsInputs[currentEffect][j];

				//Assume that the input effects have had their audio
				//calculated already, add it to tempInput.
				for(k=0;k<sampleFrames;++k)
				{
					tempInput[0][k] += tempBlock[inputIndex][0][k];
					tempInput[1][k] += tempBlock[inputIndex][1][k];
				}
			}
		}

		//Calculate this effects' audio block.
		tempBlock[currentEffect] = effects[currentEffect]->getBlock(tempInput,
																	sampleFrames);
	}

	//If all effects are bypassed, write inputs to outputs.
	if(numEffectsSkipped == 6)
	{
		//int tempReadPointer;

		//delayCompSize = 3072;
		j = delayCompStart;
		for(i=0;i<sampleFrames;++i)
		{
			//Update modAmt.
			modAmt = parameters[outputMW1] +
					 parameters[outputMW2] +
					 parameters[outputMW3];

			//Update level.
			if(modAmt > 0.0f)
			{
				level = parameters[outputIndex] * (1.0f-modAmt);
				level += parameters[outputMW1] * modBuffers[0][i];
				level += parameters[outputMW2] * modBuffers[1][i];
				level += parameters[outputMW3] * modBuffers[2][i];
			}
			else
				level = parameters[outputIndex];

			//Make sure to compensate for the delay introduced by the PVOC
			//effects (and conversely compensated by the host).
			/*delayCompensation[0][delayCompWrite] = (inputs[0][i] * level);
			delayCompensation[1][delayCompWrite] = (inputs[1][i] * level);
			++delayCompWrite;
			if(delayCompWrite >= 3072)
				delayCompWrite = 0;

			tempReadPointer = delayCompWrite - delayCompSize;
			if(tempReadPointer < 0)
				tempReadPointer += 3072;
			outputs[0][i] = delayCompensation[0][tempReadPointer];
			outputs[1][i] = delayCompensation[1][tempReadPointer];*/
			outputs[0][i] += delayCompensation2[0][j] * level;
			outputs[1][i] += delayCompensation2[1][j] * level;
			++j;
			if(j >= currentBlockSize)
				j = 0;
		}
	}
	//Otherwise, write the correct effects' output.
	else
	{
		//int tempReadPointer;
		//int numPVOCActive = 0;
		int tempint1 = -1;
		int tempint2 = -1;

		//Clear tempBuffer.
		for(j=0;j<sampleFrames;++j)
		{
			tempBuffer[0][j] = 0.0f;
			tempBuffer[1][j] = 0.0f;
		}

		//Calculate input audio for the plugin output.
		for(j=0;j<6;++j)
		{
			//Skip if this input is switched off.
			if(effectsInputs[6][j] < 0)
				continue;

			//If it's getting its input from the plugin's input.
			//Should this be here?
			if(effectsInputs[6][j] == 6)
			{
				int l = delayCompStart;
				//Add inputs to tempInput.
				for(k=0;k<sampleFrames;++k)
				{
					/*tempBuffer[0][k] += inputs[0][k];
					tempBuffer[1][k] += inputs[1][k];*/
					tempBuffer[0][k] += delayCompensation2[0][l];
					tempBuffer[1][k] += delayCompensation2[1][l];

					++l;
					if(l >= currentBlockSize)
						l = 0;
				}
			}
			//Its input is from another effect.
			else if(effectsInputs[6][j] < 6)
			{
				//For readability...
				int inputIndex = effectsInputs[6][j];

				//Assume that the input effects have had their audio
				//calculated already, add it to tempInput.
				for(k=0;k<sampleFrames;++k)
				{
					tempBuffer[0][k] += tempBlock[inputIndex][0][k];
					tempBuffer[1][k] += tempBlock[inputIndex][1][k];
				}
			}
		}

		//Calculate correct delayCompensation value.
		/*for(i=2;i<5;++i)
		{
			if(effectsRouting[i] > 0)
				++numPVOCActive;
		}

		switch(numPVOCActive)
		{
			case 0:
				delayCompSize = 3072;
				break;
			case 1:
				delayCompSize = 2048;
				break;
			case 2:
				for(i=2;i<5;++i)
				{
					if((effectsRouting[i] > 0) && (tempint1 == -1))
						tempint1 = effectsRouting[i];
					else if(effectsRouting[i] == tempint1)
						delayCompSize = 2048;
					else
						delayCompSize = 1024;
				}
				break;
			case 3:
				if(effectsRouting[2] == effectsRouting[3] == effectsRouting[4])
					delayCompSize = 2048;
				else if((effectsRouting[2] == effectsRouting[3]) ||
						(effectsRouting[2] == effectsRouting[4]) ||
						(effectsRouting[3] == effectsRouting[4]))
					delayCompSize = 1024;
				else
					delayCompSize = 0;
				break;
		}*/

		//Write effects' audio to outputs (remembering to filter for dc).
		//j = delayCompStart - (3072-delayCompSize);
		j = delayCompStart - (7680-delayCompSize);
		if(j < 0)
			j += currentBlockSize;
		for(i=0;i<frames;++i)
		{
			//Update modAmt.
			modAmt = parameters[outputMW1] +
					 parameters[outputMW2] +
					 parameters[outputMW3];

			//Update level.
			if(modAmt > 0.0f)
			{
				level = parameters[outputIndex] * (1.0f-modAmt);
				level += parameters[outputMW1] * modBuffers[0][i];
				level += parameters[outputMW2] * modBuffers[1][i];
				level += parameters[outputMW3] * modBuffers[2][i];
			}
			else
				level = parameters[outputIndex];

			//Delay input for the mix parameter.
			/*inputCompensation[0][inputCompWrite] = inputs[0][i];
			inputCompensation[1][inputCompWrite] = inputs[1][i];
			++inputCompWrite;
			if(inputCompWrite >= 3072)
				inputCompWrite = 0;*/

			dcSpeedLeft += (tempBuffer[0][i] - dcPosLeft) * 0.000004567f;
			dcPosLeft += dcSpeedLeft;
			dcSpeedLeft *= 0.96f;

			//In case we get a ridiculously large input, need to reset the
			//filter or it'll break badly.
			if(fabs(dcPosLeft) > 1.0f)
				dcPosLeft = 0.0f;
			if(fabs(dcSpeedLeft) > 1.0f)
				dcSpeedLeft = 0.0f;

			dcSpeedRight += (tempBuffer[1][i] - dcPosRight) * 0.000004567f;
			dcPosRight += dcSpeedRight;
			dcSpeedRight *= 0.96f;

			//In case we get a ridiculously large input, need to reset the
			//filter or it'll break badly.
			if(fabs(dcPosRight) > 1.0f)
				dcPosRight = 0.0f;
			if(fabs(dcSpeedRight) > 1.0f)
				dcSpeedRight = 0.0f;

			tempf = tempBuffer[0][i] - dcPosLeft;
			tempf *= parameters[mixIndex];
			/*delayCompensation[0][delayCompWrite] = tempf + ((1.0f-parameters[mixIndex]) * inputCompensation[0][inputCompWrite]);
			delayCompensation[0][delayCompWrite] *= level;*/
			tempf += ((1.0f-parameters[mixIndex]) * delayCompensation2[0][j]);
			tempf *= level;
			outputs[0][i] += tempf;

			tempf = tempBuffer[1][i] - dcPosRight;
			tempf *= parameters[mixIndex];
			/*delayCompensation[1][delayCompWrite] = tempf + ((1.0f-parameters[mixIndex]) * inputCompensation[1][inputCompWrite]);
			delayCompensation[1][delayCompWrite] *= level;*/
			tempf += ((1.0f-parameters[mixIndex]) * delayCompensation2[1][j]);
			tempf *= level;
			outputs[1][i] += tempf;

			++j;
			if(j >= currentBlockSize)
				j = 0;

			//Make sure to compensate for the delay introduced by the PVOC
			//effects (and conversely compensated by the host).
			/*++delayCompWrite;
			if(delayCompWrite >= 3072)
				delayCompWrite = 0;

			tempReadPointer = delayCompWrite - delayCompSize;
			if(tempReadPointer < 0)
				tempReadPointer += 3072;

			outputs[0][i] = delayCompensation[0][tempReadPointer];
			outputs[1][i] = delayCompensation[1][tempReadPointer];*/
		}
	}

	//----Temp----
	/*for(i=0;i<sampleFrames;++i,--barStartSamples)
	{
		if(barStartSamples == 0)
			outputs[0][i] = outputs[1][i] = 1.0f;
		else
			outputs[0][i] = outputs[1][i] = -1.0f;
	}*/
	//----Temp----

	//MIDI stuff.
	for(i=0;i<frames;++i)
	{
		//Process MIDI events,if there are any.
		if(numEvents>0)
			processMIDI(i);

		/*outputs[0][i] = inputs[0][i];
		outputs[1][i] = inputs[1][i];*/
	}

	//If there are events remaining in the queue, update their delta values.
	if(numPendingEvents > 0)
	{
		for(i=0;i<numPendingEvents;++i)
			midiEvent[eventNumArray[i]]->deltaFrames -= frames;
	}
}

//	----------------------------------------------------------------------------
void VstPlugin::processReplacing(float **inputs,
								 float **outputs,
								 VstInt32 sampleFrames)
{
	VstInt32 i, j, k;
	int tempValue;
	float tempf;
	
	float *modBuffers[3];
	float modAmt; //The amount by which the level parameter are modulated.
	float level; //The up-to-date level parameter value.

	//The starting point in the delayCompensation2 buffer for this block.
	int delayCompStart = 0;

	//Outputs of the 5 effects.
	float **tempBlock[6];

	//Timing information structure.
	VstTimeInfo *timeInfo;
	double currentPPQ;
	double quartersPerBar;
	double remainingTime;
	double quartersPerSecond;
	double delayTime = 7680.0/static_cast<double>(samplerate);
	VstInt32 barStartSamples = -1;

	int numEffectsSkipped = 0;

	assert(tempBuffer[0] != 0);
	assert(tempBuffer[1] != 0);
	assert(tempInput[0] != 0);
	assert(tempInput[1] != 0);

	frames = sampleFrames;

	//Get timing information.
	timeInfo = getTimeInfo(kVstPpqPosValid |
						   kVstTempoValid |
						   kVstBarsValid |
						   kVstTimeSigValid);
	if(timeInfo)
	{
		//Tempo stuff.
		tempo = timeInfo->tempo;

		for(i=0;i<6;++i)
			effects[i]->setTempo(timeInfo->tempo);
		for(i=0;i<3;++i)
			modSources[i]->setTempo(timeInfo->tempo);

		//Bar start stuff.
		if(timeInfo->flags & kVstTransportPlaying)
		{
			//If these flags aren't valid, we're stuffed...
			if((timeInfo->flags & kVstPpqPosValid)&&
			   (timeInfo->flags & kVstBarsValid) &&
			   (timeInfo->flags & kVstTimeSigValid))
			{
				quartersPerSecond = tempo/60.0;
				currentPPQ = timeInfo->ppqPos - timeInfo->barStartPos;
				quartersPerBar = ((1.0/timeInfo->timeSigDenominator) * 4.0) *
								  timeInfo->timeSigNumerator;
				remainingTime = (60.0/static_cast<double>(tempo)) *
								(quartersPerBar - currentPPQ);
				barStartSamples = static_cast<long>(remainingTime *
								  samplerate);

				//Let LFOs reset their position if we've just started playing.
				/*if((timeInfo->flags&kVstTransportChanged) &&
				   (timeInfo->flags&kVstTransportPlaying))*/
				if((!currentlyPlaying) && (currentPPQ != lastPpq))
				{
					currentlyPlaying = true;
					for(i=0;i<3;++i)
						modSources[i]->setSamplerate(samplerate);
				}
				else if(currentlyPlaying && (currentPPQ == lastPpq))
					currentlyPlaying = false;

				lastPpq = currentPPQ;
			}
		}
	}

	//Update effectsRouting, reset effectsOrder.
	for(i=0;i<6;++i)
	{
		effectsRouting[i] = static_cast<int>(parameters[routing[i]] * 6.0f);
		effectsOrder[i] = i;
	}

	//Update delayCompensation info.
	{
		int tempint1 = -1;
		int numPVOCActive = 0;

		//Calculate correct delayCompensation value.
		for(i=2;i<5;++i)
		{
			if(effectsRouting[i] > 0)
				++numPVOCActive;
		}

		switch(numPVOCActive)
		{
			case 0:
				//delayCompSize = 3072;
				delayCompSize = 7680;
				break;
			case 1:
				//delayCompSize = 2048;
				delayCompSize = 5120;
				break;
			case 2:
				for(i=2;i<5;++i)
				{
					if((effectsRouting[i] > 0) && (tempint1 == -1))
						tempint1 = effectsRouting[i];
					else if(effectsRouting[i] == tempint1)
						//delayCompSize = 2048;
						delayCompSize = 5120;
					else
						//delayCompSize = 1024;
						delayCompSize = 2560;
				}
				break;
			case 3:
				if(effectsRouting[2] == effectsRouting[3] == effectsRouting[4])
					//delayCompSize = 2048;
					delayCompSize = 5120;
				else if((effectsRouting[2] == effectsRouting[3]) ||
						(effectsRouting[2] == effectsRouting[4]) ||
						(effectsRouting[3] == effectsRouting[4]))
					//delayCompSize = 1024;
					delayCompSize = 2560;
				else
					delayCompSize = 0;
				break;
		}

		delayCompStart = delayCompWrite - delayCompSize;
		if(delayCompStart < 0)
			delayCompStart += currentBlockSize;

		for(i=0;i<sampleFrames;++i)
		{
			delayCompensation2[0][delayCompWrite] = inputs[0][i];
			delayCompensation2[1][delayCompWrite] = inputs[1][i];

			//Make sure to compensate for the delay introduced by the PVOC
			//effects (and conversely compensated by the host).
			++delayCompWrite;
			if(delayCompWrite >= currentBlockSize)
				delayCompWrite = 0;

			/*tempReadPointer = delayCompWrite - delayCompSize;
			if(tempReadPointer < 0)
				tempReadPointer += 3072;*/
		}
	}

	//Set ModulationManager's copy of the input audio.
	modManager.setInputAudio(delayCompensation2);

	//Get modBuffers.
	for(i=0;i<3;++i)
		modBuffers[i] = modManager[i];

	//Update mod sources.
	for(i=0;i<3;++i)
	{
		modSources[i]->generateBlock(sampleFrames,
									 barStartSamples,
									 (float)quartersPerBar);
	}


	//Calculate effectsOrder (insertion sort).
	//This is a bit complicated, but what it's doing is using the
	//effectsRouting *values* to sort the effectsRouting *indices* into
	//effectsOrder, so that the effects get processed in the correct order.

	//e.g. what we're interested in here is the *Sorted Routing Index* values:
	//			Unsorted				|				Sorted
	//Routing Index	|	Routing Value	|	Routing Index	| Routing Value
	//		0		|		1			|		4			|		0
	//		1		|		4			|		0			|		1
	//		2		|		4			|		3			|		2
	//		3		|		2			|		1			|		4
	//		4		|		0			|		2			|		4
	for(i=1;i<6;++i)
	{
		tempValue = effectsRouting[i];

		for(j=i-1;
			(j >= 0) && (effectsRouting[effectsOrder[j]] > tempValue);
			--j)
		{
			effectsOrder[j+1] = effectsOrder[j];
		}
		effectsOrder[j+1] = i;
	}

	//Calculate effectsInputs.
	for(i=0;i<6;++i)
	{
		//For readability...
		int currentEffect = effectsOrder[i];

		//Handle disabled effects.
		if(effectsRouting[currentEffect] == 0)
		{
			for(j=0;j<6;++j)
				effectsInputs[currentEffect][j] = -1;
		}
		//Handle Effects only connected to the plugin input.
		else if(effectsRouting[currentEffect] == 1)
		{
			effectsInputs[currentEffect][0] = 6;
			for(j=1;j<6;++j)
				effectsInputs[currentEffect][j] = -1;
		}
		//Handle all other effects.
		else
		{
			int currentInput = 0;
			tempValue = 0;

			//First set all effectsInputs to -1.
			for(j=0;j<6;++j)
				effectsInputs[currentEffect][j] = -1;

			//Go back through previous effects, look for closest prior
			//effects.
			/*for(j=(i-1);j>=0;--j)
			{
				if(effectsRouting[effectsOrder[j]] == effectsRouting[effectsOrder[i]])
					continue;
				else if(effectsRouting[effectsOrder[j]] == 0)
					continue;
				else if(tempValue == -1)
				{
					tempValue = effectsRouting[effectsOrder[j]];
					effectsInputs[currentEffect][currentInput] = effectsOrder[j];
					++currentInput;
				}
				else if(effectsOrder[j] == tempValue)
				{
					effectsInputs[currentEffect][currentInput] = effectsOrder[j];
					++currentInput;
				}
				/*if(effectsRouting[effectsOrder[j]] == effectsRouting[effectsOrder[i]])
					continue;
				else if(j == (i-1))
				{
					tempValue = effectsRouting[effectsOrder[j]];
					if(tempValue == 0)
					{
						effectsInputs[currentEffect][currentInput] = 6;
						break;
					}
					else
						effectsInputs[currentEffect][currentInput] = effectsOrder[j];
					++currentInput;
				}
				else if(effectsRouting[effectsOrder[j]] == tempValue)
				{
					effectsInputs[currentEffect][currentInput] = effectsOrder[j];
					++currentInput;
				}
				else
					break;*//*
			}
			if(tempValue == -1)
				effectsInputs[currentEffect][currentInput] = 6;*/
			for(j=0;j<6;++j)
			{
				if((effectsRouting[effectsOrder[j]] > tempValue) &&
				   (effectsRouting[effectsOrder[j]] < effectsRouting[effectsOrder[i]]))
				   tempValue = effectsRouting[effectsOrder[j]];
			}
			if(tempValue > 0)
			{
				for(j=0;j<6;++j)
				{
					if(effectsRouting[effectsOrder[j]] == tempValue)
					{
						effectsInputs[currentEffect][currentInput] = effectsOrder[j];
						++currentInput;
					}
				}
			}
			else
				effectsInputs[currentEffect][currentInput] = 6;
		}
	}

	//Calculate effectsInputs[6] (i.e. which effects output to the plugin's
	//output.
	{
		int currentInput = 0;
		tempValue = 0;

		//First set all effectsInputs to -1.
		for(j=0;j<6;++j)
			effectsInputs[6][j] = -1;

		//Go back through previous effects, look for closest prior
		//effects.
		for(j=(i-1);j>=0;--j)
		{
			if((j == (i-1)) && (effectsRouting[effectsOrder[j]] > 0))
			{
				tempValue = effectsRouting[effectsOrder[j]];
				effectsInputs[6][currentInput] = effectsOrder[j];
				++currentInput;
			}
			else if((effectsRouting[effectsOrder[j]] == tempValue) &&
					(effectsRouting[effectsOrder[j]] > 0))
			{
				effectsInputs[6][currentInput] = effectsOrder[j];
				++currentInput;
			}
			else
				break;
		}
	}

	//Calculate effects' audio.
	for(i=0;i<6;++i)
	{
		//For readability...
		int currentEffect = effectsOrder[i];

		//Skip if this effect is switched off.
		if(effectsRouting[currentEffect] == 0)
		{
			++numEffectsSkipped;
			continue;
		}

		//Clear tempInput.
		for(j=0;j<sampleFrames;++j)
		{
			tempInput[0][j] = 0.0f;
			tempInput[1][j] = 0.0f;
		}

		//Calculate input audio for this effect.
		for(j=0;j<6;++j)
		{
			//Skip if this input is switched off.
			if(effectsInputs[currentEffect][j] < 0)
				continue;

			//If it's getting its input from the plugin's input.
			if(effectsInputs[currentEffect][j] == 6)
			{
				int l = delayCompStart;
				//Add inputs to tempInput.
				for(k=0;k<sampleFrames;++k)
				{
					/*tempInput[0][k] += inputs[0][k];
					tempInput[1][k] += inputs[1][k];*/
					tempInput[0][k] += delayCompensation2[0][l];
					tempInput[1][k] += delayCompensation2[1][l];
					++l;
					if(l >= currentBlockSize)
						l = 0;
				}
			}
			//Its input is from another effect.
			else if(effectsInputs[currentEffect][j] < 6)
			{
				//For readability...
				int inputIndex = effectsInputs[currentEffect][j];

				//Assume that the input effects have had their audio
				//calculated already, add it to tempInput.
				for(k=0;k<sampleFrames;++k)
				{
					tempInput[0][k] += tempBlock[inputIndex][0][k];
					tempInput[1][k] += tempBlock[inputIndex][1][k];
				}
			}
		}

		//Calculate this effects' audio block.
		tempBlock[currentEffect] = effects[currentEffect]->getBlock(tempInput,
																	sampleFrames);
	}

	//If all effects are bypassed, write inputs to outputs.
	if(numEffectsSkipped == 6)
	{
		//int tempReadPointer;

		//delayCompSize = 3072;
		j = delayCompStart;
		for(i=0;i<sampleFrames;++i)
		{
			//Update modAmt.
			modAmt = parameters[outputMW1] +
					 parameters[outputMW2] +
					 parameters[outputMW3];

			//Update level.
			if(modAmt > 0.0f)
			{
				level = parameters[outputIndex] * (1.0f-modAmt);
				level += parameters[outputMW1] * modBuffers[0][i];
				level += parameters[outputMW2] * modBuffers[1][i];
				level += parameters[outputMW3] * modBuffers[2][i];
			}
			else
				level = parameters[outputIndex];

			//Make sure to compensate for the delay introduced by the PVOC
			//effects (and conversely compensated by the host).
			/*delayCompensation[0][delayCompWrite] = (inputs[0][i] * level);
			delayCompensation[1][delayCompWrite] = (inputs[1][i] * level);
			++delayCompWrite;
			if(delayCompWrite >= 3072)
				delayCompWrite = 0;

			tempReadPointer = delayCompWrite - delayCompSize;
			if(tempReadPointer < 0)
				tempReadPointer += 3072;
			outputs[0][i] = delayCompensation[0][tempReadPointer];
			outputs[1][i] = delayCompensation[1][tempReadPointer];*/
			outputs[0][i] = delayCompensation2[0][j] * level;
			outputs[1][i] = delayCompensation2[1][j] * level;
			++j;
			if(j >= currentBlockSize)
				j = 0;
		}
	}
	//Otherwise, write the correct effects' output.
	else
	{
		//int tempReadPointer;
		//int numPVOCActive = 0;
		int tempint1 = -1;
		int tempint2 = -1;

		//Clear tempBuffer.
		for(j=0;j<sampleFrames;++j)
		{
			tempBuffer[0][j] = 0.0f;
			tempBuffer[1][j] = 0.0f;
		}

		//Calculate input audio for the plugin output.
		for(j=0;j<6;++j)
		{
			//Skip if this input is switched off.
			if(effectsInputs[6][j] < 0)
				continue;

			//If it's getting its input from the plugin's input.
			//Should this be here?
			if(effectsInputs[6][j] == 6)
			{
				int l = delayCompStart;
				//Add inputs to tempInput.
				for(k=0;k<sampleFrames;++k)
				{
					/*tempBuffer[0][k] += inputs[0][k];
					tempBuffer[1][k] += inputs[1][k];*/
					tempBuffer[0][k] += delayCompensation2[0][l];
					tempBuffer[1][k] += delayCompensation2[1][l];

					++l;
					if(l >= currentBlockSize)
						l = 0;
				}
			}
			//Its input is from another effect.
			else if(effectsInputs[6][j] < 6)
			{
				//For readability...
				int inputIndex = effectsInputs[6][j];

				//Assume that the input effects have had their audio
				//calculated already, add it to tempInput.
				for(k=0;k<sampleFrames;++k)
				{
					tempBuffer[0][k] += tempBlock[inputIndex][0][k];
					tempBuffer[1][k] += tempBlock[inputIndex][1][k];
				}
			}
		}

		//Calculate correct delayCompensation value.
		/*for(i=2;i<5;++i)
		{
			if(effectsRouting[i] > 0)
				++numPVOCActive;
		}

		switch(numPVOCActive)
		{
			case 0:
				delayCompSize = 3072;
				break;
			case 1:
				delayCompSize = 2048;
				break;
			case 2:
				for(i=2;i<5;++i)
				{
					if((effectsRouting[i] > 0) && (tempint1 == -1))
						tempint1 = effectsRouting[i];
					else if(effectsRouting[i] == tempint1)
						delayCompSize = 2048;
					else
						delayCompSize = 1024;
				}
				break;
			case 3:
				if(effectsRouting[2] == effectsRouting[3] == effectsRouting[4])
					delayCompSize = 2048;
				else if((effectsRouting[2] == effectsRouting[3]) ||
						(effectsRouting[2] == effectsRouting[4]) ||
						(effectsRouting[3] == effectsRouting[4]))
					delayCompSize = 1024;
				else
					delayCompSize = 0;
				break;
		}*/

		//Write effects' audio to outputs (remembering to filter for dc).
		//j = delayCompStart - (3072-delayCompSize);
		j = delayCompStart - (7680-delayCompSize);
		if(j < 0)
			j += currentBlockSize;
		for(i=0;i<frames;++i)
		{
			//Update modAmt.
			modAmt = parameters[outputMW1] +
					 parameters[outputMW2] +
					 parameters[outputMW3];

			//Update level.
			if(modAmt > 0.0f)
			{
				level = parameters[outputIndex] * (1.0f-modAmt);
				level += parameters[outputMW1] * modBuffers[0][i];
				level += parameters[outputMW2] * modBuffers[1][i];
				level += parameters[outputMW3] * modBuffers[2][i];
			}
			else
				level = parameters[outputIndex];

			//Delay input for the mix parameter.
			/*inputCompensation[0][inputCompWrite] = inputs[0][i];
			inputCompensation[1][inputCompWrite] = inputs[1][i];
			++inputCompWrite;
			if(inputCompWrite >= 3072)
				inputCompWrite = 0;*/

			dcSpeedLeft += (tempBuffer[0][i] - dcPosLeft) * 0.000004567f;
			dcPosLeft += dcSpeedLeft;
			dcSpeedLeft *= 0.96f;

			//In case we get a ridiculously large input, need to reset the
			//filter or it'll break badly.
			if(fabs(dcPosLeft) > 1.0f)
				dcPosLeft = 0.0f;
			if(fabs(dcSpeedLeft) > 1.0f)
				dcSpeedLeft = 0.0f;

			dcSpeedRight += (tempBuffer[1][i] - dcPosRight) * 0.000004567f;
			dcPosRight += dcSpeedRight;
			dcSpeedRight *= 0.96f;

			//In case we get a ridiculously large input, need to reset the
			//filter or it'll break badly.
			if(fabs(dcPosRight) > 1.0f)
				dcPosRight = 0.0f;
			if(fabs(dcSpeedRight) > 1.0f)
				dcSpeedRight = 0.0f;

			tempf = tempBuffer[0][i] - dcPosLeft;
			tempf *= parameters[mixIndex];
			/*delayCompensation[0][delayCompWrite] = tempf + ((1.0f-parameters[mixIndex]) * inputCompensation[0][inputCompWrite]);
			delayCompensation[0][delayCompWrite] *= level;*/
			tempf += ((1.0f-parameters[mixIndex]) * delayCompensation2[0][j]);
			tempf *= level;
			outputs[0][i] = tempf;

			tempf = tempBuffer[1][i] - dcPosRight;
			tempf *= parameters[mixIndex];
			/*delayCompensation[1][delayCompWrite] = tempf + ((1.0f-parameters[mixIndex]) * inputCompensation[1][inputCompWrite]);
			delayCompensation[1][delayCompWrite] *= level;*/
			tempf += ((1.0f-parameters[mixIndex]) * delayCompensation2[1][j]);
			tempf *= level;
			outputs[1][i] = tempf;

			++j;
			if(j >= currentBlockSize)
				j = 0;

			//Make sure to compensate for the delay introduced by the PVOC
			//effects (and conversely compensated by the host).
			/*++delayCompWrite;
			if(delayCompWrite >= 3072)
				delayCompWrite = 0;

			tempReadPointer = delayCompWrite - delayCompSize;
			if(tempReadPointer < 0)
				tempReadPointer += 3072;

			outputs[0][i] = delayCompensation[0][tempReadPointer];
			outputs[1][i] = delayCompensation[1][tempReadPointer];*/
		}
	}

	//----Temp----
	/*for(i=0;i<sampleFrames;++i,--barStartSamples)
	{
		if(barStartSamples == 0)
			outputs[0][i] = outputs[1][i] = 1.0f;
		else
			outputs[0][i] = outputs[1][i] = -1.0f;
	}*/
	//----Temp----

	//MIDI stuff.
	for(i=0;i<frames;++i)
	{
		//Process MIDI events,if there are any.
		if(numEvents>0)
			processMIDI(i);

		/*outputs[0][i] = inputs[0][i];
		outputs[1][i] = inputs[1][i];*/
	}

	//If there are events remaining in the queue, update their delta values.
	if(numPendingEvents > 0)
	{
		for(i=0;i<numPendingEvents;++i)
			midiEvent[eventNumArray[i]]->deltaFrames -= frames;
	}
}

//	----------------------------------------------------------------------------
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
			for(k=1;k<MaxNumEvents;++k)
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
			if((j > 0)&&(numEvents < MaxNumEvents))
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

//	----------------------------------------------------------------------------
void VstPlugin::resume()
{
	int i;

	//Let the host know we want to receive MIDI events.
	AudioEffectX::resume();

	//Get samplerate.
	samplerate = getSampleRate();
	if(samplerate <= 11025.0f)
		samplerate = 44100.0f;

	//Update mod sources' samplerates.
	for(i=0;i<3;++i)
		modSources[i]->setSamplerate(samplerate);
}

//	----------------------------------------------------------------------------
void VstPlugin::suspend()
{
    
}

//	----------------------------------------------------------------------------
void VstPlugin::setBlockSize(VstInt32 blockSize)
{
	int i;

	currentBlockSize = blockSize * 2;
	/*if(currentBlockSize < 6144)
		currentBlockSize = 6144;*/
	if(currentBlockSize < 15360)
		currentBlockSize = 15360;

	if(tempBuffer[0])
		delete [] tempBuffer[0];
	if(tempBuffer[1])
		delete [] tempBuffer[1];
	tempBuffer[0] = new float[blockSize];
	tempBuffer[1] = new float[blockSize];

	if(tempInput[0])
		delete [] tempInput[0];
	if(tempInput[1])
		delete [] tempInput[1];
	tempInput[0] = new float[blockSize];
	tempInput[1] = new float[blockSize];

	if(delayCompensation2[0])
		delete [] delayCompensation2[0];
	if(delayCompensation2[1])
		delete [] delayCompensation2[1];
	delayCompensation2[0] = new float[currentBlockSize];
	delayCompensation2[1] = new float[currentBlockSize];

	for(i=0;i<currentBlockSize;++i)
	{
		delayCompensation2[0][i] = 0.0f;
		delayCompensation2[1][i] = 0.0f;
	}

	for(i=0;i<3;++i)
		modSources[i]->setBlockSize(blockSize);
	for(i=0;i<6;++i)
		effects[i]->setBlockSize(blockSize);
}

//	----------------------------------------------------------------------------
void VstPlugin::parameterChanged(VstInt32 index, float val)
{
	MultiModSource *modSource;

	if(index == modSourceType[0])
	{
		modSource = dynamic_cast<MultiModSource *>(modSources[0]);

		if(modSource)
			modSource->changeModType((unsigned int)(val * 2.0f));
	}
	else if(index == modSourceType[1])
	{
		modSource = dynamic_cast<MultiModSource *>(modSources[1]);

		if(modSource)
			modSource->changeModType((unsigned int)(val * 2.0f));
	}
	else if(index == modSourceType[2])
	{
		modSource = dynamic_cast<MultiModSource *>(modSources[2]);

		if(modSource)
			modSource->changeModType((unsigned int)(val * 2.0f));
	}
	else if(index == HostSync)
	{
		int i;

		for(i=0;i<6;++i)
			effects[i]->setSyncMode(val > 0.5f);
		for(i=0;i<3;++i)
			modSources[i]->setSyncMode(val > 0.5f);
	}
#ifdef PRESETSAVER
	else if((index == PresetSaver) && (val > 0.5f))
	{
		VstInt32 i;

		ofstream outf("FragmentalPreset.cpp", ios::trunc);

		if(outf.good())
		{
			outf << "\tsetProgramName(\"" << programs[curProgram].name;
			outf << "\");" << endl;
			for(i=0;i<PresetSaver;++i)
			{
				outf << "\tsetParameter(" << i;
				outf << ", " << parameters[i];
				outf << "f);" << endl;
			}
			outf.close();
		}
	}
#endif
}

//	----------------------------------------------------------------------------
float VstPlugin::getValue(VstInt32 index)
{
	return parameters[index];
}

//	----------------------------------------------------------------------------
string VstPlugin::getTextValue(VstInt32 index)
{
	int i;
	ostringstream tempstr;

	if(index == mixIndex)
		tempstr << parameters[mixIndex];
	else if(index == outputIndex)
		tempstr << parameters[outputIndex] * 2.0f;
	else if(index == outputMW1)
		tempstr << parameters[outputMW1];
	else if(index == outputMW2)
		tempstr << parameters[outputMW2];
	else if(index == outputMW3)
		tempstr << parameters[outputMW3];
	else
	{
		for(i=0;i<6;++i)
		{
			if(index == routing[i])
			{
				tempstr <<static_cast<int>(parameters[routing[i]] * 6.0f);
				break;
			}

		}
	}

	return tempstr.str();
}

//	----------------------------------------------------------------------------
void VstPlugin::setProgram(VstInt32 program)
{
	int i;

	curProgram = program;

	for(i=0;i<NumParameters;++i)
	{
		if((i != (ModRed+LFOPreset)) &&
		   (i != (ModGreen+LFOPreset)) &&
		   (i != (ModBlue+LFOPreset)))
		{
			setParameter(i, programs[curProgram].parameters[i]);
		}
	}
}

//	----------------------------------------------------------------------------
void VstPlugin::setProgramName(char *name)
{
	programs[curProgram].name = name;
}


//	----------------------------------------------------------------------------
void VstPlugin::getProgramName(char *name)
{
	strcpy(name, programs[curProgram].name.c_str());
}

//	----------------------------------------------------------------------------
bool VstPlugin::getProgramNameIndexed(VstInt32 category, VstInt32 index, char* text)
{
	bool retval = false;

    if(index < NumPrograms)
    {
		strcpy(text, programs[index].name.c_str());
		retval = true;
    }

	return retval;
}

//	----------------------------------------------------------------------------
bool VstPlugin::copyProgram(VstInt32 destination)
{
	bool retval = false;

    if(destination < NumPrograms)
    {
		programs[destination] = programs[curProgram];
        retval = true;
    }

    return retval;
}

//	----------------------------------------------------------------------------
void VstPlugin::setParameter(VstInt32 index, float value)
{
	parameters[index] = value;
	programs[curProgram].parameters[index] = parameters[index];

	paramManager.setParameter(index, value);

	if(editor)
		((MultiGUIEditor *)editor)->setParameter(index, value);
}

//	----------------------------------------------------------------------------
float VstPlugin::getParameter(VstInt32 index)
{
	return paramManager[index];
}

//	----------------------------------------------------------------------------
void VstPlugin::getParameterLabel(VstInt32 index, char *label)
{
	strcpy(label, paramManager.getParameterUnits(index).c_str());
}

//	----------------------------------------------------------------------------
void VstPlugin::getParameterDisplay(VstInt32 index, char *text)
{
	strcpy(text, paramManager.getParameterValue(index).c_str());
}

//	----------------------------------------------------------------------------
void VstPlugin::getParameterName(VstInt32 index, char *label)
{
	strcpy(label, paramManager.getParameterName(index).c_str());
}

//	----------------------------------------------------------------------------
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

//	----------------------------------------------------------------------------
float VstPlugin::getVu()
{
	return 0.0f;
}

//	----------------------------------------------------------------------------
bool VstPlugin::getEffectName(char* name)
{
    strcpy(name, effectName.c_str());

    return true;
}

//	----------------------------------------------------------------------------
bool VstPlugin::getVendorString(char* text)
{
    strcpy(text, vendorName.c_str());

    return true;
}

//	----------------------------------------------------------------------------
bool VstPlugin::getProductString(char* text)
{
    strcpy(text, effectName.c_str());

    return true;
}

//	----------------------------------------------------------------------------
VstInt32 VstPlugin::getVendorVersion()
{
    return VersionNumber;
}

//	----------------------------------------------------------------------------
VstPlugCategory VstPlugin::getPlugCategory()
{
    return(kPlugCategEffect);
}

//	----------------------------------------------------------------------------
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

//	----------------------------------------------------------------------------
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

//	----------------------------------------------------------------------------
VstInt32 VstPlugin::getGetTailSize()
{
	return 1; //1=no tail, 0=don't know, everything else=tail size
}

//	----------------------------------------------------------------------------
float VstPlugin::getKnobPosition(int modSource) const
{
	float retval = 0;
	MultiModSource *tempSource;

	switch(modSource)
	{
		case ModBlue:
			tempSource = dynamic_cast<MultiModSource *>(modSources[0]);
			break;
		case ModRed:
			tempSource = dynamic_cast<MultiModSource *>(modSources[1]);
			break;
		case ModGreen:
			tempSource = dynamic_cast<MultiModSource *>(modSources[2]);
			break;
	}

	if(tempSource)
		retval = tempSource->getKnobPos();

	return retval;
}

//	----------------------------------------------------------------------------
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

//	----------------------------------------------------------------------------
void VstPlugin::MIDI_NoteOn(int ch, int note, int val, int delta)
{
	midiEvent[0]->midiData[0] = 0x90 + ch;
	midiEvent[0]->midiData[1] = note;
	midiEvent[0]->midiData[2] = val;
	midiEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(tempEvents);
}

//	----------------------------------------------------------------------------
void VstPlugin::MIDI_NoteOff(int ch, int note, int val, int delta)
{
	midiEvent[0]->midiData[0] = 0x80 + ch;
	midiEvent[0]->midiData[1] = note;
	midiEvent[0]->midiData[2] = val;
	midiEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(tempEvents);
}

//	----------------------------------------------------------------------------
void VstPlugin::MIDI_PolyAftertouch(int ch, int note, int val, int delta)
{
	midiEvent[0]->midiData[0] = 0xA0 + ch;
	midiEvent[0]->midiData[1] = note;
	midiEvent[0]->midiData[2] = val;
	midiEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(tempEvents);
}

//	----------------------------------------------------------------------------
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

//	----------------------------------------------------------------------------
void VstPlugin::MIDI_ProgramChange(int ch, int val, int delta)
{
	if(val < NumPrograms)
		setProgram(val);

	midiEvent[0]->midiData[0] = 0xD0 + ch;
	midiEvent[0]->midiData[1] = val;
	midiEvent[0]->midiData[2] = 0;
	midiEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(tempEvents);
}

//	----------------------------------------------------------------------------
void VstPlugin::MIDI_ChannelAftertouch(int ch, int val, int delta)
{
	midiEvent[0]->midiData[0] = 0xD0 + ch;
	midiEvent[0]->midiData[1] = val;
	midiEvent[0]->midiData[2] = 0;
	midiEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(tempEvents);
}

//	----------------------------------------------------------------------------
void VstPlugin::MIDI_PitchBend(int ch, int x1, int x2, int delta)
{
	midiEvent[0]->midiData[0] = 0xE0 + ch;
	midiEvent[0]->midiData[1] = x1;
	midiEvent[0]->midiData[2] = x2;
	midiEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(tempEvents);
}
