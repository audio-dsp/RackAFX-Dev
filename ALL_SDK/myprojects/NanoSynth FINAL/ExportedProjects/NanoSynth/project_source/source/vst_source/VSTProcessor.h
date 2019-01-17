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

#ifndef __rackafx_2_vst3_processor__
#define __rackafx_2_vst3_processor__

#include "public.sdk/source/vst/vstsinglecomponenteffect.h"

// NOTE: the wrapper include here MUST:
//			be AFTER the #include vstsinglecomponenteffect AND
//		    PRECEDE any #include that refernces vsteditcontroller,
//          which is #include "vstgui/plugin-bindings/vst3editor.h" below!
#include "public.sdk/source/vst/vst2wrapper/vst2wrapper.h"

// --- MIDI EVENTS
#include "pluginterfaces/vst/ivstevents.h"

// --- WString Support
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

// --- VST3EditorDelegate
#include "vstgui/plugin-bindings/vst3editor.h"

// --- RackAFX Specific stuff
#include "synthfunctions.h"
#include "plugin.h"
#include "KnobWP.h"
#include "vstgui4constants.h"
#include <vector>

const UINT LCD_VISIBLE			= 25;
const UINT JS_VISIBLE			= 28;
const UINT ASSIGNBUTTON_1		= 32768;
const UINT ASSIGNBUTTON_2		= 32769;
const UINT ASSIGNBUTTON_3		= 32770;
const UINT ALPHA_WHEEL			= 32771;
const UINT LCD_KNOB				= 32772;
const UINT JOYSTICK_X_PARAM		= 32773;
const UINT JOYSTICK_Y_PARAM		= 32774;
const UINT JOYSTICK				= 32775; // used in RAFX only!
const UINT TRACKPAD 			= 32776;
const UINT LCD_TITLE 			= 32777;
const UINT LCD_COUNT 			= 32778;

// --- v6.6 custom
const UINT RAFX_VERSION = 0;
const UINT VSTGUI_VERSION = 1;
const UINT KNOB_MODE = 2;
const UINT LATENCY_IN_SAMPLES = 0;
const UINT TAILTIME_IN_MSEC = 1;
// --- for pCtrl->uControlTheme
const UINT GUI_SIG_DIGITS = 16;

#define MAX_VOICES 16
#define OUTPUT_CHANNELS 2 // stereo only!
#define INPUT_CHANNELS 2 // stereo only!

namespace Steinberg {
namespace Vst {
namespace Sock2VST3 {
/*
	The Processor object here ALSO contains the Edit Controller component since these
	are combined as SingleComponentEffect; see documentation
*/
class VST3EditorWP;
class CRafxCustomView;
class CVSTParamUpdateQueue;
class CVSTMidiEventList;

class Processor : public SingleComponentEffect, public IMidiMapping, public VST3EditorDelegate
{
public:
	// --- constructor
	Processor();

	// --- destructor
	~Processor();

	/*** IAudioProcessor Interface ***/
	// --- One time init to define our I/O and vsteditcontroller parameters
	tresult PLUGIN_API initialize(FUnknown* context) override;

	// --- Define the audio I/O we support
	tresult PLUGIN_API setBusArrangements(SpeakerArrangement* inputs, int32 numIns, SpeakerArrangement* outputs, int32 numOuts) override;

	// --- Define our word-length capabilities (currently 32 bit only)
	tresult PLUGIN_API canProcessSampleSize(int32 symbolicSampleSize) override;

	// --- you can access info about the processing via ProcessSetup; see ivstaudioprocessor.h
	tresult PLUGIN_API setupProcessing(ProcessSetup& newSetup) override;

	// --- Turn on/off; this is equivalent to prepareForPlay() in RAFX
	tresult PLUGIN_API setActive(TBool state) override;

	// --- Serialization: Save and load presets from a file stream
	//					  These get/set the RackAFX variables
	tresult PLUGIN_API setState(IBStream* fileStream) override;
	tresult PLUGIN_API getState(IBStream* fileStream) override;

	// --- functions to reduce size of process()
	//     Update the GUI control variables
	bool doControlUpdate(ProcessData& data);

	// --- for MIDI note-on/off, aftertouch
	bool doProcessEvent(Event& vstEvent);

	// --- The all important process method where the audio is rendered/effected
	tresult PLUGIN_API process(ProcessData& data) override;


	/*** EditController Functions ***/
	//
	// --- IMidiMapping
	virtual tresult PLUGIN_API getMidiControllerAssignment(int32 busIndex, int16 channel, CtrlNumber midiControllerNumber, ParamID& id/*out*/) override;

	// --- IPlugView: create our custom GUI
	IPlugView* PLUGIN_API createView(const char* _name) override;

	// --- VST3EditorDelegate
#ifndef VSTGUI_43
	IController* createSubController(UTF8StringPtr name, IUIDescription* description, VST3Editor* editor) override;
	virtual CView* createCustomView (UTF8StringPtr name, const UIAttributes& attributes, IUIDescription* description, VST3Editor* editor);
#else
	IController* createSubController (UTF8StringPtr name, const IUIDescription* description, VST3Editor* editor) override;
	virtual CView* createCustomView (UTF8StringPtr name, const UIAttributes& attributes, const IUIDescription* description, VST3Editor* editor) override;
#endif
	virtual void didOpen(VST3Editor* editor) override;		///< called after the editor was opened
	virtual void willClose(VST3Editor* editor) override;	///< called before the editor will close

	// --- oridinarily not needed; see documentation on Automation for using these
	virtual ParamValue PLUGIN_API normalizedParamToPlain(ParamID id, ParamValue valueNormalized) override;
	virtual ParamValue PLUGIN_API plainParamToNormalized(ParamID id, ParamValue plainValue) override;

	// --- end. this destroys the RackAFX core
	tresult PLUGIN_API terminate() override;

	// --- for GUI_TIMER_PING and recreate view operations
	//void updatePluginParams();
	virtual tresult receiveText(const char8* text) override;

	// --- helper function for serialization
	tresult PLUGIN_API setParamNormalizedFromFile(ParamID tag, ParamValue value);

	// --- serialize-read from file to setup the GUI parameters
	tresult PLUGIN_API setComponentState(IBStream* fileStream) override;

	// --- for RAFX Wrapper
	char* getEnumString(char* string, int index);

	// --- for meters
	void updateMeters(ProcessData& data, bool bForceOff = false);

	// --- our COM creation method
	static FUnknown* createInstance(void* context) {return (IAudioProcessor*)new Processor(); }

	// --- our Globally Unique ID
	static FUID cid;

	// --- for future compat; not curently supporting program lists; only have/need Factory Presets!
	bool addProgramList (ProgramList* list);
	ProgramList* getProgramList(ProgramListID listId) const;
	tresult notifyPogramListChange(ProgramListID listId, int32 programIndex = kAllProgramInvalid);

	// --- receives program changes
	tresult PLUGIN_API setParamNormalized (ParamID tag, ParamValue value) override;

	virtual int32 PLUGIN_API getProgramListCount() override;
	virtual tresult PLUGIN_API getProgramListInfo(int32 listIndex, ProgramListInfo& info /*out*/) override;
	virtual tresult PLUGIN_API getProgramInfo(ProgramListID listId, int32 programIndex, CString attributeId /*in*/, String128 attributeValue /*out*/) override;
	virtual tresult setProgramName(ProgramListID listId, int32 programIndex, const String128 name /*in*/) override;
	virtual tresult PLUGIN_API getProgramName(ProgramListID listId, int32 programIndex, String128 name /*out*/) override;

	// --- latency support
	uint32 m_uLatencyInSamples; // set in constructor with plugin
	virtual uint32 PLUGIN_API getLatencySamples() override {
		return m_uLatencyInSamples; }

	// --- tail time
	virtual uint32 PLUGIN_API getTailSamples() override;

	// --- define the IMidiMapping interface
	OBJ_METHODS(Processor, SingleComponentEffect)
	DEFINE_INTERFACES
		DEF_INTERFACE(IMidiMapping)
	END_DEFINE_INTERFACES(SingleComponentEffect)
	REFCOUNT_METHODS(SingleComponentEffect)

private:
	CPlugIn* m_pRAFXPlugIn;
	double m_dJoystickX;
	double m_dJoystickY;
	bool m_bPlugInSideBypass;
	std::vector<int> meters;
	bool m_bHasSidechain;
	bool m_bAppendUnits; // flag for disabling automatic appending of units to labels and edit boxes, by default it is TRUE

protected:
	// --- sample accurate parameter automation
	CVSTParamUpdateQueue ** m_pParamUpdateQueueArray;
	int m_nSampleAccuracy;
	bool m_bEnableSAAVST3;

	// --- sample accurate MIDI event list
	CVSTMidiEventList* m_pMidiEventList;
    bool processEventsForInterval(IEventList* inputEvents, int sampleInterval, int& startIndex);

	// --- IUnitInfo
	//TArray<IPtr<Unit> > units;
	//TDictionary<ProgramListID, IPtr<ProgramList> > programLists; // for future compat; not curently supporting program lists; only have/need Factory Presets!

	typedef std::vector<IPtr<ProgramList> > ProgramListVector;
	typedef std::map<ProgramListID, ProgramListVector::size_type> ProgramIndexMap;
	typedef std::vector<IPtr<Unit> > UnitVector;
	UnitVector units;
	ProgramListVector programLists;
	ProgramIndexMap programIndexMap;
//	UnitID selectedUnit;

	UnitID selectedUnit;
	void doBulkCheckUpdateGUI();
	void checkSendUpdateGUI(int nControlId, float fValue, bool bLoadingPreset);
	void setPureCustomGUIParamNormalized(ParamID tag, ParamValue value);

	inline int getGUI_UICtrlIndexByControlID(UINT uID)
    {
		if(!m_pRAFXPlugIn) return -1;

		int nCount = m_pRAFXPlugIn->getControlCount();
        for(int i=0; i<nCount; i++)
        {
			CUICtrl* p = m_pRAFXPlugIn->getUICtrlByListIndex(i);
            if(p->uControlId == uID)
                return i;
        }
        return -1;
    }

	// --- for new RAFX GUI Customization API
	VSTGUI_VIEW_INFO guiInfoStruct;

	// --- v6.6 for log/exp controls
	static inline float fastpow2 (float p)
	{
	  float offset = (p < 0) ? 1.0f : 0.0f;
	  float clipp = (p < -126) ? -126.0f : p;
	  int w = clipp;
	  float z = clipp - w + offset;
	  union { unsigned int i; float f; } v = { static_cast<unsigned int> ( (1 << 23) * (clipp + 121.2740575f + 27.7280233f / (4.84252568f - z) - 1.49012907f * z) ) };

	  return v.f;
	}

	static inline float fastlog2 (float x)
	{
	  union { float f; unsigned int i; } vx = { x };
	  union { unsigned int i; float f; } mx = { (vx.i & 0x007FFFFF) | 0x3f000000 };
	  float y = vx.i;
	  y *= 1.1920928955078125e-7f;

	  return y - 124.22551499f
			   - 1.498030302f * mx.f
			   - 1.72587999f / (0.3520887068f + mx.f);
	}

	// fNormalizedParam = 0->1
	// returns log scaled version 0->1
	inline float calcLogParameter(float fNormalizedParam)
	{
		return (pow(10.f, fNormalizedParam) - 1.0)/9.0;
	}

	// fPluginValue = log scaled version 0->1
	// returns normal 0->1
	inline float calcLogPluginValue(float fPluginValue)
	{
		return log10(9.0*fPluginValue + 1.0);
	}

	// cooked to VA Scaled 0->1 param
	inline float calcVoltOctaveParameter(float fCookedParam, CUICtrl* pCtrl)
	{
		double dOctaves = fastlog2(pCtrl->fUserDisplayDataHiLimit/pCtrl->fUserDisplayDataLoLimit);
		return fastlog2(fCookedParam/pCtrl->fUserDisplayDataLoLimit)/dOctaves;
	}

	// fPluginValue = VA scaled version 0->1
	// returns normal 0->1
	inline float calcVoltOctavePluginValue(float fPluginValue, CUICtrl* pCtrl)
	{
		if(pCtrl->uUserDataType == UINTData)
			return *(pCtrl->m_pUserCookedUINTData);

		double dOctaves = fastlog2(pCtrl->fUserDisplayDataHiLimit/pCtrl->fUserDisplayDataLoLimit);
		float fDisplay = pCtrl->fUserDisplayDataLoLimit*fastpow2(fPluginValue*dOctaves); //(m_fDisplayMax - m_fDisplayMin)*value + m_fDisplayMin; //m_fDisplayMin*fastpow2(value*dOctaves);
		float fDiff = pCtrl->fUserDisplayDataHiLimit - pCtrl->fUserDisplayDataLoLimit;
		return (fDisplay - pCtrl->fUserDisplayDataLoLimit)/fDiff;

	}
	// --- helpers
	float getNormalizedRackAFXVariable(float fCookedVariable, CUICtrl* pUICtrl, float& fGUIWarpedVariable)
	{
		// --- calc normalized value
		float fRawValue = calcSliderVariable(pUICtrl->fUserDisplayDataLoLimit,
											 pUICtrl->fUserDisplayDataHiLimit,
											 fCookedVariable);
		// --- for linear
		fGUIWarpedVariable = fRawValue;

		// --- for nonlinear
		if(pUICtrl->bLogSlider)
		{
			fGUIWarpedVariable = calcLogParameter(fRawValue);
		}
		else if (pUICtrl->bExpSlider)
		{
			fGUIWarpedVariable = calcVoltOctaveParameter(fCookedVariable, pUICtrl);
		}

		return fRawValue;
	}

	inline static bool parseSize (const std::string& str, CPoint& point)
	{
		size_t sep = str.find (',', 0);
		if (sep != std::string::npos)
		{
			point.x = strtol (str.c_str (), 0, 10);
			point.y = strtol (str.c_str () + sep+1, 0, 10);
			return true;
		}
		return false;
	}

	inline bool trimString(char* str)
	{
	    while(*str == ' ' || *str == '\t' || *str == '\n')
            str++;

		int len = strlen(str);
		if(len <= 0) return false;

		while(len >= 0 && (str[len - 1] == ' ' || str[len - 1] == '\t' || *str == '\n'))
		{
            *(str + len - 1) = '\0';
            len--;
		}

		return true;
	}

	// old VST2 function for safe strncpy()
	inline char* vst_strncpy(char* dst, const char* src, size_t maxLen)
	{
		char* result = strncpy(dst, src, maxLen);
		dst[maxLen] = 0;
		return result;
	}

	// ---helpers from Steinberg againsimile sample
	/** Return the current channelBuffers used (depending of symbolicSampleSize). */
	inline void** getChannelBuffersPointer(const ProcessSetup& processSetup, const AudioBusBuffers& bufs)
	{
		if (processSetup.symbolicSampleSize == kSample32)
			return (void**)bufs.channelBuffers32;
		return (void**)bufs.channelBuffers64;
	}

	inline int32 getChannelCount(const ProcessSetup& processSetup, const AudioBusBuffers& bufs)
	{
		return bufs.numChannels;
	}

	/** Return the size in bytes of numSamples for one channel depending of symbolicSampleSize.*/
	inline uint32 getSampleFramesSizeInBytes(const ProcessSetup& processSetup, int32 numSamples)
	{
		if (processSetup.symbolicSampleSize == kSample32)
			return numSamples * sizeof(Sample32);
		return numSamples * sizeof(Sample64);
	}

	#if defined _WINDOWS || defined _WINDLL
	char* getMyDLLDirectory(UString cPluginName)
	{
		HMODULE hmodule = GetModuleHandle(cPluginName);

		TCHAR dir[MAX_PATH];
		memset(&dir[0], 0, MAX_PATH*sizeof(TCHAR));
		dir[MAX_PATH-1] = '\0';

		if(hmodule)
			GetModuleFileName(hmodule, &dir[0], MAX_PATH);
		else
			return NULL;

		// convert to UString
		UString DLLPath(&dir[0], MAX_PATH);

		char* pFullPath = new char[MAX_PATH];
		char* pDLLRoot = new char[MAX_PATH];

		DLLPath.toAscii(pFullPath, MAX_PATH);

		int nLenDir = strlen(pFullPath);
		int nLenDLL = wcslen(cPluginName) + 1;	// +1 is for trailing backslash
		memcpy(pDLLRoot, pFullPath, nLenDir-nLenDLL);
		pDLLRoot[nLenDir-nLenDLL] = '\0';

		delete [] pFullPath;

		// caller must delete this after use
		return pDLLRoot;
	}
	#endif
};

// --- sample accurate event interface
class CVSTMidiEventList : public IMidiEventList
{
protected:
	CPlugIn* m_pPlugIn;
	IEventList* inputEvents;
	unsigned int currentEventIndex;
public:
	CVSTMidiEventList(CPlugIn* pPlugIn);
    virtual ~CVSTMidiEventList(){}

	// --- set list, reset index
	void setEventList(IEventList* _inputEvents) { inputEvents = _inputEvents; currentEventIndex = 0; }

	// --- IMidiEventList
	UINT getEventCount();
	bool fireMidiEvent(unsigned int uSampleOffset);
};

// --- sample accurate parameter interface
class CVSTParamUpdateQueue : public IParamUpdateQueue
{
protected:
	int m_nBufferSize;
	//ParamValue is a glorified double
	Steinberg::Vst::ParamValue m_dInitialValue;
	//Double variable to store the last value
	Steinberg::Vst::ParamValue m_dPreviousValue;

	Steinberg::Vst::ParamValue m_dMaxValue;
	Steinberg::Vst::ParamValue m_dMinValue;

	//Store slope and b so that it needs to be calculated only once.
	Steinberg::Vst::ParamValue m_dSlope;
	Steinberg::Vst::ParamValue m_dIntercept;
	//Controls granularity
	int* m_pSampleAccuracy;

	int m_nQueueIndex;
	int m_nQueueSize;
	Steinberg::Vst::IParamValueQueue* m_pParameterQueue;
	int x1, x2;
	double y1, y2;
	bool m_bDirtyBit;

	int m_lSampleOffset;

public:
	CVSTParamUpdateQueue(void);
    virtual ~CVSTParamUpdateQueue(){}
	void initialize(Steinberg::Vst::ParamValue dInitialValue, Steinberg::Vst::ParamValue dMinValue, Steinberg::Vst::ParamValue dMaxValue, int* pSampleAccuracy);
	void setParamValueQueue(Steinberg::Vst::IParamValueQueue* pParamValueQueue, int nBufferSize);
	void setSlope();
	Steinberg::Vst::ParamValue interpolate(int x1, int x2, Steinberg::Vst::ParamValue y1, Steinberg::Vst::ParamValue y2, int x);
	int needsUpdate(int x, Steinberg::Vst::ParamValue  &value);

	//IParamUpdateQueue
	UINT getParameterIndex();
	bool getValueAtOffset(long int lSampleOffset, double dPreviousValue, double &dNextValue);
	bool getNextValue(double &dNextValue);
};

// --- little helper to avoid roundabout with IControlListener for simple updates
class CVSTParameterConnector
{
public:
    CVSTParameterConnector(){}
    ~CVSTParameterConnector(){}
	virtual void beginValueChange(int nTag){};
	virtual void endValueChange(int nTag){};
	virtual void guiControlValueChanged(int nTag, float fNormalizedValue){};
};

// CRafxCustomView creates the frame, populates with plugin view, then resizes frame
class CRafxCustomView: public CVSTParameterConnector, public VSTGUIEditor
{
public:
    CRafxCustomView(void* controller, ViewRect* size = 0);
    virtual ~CRafxCustomView();

	// --- VST3Editor ---
	/** Called when the editor will be opened. */
	virtual bool PLUGIN_API open (void* parent, const PlatformType& platformType = kDefaultNative) override;

	/** Called when the editor will be closed. */
	virtual void PLUGIN_API close() override;

	// --- sync params to freshly opened GUI
	void syncGUIToParams();

	// --- fixes bug with window not appearing, will need to roll over into Make VST
	bool beforeSizeChange(const CRect& newSize, const CRect& oldSize) override;

	// --- non IControlListener function to receive updates from GUI
	virtual void beginValueChange(int nTag) override;
	virtual void endValueChange(int nTag) override;
	virtual void guiControlValueChanged(int nTag, float fNormalizedValue) override;

	// --- our buddy plugin
	void setPlugIn(CPlugIn* pPlugIn){m_pPlugIn = pPlugIn;}

protected:
	CPlugIn* m_pPlugIn;

};

}}} // namespaces

#endif





