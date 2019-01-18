//-----------------------------------------------------------------------------
// LICENSE
// (c) 2013, Steinberg Media Technologies GmbH, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this
//     software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

// --- first
#include "VSTProcessor.h"

#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/base/ftypes.h"
#include "pluginterfaces/base/futils.h"

#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/ivstmidicontrollers.h"

#include "base/source/fstreamer.h"

#include "version.h"	// for versioning
#include "PeakParameter.h"
#include "LogParameter.h"	// WP Custom paremeter for RAFX
#include "VoltOctaveParameter.h"	// WP Custom paremeter for RAFX

// --- RackAFX Core
#include "synthfunctions.h"
#include "SynthParamLimits.h"
#include "RafxPluginFactory.h"

// --- for RackAFX GUI support
#include "vstgui/plugin-bindings/vst3padcontroller.h"
#include "vstgui/plugin-bindings/vst3groupcontroller.h"
#include "vstgui/uidescription/xmlparser.h"
#include "vstgui/uidescription/uiattributes.h"

// --- v6.6
#include "vstgui/uidescription/uiviewswitchcontainer.h"

// --- RackAFX custom VSTGUI4 derived classes
#include "VST3EditorWP.h"
#include "XYPadWP.h"
#include "VuMeterWP.h"
#include "SliderWP.h"
#include "XYPadWP.h"
#include "PadControllerWP.h"
#include "RafxPluginFactory.h"
#include "KickButtonWP.h"
#include "TextLabelEditWP.h"

// --- Synth Stuff
//     NOTE: as of RAFX v6.8.0.8, MIDI Rendering is now sample accurate; there is no more synth sub-buffer
#define MAX_CHANNELS 32
// --- these dummy variables are only for RackAFX's processAudioFrame( ) function with synths, see Process() function
float dummyInputL[MAX_CHANNELS];
float dummyInputR[MAX_CHANNELS];
float* dummyInputPtr[2];
extern void* moduleHandle;

enum {
	kPresetParam = 'prst',
};
// --- VST2 Wrapper built-in
::AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
    return Steinberg::Vst::Vst2Wrapper::create(GetPluginFactory(),	/* calls factory.cpp macro */
											   Steinberg::Vst::Sock2VST3::Processor::cid,		/* proc CID */
											   'fuck',	/* 10 dig code for this project (set in RackAFX when you create the export */
											   audioMaster);
}

namespace Steinberg {
namespace Vst {
namespace Sock2VST3 {

using namespace std;

const UINT FILTER_CONTROL_USER_VARIABLE				= 105; // user variables 8/7/14
const UINT FILTER_CONTROL_USER_VSTGUI_VARIABLE		= 106; // user variables 8/7/14

// --- for versioning in serialization
static uint64 VSTPluginVersion = 1;

// --- the unique identifier (use guidgen.exe to generate)
FUID Processor::cid(4294967275, 4294967218, 4294967267, 4294934619);
/*
	Processor::Processor()
	construction
*/
Processor::Processor()
{
	// --- just to be a good programmer
	m_pRAFXPlugIn = NULL;
	m_pParamUpdateQueueArray = NULL;
	m_pMidiEventList = NULL;
	m_bHasSidechain = false;
	m_bAppendUnits = true; // change this to false if you don't want this GUI behavior (which mimics RackAFX)
	m_bEnableSAAVST3 = false;
	m_nSampleAccuracy = 1;
	m_dJoystickX = 0.5;
	m_dJoystickY = 0.5;
	m_bPlugInSideBypass = false;
}

/*
	Processor::~Processor()
	destruction
*/
Processor::~Processor()
{
	if(m_pRAFXPlugIn)
		delete m_pRAFXPlugIn;
	m_pRAFXPlugIn = NULL;
}
/*
	Processor::initialize()
	Call the base class
	Add a Stereo Audio Output
	Add a MIDI event inputs (16: one for each channel)
	Add GUI parameters (EditController part)
*/
tresult PLUGIN_API Processor::initialize(FUnknown* context)
{
	tresult result = SingleComponentEffect::initialize(context);

	// ---  now set plugin buddy
	m_pRAFXPlugIn = CRafxPluginFactory::getRafxPlugIn();
	assert(m_pRAFXPlugIn);

	// --- set latency, sidechain
	if (m_pRAFXPlugIn)
	{
		m_pRAFXPlugIn->initialize();
		m_uLatencyInSamples = (uint32)(m_pRAFXPlugIn->m_fPlugInEx[LATENCY_IN_SAMPLES]);
		m_bHasSidechain = m_pRAFXPlugIn->m_uPlugInEx[ENABLE_SIDECHAIN_VSTAU];
		m_bEnableSAAVST3 = m_pRAFXPlugIn->m_uPlugInEx[ENABLE_SAA_VST3];
		m_nSampleAccuracy = m_pRAFXPlugIn->m_uPlugInEx[SAA_VST3_GRANULARITY];
	}

	// --- finish init
	if(result == kResultTrue)
	{
		// --- stereo output bus (SYNTH and FX)
		addAudioOutput(STR16("Audio Output"), SpeakerArr::kStereo);

		if(!m_pRAFXPlugIn->m_bOutputOnlyPlugIn)
		{
			// stereo input bus (FX ONLY)
			addAudioInput(STR16("AudioInput"), SpeakerArr::kStereo);

			if(m_bHasSidechain)
				addAudioInput(STR16("AuxInput"), SpeakerArr::kStereo, kAux);
		}

		// --- SYNTH/FX: MIDI event input bus, 16 channels
		addEventInput(STR16("Event Input"), 16);

		// --- Init parameters
		Parameter* param;

		if(m_pRAFXPlugIn)
		{
			// --- create our sample accurate list
			m_pMidiEventList = new CVSTMidiEventList(m_pRAFXPlugIn);

			PROCESS_INFO processInfo = { 0 };
			processInfo.pIMidiEventList = m_pMidiEventList;

			// --- hand it to the plugin
			m_pRAFXPlugIn->processRackAFXMessage(midiEventList, processInfo);

			// --- iterate
			int nParams = m_pRAFXPlugIn->getControlCount();

			// --- create the queue
			if (m_bEnableSAAVST3)
			{
				m_pParamUpdateQueueArray = new CVSTParamUpdateQueue *[nParams];
				memset(m_pParamUpdateQueueArray, 0, sizeof(CVSTParamUpdateQueue *) * nParams);
			}

			// iterate
			for(int i = 0; i < nParams; i++)
			{
				// they are in VST proper order in the ControlList - do NOT reference them with RackAFX ID values any more!
				CUICtrl* pUICtrl = m_pRAFXPlugIn->getUICtrlByListIndex(i);

				if(pUICtrl)
				{
					float fDefault = m_pRAFXPlugIn->getParameterValue(i);

					// --- sample accurate automation
					if (m_bEnableSAAVST3)
					{
						m_pParamUpdateQueueArray[i] = new CVSTParamUpdateQueue();
						m_pParamUpdateQueueArray[i]->initialize(fDefault, pUICtrl->fUserDisplayDataLoLimit, pUICtrl->fUserDisplayDataHiLimit, &m_nSampleAccuracy);
					}

					if(pUICtrl->uControlType == FILTER_CONTROL_LED_METER)
					{
						PeakParameter* peakParam = new PeakParameter(ParameterInfo::kIsReadOnly, i, USTRING(pUICtrl->cControlName));
						peakParam->setNormalized(0.0);
						parameters.addParameter(peakParam);
						meters.push_back(i); // save tag
					}
					else if(pUICtrl->uControlType == FILTER_CONTROL_CONTINUOUSLY_VARIABLE ||
							pUICtrl->uControlType == FILTER_CONTROL_RADIO_SWITCH_VARIABLE ||
							pUICtrl->uControlType == FILTER_CONTROL_USER_VSTGUI_VARIABLE ||
							pUICtrl->uControlType == FILTER_CONTROL_COMBO_VARIABLE)
					{
						if(pUICtrl->uUserDataType != UINTData)
						{
							char* pName = pUICtrl->cControlName;
							Parameter* param = NULL;

							if(pUICtrl->bLogSlider)
							{
								param = new LogParameter(USTRING(pName),
														   i, /* INDEX !! */
														   USTRING(pUICtrl->cControlUnits),
														   pUICtrl->fUserDisplayDataLoLimit,
														   pUICtrl->fUserDisplayDataHiLimit,
														   fDefault);
								param->setPrecision(pUICtrl->uUserDataType == intData ? 0 : pUICtrl->uControlTheme[GUI_SIG_DIGITS]); // fractional sig digits
							}
							else if(pUICtrl->bExpSlider)
							{
								param = new VoltOctaveParameter(USTRING(pName),
														   i, /* INDEX !! */
														   USTRING(pUICtrl->cControlUnits),
														   pUICtrl->fUserDisplayDataLoLimit,
														   pUICtrl->fUserDisplayDataHiLimit,
														   fDefault);
								param->setPrecision(pUICtrl->uUserDataType == intData ? 0 :  pUICtrl->uControlTheme[GUI_SIG_DIGITS]); // fractional sig digits
							}
							else // linear
							{
								param = new RangeParameter(USTRING(pName),
														   i, /* INDEX !! */
														   USTRING(pUICtrl->cControlUnits),
														   pUICtrl->fUserDisplayDataLoLimit,
														   pUICtrl->fUserDisplayDataHiLimit,
														   fDefault);
								param->setPrecision(pUICtrl->uUserDataType == intData ? 0 :  pUICtrl->uControlTheme[GUI_SIG_DIGITS]); // fractional sig digits
							}
							//
							// --- add it
							parameters.addParameter(param);
						}
						else
						{
							char* pName = pUICtrl->cControlName;
							StringListParameter* enumStringParam = new StringListParameter(USTRING(pName), i);
							int m = 0;
							char* pEnumString = NULL;

							pEnumString = getEnumString(pUICtrl->cEnumeratedList, m++);
							while(pEnumString)
							{
								enumStringParam->appendString(USTRING(pEnumString));
								delete [] pEnumString;
								pEnumString = getEnumString(pUICtrl->cEnumeratedList, m++);
							}
							parameters.addParameter(enumStringParam);
						}
					}
				}
			}

			// --- NOTE: these must be the following 2 parameters after the normal UI Control List params
			//           to keep consistent with AU, AAX, RAFX
			if(true) // for off-screen view support
			{
				param = new RangeParameter(USTRING("VectorJoystick X"), JOYSTICK_X_PARAM, USTRING(""), 0, 1, 0.5);
				param->setPrecision(2); // fractional sig digits
				parameters.addParameter(param);

				param = new RangeParameter(USTRING("VectorJoystick Y"), JOYSTICK_Y_PARAM, USTRING(""), 0, 1, 0.5);
				param->setPrecision(2); // fractional sig digits
				parameters.addParameter(param);
			}

			char* p = m_pRAFXPlugIn->m_AssignButton1Name;
			int n = strlen(p);
			if(n > 0)
			{
				param = new RangeParameter(USTRING(m_pRAFXPlugIn->m_AssignButton1Name), ASSIGNBUTTON_1, USTRING(""),
										   MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_UNIPOLAR);
				param->setPrecision(0); // fractional sig digits
				parameters.addParameter(param);
			}

			p = m_pRAFXPlugIn->m_AssignButton2Name;
			n = strlen(p);
			if(n > 0)
			{
				param = new RangeParameter(USTRING(m_pRAFXPlugIn->m_AssignButton2Name), ASSIGNBUTTON_2, USTRING(""),
										   MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_UNIPOLAR);
				param->setPrecision(0); // fractional sig digits
				parameters.addParameter(param);
			}

			p = m_pRAFXPlugIn->m_AssignButton3Name;
			n = strlen(p);
			if(n > 0)
			{
				param = new RangeParameter(USTRING(m_pRAFXPlugIn->m_AssignButton3Name), ASSIGNBUTTON_3, USTRING(""),
										   MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_UNIPOLAR);
				param->setPrecision(0); // fractional sig digits
				parameters.addParameter(param);
			}
		}

		// --- one and only bypass parameter
		param = new RangeParameter(USTRING("Bypass"), PLUGIN_SIDE_BYPASS, USTRING(""),
								   0, 1, 0, 0, ParameterInfo::kCanAutomate|ParameterInfo::kIsBypass);
		parameters.addParameter(param);

		// MIDI Params - these have no knobs in main GUI but do have to appear in default
		// NOTE: this is for VST3 ONLY! Not needed in AU or RAFX
		param = new RangeParameter(USTRING("PitchBend"), MIDI_PITCHBEND, USTRING(""),
								   MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_UNIPOLAR);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);

		param = new RangeParameter(USTRING("MIDI Vol"), MIDI_VOLUME_CC7, USTRING(""),
								   MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_UNIPOLAR);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);

		param = new RangeParameter(USTRING("MIDI Pan"), MIDI_PAN_CC10, USTRING(""),
								   MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_UNIPOLAR);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);

		param = new RangeParameter(USTRING("MIDI Mod Wheel"), MIDI_MODWHEEL, USTRING(""),
								   MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_UNIPOLAR);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);

		param = new RangeParameter(USTRING("MIDI Expression"), MIDI_EXPRESSION_CC11, USTRING(""),
								   MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_UNIPOLAR);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);

		param = new RangeParameter(USTRING("MIDI Channel Pressure"), MIDI_CHANNEL_PRESSURE, USTRING(""),
								   MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_UNIPOLAR);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);

		param = new RangeParameter(USTRING("MIDI Sustain Pedal"), MIDI_SUSTAIN_PEDAL, USTRING(""),
								   MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_UNIPOLAR);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);

		param = new RangeParameter(USTRING("All Notes Off"), MIDI_ALL_NOTES_OFF, USTRING(""),
								   MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_UNIPOLAR);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);
	}

	// --- check for presets, avoid crashy
	int nPresets = 0;
	for(int i=0; i<PRESET_COUNT; i++)
	{
		char* p = m_pRAFXPlugIn->m_PresetNames[i];
		if(p)
		{
			if(strlen(p) > 0)
				nPresets++;
		}
	}

	// --- root
	UnitInfo uinfoRoot;
	uinfoRoot.id = 1;
	uinfoRoot.parentUnitId = kRootUnitId;
	uinfoRoot.programListId = kNoProgramListId;
	Steinberg::UString (uinfoRoot.name, USTRINGSIZE (uinfoRoot.name)).assign (USTRING ("RootUnit"));
	addUnit(new Unit (uinfoRoot));

	if(nPresets > 0)
	{
		// --- add presets
		UnitInfo uinfoPreset;
		uinfoPreset.id = kRootUnitId;
		uinfoPreset.parentUnitId = 1;
		uinfoPreset.programListId = kPresetParam;
		UString name(uinfoPreset.name, 128);
		name.fromAscii("PresetsUnit");
		addUnit(new Unit (uinfoPreset));

		// --- the PRESET parameter
		StringListParameter* presetParam = new StringListParameter(USTRING("Factory Presets"),
																   kPresetParam, USTRING(""),
																   ParameterInfo::kIsProgramChange | ParameterInfo::kIsList,
																   kRootUnitId);
		// --- enumerate names
		for(int i=0; i<PRESET_COUNT; i++)
		{
			char* p = m_pRAFXPlugIn->m_PresetNames[i];
			if(p)
			{
				if(strlen(p) > 0)
					presetParam->appendString(USTRING(p));
			}
		}

		// --- add preset
		parameters.addParameter(presetParam);
	}

	return result;
}

/*
	Processor::setBusArrangements()
	Client queries us for our supported Busses; this is where you can modify to support mono, surround, etc...
*/
tresult PLUGIN_API Processor::setBusArrangements(SpeakerArrangement* inputs, int32 numIns,
												   SpeakerArrangement* outputs, int32 numOuts)
{
	if(m_pRAFXPlugIn->m_bOutputOnlyPlugIn)
	{
		// SYNTH: one stereo output bus
		if(numIns == 0 && numOuts == 1 && outputs[0] == SpeakerArr::kStereo)
		{
			return SingleComponentEffect::setBusArrangements (inputs, numIns, outputs, numOuts);
		}
	}
	else
	{
		// FX: one input bus and one output bus of same channel count
		if(numIns == 1 && numOuts == 1 && inputs[0] == outputs[0])
		{
			return SingleComponentEffect::setBusArrangements (inputs, numIns, outputs, numOuts);
		}
	}

	return kResultFalse;
}

/*
	Processor::canProcessSampleSize()
	Client queries us for our supported sample lengths
*/
tresult PLUGIN_API Processor::canProcessSampleSize(int32 symbolicSampleSize)
{
	// this is a challenge in the book; here is where you say you support it but
	// you will need to deal with different buffers in the process() method
//	if (symbolicSampleSize == kSample32 || symbolicSampleSize == kSample64)

	// --- currently 32 bit only
	if (symbolicSampleSize == kSample32)
	{
		return kResultTrue;
	}
	return kResultFalse;
}

/*
	Processor::setupProcessing()

	we get information about sample rate, bit-depth, etc...
*/
tresult PLUGIN_API Processor::setupProcessing(ProcessSetup& newSetup)
{
	if(m_pRAFXPlugIn) // should never fail to have plugin
	{
		m_pRAFXPlugIn->m_nSampleRate = (int)processSetup.sampleRate;
		m_pRAFXPlugIn->prepareForPlay();
	}

	// --- base class
	return SingleComponentEffect::setupProcessing(newSetup);
}

/*
	Processor::getTailSamples()
	Returns the tail-time in samples.
*/
uint32 PLUGIN_API Processor::getTailSamples()
{
	if (m_pRAFXPlugIn)
	{
		if (m_pRAFXPlugIn->m_uPlugInEx[VST_INFINITE_TAIL] == 1)
			return kInfiniteTail;
		else
			return (uint32)(processSetup.sampleRate*(m_pRAFXPlugIn->m_fPlugInEx[TAILTIME_IN_MSEC] / 1000.0));
	}
	else
		return 0;
}

/*
	Processor::setActive()
	This is the analog of prepareForPlay() in RAFX since the Sample Rate is now set.

	VST3 plugins may be turned on or off; you are supposed to dynamically delare stuff when activated
	then delete when de-activated.
*/
tresult PLUGIN_API Processor::setActive(TBool state)
{
	if(state)
	{
		// --- do ON stuff; dynamic allocations
		if(m_pRAFXPlugIn) // should never fail to have plugin
		{
			m_pRAFXPlugIn->m_nSampleRate = (int)processSetup.sampleRate;
			m_pRAFXPlugIn->prepareForPlay();
		}
	}
	else
	{
		// --- do OFF stuff
		// do not delete RAFX plugin or other dynamically allocated stuff here, use Processor::terminate()
	}

	// --- base class method call is last
	return SingleComponentEffect::setActive (state);
}

/*
	Processor::setState()
	This is the READ part of the serialization process. We get the stream interface and use it
	to read from the filestream.

	NOTE: The datatypes/read order must EXACTLY match the getState() version or crashes may happen or variables
	      not initialized properly.
*/
tresult PLUGIN_API Processor::setState(IBStream* fileStream)
{
	IBStreamer s(fileStream, kLittleEndian);
	uint64 version = 0;

	// --- needed to convert to our UINT reads
	uint32 udata = 0;
	int32 data = 0;
	float fdata = 0;
	double ddata = 0;

	// --- read the version
	if(!s.readInt64u(version)) return kResultFalse;

	int nParams = m_pRAFXPlugIn->getControlCount();

	// --- iterate
	for(int i = 0; i < nParams; i++)
	{
		// they are in VST proper order in the ControlList - do NOT reference them with RackAFX ID values any more!
		CUICtrl* pUICtrl = m_pRAFXPlugIn->getUICtrlByListIndex(i);

		// --- iterate
		if(pUICtrl)
		{
			// --- temporarily disable smoothing
			bool smooth = pUICtrl->bEnableParamSmoothing;
			pUICtrl->bEnableParamSmoothing = false;

			if(pUICtrl->uUserDataType == intData){
				if(!s.readInt32(data)) return kResultFalse; else m_pRAFXPlugIn->setParameterValue(i, (float)data);}
			else if(pUICtrl->uUserDataType == floatData){
				if(!s.readFloat(fdata)) return kResultFalse; else m_pRAFXPlugIn->setParameterValue(i, fdata);}
			else if(pUICtrl->uUserDataType == doubleData){
				if(!s.readDouble(ddata)) return kResultFalse; else m_pRAFXPlugIn->setParameterValue(i, (float)ddata);}
			else if(pUICtrl->uUserDataType == UINTData){
				if(!s.readInt32u(udata)) return kResultFalse; else m_pRAFXPlugIn->setParameterValue(i, (float)udata);}

			// --- reset
			pUICtrl->bEnableParamSmoothing = smooth;
		}
	}

	// --- add plugin side bypassing
	if(!s.readBool(m_bPlugInSideBypass)) return kResultFalse;

	// --- do next version...
	if(version >= 1)
	{
		// --- v1: adds vector joystick
		if(!s.readFloat(fdata))
			return kResultFalse;
		else
			m_pRAFXPlugIn->setVectorJSXValue(fdata);

		if(!s.readFloat(fdata))
			return kResultFalse;
		else
			m_pRAFXPlugIn->setVectorJSYValue(fdata);
	}
	return kResultTrue;
}

/*
	Processor::getState()
	This is the WRITE part of the serialization process. We get the stream interface and use it
	to write to the filestream. This is important because it is how the Factory Default is set
	at startup, as well as when writing presets.
*/
tresult PLUGIN_API Processor::getState(IBStream* fileStream)
{
	// --- get a stream I/F
	IBStreamer s(fileStream, kLittleEndian);

	// --- Sock2VST3Version - place this at top so versioning can be used during the READ operation
	if(!s.writeInt64u(VSTPluginVersion)) return kResultFalse;

	// --- write out all of the params
	int nParams = m_pRAFXPlugIn->getControlCount();

	// iterate
	for(int i = 0; i < nParams; i++)
	{
		// they are in VST proper order in the ControlList - do NOT reference them with RackAFX ID values any more!
		CUICtrl* pUICtrl = m_pRAFXPlugIn->getUICtrlByListIndex(i);

		// iterate
		if(pUICtrl)
		{
			if(pUICtrl->uUserDataType == intData){
				if(!s.writeInt32((int32)m_pRAFXPlugIn->getParameterValue(i))) return kResultFalse;}
			else if(pUICtrl->uUserDataType == floatData){
				if(!s.writeFloat(m_pRAFXPlugIn->getParameterValue(i))) return kResultFalse;}
			else if(pUICtrl->uUserDataType == doubleData){
				if(!s.writeDouble(m_pRAFXPlugIn->getParameterValue(i))) return kResultFalse;}
			else if(pUICtrl->uUserDataType == UINTData){
				if(!s.writeInt32u((uint32)m_pRAFXPlugIn->getParameterValue(i))) return kResultFalse;}
		}
	}
	// --- add plugin side bypassing
	if(!s.writeBool(m_bPlugInSideBypass)) return kResultFalse;

	// --- v1: adds vector joystick
	float fJS_X = 0.0;
	float fJS_Y = 0.0;
	m_pRAFXPlugIn->getVectorJSValues(fJS_X, fJS_Y);

	if(!s.writeFloat(fJS_X)) return kResultFalse;
	if(!s.writeFloat(fJS_Y)) return kResultFalse;

	return kResultTrue;
}

/*
	Processor::doControlUpdate()
	Find and issue Control Changes (same as userInterfaceChange() in RAFX)
	returns true if a control was changed
*/
bool Processor::doControlUpdate(ProcessData& data)
{
	bool paramChange = false;

	// --- check
	if(!data.inputParameterChanges)
		return paramChange;

	// --- get the param count and setup a loop for processing queue data
	int32 count = data.inputParameterChanges->getParameterCount();

	// --- make sure there is something there
	if(count <= 0)
		return paramChange;

	// --- loop
	for(int32 i=0; i<count; i++)
	{
		// get the message queue for ith parameter
		IParamValueQueue* queue = data.inputParameterChanges->getParameterData(i);

		if(queue)
		{
			// --- check for control points
			if(queue->getPointCount() <= 0) return false;

			int32 sampleOffset = 0.0;
			ParamValue value = 0.0;
			ParamID pid = queue->getParameterId();

			// --- get the last point in queue, default behavior
			//     NOTE: if user has vst3 sample accurate updates ENABLED (m_bEnableSAAVST3 == true), we will transfer the
			//           entire queue with setParamValueQueue() below
			//           Then, the built-in queue handler will read the points, calculate slopes, etc... automatically
			//           and sample accurate-ly during the RackAFX audio processing loop
			if(queue->getPoint(queue->getPointCount()-1, /* last update point */
				sampleOffset,			/* sample offset */
				value) == kResultTrue)	/* value = [0..1] */
			{
				// --- at least one param changed
				paramChange = true;

				// NOTE: because of the strange way VST3 handles MIDI messages, the channel and note/velocity information is lost
				//       for all but the three messages in the doProcessEvent() method
				UINT uChannel = 0;
				UINT uNote = 0;
				UINT uVelocity = 0;

				// first, get the normal plugin parameters
				CUICtrl* pUICtrl = m_pRAFXPlugIn->getUICtrlByListIndex(pid);
				if(pUICtrl)
				{
					// --- v6.6
					if(pUICtrl->bLogSlider)
						value = calcLogPluginValue(value);
					else if(pUICtrl->bExpSlider)
						value = calcVoltOctavePluginValue(value, pUICtrl);

					// --- add the sample accurate queue
					if (m_bEnableSAAVST3)
					{
						m_pParamUpdateQueueArray[i]->setParamValueQueue(queue, data.numSamples);
						pUICtrl->pvAddlData = m_pParamUpdateQueueArray[i];
					}
					else
						pUICtrl->pvAddlData = NULL;

					// --- use the VST style parameter set function
					m_pRAFXPlugIn->setNormalizedParameter(pid, value); // this will call userIntefaceChange()
				}
				// --- custom RAFX
				else if(pid == ASSIGNBUTTON_1) // && value > 0.5)
					m_pRAFXPlugIn->userInterfaceChange(50);
				else if(pid == ASSIGNBUTTON_2) // && value > 0.5)
					m_pRAFXPlugIn->userInterfaceChange(51);
				else if(pid == ASSIGNBUTTON_3) // && value > 0.5)
					m_pRAFXPlugIn->userInterfaceChange(52);
				else if(pid == JOYSTICK_X_PARAM)
				{
					m_dJoystickX = unipolarToBipolar(value);
					m_pRAFXPlugIn->setVectorJSXValue(m_dJoystickX);
				}
				else if(pid == JOYSTICK_Y_PARAM)
				{
					m_dJoystickY = unipolarToBipolar(value);
					m_pRAFXPlugIn->setVectorJSYValue(m_dJoystickY);
				}
				else if(pid == PLUGIN_SIDE_BYPASS) // want 0 to 1
				{
					if(value == 0)
						m_bPlugInSideBypass = false;
					else
						m_bPlugInSideBypass = true;
					break;
				}
				else // try the MIDI controls
				{
					switch(pid) // same as RAFX uControlID
					{
						// --- MIDI messages
						case MIDI_PITCHBEND: // want -1 to +1
						{
							double dMIDIPitchBend = unipolarToBipolar(value);
							int nPitchBend = dMIDIPitchBend == -1.0 ? -8192 : (int)(dMIDIPitchBend*8191.0);
							if(m_pRAFXPlugIn)
								m_pRAFXPlugIn->midiPitchBend(uChannel, nPitchBend, dMIDIPitchBend);

							break;
						}
						case MIDI_MODWHEEL: // want 0 to 127
						{
							if(m_pRAFXPlugIn)
								m_pRAFXPlugIn->midiModWheel(uChannel, unipolarToMIDI(value));

							break;
						}
						case MIDI_ALL_NOTES_OFF:
						{
							if(m_pRAFXPlugIn)
								m_pRAFXPlugIn->midiNoteOff(uChannel, uNote, uVelocity, true);
							break;
						}
						case MIDI_VOLUME_CC7: // want 0 to 127
						{
							if(m_pRAFXPlugIn)
								m_pRAFXPlugIn->midiMessage(uChannel, VOLUME_CC07, unipolarToMIDI(value), 0);
							break;
						}
						case MIDI_PAN_CC10: // want 0 to 127
						{
							if(m_pRAFXPlugIn)
								m_pRAFXPlugIn->midiMessage(uChannel, PAN_CC10, unipolarToMIDI(value), 0);
							break;
						}
						case MIDI_EXPRESSION_CC11: // want 0 to 127
						{
							if(m_pRAFXPlugIn)
								m_pRAFXPlugIn->midiMessage(uChannel, EXPRESSION_CC11, unipolarToMIDI(value), 0);
							break;
						}
						case MIDI_CHANNEL_PRESSURE:
						{
							if(m_pRAFXPlugIn)
								m_pRAFXPlugIn->midiMessage(uChannel, CHANNEL_PRESSURE, unipolarToMIDI(value), 0);
							break;
						}
						case MIDI_SUSTAIN_PEDAL: // want 0 to 1
						{
							UINT uSustain = value > 0.5 ? 127 : 0;
							if(m_pRAFXPlugIn)
								m_pRAFXPlugIn->midiMessage(uChannel, SUSTAIN_PEDAL, uSustain, 0);
							break;
						}
					}
				}
			}
		}
	}

	return paramChange;
}

/*
	Processor::doProcessEvent()
	process MIDI events on a sample-accurate basis
	returns true if a midi message was issued
*/
bool Processor::doProcessEvent(Event& vstEvent)
{
	bool noteEvent = false;

	// --- process Note On or Note Off messages
	switch(vstEvent.type)
	{
		// --- NOTE ON
		case Event::kNoteOnEvent:
		{
			// --- get the channel/note/vel
			UINT uMIDIChannel = (UINT)vstEvent.noteOn.channel;
			UINT uMIDINote = (UINT)vstEvent.noteOn.pitch;
			UINT uMIDIVelocity = (UINT)(127.0*vstEvent.noteOn.velocity);
			noteEvent = true;

			if(m_pRAFXPlugIn)
				m_pRAFXPlugIn->midiNoteOn(uMIDIChannel, uMIDINote, uMIDIVelocity);

			break;
		}

		// --- NOTE OFF
		case Event::kNoteOffEvent:
		{
			// --- get the channel/note/vel
			UINT uMIDIChannel = (UINT)vstEvent.noteOff.channel;
			UINT uMIDINote = (UINT)vstEvent.noteOff.pitch;
			UINT uMIDIVelocity = (UINT)(127.0*vstEvent.noteOff.velocity);
			noteEvent = true;

			if(m_pRAFXPlugIn)
				m_pRAFXPlugIn->midiNoteOff(uMIDIChannel, uMIDINote, uMIDIVelocity, false);

			break;
		}

		// --- polyphonic aftertouch 0xAn
		case Event::kPolyPressureEvent:
		{
			// --- get the channel
			UINT uMIDIChannel = (UINT)vstEvent.polyPressure.channel;
			UINT uMIDINote = (UINT)vstEvent.polyPressure.pitch;
			UINT uMIDIPressure = (UINT)(127.0*vstEvent.polyPressure.pressure);

			if(m_pRAFXPlugIn)
				m_pRAFXPlugIn->midiMessage(uMIDIChannel, POLY_PRESSURE, uMIDINote, uMIDIPressure);

			break;
		}
	}

	// -- note event occurred?
	return noteEvent;
}

/*
 Processor::processEventsForInterval()
 Process all MIDI events for a given sample interval
 
 returns true if at least one event was processed
 */
bool Processor::processEventsForInterval(IEventList* inputEvents, int sampleInterval, int& startIndex)
{
    if(!inputEvents) return false;
    
    bool working = true;
    bool processed = false;
    while(working)
    {
        Event e = { 0 };
        if (inputEvents->getEvent(startIndex, e) == kResultTrue)
        {
            if (e.sampleOffset == sampleInterval)
            {
                doProcessEvent(e);
                processed = true;
                startIndex++;
            }
            else
                working = false;
        }
        else
            working = false;
    }
    
    return processed;
}

/*
	Processor::process()
	The most important function handles:
		Control Changes (same as userInterfaceChange() in RAFX)
		Synth voice rendering
		Output GUI Changes (allows you to write back to the GUI, advanced.
*/
tresult PLUGIN_API Processor::process(ProcessData& data)
{
	if (!m_pRAFXPlugIn) return kResultFalse;

	SpeakerArrangement arr;
	getBusArrangement(kOutput, 0, arr);
	int32 numChannels = SpeakerArr::getChannelCount(arr);

	// v6.6 FIX
	if (m_pRAFXPlugIn->m_bOutputOnlyPlugIn && !data.outputs)
		return kResultTrue;
	else if (!m_pRAFXPlugIn->m_bOutputOnlyPlugIn && (!data.inputs || !data.outputs))
		return kResultTrue;

	// --- for synths, VST3 clients have NULL input buffers!;
	//	   RAFX wants non-null buffers even if it isn't going to use them
	//	   the dummy buffers are declared at the top of this file as globals
	dummyInputPtr[0] = &dummyInputL[0];
	dummyInputPtr[1] = &dummyInputR[0];

	// --- check for control chages and update if needed
	//     NOTE: this handles sample accurate automation as well, if the RackAFX plugin
	//           sets the sample accurate flag: m_bEnableSAAVST3
	doControlUpdate(data);

	// --- check for zero i/o conditions
    if (!m_pRAFXPlugIn->m_bOutputOnlyPlugIn && (data.numInputs == 0 || data.numOutputs == 0))
    {
        // nothing to do
        return kResultOk;
    }
    else if (m_pRAFXPlugIn->m_bOutputOnlyPlugIn && (data.numInputs == 0 && data.numOutputs == 0))
    {
        // nothing to do
        return kResultOk;
    }

	// --- set host data
	HOST_INFO hostInfo = { 0 };
	PROCESS_INFO processInfo = { 0 };
	double dSampleInterval = 1.0 / (double)m_pRAFXPlugIn->m_nSampleRate;

	if (data.processContext) // NOTE: data.processContext is NULL during the VSTValidator process Test part I
	{
		hostInfo.dBPM = data.processContext->tempo;
		hostInfo.fTimeSigNumerator = data.processContext->timeSigNumerator;
		hostInfo.uTimeSigDenomintor = (unsigned int)data.processContext->timeSigDenominator;
		hostInfo.uAbsoluteFrameBufferIndex = (unsigned int)data.processContext->projectTimeSamples;
		hostInfo.dAbsoluteFrameBufferTime = (double)hostInfo.uAbsoluteFrameBufferIndex*dSampleInterval;

		processInfo.nNumParams = -1; // not valid for this message
		processInfo.pHostInfo = &hostInfo;

		// --- set host info for top of buffer
		m_pRAFXPlugIn->processRackAFXMessage(updateHostInfo, processInfo);
	}

	// --- setup for audio processing (use helpers provided by SMTG)
	uint32 sampleFramesSize = getSampleFramesSizeInBytes(processSetup, data.numSamples);
    void** inputBuffers = m_pRAFXPlugIn->m_bOutputOnlyPlugIn ? NULL : getChannelBuffersPointer(processSetup, data.inputs[0]);
	void** outputBuffers = getChannelBuffersPointer(processSetup, data.outputs[0]);

	// ---check silence---------------
	if (data.numInputs > 0 && data.inputs[0].silenceFlags != 0)
	{
		// --- mark output silence too
		data.outputs[0].silenceFlags = data.inputs[0].silenceFlags;

		// --- the Plug-in has to be sure that if it sets the flags silence that the output buffer are clear
		for (int32 i = 0; i < numChannels; i++)
		{
			// --- do not need to be cleared if the buffers are the same (in this case input buffer are already cleared by the host)
			if (inputBuffers[i] != outputBuffers[i])
			{
				memset(outputBuffers[i], 0, sampleFramesSize);
			}
		}
		return kResultOk;
	}

	// --- mark our outputs has not silent
	data.outputs[0].silenceFlags = 0;

	// --- soft bypass for FX plugins
	if (!m_pRAFXPlugIn->m_bOutputOnlyPlugIn && m_bPlugInSideBypass)
	{
		for (int32 i = 0; i < numChannels; i++)
		{
			for (int32 i = 0; i < numChannels; i++)
			{
				// do not need to be copied if the buffers are the same
				if (inputBuffers[i] != outputBuffers[i])
				{
					memcpy(outputBuffers[i], inputBuffers[i], sampleFramesSize);
				}
			}
		}

		// --- un-comment to update meters (may confuse user to see meters with PI bypassed)
		// updateMeters(data, true);

		return kResultTrue;
	}

	// --- process MIDI events
	IEventList* inputEvents = data.inputEvents;

	// --- set on helper
	if (m_pMidiEventList)
		m_pMidiEventList->setEventList(inputEvents);

	// --- sidechain off
	audioProcessData auxInputProcessData;
	auxInputProcessData.uInputBus = 1;
	auxInputProcessData.bInputEnabled = false;
	auxInputProcessData.uNumInputChannels = 0;
	auxInputProcessData.uBufferSize = 0;
	auxInputProcessData.pFrameInputBuffer = NULL;
	auxInputProcessData.pRAFXInputBuffer = NULL;
	auxInputProcessData.ppVSTInputBuffer = NULL;

	// --- see if plugin will process natively
	if (m_pRAFXPlugIn->m_bWantVSTBuffers)
	{
		// --- send sidechain for this WHOLE buffer
		if (m_bHasSidechain)
		{
			BusList* busList = getBusList(kAudio, kInput);
			Bus* bus = busList ? (Bus*)busList->at(1) : 0;
			if (bus && bus->isActive())
			{
				void** scInput = getChannelBuffersPointer(processSetup, data.inputs[1]);
				auxInputProcessData.bInputEnabled = true;
				auxInputProcessData.uNumInputChannels = data.inputs[1].numChannels;
				auxInputProcessData.ppVSTInputBuffer = (Sample32**)scInput; //** to sidechain

				// --- process sidechain
				m_pRAFXPlugIn->processAuxInputBus(&auxInputProcessData);
			}
		}

		// --- now process buffer
		if (m_pRAFXPlugIn->m_bOutputOnlyPlugIn)
		{
			m_pRAFXPlugIn->processVSTAudioBuffer(&dummyInputPtr[0],
				(Sample32**)inputBuffers,
				numChannels,
				data.numSamples);
		}
		else
		{
			m_pRAFXPlugIn->processVSTAudioBuffer((Sample32**)inputBuffers,
				(Sample32**)outputBuffers,
				numChannels,
				data.numSamples);
		}

		// --- update the meters
		updateMeters(data, true);

		return kResultTrue;
	}

    // --- MIDI event counter
    int32 eventIndex = 0;

	// --- static buffs for RAFX
	float fInputs[2]; fInputs[0] = 0.0; fInputs[1] = 0.0;
	float fOutputs[2]; fOutputs[0] = 0.0; fOutputs[1] = 0.0;
	float fAuxInput[2]; fAuxInput[0] = 0.0; fAuxInput[1] = 0.0;

	for (int32 sample = 0; sample < data.numSamples; sample++)
	{
		if (m_bHasSidechain)
		{
			BusList* busList = getBusList(kAudio, kInput);
			Bus* bus = busList ? (Bus*)busList->at(1) : 0;
			if (bus && bus->isActive())
			{
				fAuxInput[0] = (data.inputs[1].channelBuffers32[0])[sample];

				if (data.inputs[1].numChannels == 2)
					fAuxInput[1] = (data.inputs[1].channelBuffers32[1])[sample];

				auxInputProcessData.bInputEnabled = true;
				auxInputProcessData.uNumInputChannels = data.inputs[1].numChannels;
				auxInputProcessData.pFrameInputBuffer = &fAuxInput[0];

				// --- process sidechain
				m_pRAFXPlugIn->processAuxInputBus(&auxInputProcessData);
			}
		}

        // --- process MIDI block
        processEventsForInterval(inputEvents, sample, (int&)eventIndex);

		// --- setup for processAudioFrame()
		if (!m_pRAFXPlugIn->m_bOutputOnlyPlugIn)
		{
			fInputs[0] = (data.inputs[0].channelBuffers32[0])[sample];
			if (numChannels == 2)
				fInputs[1] = (data.inputs[0].channelBuffers32[1])[sample];
		}

		if (m_pRAFXPlugIn)
			m_pRAFXPlugIn->processAudioFrame(&fInputs[0], &fOutputs[0], numChannels, numChannels);

		// --- write outputs
		(data.outputs[0].channelBuffers32[0])[sample] = fOutputs[0];
		if (numChannels == 2)
			(data.outputs[0].channelBuffers32[1])[sample] = fOutputs[1];

		hostInfo.uAbsoluteFrameBufferIndex++;
		hostInfo.dAbsoluteFrameBufferTime = (double)hostInfo.uAbsoluteFrameBufferIndex*dSampleInterval;

		// --- set host info for next frame
		m_pRAFXPlugIn->processRackAFXMessage(updateHostInfo, processInfo);
	}

	// --- update the meters
	updateMeters(data);

	return kResultTrue;

}

/*
	Processor::updateMeters()
	updates the meter variables
*/
void Processor::updateMeters(ProcessData& data, bool bForceOff)
{
	if(!m_pRAFXPlugIn) return;

	if(data.outputParameterChanges)
	{
		int nCount = meters.size();
		for(int i=0; i<nCount; i++)
		{
			CUICtrl* pUICtrl = m_pRAFXPlugIn->getUICtrlByListIndex(meters[i]);
			if(pUICtrl)
			{
				int32 index;
				IParamValueQueue* queue = data.outputParameterChanges->addParameterData(meters[i], index);

				if(queue && pUICtrl->m_pCurrentMeterValue)
				{
					float fMeter = bForceOff ? 0.0 : *pUICtrl->m_pCurrentMeterValue;
					queue->addPoint(i, fMeter, index);
				}
			}
		}
	}
}

/*
	--- IMIDIMapping Interface
	Processor::getMidiControllerAssignment()

	// --- NOTE: using proxy variables here. Steinberg recommends letting the host & user map the MIDI CCs
	//           rather than use proxy variables. The way it is done here creates a GLOBAL MIDI CC map
	//           for the same CCs on ANY MIDI channel. You can customize this as you like, or disable it altogether
	//           by returning kResultFalse.

	The client queries this 129 times for 130 possible control messages, see ivstsmidicontrollers.h for
	the VST defines for kPitchBend, kCtrlModWheel, etc... for each MIDI Channel in our Event Bus

	We respond with our ControlID value that we'll use to process the MIDI Messages in Processor::process().

	On the default GUI, these controls will actually move with the MIDI messages, but we don't want that on
	the final UI so that we can have any Modulation Matrix mapping we want.
*/
tresult PLUGIN_API Processor::getMidiControllerAssignment(int32 busIndex, int16 channel, CtrlNumber midiControllerNumber, ParamID& id/*out*/)
{
	if(!m_pRAFXPlugIn) return kResultFalse;

	// NOTE: we only have one EventBus(0)
	//       but it has 16 channels on it
	if(busIndex == 0)
	{
		// v6.6 FIX
		id = -1;
		bool bFoundIt = false;
		switch(midiControllerNumber)
		{
			// these messages handled in the Processor::process() method
			case kPitchBend:
				id = MIDI_PITCHBEND;
				break;
			case kCtrlModWheel:
				id = MIDI_MODWHEEL;
				break;
			case kCtrlVolume:
				id = MIDI_VOLUME_CC7;
				break;
			case kCtrlPan:
				id = MIDI_PAN_CC10;
				break;
			case kCtrlExpression:
				id = MIDI_EXPRESSION_CC11;
				break;
			case kAfterTouch:
				id = MIDI_CHANNEL_PRESSURE;
				break;
			case kCtrlSustainOnOff:
				id = MIDI_SUSTAIN_PEDAL;
				break;
			case kCtrlAllNotesOff:
				id = MIDI_ALL_NOTES_OFF;
				break;
		}

		if(id == -1)
		{
			int nParams = m_pRAFXPlugIn->getControlCount();

			// iterate
			for(int i = 0; i < nParams; i++)
			{
				// they are in VST proper order in the ControlList - do NOT reference them with RackAFX ID values any more!
				CUICtrl* pUICtrl = m_pRAFXPlugIn->getUICtrlByListIndex(i);

				// for RAFX MIDI Control only
				if(pUICtrl)
				{
					if(pUICtrl->bMIDIControl &&
					   pUICtrl->uMIDIControlName == midiControllerNumber)
					{
						bFoundIt = true; // this is because ID = -1 is illegal
						id = i;
					}
				}
			}
		}
		else
			bFoundIt = true;

		// v6.6 FIX
		if(id == -1)
		{
			id = 0;
			return kResultFalse;
		}
		else
			return kResultTrue;
	}

	return kResultFalse;
}

/*
	Processor::createView()
	create our custom view here
*/
IPlugView* PLUGIN_API Processor::createView(const char* _name)
{
   // return NULL;

	if(!m_pRAFXPlugIn) return NULL;

	ConstString name(_name);
	if(name == ViewType::kEditor)
	{
		// --- get from RAFX
		if(!m_pRAFXPlugIn->m_bUseCustomVSTGUI)
			return NULL;

		// --- see if there is a pure custom view
		VSTGUI_VIEW_INFO info;
		info.message = GUI_HAS_USER_CUSTOM;
		info.bHasUserCustomView = false; // this flag will be set if they do
		m_pRAFXPlugIn->showGUI((void*)&info);
		if(info.bHasUserCustomView)
		{
			// CRafxCustomView creates the frame, populates with plugin view, then resizes frame during open( )
			ViewRect rect(0, 0, 100, 100);
			CRafxCustomView* pRafxCustomView = new CRafxCustomView(this, &rect);
			pRafxCustomView->setPlugIn(m_pRAFXPlugIn);
			return pRafxCustomView;
		}

		// --- create the editor using the RackAFX.uidesc file (in THIS project's \resources folder)
		/*
				This file was originally created and edited with GUI Designer in RackAFX.
		*/

#if defined _WINDOWS || defined _WINDLL
		VST3EditorWP* pVST3Editor = new VST3EditorWP(this, "Editor", "rafx.uidesc"); // For WIN uses old resource ID for back compatibility; "rafx.uidesc" is the ID name, NOT the filename
#else
		VST3EditorWP* pVST3Editor = new VST3EditorWP(this, "Editor", "RackAFX.uidesc"); // For MAC, uses actual file name (string) "RackAFX.uidesc"
#endif
		// --- NOTE: because of the way the VST2 wrapper creates, then forgets the editor when testing for existence
		//           you can not rely on this pointer being valid as we are never informed that our editor object was destroyed.
		if(pVST3Editor)
		{
			// --- v6.6 you can now control the knob mode; see RAFX GUIDesigner
			pVST3Editor->setKnobMode(m_pRAFXPlugIn->m_uPlugInEx[KNOB_MODE]);

			// --- this is only for calling the checkUpdateGUI() function, does not write any variable info
			pVST3Editor->setPlugIn(m_pRAFXPlugIn);

			return pVST3Editor;
		}

		// else - blank UI
		return new VST3Editor(this, "Editor", "vstgui.uidesc");
	}
	return 0;
}
void Processor::didOpen(VST3Editor* editor)		///< called after the editor was opened
{
	if(!m_pRAFXPlugIn) return;

	if(editor)
	{
		// --- fill in the struct
		guiInfoStruct.message = GUI_DID_OPEN;
		guiInfoStruct.customViewName = "";
		guiInfoStruct.subControllerName = "";
		guiInfoStruct.listener = NULL;
		guiInfoStruct.editor = NULL;
		guiInfoStruct.bHasUserCustomView = false;

		m_pRAFXPlugIn->showGUI((void*)(&guiInfoStruct));
	}
}

void Processor::willClose(VST3Editor* editor)	///< called before the editor will close
{
	if(!m_pRAFXPlugIn) return;

	// --- fill in the struct
	guiInfoStruct.message = GUI_WILL_CLOSE;
	guiInfoStruct.customViewName = "";
	guiInfoStruct.subControllerName = "";
	guiInfoStruct.editor = NULL;
	guiInfoStruct.listener = NULL;
	guiInfoStruct.bHasUserCustomView = false;

	m_pRAFXPlugIn->showGUI((void*)(&guiInfoStruct));
}


/*
	Processor::createSubController()
	create subcontrollers for customized objects
	see VSTGUI4.2 documentation
*/
#ifndef VSTGUI_43
IController* Processor::createSubController(UTF8StringPtr _name, IUIDescription* description, VST3Editor* editor)
#else
IController* Processor::createSubController (UTF8StringPtr name, const IUIDescription* description, VST3Editor* editor)
#endif
{
	if(!m_pRAFXPlugIn) return NULL;

#ifndef VSTGUI_43
	ConstString name (_name);
#endif
	string strName(name);

	if(strName == "VectorJoystick")
	{
		Parameter* jsX = getParameterObject(JOYSTICK_X_PARAM);
		Parameter* jsY = getParameterObject(JOYSTICK_Y_PARAM);
		PadController* padController = new PadController(editor, this, jsX, jsY);
		return padController;
	}

	int nJS = strName.find("Joystick_");
	if(nJS >= 0)
	{
		// --- decoding code
		int nX = strName.find("_X");
		int nY = strName.find("_Y");
		int len = strName.length();

		if(nX < 0 || nY < 0 || len < 0)
			return NULL;

		if(nX < nY && nY < len)
		{
			string sX = strName.substr(nX + 2, nY - 2 - nX);
			string sY = strName.substr(nY + 2, len - 2 - nY);
			int nParamX = atoi(sX.c_str());
			int nParamY = atoi(sY.c_str());
			Parameter* jsX = getParameterObject(nParamX);
			Parameter* jsY = getParameterObject(nParamY);
			CPadControllerWP* padController = new CPadControllerWP(editor, this, jsX, jsY);

			return padController;
		}
	}

	int nTP = strName.find("TrackPad_");
	if(nTP >= 0)
	{
		// --- decoding code
		int nX = strName.find("_X");
		int nY = strName.find("_Y");
		int len = strName.length();

		if(nX < 0 || nY < 0 || len < 0)
			return NULL;

		if(nX < nY && nY < len)
		{
			string sX = strName.substr(nX + 2, nY - 2 - nX);
			string sY = strName.substr(nY + 2, len - 2 - nY);
			int nParamX = atoi(sX.c_str());
			int nParamY = atoi(sY.c_str());
			Parameter* jsX = getParameterObject(nParamX);
			Parameter* jsY = getParameterObject(nParamY);
			CPadControllerWP* padController = new CPadControllerWP(editor, this, jsX, jsY);
			return padController;
		}
	}

	return NULL;
}

/*
	Processor::createCustomView()
	create custom views for customized objects
	see VSTGUI4.2 documentation
*/
#ifndef VSTGUI_43
CView* Processor::createCustomView(UTF8StringPtr name, const UIAttributes& attributes, IUIDescription* description, VST3Editor* editor)
#else
CView* Processor::createCustomView (UTF8StringPtr name, const UIAttributes& attributes, const IUIDescription* description, VST3Editor* editor)
#endif
{
	if(!m_pRAFXPlugIn) return NULL;

	ConstString viewname(name);
	if(viewname == "RafxKickButton" ||
		viewname == "RafxKickButtonDU" ||
		viewname == "RafxKickButtonU" ||
		viewname == "RafxKickButtonD")
	{
		const std::string* sizeString = attributes.getAttributeValue("size");
		const std::string* originString = attributes.getAttributeValue("origin");
		const std::string* bitmapString = attributes.getAttributeValue("bitmap");
		const std::string* tagString = attributes.getAttributeValue("control-tag");
		const std::string* offsetString = attributes.getAttributeValue("background-offset");

		// --- rect
		CPoint origin;
		CPoint size;
		parseSize(*sizeString, size);
		parseSize(*originString, origin);

		const CRect rect(origin, size);

		// --- tag
		int32_t tag = description->getTagForName(tagString->c_str());

		// --- listener "hears" the control
		const char* controlTagName = tagString->c_str();
		CControlListener* listener = description->getControlListener(controlTagName);

		// --- bitmap
		std::string BMString = *bitmapString;
		BMString += ".png";
		UTF8StringPtr bmp = BMString.c_str();
		CResourceDescription bmpRes(bmp);
		CBitmap* pBMP = new CBitmap(bmpRes);

		// --- offset
		CPoint offset;
		parseSize(*offsetString, offset);

		CKickButtonWP* p = new CKickButtonWP(rect, listener, tag, pBMP, offset);
		if(p)
		{
			if(viewname == "RafxKickButtonDU")
				p->setMouseMode(mouseUpAndDown);
			else if(viewname == "RafxKickButtonD")
				p->setMouseMode(mouseDown);
			else if(viewname == "RafxKickButtonU")
				p->setMouseMode(mouseUp);
			else
				p->setMouseMode(mouseUp); // old
		}

		return p;
	}

	// attributes - has XML atts
	// description - use IUIDescription to convert color strings to CColors or bitmap strings to CBitmap*s
	if(viewname == "TrackPad" || viewname == "Joystick")
	{
		const std::string* sizeString = attributes.getAttributeValue("size");
		const std::string* originString = attributes.getAttributeValue("origin");
		const std::string* backColorString = attributes.getAttributeValue("back-color"); // c0lor of background
		const std::string* frameColorString = attributes.getAttributeValue("frame-color"); // frame (may not use)
		const std::string* puckColorString = attributes.getAttributeValue("font-color");// color of puck
		const std::string* frameWidthString = attributes.getAttributeValue("frame-width");
		const std::string* rrrString = attributes.getAttributeValue("round-rect-radius");
		const std::string* styleRRRString = attributes.getAttributeValue("style-round-rect");
		const std::string* styleNoFrameString = attributes.getAttributeValue("style-no-frame");

		// --- rect
		CPoint origin;
		CPoint size;
		parseSize(*sizeString, size);
		parseSize(*originString, origin);

		const CRect rect(origin, size);

		// --- colors
		CColor backColor;
		description->getColor(backColorString->c_str(), backColor);

		CColor frameColor;
		description->getColor(frameColorString->c_str(), frameColor);

		CColor puckColor;
		description->getColor(puckColorString->c_str(), puckColor);

		// --- the pad
		CXYPadWP* p = new CXYPadWP(rect);
		if(viewname == "TrackPad")
			p->m_bIsJoystickPad = false;
		else
			p->m_bIsJoystickPad = true;

		p->setBackColor(backColor);
		p->setFrameColor(frameColor);
		p->setFontColor(puckColor);

		p->setFrameWidth(atoi(frameWidthString->c_str()));
		p->setRoundRectRadius(atoi(rrrString->c_str()));

		if(strcmp(styleRRRString->c_str(), "true") == 0)
			p->setStyle(p->getStyle() | kRoundRectStyle);
		else
			p->setStyle(p->getStyle() & ~kRoundRectStyle);

		if(strcmp(styleNoFrameString->c_str(), "true") == 0)
			p->setStyle(p->getStyle() | kNoFrame);
		else
			p->setStyle(p->getStyle() & ~kNoFrame);

		return p;
	}

	if(viewname == "UnitsEdit" && m_bAppendUnits)
	{
		const std::string* originString = attributes.getAttributeValue("origin");
		const std::string* sizeString = attributes.getAttributeValue("size");
		const std::string* bitmapString = attributes.getAttributeValue("bitmap");
		const std::string* titleString = attributes.getAttributeValue("title");
		const std::string* tagString = attributes.getAttributeValue("control-tag");

		// --- rect
		CPoint origin;
		CPoint size;
		parseSize(*sizeString, size);
		parseSize(*originString, origin);

		const CRect rect(origin, size);

		// --- bitmap
		CBitmap* pBMP = NULL;
		if(bitmapString && bitmapString->size() > 0)
		{
			std::string BMString = *bitmapString;
			BMString += ".png";
			UTF8StringPtr bmp = BMString.c_str();
			CResourceDescription bmpRes(bmp);
			pBMP = new CBitmap(bmpRes);
		}

		// --- listener "hears" the control
		const char* controlTagName = tagString->c_str();
		CControlListener* listener = description->getControlListener(controlTagName);

		// --- tag
		int32_t tag = description->getTagForName(tagString->c_str());

		CUICtrl* pUICtrl = m_pRAFXPlugIn->getUICtrlByListIndex(tag);
		if(!pUICtrl) return NULL;

		// --- CTextEditWP
		CTextEditWP* p = new CTextEditWP(rect, listener, tag, titleString->c_str(), pBMP);
		p->setUnitsString(pUICtrl->cUnits);

		return p;
	}

	if(viewname == "UnitsLabel" && m_bAppendUnits)
	{
		const std::string* originString = attributes.getAttributeValue("origin");
		const std::string* sizeString = attributes.getAttributeValue("size");
		const std::string* bitmapString = attributes.getAttributeValue("bitmap");
		const std::string* titleString = attributes.getAttributeValue("title");
		const std::string* tagString = attributes.getAttributeValue("control-tag");

		// --- rect
		CPoint origin;
		CPoint size;
		parseSize(*sizeString, size);
		parseSize(*originString, origin);

		const CRect rect(origin, size);

		// --- bitmap
		CBitmap* pBMP = NULL;
		if(bitmapString && bitmapString->size() > 0)
		{
			std::string BMString = *bitmapString;
			BMString += ".png";
			UTF8StringPtr bmp = BMString.c_str();
			CResourceDescription bmpRes(bmp);
			pBMP = new CBitmap(bmpRes);
		}

		// --- tag
		int32_t tag = description->getTagForName(tagString->c_str());

		CUICtrl* pUICtrl = m_pRAFXPlugIn->getUICtrlByListIndex(tag);
		if(!pUICtrl) return NULL;

		// --- CTextLabelWP
		CTextLabelWP* p = new CTextLabelWP(rect, titleString->c_str(), pBMP);
		p->setUnitsString(pUICtrl->cUnits);

		return p;
	}

	/* these were removed wih VSTGUI4.3, which now *does* implement the switchy controls properly
	   I'm keeping the code in case it ever reverts back to the old way in VSTGUI...
	if(viewname == "KnobSwitchView")
	{
		const std::string* sizeString = attributes.getAttributeValue("size");
		const std::string* offsetString = attributes.getAttributeValue("background-offset");
		const std::string* bitmapString = attributes.getAttributeValue("bitmap");
		const std::string* tagString = attributes.getAttributeValue("control-tag");
		const std::string* heightOneImageString = attributes.getAttributeValue("height-of-one-image");
		const std::string* subPixmapsString = attributes.getAttributeValue("sub-pixmaps");
		const std::string* originString = attributes.getAttributeValue("origin");

		// --- rect
		CPoint origin;
		CPoint size;
		parseSize(*sizeString, size);
		parseSize(*originString, origin);

		const CRect rect(origin, size);

		// --- listener "hears" the control
		const char* controlTagName = tagString->c_str();
		CControlListener* listener = description->getControlListener(controlTagName);

		// --- tag
		int32_t tag = description->getTagForName(tagString->c_str());

		// --- subPixmaps
		int32_t subPixmaps = strtol(subPixmapsString->c_str(), 0, 10);

		// --- height of one image
		CCoord heightOfOneImage = strtod(heightOneImageString->c_str(), 0);

		// --- bitmap
		std::string BMString = *bitmapString;
		BMString += ".png";
		UTF8StringPtr bmp = BMString.c_str();
		CResourceDescription bmpRes(bmp);
		CBitmap* pBMP = new CBitmap(bmpRes);

		// --- offset
		CPoint offset;
		parseSize(*offsetString, offset);
		const CPoint offsetPoint(offset);

		CUICtrl* pUICtrl = m_pRAFXPlugIn->getUICtrlByListIndex(tag);
		if(!pUICtrl) return NULL;

		// --- the knobswitch; more well behaved than the VSTGUI4 object IMO
		CKnobWP* p = new CKnobWP(rect, listener, tag, subPixmaps, heightOfOneImage, pBMP, offsetPoint, true); // true, IS swtchknob
		p->setSwitchMax(pUICtrl->fUserDisplayDataHiLimit);

		return p;
	}

	if(viewname == "SliderSwitchView")
	{
		const std::string* sizeString = attributes.getAttributeValue("size");
		const std::string* offsetString = attributes.getAttributeValue("handle-offset");
		const std::string* bitmapString = attributes.getAttributeValue("bitmap");
		const std::string* handleBitmapString = attributes.getAttributeValue("handle-bitmap");
		const std::string* tagString = attributes.getAttributeValue("control-tag");
		const std::string* originString = attributes.getAttributeValue("origin");
		const std::string* styleString = attributes.getAttributeValue("orientation");

		// --- rect
		CPoint origin;
		CPoint size;
		parseSize(*sizeString, size);
		parseSize(*originString, origin);

		const CRect rect(origin, size);

		// --- listener
		const char* controlTagName = tagString->c_str();
		CControlListener* listener = description->getControlListener(controlTagName);

		// --- tag
		int32_t tag = description->getTagForName(tagString->c_str());

		// --- bitmap
		std::string BMString = *bitmapString;
		BMString += ".png";
		UTF8StringPtr bmp = BMString.c_str();
		CResourceDescription bmpRes(bmp);
		CBitmap* pBMP_back = new CBitmap(bmpRes);

		std::string BMStringH = *handleBitmapString;
		BMStringH += ".png";
		UTF8StringPtr bmpH = BMStringH.c_str();
		CResourceDescription bmpResH(bmpH);
		CBitmap* pBMP_hand = new CBitmap(bmpResH);

		// --- offset
		CPoint offset;
		parseSize(*offsetString, offset);
		const CPoint offsetPoint(offset);

		// --- the knobswitch
		if(strcmp(styleString->c_str(), "vertical") == 0)
		{
			CUICtrl* pUICtrl = m_pRAFXPlugIn->getUICtrlByListIndex(tag);
			if(!pUICtrl) return NULL;

			CVerticalSliderWP* p = new CVerticalSliderWP(rect, listener, tag, 0, 1, pBMP_hand, pBMP_back, offsetPoint);
			p->setSwitchSlider(true);
			p->setSwitchMax(pUICtrl->fUserDisplayDataHiLimit);
			return p;
		}
		else
		{
			CUICtrl* pUICtrl = m_pRAFXPlugIn->getUICtrlByListIndex(tag);
			if(!pUICtrl) return NULL;

			CHorizontalSliderWP* p = new CHorizontalSliderWP(rect, listener, tag, 0, 1, pBMP_hand, pBMP_back, offsetPoint);
			p->setSwitchSlider(true);
			p->setSwitchMax(pUICtrl->fUserDisplayDataHiLimit);
			return p;
		}

		// return NULL;
	}*/

	string customView(viewname);
	string analogMeter("AnalogMeterView");
	string invAnalogMeter("InvertedAnalogMeterView");
	int nAnalogMeter = customView.find(analogMeter);
	int nInvertedAnalogMeter = customView.find(invAnalogMeter);

	if(nAnalogMeter >= 0)
	{
		const std::string* sizeString = attributes.getAttributeValue("size");
		const std::string* originString = attributes.getAttributeValue("origin");
		const std::string* ONbitmapString = attributes.getAttributeValue("bitmap");
		const std::string* OFFbitmapString = attributes.getAttributeValue("off-bitmap");
		const std::string* numLEDString = attributes.getAttributeValue("num-led");
		const std::string* tagString = attributes.getAttributeValue("control-tag");

		if(sizeString && originString && ONbitmapString && OFFbitmapString && numLEDString)
		{
			CPoint origin;
			CPoint size;
			parseSize(*sizeString, size);
			parseSize(*originString, origin);

			const CRect rect(origin, size);

			std::string onBMString = *ONbitmapString;
			onBMString += ".png";
			UTF8StringPtr onbmp = onBMString.c_str();
			CResourceDescription bmpRes(onbmp);
			CBitmap* onBMP = new CBitmap(bmpRes);

			std::string offBMString = *OFFbitmapString;
			offBMString += ".png";
			UTF8StringPtr offbmp = offBMString.c_str();
			CResourceDescription bmpRes2(offbmp);
			CBitmap* offBMP = new CBitmap(bmpRes2);

			int32_t nbLed = strtol(numLEDString->c_str(), 0, 10);

			CVuMeterWP* p = NULL;

			if(nInvertedAnalogMeter >= 0)
				p = new CVuMeterWP(rect, onBMP, offBMP, nbLed, true, true); // inverted, analog
			else
				p = new CVuMeterWP(rect, onBMP, offBMP, nbLed, false, true); // inverted, analog

			// --- decode our stashed variables
			// decode hieght one image and zero db frame
			int nX = customView.find("_H");
			int nY = customView.find("_Z");
			int len = customView.length();
			string sH = customView.substr(nX + 2, nY - 2 - nX);
			string sZ = customView.substr(nY + 2, len - 2 - nY);

			p->setHtOneImage(atof(sH.c_str()));
			p->setImageCount(atof(numLEDString->c_str()));
			p->setZero_dB_Frame(atof(sZ.c_str()));

			// --- connect meters/variables
			int nParams = m_pRAFXPlugIn->getControlCount();

			// iterate
			for(int i = 0; i < nParams; i++)
			{
				// they are in VST proper order in the ControlList - do NOT reference them with RackAFX ID values any more!
				CUICtrl* pUICtrl = m_pRAFXPlugIn->getUICtrlByListIndex(i);
				if(pUICtrl)
				{
					if(strcmp(pUICtrl->cControlName, tagString->c_str()) == 0 && pUICtrl->m_pCurrentMeterValue)
					{
						// --- identical to RAFX, meters should behave the same way
						float fSampleRate = 1.0/(METER_UPDATE_INTERVAL_MSEC*0.001);
						p->initDetector(fSampleRate, pUICtrl->fMeterAttack_ms,
										pUICtrl->fMeterRelease_ms, true,
										pUICtrl->uDetectorMode,
										pUICtrl->bLogMeter);
					}
				}
			}
			return p;
		}
	}

	if(viewname == "InvertedMeterView" || viewname == "MeterView")
	{
		const std::string* sizeString = attributes.getAttributeValue("size");
		const std::string* originString = attributes.getAttributeValue("origin");
		const std::string* ONbitmapString = attributes.getAttributeValue("bitmap");
		const std::string* OFFbitmapString = attributes.getAttributeValue("off-bitmap");
		const std::string* numLEDString = attributes.getAttributeValue("num-led");
		const std::string* tagString = attributes.getAttributeValue("control-tag");

		if(sizeString && originString && ONbitmapString && OFFbitmapString && numLEDString)
		{
			CPoint origin;
			CPoint size;
			parseSize(*sizeString, size);
			parseSize(*originString, origin);

			const CRect rect(origin, size);

			std::string onBMString = *ONbitmapString;
			onBMString += ".png";
			UTF8StringPtr onbmp = onBMString.c_str();
			CResourceDescription bmpRes(onbmp);
			CBitmap* onBMP = new CBitmap(bmpRes);

			std::string offBMString = *OFFbitmapString;
			offBMString += ".png";
			UTF8StringPtr offbmp = offBMString.c_str();
			CResourceDescription bmpRes2(offbmp);
			CBitmap* offBMP = new CBitmap(bmpRes2);

			int32_t nbLed = strtol(numLEDString->c_str(), 0, 10);

			bool bInverted = false;

			if(viewname == "InvertedMeterView")
				bInverted = true;

			CVuMeterWP* p = new CVuMeterWP(rect, onBMP, offBMP, nbLed, bInverted, false); // inverted, analog

			// --- connect meters/variables
			int nParams = m_pRAFXPlugIn->getControlCount();

			// iterate
			for(int i = 0; i < nParams; i++)
			{
				// they are in VST proper order in the ControlList - do NOT reference them with RackAFX ID values any more!
				CUICtrl* pUICtrl = m_pRAFXPlugIn->getUICtrlByListIndex(i);
				if(pUICtrl)
				{
					if(strcmp(pUICtrl->cControlName, tagString->c_str()) == 0)
					{
						// --- identical to RAFX, meters should behave the same way
						float fSampleRate = 1.0/(METER_UPDATE_INTERVAL_MSEC*0.001);
						p->initDetector(fSampleRate, pUICtrl->fMeterAttack_ms,
										pUICtrl->fMeterRelease_ms, true,
										pUICtrl->uDetectorMode,
										pUICtrl->bLogMeter);
					}
				}
			}
			return p;
		}
	}

	// --- try plugin
	if(strlen(name) > 0)
	{
		// --- try plugin
		guiInfoStruct.message = GUI_CUSTOMVIEW;
		guiInfoStruct.subControllerName = "";
		guiInfoStruct.listener = NULL;
		guiInfoStruct.window = NULL;
		guiInfoStruct.editor = NULL;
		guiInfoStruct.bHasUserCustomView = false;

		guiInfoStruct.hPlugInInstance = NULL;
		guiInfoStruct.hRAFXInstance = NULL;
		guiInfoStruct.size.width = 0; guiInfoStruct.size.height = 0;

		// --- custom view setup --- //
		guiInfoStruct.customViewName = (char*)name;

		guiInfoStruct.listener = NULL;

		const std::string* tagString = attributes.getAttributeValue("control-tag");

		// --- setup listener
		if(tagString)
		{
			if(tagString->size() > 0)
			{
				// --- set tag as int
				int32_t tag = description->getTagForName(tagString->c_str());
				guiInfoStruct.customViewTag = tag;

				CControlListener* listener = description->getControlListener(tagString->c_str());
				guiInfoStruct.listener = (void*)listener;
			}
		}

		// --- rect
		const std::string* sizeString = attributes.getAttributeValue("size");
		const std::string* originString = attributes.getAttributeValue("origin");

		// --- rect
		CPoint origin;
		CPoint size;
		if(sizeString)
			parseSize(*sizeString, size);
		if(originString)
			parseSize(*originString, origin);

		const CRect rect(origin, size);
		guiInfoStruct.customViewRect.top = rect.top;
		guiInfoStruct.customViewRect.bottom = rect.bottom;
		guiInfoStruct.customViewRect.left = rect.left;
		guiInfoStruct.customViewRect.right = rect.right;

		const std::string* offsetString = attributes.getAttributeValue("background-offset");
		CPoint offset;
		if(offsetString)
			parseSize(*offsetString, offset);

		const CPoint offsetPoint(offset);
		guiInfoStruct.customViewOffset.x = offsetPoint.x;
		guiInfoStruct.customViewOffset.y = offsetPoint.y;

		const std::string* bitmapString = attributes.getAttributeValue("bitmap");
		if(bitmapString && bitmapString->size() > 0)
		{
			std::string BMString = *bitmapString;
			BMString.append(".png");
			guiInfoStruct.customViewBitmapName = (char*)BMString.c_str();
		}
		else
			guiInfoStruct.customViewBitmapName = "";

		const std::string* bitmap2String = attributes.getAttributeValue("handle-bitmap");
		const std::string* bitmap3String = attributes.getAttributeValue("off-bitmap");
		guiInfoStruct.customViewHandleBitmapName = "";
		guiInfoStruct.customViewOffBitmapName = "";

		if(bitmap2String && bitmap2String->size() > 0)
		{
			std::string BMString = *bitmap2String;
			BMString.append(".png");
			guiInfoStruct.customViewHandleBitmapName = (char*)BMString.c_str();
		}
		else if(bitmap3String && bitmap3String->size() > 0)
		{
			std::string BMString = *bitmap3String;
			BMString.append(".png");
			guiInfoStruct.customViewOffBitmapName = (char*)BMString.c_str();
		}

		guiInfoStruct.customViewOrientation = "";
		const std::string* styleString = attributes.getAttributeValue("orientation");
		if(styleString)
			guiInfoStruct.customViewOrientation = (char*)styleString->c_str();

		guiInfoStruct.customViewBackColor = NULL;
		guiInfoStruct.customViewFrameColor = NULL;
		guiInfoStruct.customViewFontColor = NULL;

		const std::string* backColorString = attributes.getAttributeValue("back-color"); // c0lor of background
		if(backColorString && backColorString->size() > 0)
		{
			CColor backColor;
			description->getColor(backColorString->c_str(), backColor);
			guiInfoStruct.customViewBackColor = (void*)&backColor;
		}

		const std::string* frameColorString = attributes.getAttributeValue("frame-color"); // frame (may not use)
		if(frameColorString && frameColorString->size() > 0)
		{
			CColor frameColor;
			description->getColor(frameColorString->c_str(), frameColor);
			guiInfoStruct.customViewFrameColor = (void*)&frameColor;
		}

		const std::string* fontColorString = attributes.getAttributeValue("font-color");// color of puck
		if(fontColorString && fontColorString->size() > 0)
		{
			CColor fontColor;
			description->getColor(fontColorString->c_str(), fontColor);
			guiInfoStruct.customViewFontColor = (void*)&fontColor;
		}

		guiInfoStruct.customViewFrameWidth = 0;
		const std::string* frameWidthString = attributes.getAttributeValue("frame-width");
		if(frameWidthString && frameWidthString->size() > 0)
			guiInfoStruct.customViewFrameWidth = atoi(frameWidthString->c_str());


		guiInfoStruct.customViewRoundRectRadius = 0;
		const std::string* rrrString = attributes.getAttributeValue("round-rect-radius");
		if(rrrString && rrrString->size() > 0)
			guiInfoStruct.customViewRoundRectRadius = atoi(rrrString->c_str());

		guiInfoStruct.customViewStyleRoundRect = false;
		const std::string* styleRRRString = attributes.getAttributeValue("style-round-rect");
		if(styleRRRString && styleRRRString->size() > 0)
			if(strcmp(styleRRRString->c_str(), "true") == 0) guiInfoStruct.customViewStyleRoundRect = true;

		guiInfoStruct.customViewStyleNoFrame = false;
		const std::string* styleNoFrameString = attributes.getAttributeValue("style-no-frame");
		if(styleNoFrameString && styleNoFrameString->size() > 0)
			if(strcmp(styleNoFrameString->c_str(), "true") == 0) guiInfoStruct.customViewStyleNoFrame = true;

		guiInfoStruct.customViewHtOneImage = 0;
		const std::string* heightOneImageString = attributes.getAttributeValue("height-of-one-image");
		if(heightOneImageString && heightOneImageString->size() > 0)
			guiInfoStruct.customViewHtOneImage = atoi(heightOneImageString->c_str());

		guiInfoStruct.customViewSubPixmaps = 0;
		const std::string* subPixmapsString = attributes.getAttributeValue("sub-pixmaps");
		if(subPixmapsString && subPixmapsString->size() > 0)
			guiInfoStruct.customViewSubPixmaps = atoi(subPixmapsString->c_str());

		void* pluginview = m_pRAFXPlugIn->showGUI((void*)(&guiInfoStruct));
		if(pluginview)
			return (CView*)pluginview;

	}

	return NULL;
}

/* See Automation in the docs
	Non-linear Scaling
	If the DSP representation of a value does not scale in a linear way to the exported
	normalized representation (which can happen when a decibel scale is used for example),
	the edit controller must provide a conversion to a plain representation.
	This allows the host to move automation data (being in GUI representation)
	and keep the original value relations intact.
	(Steinberg::Vst::IEditController::normalizedParamToPlain / Steinberg::Vst::IEditController::plainParamToNormalized).

	*** NOTE ***
	We do not use these since our controls are linear or logscale-controlled.
	I am just leaving them here in case you need to implement them. See docs.
*/
ParamValue PLUGIN_API Processor::plainParamToNormalized(ParamID tag, ParamValue plainValue)
{
	return EditController::plainParamToNormalized(tag, plainValue);
}
ParamValue PLUGIN_API Processor::normalizedParamToPlain(ParamID tag, ParamValue valueNormalized)
{
	return EditController::normalizedParamToPlain(tag, valueNormalized);
}

/*
	Processor::setParamNormalizedFromFile()
	helper function for setComponentState()
*/
tresult PLUGIN_API Processor::setParamNormalizedFromFile(ParamID tag, ParamValue value)
{
	// --- get the parameter
	Parameter* pParam = SingleComponentEffect::getParameterObject(tag);

	// --- verify pointer
	if(!pParam) return kResultFalse;

	//  --- convert serialized value to normalized (raw)
	return SingleComponentEffect::setParamNormalized(tag, pParam->toNormalized(value));
}

/*
	Processor::terminate()
	the end - delete the plugin
*/
tresult PLUGIN_API Processor::terminate()
{
	// --- clear out the index values
	meters.clear();

	// --- sample accurate automation
	for (int i = 0; i < m_pRAFXPlugIn->getControlCount(); i++)
	{
		if (m_pParamUpdateQueueArray)
		{
			if (m_pParamUpdateQueueArray[i])
				delete m_pParamUpdateQueueArray[i];
		}
	}

	if(m_pParamUpdateQueueArray)
		delete[] m_pParamUpdateQueueArray;

	if (m_pMidiEventList)
		delete m_pMidiEventList;

	return SingleComponentEffect::terminate();
}

/*
	Processor::receiveText()
	for RackAFX sendUpdateGUI() support
	could also add more communication with VST3EditorWP here
*/
tresult Processor::receiveText(const char8* text)
{
	if(!m_pRAFXPlugIn) return kResultTrue;

	if(strcmp(text, "VSTGUITimerPing") == 0)
	{
		// --- timer ping -> PlugIn
		guiInfoStruct.message = GUI_TIMER_PING;
		m_pRAFXPlugIn->showGUI((void*)&guiInfoStruct);
	}
	if(strcmp(text, "RecreateView") == 0)
	{
		// --- fill in the struct
		guiInfoStruct.message = GUI_DID_OPEN;
		guiInfoStruct.customViewName = "";
		guiInfoStruct.subControllerName = "";
		guiInfoStruct.editor = NULL;
		guiInfoStruct.listener = NULL;
		guiInfoStruct.bHasUserCustomView = false;

		m_pRAFXPlugIn->showGUI((void*)(&guiInfoStruct));
	}

	return kResultTrue;
}

/*
	Processor::setComponentState()
	This is the serialization-read function so the GUI can
	be updated from a preset or startup.

	fileStream - the IBStream interface from the client
*/
tresult PLUGIN_API Processor::setComponentState(IBStream* fileStream)
{
	if(!m_pRAFXPlugIn) return kResultFalse;

	// --- make a streamer interface using the
	//     IBStream* fileStream; this is for PC so
	//     data is LittleEndian
	IBStreamer s(fileStream, kLittleEndian);

	// --- variables for reading
	uint64 version = 0;
	double dDoubleParam = 0;
	float fFloatParam = 0;

	// --- needed to convert to our UINT reads
	uint32 udata = 0;
	int32 data = 0;

	// --- read the version
	if(!s.readInt64u(version)) return kResultFalse;

	int nParams = m_pRAFXPlugIn->getControlCount();

	// iterate
	for(int i = 0; i < nParams; i++)
	{
		// they are in VST proper order in the ControlList - do NOT reference them with RackAFX ID values any more!
		CUICtrl* pUICtrl = m_pRAFXPlugIn->getUICtrlByListIndex(i);

		// iterate
		if(pUICtrl)
		{
			if(pUICtrl->uUserDataType == intData){
				if(!s.readInt32(data)) return kResultFalse; else setParamNormalizedFromFile(i, (ParamValue)data);}
			else if(pUICtrl->uUserDataType == floatData){
				if(!s.readFloat(fFloatParam)) return kResultFalse; else setParamNormalizedFromFile(i, (ParamValue)fFloatParam);}
			else if(pUICtrl->uUserDataType == doubleData){
				if(!s.readDouble(dDoubleParam)) return kResultFalse; else setParamNormalizedFromFile(i, dDoubleParam);}
			else if(pUICtrl->uUserDataType == UINTData){
				if(!s.readInt32u(udata)) return kResultFalse; else setParamNormalizedFromFile(i, (ParamValue)udata);}
		}
	}

	// --- add plugin side bypassing
	bool dummy = false;
	if(!s.readBool(dummy))
		return kResultFalse;
	else
		setParamNormalizedFromFile(PLUGIN_SIDE_BYPASS, dummy);

	// --- do next version...
	if (version >= 1)
	{
		// --- v1: adds vector joystick
		if(!s.readFloat(fFloatParam)) return kResultFalse; else setParamNormalizedFromFile(JOYSTICK_X_PARAM, (ParamValue)bipolarToUnipolar(fFloatParam));
		if(!s.readFloat(fFloatParam)) return kResultFalse; else setParamNormalizedFromFile(JOYSTICK_Y_PARAM, (ParamValue)bipolarToUnipolar(fFloatParam));
	}

	// --- bulk update check
	doBulkCheckUpdateGUI();

	return kResultTrue;
}

/*
	Processor::getEnumString()
	helper function for initializing parameters
*/
char* Processor::getEnumString(char* string, int index)
{
	int nLen = strlen(string);
	char* copyString = new char[nLen+1];

	vst_strncpy(copyString, string, strlen(string));

	for(int i=0; i<index+1; i++)
	{
		char * comma = ",";

		int j = strcspn (copyString,comma);

		if(i==index)
		{
			char* pType = new char[j+1];
			strncpy (pType, copyString, j);
			pType[j] = '\0';
			delete [] copyString;

			// special support for 2-state switches
			// (new in RAFX 5.4.14)
			if(strcmp(pType, "SWITCH_OFF") == 0)
			{
				delete [] pType;
				pType = new char[3+1];
				strncpy (pType, "OFF", 3);
				pType[3] = '\0';

				return pType;
			}
			else if(strcmp(pType, "SWITCH_ON") == 0)
			{
				delete [] pType;
				pType = new char[2+1];
				strncpy (pType, "ON", 2);
				pType[2] = '\0';

				return pType;
			}

			return pType;
		}
		else // remove it
		{
			char* pch = strchr(copyString,',');

			if(!pch)
			{
				delete [] copyString;
				return NULL;
			}

			int nLen = strlen(copyString);
			memcpy (copyString,copyString+j+1,nLen-j);
		}
	}

	delete [] copyString;
	return NULL;
}

/*
	Processor::setParamNormalized()
	This is overridden only for selecting a preset.
*/
tresult PLUGIN_API Processor::setParamNormalized(ParamID tag, ParamValue value)
{
	// --- base class call
	tresult res = SingleComponentEffect::setParamNormalized(tag, value);

	// --- for pure custom GUIs
	setPureCustomGUIParamNormalized(tag, value);

	// --- handle preset changes
	if (res == kResultOk && tag == kPresetParam)
	{
		int32 program = parameters.getParameter(tag)->toPlain(value);

		if(m_pRAFXPlugIn)
		{
			int nParams = m_pRAFXPlugIn->getControlCount();

			// iterate
			for(int i = 0; i < nParams; i++)
			{
				float fGUIWarpedVariable = 0.0;
				CUICtrl* pUICtrl = m_pRAFXPlugIn->getUICtrlByListIndex(i);
				if(pUICtrl)
				{
					// --- we store COOKED data in presets
					double dPreset = pUICtrl->dPresetData[program];

					// --- normalize, apply log/volt-octave if needed for GUI Warped Variable
					double dNormalizedValue = getNormalizedRackAFXVariable(dPreset, pUICtrl, fGUIWarpedVariable);

					// --- set it on plugin
					m_pRAFXPlugIn->setNormalizedParameter(i, dNormalizedValue); // this will call userIntefaceChange()

					// --- set it on GUI
					SingleComponentEffect::setParamNormalized(i, fGUIWarpedVariable);

					// --- for pure custom GUIs
					setPureCustomGUIParamNormalized(i, fGUIWarpedVariable);
				}
			}

			// --- now do Vector Joystick X:
			double fJS_X = m_pRAFXPlugIn->m_JS_XCtrl.dPresetData[program];
			double fJS_Y = m_pRAFXPlugIn->m_JS_YCtrl.dPresetData[program];

			RotatePoint45CCW(fJS_X, fJS_Y);
			fJS_X /= 1.414;
			fJS_Y /= 1.414;

			// --- set on plugin
			m_pRAFXPlugIn->setVectorJSXValue(fJS_X);

			// --- set it on GUI
			SingleComponentEffect::setParamNormalized(JOYSTICK_X_PARAM, bipolarToUnipolar(fJS_X));

			// --- for pure custom GUIs
			setPureCustomGUIParamNormalized(JOYSTICK_X_PARAM, bipolarToUnipolar(fJS_X));

			// --- set on plugin
			m_pRAFXPlugIn->setVectorJSYValue(fJS_Y);

			// --- set it on GUI
			SingleComponentEffect::setParamNormalized(JOYSTICK_Y_PARAM, bipolarToUnipolar(fJS_Y));

			// --- for pure custom GUIs
			setPureCustomGUIParamNormalized(JOYSTICK_Y_PARAM, bipolarToUnipolar(fJS_Y));

			// --- restart
			componentHandler->restartComponent(kParamValuesChanged);

			// --- bulk update check
			doBulkCheckUpdateGUI();
		}
	}
	return res;
}

/*
	Processor::addProgramList()
	Adds a program list (aka preset list)
	For future compatibility: currently unused (presets only)
*/
bool Processor::addProgramList(ProgramList* list)
{
	programIndexMap[list->getID()] = programLists.size();
	programLists.push_back(IPtr<ProgramList>(list, false));
	list->addDependent(this);
	return true;
}

/*
	Processor::getProgramList()
	Selects a program list (aka preset list)
	For future compatibility: currently unused (presets only)
*/
ProgramList* Processor::getProgramList(ProgramListID listId) const
{
	ProgramIndexMap::const_iterator it = programIndexMap.find(listId);
	return it == programIndexMap.end() ? 0 : programLists[it->second];
}

/*
	Processor::notifyPogramListChange()
	If list changes; should not be called as we only have one program list
	For future compatibility: currently unused (presets only)
*/
tresult Processor::notifyPogramListChange (ProgramListID listId, int32 programIndex)
{
	tresult result = kResultFalse;
	FUnknownPtr<IUnitHandler> unitHandler(componentHandler);
	if (unitHandler)
		result = unitHandler->notifyProgramListChange (listId, programIndex);
	return result;
}

/*
	Processor::getProgramListCount()
	We have one list for our one set of presets
*/
int32 PLUGIN_API Processor::getProgramListCount ()
{
	if (parameters.getParameter(kPresetParam))
		return 1;
	return 0;
}

/*
	Processor::getProgramListInfo()
	Get information about our preset list.
*/
tresult PLUGIN_API Processor::getProgramListInfo(int32 listIndex, ProgramListInfo& info /*out*/)
{
	Parameter* param = parameters.getParameter(kPresetParam);
	if(param && listIndex == 0)
	{
		info.id = kPresetParam;
		info.programCount = (int32)param->toPlain (1) + 1;
		UString name (info.name, 128);
		name.fromAscii("Presets");
		return kResultTrue;
	}
	return kResultFalse;

}

/*
	Processor::getProgramName()
	Get preset name
*/
tresult PLUGIN_API Processor::getProgramName(ProgramListID listId, int32 programIndex, String128 name /*out*/)
{
	if(listId == kPresetParam)
	{
		Parameter* param = parameters.getParameter(kPresetParam);
		if (param)
		{
			ParamValue normalized = param->toNormalized (programIndex);
			param->toString (normalized, name);
			return kResultTrue;
		}
	}
	return kResultFalse;
}

/*
	Processor::setProgramName()
	Set preset name
*/
tresult Processor::setProgramName(ProgramListID listId, int32 programIndex, const String128 name /*in*/)
{
	ProgramIndexMap::const_iterator it = programIndexMap.find(listId);
	if (it != programIndexMap.end())
	{
		return programLists[it->second]->setProgramName(programIndex, name);
	}
	return kResultFalse;

}

/*
	Processor::getProgramInfo()
	Only used for presets.
*/
tresult PLUGIN_API Processor::getProgramInfo(ProgramListID listId, int32 programIndex, CString attributeId /*in*/, String128 attributeValue /*out*/)
{
	ProgramIndexMap::const_iterator it = programIndexMap.find(listId);
	if (it != programIndexMap.end())
	{
		return programLists[it->second]->getProgramInfo(programIndex, attributeId, attributeValue);
	}
	return kResultFalse;

}

/*
	Processor::doBulkCheckUpdateGUI()
	Used for threadsafe version of old sendUpdateGUI() from a preset load
*/
void Processor::doBulkCheckUpdateGUI()
{
	if (!m_pRAFXPlugIn) return;

	int nParams = m_pRAFXPlugIn->getControlCount();

	// iterate
	for (int i = 0; i < nParams; i++)
	{
		Steinberg::Vst::ParamValue value = getParamNormalized(i);
		Steinberg::Vst::ParamValue actualValue = normalizedParamToPlain(i, value);
		CUICtrl* pUICtrl = m_pRAFXPlugIn->getUICtrlByListIndex(i);
		if (pUICtrl)
		{
			checkSendUpdateGUI(pUICtrl->uControlId, actualValue, true);
		}
	}
}

/*
	Processor::checkSendUpdateGUI()
	Used for threadsafe version of old sendUpdateGUI() from a preset load
*/
void Processor::checkSendUpdateGUI(int nControlId, float fValue, bool bLoadingPreset)
{
    if(!m_pRAFXPlugIn) return;
    CLinkedList<GUI_PARAMETER> updateGUIParameters;

    // --- ask plugin for updates
    if(m_pRAFXPlugIn->checkUpdateGUI(nControlId, fValue, updateGUIParameters, bLoadingPreset))
    {
        for(int i = 0; i < updateGUIParameters.count(); i++)
        {
            GUI_PARAMETER* pParam = updateGUIParameters.getAt(i);
            if(pParam)
            {
				int32_t tag = getGUI_UICtrlIndexByControlID(pParam->uControlId);
				if(tag >= 0)
				{
					Steinberg::Vst::ParamValue normalizedValue = plainParamToNormalized(tag, pParam->fActualValue);
					setParamNormalized(tag, normalizedValue); // updates GUI
					performEdit(tag, normalizedValue); // changes variable
				}
			}
        }

        // --- plugin needs to clean because it allocted from its address space
        m_pRAFXPlugIn->clearUpdateGUIParameters(updateGUIParameters);
    }
}

/*
	Processor::setPureCustomGUIParamNormalized()
	special support for pure custom GUIs
*/
void Processor::setPureCustomGUIParamNormalized(ParamID tag, ParamValue value)
{
	// --- for pure Custom GUIs only
	if(m_pRAFXPlugIn)
	{
		// --- fill in the struct
		VSTGUI_VIEW_INFO guiInfoStruct = {0};
		guiInfoStruct.message = GUI_EXTERNAL_SET_NORM_VALUE;
		guiInfoStruct.normalizedParameterValue = value;
		guiInfoStruct.parameterTag = tag;
		m_pRAFXPlugIn->showGUI((void*)(&guiInfoStruct));
	}
}
CVSTMidiEventList::CVSTMidiEventList(CPlugIn* pPlugIn)
{
	m_pPlugIn = pPlugIn;
	inputEvents = NULL;
	currentEventIndex = 0;
}

// --- IMidiEventList
UINT CVSTMidiEventList::getEventCount()
{
	if (inputEvents)
		return inputEvents->getEventCount();

	return 0;
}

bool CVSTMidiEventList::fireMidiEvent(unsigned int uSampleOffset)
{
	Event e = { 0 };
	bool eventOccurred = false;
	bool haveEvents = false;
	if (inputEvents->getEvent(currentEventIndex, e) == kResultTrue)
		haveEvents = true;
	else
		return false;

	while (haveEvents)
	{
		if (inputEvents->getEvent(currentEventIndex, e) == kResultTrue)
		{
			if (e.sampleOffset != uSampleOffset)
				return false;

			// --- process Note On or Note Off messages
			switch (e.type)
			{
				// --- NOTE ON
			case Event::kNoteOnEvent:
			{
				// --- get the channel/note/vel
				UINT uMIDIChannel = (UINT)e.noteOn.channel;
				UINT uMIDINote = (UINT)e.noteOn.pitch;
				UINT uMIDIVelocity = (UINT)(127.0*e.noteOn.velocity);
				eventOccurred = true;

				if (m_pPlugIn)
				{
					m_pPlugIn->midiNoteOn(uMIDIChannel, uMIDINote, uMIDIVelocity);
				}
				break;
			}

			// --- NOTE OFF
			case Event::kNoteOffEvent:
			{
				// --- get the channel/note/vel
				UINT uMIDIChannel = (UINT)e.noteOff.channel;
				UINT uMIDINote = (UINT)e.noteOff.pitch;
				UINT uMIDIVelocity = (UINT)(127.0*e.noteOff.velocity);
				eventOccurred = true;

				if (m_pPlugIn)
				{
					m_pPlugIn->midiNoteOff(uMIDIChannel, uMIDINote, uMIDIVelocity, false);
				}
				break;
			}

			// --- polyphonic aftertouch 0xAn
			case Event::kPolyPressureEvent:
			{
				// --- get the channel
				UINT uMIDIChannel = (UINT)e.polyPressure.channel;
				UINT uMIDINote = (UINT)e.polyPressure.pitch;
				UINT uMIDIPressure = (UINT)(127.0*e.polyPressure.pressure);
				eventOccurred = true;

				if (m_pPlugIn)
				{
					m_pPlugIn->midiMessage(uMIDIChannel, POLY_PRESSURE, uMIDINote, uMIDIPressure);
				}

				break;
			}
			} // switch

			  // --- have next event?
			if (inputEvents->getEvent(currentEventIndex + 1, e) == kResultTrue)
			{
				if (e.sampleOffset == uSampleOffset)
				{
					// --- avance current index
					currentEventIndex++;
				}
				else
					haveEvents = false;
			}
			else
				haveEvents = false;
		}
	}

	return eventOccurred;
}

CVSTParamUpdateQueue::CVSTParamUpdateQueue(void)
{
	m_nQueueIndex = 0;
	m_nQueueSize = 0;
	m_nBufferSize = 0;
	m_pParameterQueue = NULL;
	m_pSampleAccuracy = NULL;
	x1 = 0.0;
	y1 = 0.0;
	x2 = 0.0;
	y2 = 0.0;
	m_dSlope = 1.0;
	m_bDirtyBit = false;
}

void CVSTParamUpdateQueue::initialize(ParamValue dInitialValue, ParamValue dMinValue,
	ParamValue dMaxValue, int* pSampleAccuracy)
{
	m_nBufferSize = 0;
	m_dInitialValue = 0.0;
	m_dPreviousValue = 0.0;
	m_dMinValue = dMinValue;
	m_dMaxValue = dMaxValue;
	m_pSampleAccuracy = pSampleAccuracy;
}

void CVSTParamUpdateQueue::setParamValueQueue(IParamValueQueue* pParamValueQueue, int nBufferSize)
{
	m_nBufferSize = nBufferSize;
	m_pParameterQueue = pParamValueQueue;
	m_nQueueSize = m_pParameterQueue->getPointCount();
	m_nQueueIndex = 0;
	m_lSampleOffset = 0;
}

void CVSTParamUpdateQueue::setSlope()
{
	int32 sampleOffset = 0;
	ParamValue value = 0.0;

	if (m_pParameterQueue->getPoint(m_nQueueIndex, sampleOffset, value) == Steinberg::kResultTrue)
	{
		if (m_nQueueIndex == 0 && sampleOffset == 0)
		{
			x1 = sampleOffset;
			y1 = value;

			if (m_pParameterQueue->getPointCount()<2)
			{
				x2 = m_nBufferSize;
				y2 = value;
			}
			else
			{
				m_nQueueIndex++;
				int32 sampleOffset2 = 0.0;
				ParamValue value2 = 0.0;
				if (m_pParameterQueue->getPoint(m_nQueueIndex, sampleOffset2, value2) == Steinberg::kResultTrue)
				{
					x2 = sampleOffset2;
					y2 = value2;
				}
			}
		}
		else
		{
			if (m_bDirtyBit == true)
			{
				x1 = 0;

				m_bDirtyBit = false;
			}
			else
			{
				x1 = x2;

			}
			y1 = y2;
			x2 = sampleOffset;
			y2 = value;
		}
		if (x2 == m_nBufferSize - 1)
		{
			m_bDirtyBit = true;
		}
		m_dSlope = (y2 - y1) / (float)(x2 - x1);
		m_dIntercept = y1 - (m_dSlope * x1);

		m_nQueueIndex++;
	}
}

ParamValue CVSTParamUpdateQueue::interpolate(int x1, int x2, ParamValue y1, ParamValue y2, int x)
{
	if (x == x1)
		return y1;
	if (x == x2)
		return y2;

	return (m_dSlope * x) + m_dIntercept;
}

int CVSTParamUpdateQueue::needsUpdate(int x, ParamValue &value)
{
	int nSampleGranularity = *m_pSampleAccuracy;

	ParamValue  newValue = interpolate(x1, x2, y1, y2, x);

	if (x == 0 || x == x2)
		setSlope();
	//return 0 if slope is 0
	if (m_dSlope == 0.0)
		return 0;

	if (nSampleGranularity == 0)
	{
		if (x != x1&&x != x2)
			return 0;
	}
	else if (nSampleGranularity != 0)
	{
		if (x%nSampleGranularity != 0 && x != x1&&x != x2)
			return 0;
	}

	if (newValue == m_dPreviousValue)
		return 2;
	else
	{
		value = newValue;
		m_dPreviousValue = newValue;
		return 1;
	}
}

UINT CVSTParamUpdateQueue::getParameterIndex()
{
	return m_pParameterQueue->getParameterId();
}

bool CVSTParamUpdateQueue::getValueAtOffset(long int lSampleOffset, double dPreviousValue, double &dNextValue)
{
	m_lSampleOffset = lSampleOffset;
	ParamValue dValue = 0;
	int nRC = needsUpdate(lSampleOffset, dValue);
	if (nRC == 1)
	{
		dNextValue = dValue;
		return true;
	}

	return false;
}

bool CVSTParamUpdateQueue::getNextValue(double &dNextValue)
{
	ParamValue dValue = 0;
	int nRC = needsUpdate(m_lSampleOffset++, dValue);
	if (nRC == 1)
	{
		dNextValue = dValue;
		return true;
	}

	return false;
}

// --- custom view object for plugins that support it
CRafxCustomView::CRafxCustomView(void* controller, ViewRect* size)
: VSTGUIEditor(controller, size)
{

}
CRafxCustomView::~CRafxCustomView ()
{

}

// --- create the frame and sync the GUI params
bool PLUGIN_API CRafxCustomView::open(void* parent, const PlatformType& platformType)
{
	// --- create the frame
	frame = new CFrame(CRect (0, 0, 0, 0), this);
	frame->setTransparency(true);
	frame->enableTooltips(true);

	// --- populate the window
	if(m_pPlugIn)
	{
		systemWindow = parent;

		VSTGUI_VIEW_INFO info = {0};
		info.message = GUI_USER_CUSTOM_OPEN;
		info.window = (void*)parent;
		info.editor = (void*)(VSTGUIEditor*)this;
		info.pGUISynchronizer = (void*)(CVSTParameterConnector*)this;
		info.hPlugInInstance = moduleHandle;
		info.hRAFXInstance = NULL;
		info.size.width = 0;
		info.size.height = 0;
		info.vstPlatformType = platformType;

		// --- query custom view
		void* hasCustomGUI = m_pPlugIn->showGUI((void*)&info);
		if(hasCustomGUI)
		{
			// --- sync all params, similar to AAX
			syncGUIToParams();

			// --- resize
			frame->setSize(info.size.width, info.size.height);
			return true;
		}
		else
			return false;
	}
	// --- should never happen
	return false;
}

/** Called when the editor will be closed. */
void PLUGIN_API CRafxCustomView::close()
{
	if(m_pPlugIn)
	{
		VSTGUI_VIEW_INFO info = {0};
		info.message = GUI_USER_CUSTOM_CLOSE;
		info.window = (void*)systemWindow;
		info.editor = NULL;
		info.hPlugInInstance = moduleHandle;
		info.hRAFXInstance = NULL;
		info.size.width = 0;
		info.size.height = 0;

		m_pPlugIn->showGUI((void*)&info);

		systemWindow = 0;
	}
	if(frame)
	{
		int32_t refCount = frame->getNbReference();
		if(refCount == 1)
		{
			frame->close();
			frame = 0;
		}
		else
		{
			frame->forget();
		}
	}
}

// --- query to see if we can change size, we can
bool CRafxCustomView::beforeSizeChange (const CRect& newSize, const CRect& oldSize)
{
	// --- just say yes
	return true;
}

// --- initial sync of GUI params
void CRafxCustomView::syncGUIToParams()
{
	if(!m_pPlugIn) return;

	// --- sync all, this mimics AAX a bit
	for(int i=0; i<getController()->getParameterCount(); i++)
	{
		VSTGUI_VIEW_INFO guiInfoStruct = {0};
		// --- fill in the struct
		guiInfoStruct.message = GUI_EXTERNAL_SET_NORM_VALUE;
		guiInfoStruct.parameterTag = i;
		guiInfoStruct.normalizedParameterValue = getController()->getParamNormalized(i);
		m_pPlugIn->showGUI((void*)(&guiInfoStruct));
	}
}

// --- these maintain the internal params and keep the custom and default GUIs in sync
void CRafxCustomView::beginValueChange(int nTag)
{
	getController()->beginEdit(nTag);
}
void CRafxCustomView::endValueChange(int nTag)
{
	getController()->endEdit(nTag);
}

// --- called by GUI to sync to our parameters and our default GUI
void CRafxCustomView::guiControlValueChanged(int nTag, float fNormalizedValue)
{
	// --- set parameter on object
	getController()->setParamNormalized(nTag, fNormalizedValue);

	// --- set GUI control location
	getController()->performEdit(nTag, fNormalizedValue);
}

}}} // namespaces







