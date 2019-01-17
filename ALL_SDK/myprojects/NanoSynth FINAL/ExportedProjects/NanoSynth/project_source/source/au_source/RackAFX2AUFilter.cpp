/*
 File: RackAFX2AUFilter.cpp
 www.willpirkle.com
 */

//#define fxplugin
#define synthplugin
//#define midifxplugin

// --- MIDI Event Buffer Size - increase this is you have MIDI overflows
#define MAX_AU_MIDI_EVENTS 8192

#include <AudioToolbox/AudioUnitUtilities.h>

#include "AUInstrumentBase.h"   // for Synths
#include "AUMIDIEffectBase.h"   // For FX + MIDI
#include "AUEffectBase.h"       // For FX
#include "FilterVersion.h"
#include "RackAFX2AUFilter.h"
#include <math.h>
#include <queue>

#include "NanoSynth.h"
#include "RafxVSTEditor.h"
#include "ftypes.h"

//using namespace VSTGUI;
using namespace Steinberg;

#pragma mark ____RackAFX2AUFilter
//namespace VSTGUI
//{

class AUMIDIEventList;

// 0x4201
#ifndef synthplugin
#if defined(fxplugin)
class RackAFX2AUFilter : public AUEffectBase
#elif defined (midifxplugin)
class RackAFX2AUFilter : public AUMIDIEffectBase
#endif
{
public:
    RackAFX2AUFilter(AudioUnit component);
    ~RackAFX2AUFilter();

    virtual ComponentResult	Version() {return kFilterVersion;}

    virtual ComponentResult	Initialize();

    virtual OSStatus GetPropertyInfo(AudioUnitPropertyID	inID,
                                     AudioUnitScope         nScope,
                                     AudioUnitElement       inElement,
                                     UInt32&                outDataSize,
                                     Boolean&               outWritable );

    virtual OSStatus GetProperty(AudioUnitPropertyID    inID,
                                 AudioUnitScope 		inScope,
                                 AudioUnitElement       inElement,
                                 void*                  outData );

    virtual OSStatus SetProperty(AudioUnitPropertyID inID,
                                 AudioUnitScope 	 inScope,
                                 AudioUnitElement 	 inElement,
                                 const void*		 inData,
                                 UInt32 			 inDataSize);

    virtual ComponentResult	GetParameterInfo(AudioUnitScope			inScope,
                                             AudioUnitParameterID	inParameterID,
                                             AudioUnitParameterInfo	&outParameterInfo );

    // --- handle presets:
    virtual ComponentResult	GetPresets(CFArrayRef* outData)	const;
    virtual OSStatus NewFactoryPresetSet (const AUPreset& inNewFactoryPreset);

    // --- report a 0.0000 sec tail.
    virtual	bool SupportsTail() {return true;}
    virtual Float64	GetTailTime() {return 0.0000;}

    // --- NOTE: AU takes latency in seconds, not samples; this is recalculated
    //           during init() and reset() operations
    Float64 m_fLatencyInSeconds;
    virtual Float64	GetLatency() {return m_fLatencyInSeconds;}

    virtual ComponentResult SetParameter(AudioUnitParameterID	 inID,
                                         AudioUnitScope 		 inScope,
                                         AudioUnitElement 		 inElement,
                                         AudioUnitParameterValue inValue,
                                         UInt32					 inBufferOffsetInFrames);

    virtual OSStatus Render(AudioUnitRenderActionFlags &		ioActionFlags,
                            const AudioTimeStamp &              inTimeStamp,
                            UInt32                              inNumberFrames);


    virtual OSStatus ProcessBufferLists(AudioUnitRenderActionFlags& ioActionFlags,
                                        const AudioBufferList&		inBuffer,
                                        AudioBufferList&            outBuffer,
                                        UInt32					    inFramesToProcess );

    virtual ComponentResult	Reset(AudioUnitScope   inScope,
                                  AudioUnitElement inElement);

    virtual ComponentResult	GetParameterValueStrings(AudioUnitScope		  inScope,
                                                     AudioUnitParameterID inParameterID,
                                                     CFArrayRef*		  outStrings);

    // --- need this for when user selects a NON factory-preset (ie they created the preset in the Client)
    virtual ComponentResult	RestoreState(CFPropertyListRef inData);
    virtual UInt32 SupportedNumChannels(const AUChannelInfo** outInfo);
    AUMIDIEventList* m_pMIDIEventList;

    #if defined(midifxplugin)
    // --- MIDI Functions
    virtual OSStatus HandleNoteOn(UInt8 	inChannel,
                                  UInt8 	inNoteNumber,
                                  UInt8 	inVelocity,
                                  UInt32    inStartFrame);

    virtual OSStatus HandleNoteOff(UInt8 	inChannel,
                                   UInt8 	inNoteNumber,
                                   UInt8 	inVelocity,
                                   UInt32   inStartFrame);

    // --- MIDI Pitchbend (slightly different from all other CCs)
    virtual OSStatus HandlePitchWheel(UInt8  inChannel,
                                      UInt8  inPitch1,
                                      UInt8  inPitch2,
                                      UInt32 inStartFrame);

    // --- all other MIDI CC messages
    virtual OSStatus HandleControlChange(UInt8  inChannel,
                                         UInt8  inController,
                                         UInt8  inValue,
                                         UInt32	inStartFrame);

    // --- for ALL other MIDI messages you can get them here
    OSStatus HandleMidiEvent(UInt8  status,
                             UInt8  channel,
                             UInt8  data1,
                             UInt8  data2,
                             UInt32 inStartFrame);

    #endif

    // --- *RAFX*
    CPlugIn* m_pRAFXPlugIn;
    int m_nCurrentPreset;
    void updateRAFXParameters();
    void updateHostInfo();
    void initAUParameters();
    void updateAUParameters(); // outbound

    // --- VST/AU Buffers
    float** m_pVSTInputBuffers;
    float** m_pVSTOutputBuffers;
    float** m_pVSTSidechainInputBuffers;

    // --- sidechaining
    bool m_bHasSidechain;
    AudioBufferList* m_pSidechainBufferList;
    int m_nSidechainChannelCount;

    // --- VSTGUI4
    CRafxVSTEditor m_RafxVSTGUI;
    UINT* createControlMap();
    int getGUIControlCount();

    int m_nMeterMap[MAX_USER_METERS];
    void setMeterMap();
    void clearMeterMap();
    int getNextMeterMapSlot();
    CFRunLoopTimerRef m_IdleTimer;

};
#endif

// 0x4202
#if defined(synthplugin)
class RackAFX2AUFilter : public AUInstrumentBase
{
public:
    // --- const/dest
    RackAFX2AUFilter(AudioUnit inComponentInstance);
    virtual	~RackAFX2AUFilter();

    virtual OSStatus Initialize();

    virtual UInt32 SupportedNumChannels(const AUChannelInfo** outInfo);

    virtual OSStatus Version() {return 0x00010000;}

    OSStatus RestoreState(CFPropertyListRef plist);

    virtual OSStatus Reset(AudioUnitScope   inScope,
                           AudioUnitElement inElement);

    virtual OSStatus Render(AudioUnitRenderActionFlags& ioActionFlags,
                            const AudioTimeStamp&       inTimeStamp,
                            UInt32                      inNumberFrames);

    virtual OSStatus GetParameterInfo(AudioUnitScope          inScope,
                                      AudioUnitParameterID    inParameterID,
                                      AudioUnitParameterInfo& outParameterInfo);

    virtual OSStatus GetParameterValueStrings(AudioUnitScope       inScope,
                                              AudioUnitParameterID inParameterID,
                                              CFArrayRef*          outStrings);

    virtual OSStatus GetPropertyInfo(AudioUnitPropertyID inID,
                                     AudioUnitScope      inScope,
                                     AudioUnitElement    inElement,
                                     UInt32&             outDataSize,
                                     Boolean&            outWritable);

    virtual OSStatus GetProperty(AudioUnitPropertyID inID,
                                 AudioUnitScope      inScope,
                                 AudioUnitElement    inElement,
                                 void*               outData);

    virtual OSStatus SetProperty(AudioUnitPropertyID inID,
                                 AudioUnitScope      inScope,
                                 AudioUnitElement    inElement,
                                 const void*         inData,
                                 UInt32              inDataSize);

    virtual ComponentResult SetParameter(AudioUnitParameterID		 inID,
                                         AudioUnitScope 		 inScope,
                                         AudioUnitElement 		 inElement,
                                         AudioUnitParameterValue inValue,
                                         UInt32					 inBufferOffsetInFrames);

    // --- MIDI Functions
    //
    // --- MIDI Note On
    virtual OSStatus StartNote(MusicDeviceInstrumentID     inInstrument,
                               MusicDeviceGroupID          inGroupID,
                               NoteInstanceID*             outNoteInstanceID,
                               UInt32                      inOffsetSampleFrame,
                               const MusicDeviceNoteParams &inParams);

    // --- MIDI Note Off
    virtual OSStatus StopNote(MusicDeviceGroupID inGroupID,
                              NoteInstanceID     inNoteInstanceID,
                              UInt32             inOffsetSampleFrame);

    // --- MIDI Pitchbend (slightly different from all other CCs)
    virtual OSStatus HandlePitchWheel(UInt8  inChannel,
                                      UInt8  inPitch1,
                                      UInt8  inPitch2,
                                      UInt32 inStartFrame);

    // --- all other MIDI CC messages
    virtual OSStatus HandleControlChange(UInt8  inChannel,
                                         UInt8  inController,
                                         UInt8  inValue,
                                         UInt32	inStartFrame);

    // --- for ALL other MIDI messages you can get them here
    OSStatus HandleMidiEvent(UInt8  status,
                             UInt8  channel,
                             UInt8  data1,
                             UInt8  data2,
                             UInt32 inStartFrame);

    // --- handle presets:
    virtual ComponentResult	GetPresets(CFArrayRef* outData)	const;
    virtual OSStatus NewFactoryPresetSet(const AUPreset& inNewFactoryPreset);
    AUMIDIEventList* m_pMIDIEventList;

    // --- *RAFX*
    CPlugIn* m_pRAFXPlugIn;
    int m_nCurrentPreset;
    void updateRAFXParameters();
    void updateHostInfo();
    void initAUParameters();
    void updateAUParameters(); // outbound

    // --- VST/AU Buffers
    float** m_pVSTInputBuffers;
    float** m_pVSTOutputBuffers;
    float** m_pVSTSidechainInputBuffers;

    // --- sidechaining
    bool m_bHasSidechain;
    AudioBufferList* m_pSidechainBufferList;
    int m_nSidechainChannelCount;

    // --- VSTGUI4
    CRafxVSTEditor m_RafxVSTGUI;
    UINT* createControlMap();
    int getGUIControlCount();
    int m_nMeterMap[MAX_USER_METERS];
    void setMeterMap();
    int getNextMeterMapSlot();
	void clearMeterMap();
};
// 0x4302
#endif

// --- event list for sample accurate midi events
class AUMIDIEventList : public IMidiEventList
{
protected:
    CPlugIn* m_pPlugIn;
    std::queue<MIDI_EVENT> midiEventQueue;

public:
    AUMIDIEventList(CPlugIn* pPlugIn);
    virtual ~AUMIDIEventList();
    inline void clearEvents()
    {
        while(midiEventQueue.size() > 0)
            midiEventQueue.pop();
    }
    inline void addEvent(MIDI_EVENT event){midiEventQueue.push(event);}

    // --- IMidiEventList
    inline UINT getEventCount(){return midiEventQueue.size();}
    inline bool fireMidiEvent(unsigned int uSampleOffset)
    {
        if(midiEventQueue.size() <= 0 || !m_pPlugIn) return false;
        while(midiEventQueue.size() > 0)
        {
            // --- check the current top
            MIDI_EVENT event = midiEventQueue.front();
            if(event.uSampleOffset != uSampleOffset) return false;

            // --- else we have event with proper sample offset
            //
            // --- decode message and set in plugin

            if(m_pPlugIn->m_bWantAllMIDIMessages)
                m_pPlugIn->midiMessage(event.uChannel, event.uMessage, event.uData1, event.uData2);

            // --- pop to remove
            midiEventQueue.pop();
        }
        return true;
    }
};

AUMIDIEventList::AUMIDIEventList(CPlugIn* pPlugIn)
{
    m_pPlugIn = pPlugIn;
    clearEvents();
}

AUMIDIEventList::~AUMIDIEventList()
{
    clearEvents();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// getMyComponentDirectory()
//
// returns the directory where the .component resides
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
char* getMyComponentDirectory(CFStringRef bundleID)
{
    if (bundleID != NULL)
    {
        CFBundleRef helixBundle = CFBundleGetBundleWithIdentifier( bundleID );
        if(helixBundle != NULL)
        {
            CFURLRef bundleURL = CFBundleCopyBundleURL ( helixBundle );
            if(bundleURL != NULL)
            {
                CFURLRef componentFolderPathURL = CFURLCreateCopyDeletingLastPathComponent(NULL, bundleURL);

                CFStringRef myComponentPath = CFURLCopyFileSystemPath(componentFolderPathURL, kCFURLPOSIXPathStyle);
                CFRelease(componentFolderPathURL);

                if(myComponentPath != NULL)
                {
                    int nSize = CFStringGetLength(myComponentPath);
                    char* path = new char[nSize+1];
                    memset(path, 0, (nSize+1)*sizeof(char));

                    bool success = CFStringGetCString(myComponentPath, path, nSize+1, kCFStringEncodingASCII);
                    CFRelease(myComponentPath);

                    if(success) return path;
                    else return NULL;
                }
                CFRelease(bundleURL);
            }
        }
        CFRelease(bundleID);
    }
    return NULL;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	Standard DSP AudioUnit implementation

#if defined(fxplugin)
AUDIOCOMPONENT_ENTRY(AUBaseFactory, RackAFX2AUFilter)
#elif defined (midifxplugin)
AUDIOCOMPONENT_ENTRY(AUMIDIEffectFactory, RackAFX2AUFilter)
#else
AUDIOCOMPONENT_ENTRY(AUMusicDeviceFactory, RackAFX2AUFilter)
#endif

    // Factory presets
    // -- Written by RackAFX: DO NOT MODIFY if you indend on using RackAFX to update this project later!
    //
    // -- 0xA983 --

static const int kNumberPresets = 12;

static AUPreset kPresets[kNumberPresets] = {
{ 0, CFSTR("Factory Preset") },
{ 1, CFSTR("LL") },
{ 2, CFSTR("GG") },
{ 3, CFSTR("Test1") },
{ 4, CFSTR("Seq Test 1") },
{ 5, CFSTR("Seq Test 2") },
{ 6, CFSTR("Seq Test 3") },
{ 7, CFSTR("Seq Test 4") },
{ 8, CFSTR("Seq Test 5") },
{ 9, CFSTR("Seq Test 5A") },
{ 10, CFSTR("Full FX Sequence") },
{ 11, CFSTR("MFX DUB") }
};

static const int kPresetDefault = 0;
static const int kPresetDefaultIndex = 0;

// -- 0xAF66 --

    CFStringRef vectorJSXName = CFSTR("Vector JS X");
    CFStringRef vectorJSYName = CFSTR("Vector JS Y");
    CFStringRef vectorJSUnits = CFSTR("");


const UINT FILTER_CONTROL_USER_VSTGUI_VARIABLE = 106;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#pragma mark ____Construction_Initialization


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	RackAFX2AUFilter::RackAFX2AUFilter
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#if defined(fxplugin)
    RackAFX2AUFilter::RackAFX2AUFilter(AudioUnit component) : AUEffectBase(component, false) // false = do not use Process(), use ProcessBufferList() instead
#elif defined (midifxplugin)
    RackAFX2AUFilter::RackAFX2AUFilter(AudioUnit component) : AUMIDIEffectBase(component, false) // false = do not use Process(), use ProcessBufferList() instead
#else
RackAFX2AUFilter::RackAFX2AUFilter(AudioUnit inComponentInstance) : AUInstrumentBase(inComponentInstance, 0, 1) // No inputs, One output
#endif
{
    CreateElements(); // --- create input, output ports, groups and parts

    // Create the RackAFX inner (wrapped) object
    //
    // *RAFX*
    m_pRAFXPlugIn = new CNanoSynth;

    // call the post creation initialize()
    m_pRAFXPlugIn->initialize();

    // --- create VST/AU buffers, in case user has overridden that function
    m_pVSTInputBuffers = new float*[2];
    m_pVSTOutputBuffers = new float*[2];
    m_pVSTSidechainInputBuffers = new float*[2];

    // --- init the vector joystick params
    m_pRAFXPlugIn->m_JS_XCtrl.fJoystickValue = 0.0;
    m_pRAFXPlugIn->m_JS_YCtrl.fJoystickValue = 0.0;

    // --- set using indexed params for faster servicing
    if(m_pRAFXPlugIn->m_bUseCustomVSTGUI)
    {
        int nParams = m_pRAFXPlugIn->getControlCount();
        Globals()->UseIndexedParameters(nParams + numAddtlParams);
    }
    else
    {
        int nParams = m_pRAFXPlugIn->getControlCountMinimum();
        Globals()->UseIndexedParameters(nParams);
    }

    // --- setup the meter map
    setMeterMap();

    // --- all the AU-parameters must be set to their initial values here
    //     this is equivalent to inittUI() which is the first method in the RackAFX constructor
    initAUParameters();

    // --- sidechain
    m_bHasSidechain = false;
    m_pSidechainBufferList = NULL;
    m_pMIDIEventList = NULL;
    m_nSidechainChannelCount = 0;

#if defined(fxplugin) || defined(midifxplugin)
    m_bHasSidechain = m_pRAFXPlugIn->m_uPlugInEx[ENABLE_SIDECHAIN_VSTAU];

    // --- NOTE: there is a known bug (feature?) in Logic 9 and Pro X: if the sidechain is set to No Input
    //           then the sidechain buffers will point to the same buffers as the audio input!
    if(m_bHasSidechain)
    {
        SetBusCount(kAudioUnitScope_Input, 2);
        SafeGetElement(kAudioUnitScope_Input, 0)->SetName(CFSTR("Main Input"));
        SafeGetElement(kAudioUnitScope_Input, 1)->SetName(CFSTR("Sidechain"));
    }

    m_fLatencyInSeconds = 0.0;
    SetParamHasSampleRateDependency(true);
#endif

    // --- Init Factory Preset 0
    SetAFactoryPresetAsCurrent(kPresets[0]);

    // --- create our event list
    m_pMIDIEventList = new AUMIDIEventList(m_pRAFXPlugIn);

    // --- for users who are over-riding processVSTBuffers()
    PROCESS_INFO processInfo;
    processInfo.pIMidiEventList = m_pMIDIEventList;

    // --- hand it to the plugin
    m_pRAFXPlugIn->processRackAFXMessage(midiEventList, processInfo);

}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	RackAFX2AUFilter::~RackAFX2AUFilter
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
RackAFX2AUFilter::~RackAFX2AUFilter()
{
    // --- destory all allocated objects
 	if(m_pVSTInputBuffers)
        delete [] m_pVSTInputBuffers;

    if(m_pVSTSidechainInputBuffers)
        delete [] m_pVSTSidechainInputBuffers;

    if(m_pVSTOutputBuffers)
        delete [] m_pVSTOutputBuffers;

    if(m_pMIDIEventList)
        delete m_pMIDIEventList;

	if(m_pRAFXPlugIn)
		delete m_pRAFXPlugIn;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#pragma mark ____RackAFX main Functions

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	RackAFX2AUFilter::Reset -> RackAFX::prepareForPlay()
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult	RackAFX2AUFilter::Reset(AudioUnitScope 	 inScope,
                                        AudioUnitElement inElement)
{
    // --- reset the base class
    AUBase::Reset(inScope, inElement);

    // --- this function is called once on startup and then every time PLAY is pressed
    // *RAFX*
    if(m_pRAFXPlugIn)
    {
#if defined(fxplugin) || defined(midifxplugin)
        // --- set our latency whenever sample rate changes
        m_fLatencyInSeconds = m_pRAFXPlugIn->m_fPlugInEx[0] / GetOutput(0)->GetStreamFormat().mSampleRate;
#endif

		// --- set sample rate on plugin
        m_pRAFXPlugIn->m_nSampleRate = GetOutput(0)->GetStreamFormat().mSampleRate;

        // --- forward the call
        m_pRAFXPlugIn->prepareForPlay();

        // --- clear midi events
        if(m_pMIDIEventList)
            m_pMIDIEventList->clearEvents();
    }

    return noErr;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#pragma mark ____AU Functions: Init/Restore

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	RackAFX2AUFilter::SupportedNumChannels
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
UInt32 RackAFX2AUFilter::SupportedNumChannels (const AUChannelInfo** outInfo)
{
    // --- set an array of arrays of different combinations of supported numbers
    //     of ins and outs
    if(m_pRAFXPlugIn->m_bOutputOnlyPlugIn)
    {
        // --- Synths are mono/stereo
        static const AUChannelInfo sChannels[2] = {{0, 1}, {0, 2}};

        if(outInfo) *outInfo = sChannels;
        return sizeof (sChannels) / sizeof (AUChannelInfo);

    }
    else
    {
        // --- RackAFX explicitly supports 3 modes for FX Processing: 1x1, 1x2 and 2x2
        static const AUChannelInfo sChannels[3] = { {1, 1}, {1, 2}, {2, 2}};

        if(outInfo) *outInfo = sChannels;
        return sizeof (sChannels) / sizeof (AUChannelInfo);
    }
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	RackAFX2AUFilter::Initialize
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult	RackAFX2AUFilter::Initialize()
{
    // --- this is also placed here for clients such as AULab that do NOT call Reset() before streaming audio!
    if(m_pRAFXPlugIn)
    {
#if defined(fxplugin) || defined(midifxplugin)
        // --- set our latency whenever sample rate changes
        m_fLatencyInSeconds = m_pRAFXPlugIn->m_fPlugInEx[0] / GetOutput(0)->GetStreamFormat().mSampleRate;
#endif

		// --- set sample rate on plugin
        m_pRAFXPlugIn->m_nSampleRate = GetOutput(0)->GetStreamFormat().mSampleRate;

        // forward the call
        m_pRAFXPlugIn->prepareForPlay();
    }

    ComponentResult result = noErr;

#if defined(fxplugin)
    result = AUEffectBase::Initialize();
#elif defined (midifxplugin)
    result = AUMIDIEffectBase::Initialize();
#else
    result = AUInstrumentBase::Initialize();
#endif

    return result;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	RackAFX2AUFilter::RestoreState
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// called when a user-made preset is updated
ComponentResult	RackAFX2AUFilter::RestoreState(CFPropertyListRef inData)
{
    ComponentResult result = AUBase::RestoreState(inData);
    return result;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	RackAFX2AUFilter::initAUParameters()
//
// --- one time init of global parameters
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void RackAFX2AUFilter::initAUParameters()
{
    int nParams = m_pRAFXPlugIn->m_bUseCustomVSTGUI ? m_pRAFXPlugIn->getControlCount() : m_pRAFXPlugIn->getControlCountMinimum();

    for(int i = 0; i < nParams; i++)
    {
        // they are in AU proper order in the ControlList - do NOT reference them with RackAFX ID values any more!
        CUICtrl* pUICtrl = m_pRAFXPlugIn->getUICtrlByListIndex(i);

        if(pUICtrl)
        {
            switch (pUICtrl->uUserDataType)
            {
                case intData:
                    Globals()->SetParameter(i, (int)m_pRAFXPlugIn->getParameterValue(i));
                    break;
                case floatData:
                    Globals()->SetParameter(i, m_pRAFXPlugIn->getParameterValue(i));
                    break;
                case doubleData:
                    Globals()->SetParameter(i, (double)m_pRAFXPlugIn->getParameterValue(i));
                    break;
                case UINTData:
                    Globals()->SetParameter(i, (UINT)m_pRAFXPlugIn->getParameterValue(i));
                    break;
                case nonData:
                    if(m_pRAFXPlugIn->m_bUseCustomVSTGUI)
                        Globals()->SetParameter(i, 0.0); // meters
                    break;

                default:
                    break;
            }
        }
    }

    if(m_pRAFXPlugIn->m_bUseCustomVSTGUI)
    {
        // --- set added global GUI Controls
        Globals()->SetParameter(nParams + vectorJoystickX_Offset, m_pRAFXPlugIn->m_JS_XCtrl.fJoystickValue);
        Globals()->SetParameter(nParams + vectorJoystickY_Offset, m_pRAFXPlugIn->m_JS_XCtrl.fJoystickValue);
    }
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	RackAFX2AUFilter::updateRAFXParameters()
//
// --- threadsafe update of INCOMING RAFX parameters; does not update LED meter params!
//     This is called from the processing function, prior to processing audio, NOT from the
//     GUI thread!
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void RackAFX2AUFilter::updateRAFXParameters()
{
    if(m_pRAFXPlugIn)
    {
        int nParams = m_pRAFXPlugIn->m_bUseCustomVSTGUI ? m_pRAFXPlugIn->getControlCount() : m_pRAFXPlugIn->getControlCountMinimum();

        // iterate
        for(int i = 0; i < nParams; i++)
        {
            // they are in AU proper order in the ControlList - do NOT reference them with RackAFX ID values any more!
            CUICtrl* pUICtrl = m_pRAFXPlugIn->getUICtrlByListIndex(i);

            // --- get the threadsafe global parameter
            AudioUnitParameterValue inValue = Globals()->GetParameter(i);

            if(pUICtrl)
            {
                bool bDirty = false;

                switch (pUICtrl->uUserDataType)
                {
                    case intData:
                        if((int)inValue != m_pRAFXPlugIn->getParameterValue(i))
                        {
                            m_pRAFXPlugIn->setParameterValue(i, inValue);
                            bDirty = true;
                        }
                        break;
                    case floatData:
                        if((float)inValue != m_pRAFXPlugIn->getParameterValue(i))
                        {
                            m_pRAFXPlugIn->setParameterValue(i, inValue);
                            bDirty = true;
                        }
                        break;
                    case doubleData:
                        if((double)inValue != m_pRAFXPlugIn->getParameterValue(i))
                        {
                            m_pRAFXPlugIn->setParameterValue(i, inValue);
                            bDirty = true;
                        }
                        break;
                    case UINTData:
                        if((UINT)inValue != m_pRAFXPlugIn->getParameterValue(i))
                        {
                            m_pRAFXPlugIn->setParameterValue(i, inValue);
                            bDirty = true;
                        }
                        break;

                    default:
                        break;
                }

                // --- forward the call to the inner object
                if(bDirty)
                    m_pRAFXPlugIn->userInterfaceChange(pUICtrl->uControlId);
            }
        }

        // --- only with custom GUI
        if(m_pRAFXPlugIn->m_bUseCustomVSTGUI)
        {
            // --- joystick
            AudioUnitParameterValue inJSXValue = Globals()->GetParameter(nParams + vectorJoystickX_Offset);
            AudioUnitParameterValue inJSYValue = Globals()->GetParameter(nParams + vectorJoystickY_Offset);
            float fJSX = 0.0; float fJSY = 0.0;
            m_pRAFXPlugIn->getVectorJSValues(fJSX, fJSY);

            if(inJSXValue != fJSX || inJSYValue != fJSY)
            {
                m_pRAFXPlugIn->setVectorJSXValue(inJSXValue);
                m_pRAFXPlugIn->setVectorJSYValue(inJSYValue);

                // --- if no smoothing, update immediately
                if(!m_pRAFXPlugIn->m_JS_XCtrl.bEnableParamSmoothing)
                {
                    double dA, dB, dC, dD, dAC, dBD;
                   // calculateVectorMixValues(0.0, 0.0, inJSXValue, inJSYValue, dA, dB, dC, dD, dAC, dBD, 1, true);
                    m_pRAFXPlugIn->joystickControlChange(dA, dB, dC, dD, dAC, dBD);
                }
            }
        }
    }
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	RackAFX2AUFilter::updateAUParameters()
//
// --- threadsafe update of OUTBOUND RAFX parameters (meters)
//     This is called from the processing function, prior to processing audio, NOT from the
//     GUI thread!
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void RackAFX2AUFilter::updateAUParameters() // outbound
{
    for(int i=0; i<MAX_USER_METERS; i++)
    {
        if(m_nMeterMap[i]>= 0)
        {
            CUICtrl* pUICtrl = m_pRAFXPlugIn->getUICtrlByListIndex(m_nMeterMap[i]);
            if(!pUICtrl || !pUICtrl->m_pCurrentMeterValue) continue; // should never happen

            // --- update the meter value
            Globals()->SetParameter(m_nMeterMap[i], *pUICtrl->m_pCurrentMeterValue);
        }
    }
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#pragma mark ____AU Functions: Process Audio

#if defined(fxplugin) || defined (midifxplugin)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	RackAFX2AUFilter::Render -> this is only for picking up sidechain inputs, if they exist
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus RackAFX2AUFilter::Render(AudioUnitRenderActionFlags &		ioActionFlags,
                                  const AudioTimeStamp &            inTimeStamp,
                                  UInt32                            inNumberFrames)
{
    // --- this is only for sidechain support!
    bool bSCAvailable;
    try {bSCAvailable = HasInput(1);}
    catch (...) {bSCAvailable = false;}

    if(!m_bHasSidechain || !bSCAvailable)
    {
        m_pSidechainBufferList = NULL;
        m_nSidechainChannelCount = 0;
#if defined(fxplugin)
        return  AUEffectBase::Render(ioActionFlags, inTimeStamp, inNumberFrames);
#elif defined (midifxplugin)
        return  AUMIDIEffectBase::Render(ioActionFlags, inTimeStamp, inNumberFrames);
#endif
    }

    // --- pick up the sidechain buffer list, if there is one
    m_nSidechainChannelCount = 0;

    if(bSCAvailable)
    {
        AUInputElement* SCInput = GetInput(1);
        if(SCInput != NULL)
        {
            OSStatus status = SCInput->PullInput(ioActionFlags, inTimeStamp, 1, inNumberFrames);
            if(status == noErr)
            {
                m_nSidechainChannelCount = SCInput->NumberChannels();
                m_pSidechainBufferList = &(SCInput->GetBufferList());
            }
            else
                m_pSidechainBufferList = NULL;
        }
        else
            m_pSidechainBufferList = NULL;
    }
    else
        m_pSidechainBufferList = NULL;

#if defined(fxplugin)
    return  AUEffectBase::Render(ioActionFlags, inTimeStamp, inNumberFrames);
#elif defined (midifxplugin)
    return  AUMIDIEffectBase::Render(ioActionFlags, inTimeStamp, inNumberFrames);
#endif
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	RackAFX2AUFilter::ProcessBufferLists -> RackAFX::processAudioFrames() OR processVSTBuffers()
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus RackAFX2AUFilter::ProcessBufferLists(AudioUnitRenderActionFlags& ioActionFlags,
                                              const AudioBufferList&	  inBuffer,
                                              AudioBufferList&			  outBuffer,
                                              UInt32					  inFramesToProcess )
{
    if(!m_pRAFXPlugIn)
        return noErr;

    // --- threadsafe sync to globals
    updateRAFXParameters(); // from AUEvents

    // --- populate plugin's m_HostInfo struct
    updateHostInfo();
    double dSampleInterval = 1.0/GetOutput(0)->GetStreamFormat().mSampleRate;

    // --- get information about i/o
    SInt16 auNumInputs = (SInt16) GetInput(0)->GetStreamFormat().mChannelsPerFrame;
    SInt16 auNumOutputs = (SInt16) GetOutput(0)->GetStreamFormat().mChannelsPerFrame;

    // --- get buffer pointers
    float* pInputL = (float*)inBuffer.mBuffers[0].mData;
    float* pOutputL = (float*)outBuffer.mBuffers[0].mData;
    float* pInputR = NULL;
    float* pOutputR = NULL;

    // --- RackAFX Frames; max is 2-channel but you can
    //     change this if you want to process more channels
    float input[2];
    float output[2];
    float scInput[2] = {0};

    // --- VST and AU share the same buffering system; named "VST" here because its called VST buffering in RackAFX
    m_pVSTInputBuffers[0] = pInputL;
    m_pVSTOutputBuffers[0] = pOutputL;
    m_pVSTInputBuffers[1] = NULL;
    m_pVSTOutputBuffers[1] = NULL;

    // --- sidechain
    float* pSidechainInputL = NULL;
    float* pSidechainInputR = NULL;

    audioProcessData auxInputProcessData;
    auxInputProcessData.uInputBus = 1;
    auxInputProcessData.bInputEnabled = false;
    auxInputProcessData.uNumInputChannels = 0;
    auxInputProcessData.uBufferSize = 0;
    auxInputProcessData.pFrameInputBuffer = NULL;
    auxInputProcessData.pRAFXInputBuffer = NULL;
    auxInputProcessData.ppVSTInputBuffer = NULL;

    // --- m_pSidechainBufferList is only non-null when SC is enabled in both client and RAFX
    if(m_pSidechainBufferList)
    {
        pSidechainInputL = (float*)m_pSidechainBufferList->mBuffers[0].mData;
        if(m_nSidechainChannelCount == 2)
            pSidechainInputR = (float*)m_pSidechainBufferList->mBuffers[1].mData;

        m_pVSTSidechainInputBuffers[0] = pSidechainInputL;
        m_pVSTSidechainInputBuffers[1] = pSidechainInputR;

        auxInputProcessData.bInputEnabled = true;
        auxInputProcessData.uNumInputChannels = m_nSidechainChannelCount;
    }

    if(auNumInputs == 2)
    {
        pInputR = (float*)inBuffer.mBuffers[1].mData;
        m_pVSTInputBuffers[1] = pInputR;
    }

    if(auNumOutputs == 2)
    {
        pOutputR = (float*)outBuffer.mBuffers[1].mData;
        m_pVSTOutputBuffers[1] = pOutputR;
    }

    // --- VST and AU use an identical system for buffering; see appendinx A of my FX book
    if(m_pRAFXPlugIn->m_bWantVSTBuffers)
    {
        if(m_pSidechainBufferList)
        {
            auxInputProcessData.ppVSTInputBuffer = m_pVSTSidechainInputBuffers; //** to sidechain

            // --- process sidechain
            m_pRAFXPlugIn->processAuxInputBus(&auxInputProcessData);
        }

        // NOTE: VST buffer processing only takes one channel value (assumes in and out are same numbers)
        //       So, check m_pVSTInputBuffers[1] == NULL and m_pVSTOutputBuffers[1] == NULL to figure out
        //       the channel counts.
        m_pRAFXPlugIn->processVSTAudioBuffer(m_pVSTInputBuffers, m_pVSTOutputBuffers, 2, inFramesToProcess);
    }
    else // --- standard RAFX frames
    {
        for(int i=0; i<inFramesToProcess; i++)
        {
            input[0] = pInputL[i];
            input[1] = pInputR ? pInputR[i] : 0.0;

            if(m_pSidechainBufferList)
            {
                scInput[0] = pSidechainInputL[i];
                if(m_nSidechainChannelCount == 2)
                    scInput[1] = pSidechainInputR[i];

                auxInputProcessData.pFrameInputBuffer = &scInput[0]; //* to sidechain

                // --- process sidechain
                m_pRAFXPlugIn->processAuxInputBus(&auxInputProcessData);
            }

            // --- sample-accurate MIDI
            if(m_pMIDIEventList) m_pMIDIEventList->fireMidiEvent(i);

            // --- forward the call to the inner object
            m_pRAFXPlugIn->processAudioFrame(&input[0], &output[0], auNumInputs, auNumOutputs);

            // --- update per-frame
            m_pRAFXPlugIn->m_HostProcessInfo.uAbsoluteFrameBufferIndex += 1;
            m_pRAFXPlugIn->m_HostProcessInfo.dAbsoluteFrameBufferTime += dSampleInterval;

            pOutputL[i] = output[0];
            if(pOutputR)
                pOutputR[i] = output[1];
        }
    }

    // --- thread safe update to meters
    updateAUParameters();

    return noErr;
}
#else

OSStatus RackAFX2AUFilter::Render(AudioUnitRenderActionFlags& ioActionFlags,
                                  const AudioTimeStamp&       inTimeStamp,
                                  UInt32                      nNumberFrames)
{

    // --- broadcast MIDI events
    PerformEvents(inTimeStamp);

    // --- threadsafe sync to globals
    updateRAFXParameters();

    // --- populate plugin's m_HostInfo struct
    updateHostInfo();
    double dSampleInterval = 1.0/GetOutput(0)->GetStreamFormat().mSampleRate;

    // --- get the number of channels
    AudioBufferList& bufferList = GetOutput(0)->GetBufferList();
    UInt32 numChans = bufferList.mNumberBuffers;

    // --- we only support mono/stereo
    if(numChans > 2)
        return kAudioUnitErr_FormatNotSupported;

    // --- get pointers for buffer lists
    float* pOutputL = (float*)bufferList.mBuffers[0].mData;
    float* pOutputR = numChans == 2 ? (float*)bufferList.mBuffers[1].mData : NULL;

    // --- RackAFX Frames; max is 2-channel but you can
    //     change this if you want to process more channels
    float input[2];
    float output[2];

    // --- VST and AU share the same buffering system; named "VST" here because its called VST buffering in RackAFX
    m_pVSTOutputBuffers[0] = pOutputL;
    m_pVSTOutputBuffers[1] = pOutputR;
    m_pVSTInputBuffers[0] = pOutputL; // dummy, not used in synth
    m_pVSTInputBuffers[1] = pOutputR; // dummy, not used in synth

    // --- VST and AU use an identical system for buffering; see appendinx A of my FX book
    if(m_pRAFXPlugIn->m_bWantVSTBuffers)
    {
        // NOTE: VST buffer processing only takes one channel value (assumes in and out are same numbers)
        //       So, check m_pVSTInputBuffers[1] == NULL and m_pVSTOutputBuffers[1] == NULL to figure out
        //       the channel counts.
        m_pRAFXPlugIn->processVSTAudioBuffer(m_pVSTInputBuffers, m_pVSTOutputBuffers, numChans, nNumberFrames);
    }
    else // --- standard RAFX frames
    {
        for(int i=0; i<nNumberFrames; i++)
        {
            input[0] = 0.0;
            input[1] = 0.0;

            // --- sample-accurate MIDI
            if(m_pMIDIEventList) m_pMIDIEventList->fireMidiEvent(i);

            // --- forward the call to the inner object
            m_pRAFXPlugIn->processAudioFrame(&input[0], &output[0], numChans, numChans);

            // --- update per-frame
            m_pRAFXPlugIn->m_HostProcessInfo.uAbsoluteFrameBufferIndex += 1;
            m_pRAFXPlugIn->m_HostProcessInfo.dAbsoluteFrameBufferTime += dSampleInterval;

            pOutputL[i] = output[0];
            if(pOutputR)
                pOutputR[i] = output[1];
        }
    }

    // --- threadsafe update to meters
    updateAUParameters();

    return noErr;
}
#endif


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	RackAFX2AUFilter::updateHostInfo()
//
// --- gets info from host on each process loop
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void RackAFX2AUFilter::updateHostInfo()
{
    Float64 outCurrentBeat = 0.0;
    Float64 outCurrentTempo = 0.0;
    UInt32  outDeltaSampleOffsetToNextBeat = 0.0;
    Float32 outTimeSig_Numerator = 0.0;
    UInt32  outTimeSig_Denominator = 0.0;
    Float64 outCurrentMeasureDownBeat = 0.0;
    Boolean outIsPlaying = false;
    Boolean outTransportStateChanged = false;
    Float64 outCurrentSampleInTimeLine = 0.0;
    Boolean outIsCycling = false;
    Float64 outCycleStartBeat = 0.0;
    Float64 outCycleEndBeat = 0.0;

    OSStatus status = CallHostBeatAndTempo(&outCurrentBeat, &outCurrentTempo);
    if(status == noErr)
    {
        m_pRAFXPlugIn->m_HostProcessInfo.dBPM = outCurrentTempo;
        m_pRAFXPlugIn->m_HostProcessInfo.dCurrentBeat = outCurrentBeat;
    }

    status = CallHostMusicalTimeLocation(&outDeltaSampleOffsetToNextBeat, &outTimeSig_Numerator, &outTimeSig_Denominator, &outCurrentMeasureDownBeat);
    if(status == noErr)
    {
        m_pRAFXPlugIn->m_HostProcessInfo.dCurrentMeasureDownBeat = outCurrentMeasureDownBeat;
        m_pRAFXPlugIn->m_HostProcessInfo.nDeltaSampleOffsetToNextBeat = outDeltaSampleOffsetToNextBeat;
        m_pRAFXPlugIn->m_HostProcessInfo.fTimeSigNumerator = outTimeSig_Numerator;
        m_pRAFXPlugIn->m_HostProcessInfo.uTimeSigDenomintor = outTimeSig_Denominator;
    }

    status = CallHostTransportState (&outIsPlaying, &outTransportStateChanged, &outCurrentSampleInTimeLine, &outIsCycling, &outCycleStartBeat, &outCycleEndBeat);
    {
        m_pRAFXPlugIn->m_HostProcessInfo.dCycleEndBeat = outCycleEndBeat;
        m_pRAFXPlugIn->m_HostProcessInfo.dCycleStartBeat = outCycleStartBeat;
        m_pRAFXPlugIn->m_HostProcessInfo.bIsCycling = outIsCycling;
        m_pRAFXPlugIn->m_HostProcessInfo.bTransportStateChanged = outTransportStateChanged;
        m_pRAFXPlugIn->m_HostProcessInfo.bIsPlayingAU = outIsPlaying;
        m_pRAFXPlugIn->m_HostProcessInfo.uAbsoluteFrameBufferIndex = outCurrentSampleInTimeLine;
        m_pRAFXPlugIn->m_HostProcessInfo.dAbsoluteFrameBufferTime = outCurrentSampleInTimeLine/GetOutput(0)->GetStreamFormat().mSampleRate;
    }
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	RackAFX2AUFilter::SetParameter
//
// --- this just calls base class; was changed for thread safety in RackAFX v6.8
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult RackAFX2AUFilter::SetParameter(AudioUnitParameterID	   inID,
                                               AudioUnitScope 		   inScope,
                                               AudioUnitElement 	   inElement,
                                               AudioUnitParameterValue inValue,
                                               UInt32				   inBufferOffsetInFrames)
{
    return AUBase::SetParameter(inID, inScope, inElement, inValue, inBufferOffsetInFrames);
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#pragma mark ____AU Functions: Parameters

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	RackAFX2AUFilter::GetParameterInfo
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult	RackAFX2AUFilter::GetParameterInfo(AudioUnitScope		   inScope,
                                                   AudioUnitParameterID    inParameterID,
                                                   AudioUnitParameterInfo& outParameterInfo )
{
    // --- here, the client is querying us for each of our controls. It wants a description
    //     (name) and I have set it up for custom units since that's the most general so
    //     we also give it units.
    //
    // --- You will see similarities in this and what you set up in the slider in RackAFX
    ComponentResult result = noErr;

    outParameterInfo.flags = kAudioUnitParameterFlag_IsWritable + kAudioUnitParameterFlag_IsReadable;

    if(inScope == kAudioUnitScope_Global)
    {
        int nParams = m_pRAFXPlugIn->getControlCount();

        // --- should not happen
        if(!m_pRAFXPlugIn)
            return kAudioUnitErr_InvalidParameter;

        // --- they are in AU proper order in the ControlList
        CUICtrl* pUICtrl = m_pRAFXPlugIn->getUICtrlByListIndex(inParameterID);

        if(pUICtrl)
        {
            if(!m_pRAFXPlugIn->m_bUseCustomVSTGUI && pUICtrl->uControlType == FILTER_CONTROL_LED_METER)
                ; // do nothing
            else
            {
                // --- make the name objects
                CFStringRef name = CFStringCreateWithCString(NULL, pUICtrl->cControlName, 8);
                CFStringRef units = CFStringCreateWithCString(NULL, pUICtrl->cControlUnits, 8);

                // --- fill in the name; you have to call a function to do this
                AUBase::FillInParameterName (outParameterInfo, name, false);

                // --- if UINT data, tell it we are Indexed; this will make it query us for
                //     strings to fill a dropdown control; those strings are chunks of your
                //     enum string for that control
                if(pUICtrl->uUserDataType == UINTData)
                    outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
                else
                {
                    // --- custom, set units
                    outParameterInfo.unit = kAudioUnitParameterUnit_CustomUnit;
                    outParameterInfo.unitName = units;
                }

                // --- set min and max
                outParameterInfo.minValue = pUICtrl->fUserDisplayDataLoLimit;
                outParameterInfo.maxValue = pUICtrl->fUserDisplayDataHiLimit;

                // --- initialize
                switch (pUICtrl->uUserDataType)
                {
                    case intData:
                        outParameterInfo.defaultValue = (int)m_pRAFXPlugIn->getParameterValue(inParameterID);
                        break;
                    case floatData:
                        outParameterInfo.defaultValue = m_pRAFXPlugIn->getParameterValue(inParameterID);
                        break;
                    case doubleData:
                        outParameterInfo.defaultValue = (double)m_pRAFXPlugIn->getParameterValue(inParameterID);
                        break;
                    case UINTData:
                        outParameterInfo.defaultValue = (UINT)m_pRAFXPlugIn->getParameterValue(inParameterID);
                        break;

                    default:
                        break;
                }

                // --- rest of flags
                outParameterInfo.flags += kAudioUnitParameterFlag_IsHighResolution;
            }
        }
        else if(!pUICtrl && m_pRAFXPlugIn->m_bUseCustomVSTGUI) // VJStick only with custom GUI
        {
            if(inParameterID == nParams + vectorJoystickX_Offset)
            {
                AUBase::FillInParameterName (outParameterInfo, vectorJSXName, false);
                outParameterInfo.unitName = vectorJSUnits;
                outParameterInfo.minValue = -1.0;
                outParameterInfo.maxValue = 1.0;
                outParameterInfo.defaultValue = 0.0;
                outParameterInfo.flags += kAudioUnitParameterFlag_IsHighResolution;
            }
            else if (inParameterID == nParams + vectorJoystickY_Offset)
            {
                AUBase::FillInParameterName (outParameterInfo, vectorJSYName, false);
                outParameterInfo.unitName = vectorJSUnits;
                outParameterInfo.minValue = -1.0;
                outParameterInfo.maxValue = 1.0;
                outParameterInfo.defaultValue = 0.0;
                outParameterInfo.flags += kAudioUnitParameterFlag_IsHighResolution;
            }
        }
    }
    else
    {
        result = kAudioUnitErr_InvalidParameter;
    }

    return result;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	RackAFX2AUFilter::GetParameterValueStrings
//
// --- this fills the default I/F Dropown Boxes with the enumerated strings
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult	RackAFX2AUFilter::GetParameterValueStrings(AudioUnitScope       inScope,
                                                           AudioUnitParameterID	inParameterID,
                                                           CFArrayRef *			outStrings)
{
    if (inScope == kAudioUnitScope_Global)
    {
        if (outStrings == NULL)
            return noErr;

        // --- these will be enum UINT types of controls
        CUICtrl* pUICtrl = m_pRAFXPlugIn->getUICtrlByListIndex(inParameterID);

        if(!pUICtrl)
            return kAudioUnitErr_InvalidParameter;

        // --- convert the list into an array
        CFStringRef enumList = CFStringCreateWithCString(NULL, pUICtrl->cEnumeratedList, 8);
        CFStringRef comma CFSTR(",");
        CFArrayRef strings = CFStringCreateArrayBySeparatingStrings(NULL, enumList, comma);

        // --- create the array COPY (important: these are local variables above!)
        *outStrings = CFArrayCreateCopy(NULL, strings);

        return noErr;
    }
    return kAudioUnitErr_InvalidParameter;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#pragma mark ____AU Functions: Properties

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	RackAFX2AUFilter::GetPropertyInfo
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#if defined(fxplugin) || defined(midifxplugin)
ComponentResult	RackAFX2AUFilter::GetPropertyInfo(AudioUnitPropertyID inID,
                                                  AudioUnitScope      inScope,
                                                  AudioUnitElement    inElement,
                                                  UInt32&             outDataSize,
                                                  Boolean&            outWritable)
#else
OSStatus RackAFX2AUFilter::GetPropertyInfo(AudioUnitPropertyID inID,
                                           AudioUnitScope      inScope,
                                           AudioUnitElement    inElement,
                                           UInt32&             outDataSize,
                                           Boolean&            outWritable)
#endif
{
	if (inScope == kAudioUnitScope_Global)
	{
  		switch(inID)
		{
            // --- m_bUseCustomVSTGUI: we have a Cocoa GUI
			case kAudioUnitProperty_CocoaUI:
            {
                if(m_pRAFXPlugIn->m_bUseCustomVSTGUI)
                {
                    outWritable = false;
                    outDataSize = sizeof(AudioUnitCocoaViewInfo);
                    return noErr;
                }
                ;
            }
            case kOpenGUI:
            {
                outDataSize = sizeof(TPtrInt);
                outWritable = false;
                return noErr;
            }
            /*case kMusicDeviceProperty_InstrumentCount:
            {
                outDataSize = sizeof(UInt32);
                outWritable = false;
                return noErr;

            }*/
		}
	}
#if defined(fxplugin)
    return AUEffectBase::GetPropertyInfo(inID, inScope, inElement, outDataSize, outWritable);
#elif defined (midifxplugin)
    return AUMIDIEffectBase::GetPropertyInfo(inID, inScope, inElement, outDataSize, outWritable);
#else
	return MusicDeviceBase::GetPropertyInfo(inID, inScope, inElement, outDataSize, outWritable);
#endif
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	RackAFX2AUFilter::GetProperty
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#if defined(fxplugin) || defined(midifxplugin)
ComponentResult	RackAFX2AUFilter::GetProperty(AudioUnitPropertyID inID,
                                              AudioUnitScope      inScope,
                                              AudioUnitElement    inElement,
                                              void*               outData)
#else
OSStatus RackAFX2AUFilter::GetProperty(AudioUnitPropertyID inID,
                                       AudioUnitScope      inScope,
                                       AudioUnitElement    inElement,
                                       void*               outData)
#endif
{
	if (inScope == kAudioUnitScope_Global)
	{
		switch(inID)
		{
            // --- This property allows the host application to find the UI associated with this
			case kAudioUnitProperty_CocoaUI:
			{
				// --- Look for a resource in the main bundle by name and type.
				CFBundleRef bundle = CFBundleGetBundleWithIdentifier( CFSTR("developer.audiounit.nanosynth") );

				if(bundle == NULL) return fnfErr;

				CFURLRef bundleURL = CFBundleCopyResourceURL(bundle,
                                                             CFSTR("NanoSynth_AU"),	// this is the name of the cocoa bundle as specified in the CocoaViewFactory.plist
                                                             CFSTR("bundle"),			// this is the extension of the cocoa bundle
                                                             NULL);

                if(bundleURL == NULL) return fnfErr;

				CFStringRef className = CFSTR("NanoSynth284027328_ViewFactory");	// name of the main class that implements the AUCocoaUIBase protocol
				AudioUnitCocoaViewInfo cocoaInfo = { bundleURL, {className} };
				*((AudioUnitCocoaViewInfo *)outData) = cocoaInfo;

				return noErr;
			}
            return kAudioUnitErr_InvalidProperty;
		}
	}
#if defined(fxplugin)
    return AUEffectBase::GetProperty(inID, inScope, inElement, outData);
#elif defined (midifxplugin)
    return AUMIDIEffectBase::GetProperty(inID, inScope, inElement, outData);
#else
   	return AUBase::GetProperty(inID, inScope, inElement, outData);
#endif
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	RackAFX2AUFilter::SetProperty
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus	RackAFX2AUFilter::SetProperty(AudioUnitPropertyID inID,
                                          AudioUnitScope 	  inScope,
                                          AudioUnitElement 	  inElement,
                                          const void*		  inData,
                                          UInt32 			  inDataSize)
{
   	if (inScope == kAudioUnitScope_Global)
    {
        switch (inID)
        {
            case kOpenGUI:
            {
                VIEW_STRUCT* pVS = (VIEW_STRUCT*)inData;

                // --- first see if they have a custom GUI
                void* createdCustomGUI = NULL;
                VSTGUI_VIEW_INFO info = {0};
                info.message = GUI_HAS_USER_CUSTOM;
                info.bHasUserCustomView = false; // this flag will be set if they do
                m_pRAFXPlugIn->showGUI((void*)&info);

                if(info.bHasUserCustomView)
                {
                    info.message = GUI_USER_CUSTOM_OPEN;
                    info.window = (void*)pVS->pWindow;
                    info.hPlugInInstance = (void*)pVS->au;  // for AU, this is the AU object param
                    info.hRAFXInstance = NULL;              // unused in AU
                    info.size.width = 0;
                    info.size.height = 0;
                    info.vstPlatformType = kNSView;
                    info.bHasUserCustomView = true;

                    createdCustomGUI = m_pRAFXPlugIn->showGUI((void*)&info);
                    if(createdCustomGUI)
                    {
                        pVS->width = info.size.width;
                        pVS->height = info.size.height;
                        return noErr;
                        break;
                    }
                }
                if(!createdCustomGUI)
                {
                    // --- Look for a resource in the main bundle by name and type.
                    CFBundleRef bundle = CFBundleGetBundleWithIdentifier( CFSTR("developer.audiounit.nanosynth") );

                    if (bundle == NULL) return fnfErr;

                    // --- get .uidesc file
                    CFURLRef bundleURL = CFBundleCopyResourceURL(bundle,CFSTR("RackAFX"),CFSTR("uidesc"),NULL);
                    CFStringRef xmlPath = CFURLCopyPath(bundleURL);
                    int nSize = CFStringGetLength(xmlPath);
                    char* path = new char[nSize+1];
                    memset(path, 0, (nSize+1)*sizeof(char));
                    CFStringGetCString(xmlPath, path, nSize+1, kCFStringEncodingASCII);
                    CFRelease(xmlPath);

                    // --- create GUI
                    m_RafxVSTGUI.open(pVS->pWindow, m_pRAFXPlugIn, path, pVS->au);
                    m_RafxVSTGUI.getSize(pVS->width, pVS->height);

                    // --- for pure custom GUIs
                    m_RafxVSTGUI.initPluginCustomGUI();

                    // --- setup control maps
                    UINT* pControlMap = createControlMap();
                    m_RafxVSTGUI.copyControlMap(pControlMap, getGUIControlCount());
                    delete [] pControlMap;

                    m_RafxVSTGUI.initControls();
                    m_RafxVSTGUI.copyMeterMap(&m_nMeterMap[0]);

                    delete [] path;
                    return noErr;
                    break;
                }
                break;
            }
            case kCloseGUI:
            {
                VSTGUI_VIEW_INFO info = {0};
                info.message = GUI_HAS_USER_CUSTOM;
                info.bHasUserCustomView = false; // this flag will be set if they do
                m_pRAFXPlugIn->showGUI((void*)&info);
                if(info.bHasUserCustomView)
                {
                    info.message = GUI_USER_CUSTOM_CLOSE;
                    m_pRAFXPlugIn->showGUI((void*)&info);
                }
                else
                    m_RafxVSTGUI.close();

                return noErr;
                break;
            }
        }
    }

    return kAudioUnitErr_InvalidProperty;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	RackAFX2AUFilter::createControlMap
//  creates mapping arrays from RAFX control indexes to VSTGUI indexes
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
UINT* RackAFX2AUFilter::createControlMap()
{
    int nCount = getGUIControlCount();
    UINT *pControlMap = nCount > 0 ? new UINT[nCount] : NULL;

    int m = 0;
    int nControls = m_pRAFXPlugIn->getControlCount();
    for(int i = 0; i< nControls; i++)
    {
        CUICtrl* pUICtrl = m_pRAFXPlugIn->getUICtrlByListIndex(i);

        if(pUICtrl->uControlType != FILTER_CONTROL_CONTINUOUSLY_VARIABLE &&
           pUICtrl->uControlType != FILTER_CONTROL_COMBO_VARIABLE &&
           pUICtrl->uControlType != FILTER_CONTROL_USER_VSTGUI_VARIABLE &&
           pUICtrl->uControlType != FILTER_CONTROL_RADIO_SWITCH_VARIABLE &&
           pUICtrl->uControlType != FILTER_CONTROL_LED_METER)
            continue;

        // --- add the index
        if(pControlMap)
            pControlMap[m++] = i;

    }

    return pControlMap;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	RackAFX2AUFilter::getGUIControlCount
//  returns number of VSTGUI4 controls (see RackAFX Plugin->Add/Remove User Variables
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int RackAFX2AUFilter::getGUIControlCount()
{
    int nCount = 0;
    int nControls = m_pRAFXPlugIn->getControlCount();
    for(int i = 0; i< nControls; i++)
    {
        CUICtrl* pUICtrl = m_pRAFXPlugIn->getUICtrlByListIndex(i);

        if(pUICtrl->uControlType != FILTER_CONTROL_CONTINUOUSLY_VARIABLE &&
           pUICtrl->uControlType != FILTER_CONTROL_COMBO_VARIABLE &&
           pUICtrl->uControlType != FILTER_CONTROL_USER_VSTGUI_VARIABLE &&
           pUICtrl->uControlType != FILTER_CONTROL_RADIO_SWITCH_VARIABLE &&
           pUICtrl->uControlType != FILTER_CONTROL_LED_METER)
            continue;

        // --- inc
        nCount++;
    }
    return nCount;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	RackAFX2AUFilter::setMeterMap
//
// --- sets the meter map for meter updates
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void RackAFX2AUFilter::setMeterMap()
{
    if(!m_pRAFXPlugIn) return;

    // -- clear first
    clearMeterMap();

    int nControls = m_pRAFXPlugIn->getControlCount();

    for(int i = 0; i< nControls; i++)
    {
        CUICtrl* pUICtrl = m_pRAFXPlugIn->getUICtrlByListIndex(i);
        if(!pUICtrl) return;
        if(pUICtrl->uControlType == FILTER_CONTROL_LED_METER)
        {
            int slot = getNextMeterMapSlot();
            if(slot >= 0)
                m_nMeterMap[slot] = i;
        }
    }
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	RackAFX2AUFilter::clearMeterMap
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void RackAFX2AUFilter::clearMeterMap()
{
    // --- clear map
    for(int i=0; i<MAX_USER_METERS; i++)
        m_nMeterMap[i] = -1; // -1 = no meter mapped
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	RackAFX2AUFilter::getNextMeterMapSlot
//
// --- next non(-1) slot available
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int RackAFX2AUFilter::getNextMeterMapSlot()
{
    for(int i=0; i<MAX_USER_METERS; i++)
    {
        if(m_nMeterMap[i] == -1)
            return i;
    }
    return -1; // no slots left, will never happen...
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#pragma mark ____AU Functions: Presets

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	RackAFX2AUFilter::GetPresets
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#if defined(fxplugin) || defined(midifxplugin)
ComponentResult	RackAFX2AUFilter::GetPresets(CFArrayRef *outData) const
#else
OSStatus RackAFX2AUFilter::GetPresets(CFArrayRef *outData) const
#endif
{
    // --- this is used to determine if presets are supported
    //     which in this unit they are so we implement this method!
	if(outData == NULL) return noErr;

	// make the array
	CFMutableArrayRef theArray = CFArrayCreateMutable (NULL, kNumberPresets, NULL);

    // copy our preset names
	for (int i = 0; i < kNumberPresets; ++i)
    {
		CFArrayAppendValue (theArray, &kPresets[i]);
    }

    *outData = (CFArrayRef)theArray;
	return noErr;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	RackAFX2AUFilter::NewFactoryPresetSet
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus RackAFX2AUFilter::NewFactoryPresetSet(const AUPreset & inNewFactoryPreset)
{
    SInt32 chosenPreset = inNewFactoryPreset.presetNumber;

    if (chosenPreset < 0 || chosenPreset >= kNumberPresets)
        return kAudioUnitErr_InvalidPropertyValue;

    m_nCurrentPreset = chosenPreset;

    // --- Sync Preset Name
    SetAFactoryPresetAsCurrent(kPresets[m_nCurrentPreset]);

    int nParams = m_pRAFXPlugIn->m_bUseCustomVSTGUI ? m_pRAFXPlugIn->getControlCount() : m_pRAFXPlugIn->getControlCountMinimum();

    for(int j=0; j<nParams; j++)
    {
        // get the control
        CUICtrl* pUICtrl = m_pRAFXPlugIn->getUICtrlByListIndex(j);
        if(!pUICtrl) continue; // should never happen

        // --- set the global parameter
        Globals()->SetParameter(j, pUICtrl->dPresetData[m_nCurrentPreset]);
    }

    if(m_pRAFXPlugIn->m_bUseCustomVSTGUI && Globals()->GetNumberOfParameters() == nParams + numAddtlParams)
    {
        // add the Vector Joystick Controls
        //
        Globals()->SetParameter(nParams + vectorJoystickX_Offset, m_pRAFXPlugIn->m_JS_XCtrl.dPresetData[m_nCurrentPreset]);
        Globals()->SetParameter(nParams + vectorJoystickY_Offset, m_pRAFXPlugIn->m_JS_XCtrl.dPresetData[m_nCurrentPreset]);

    }

    // now do the Joystick!
    float* pJSProg = m_pRAFXPlugIn->m_PresetJSPrograms[m_nCurrentPreset];

    for(int i=0; i<MAX_JS_PROGRAM_STEPS; i++)
    {
        m_pRAFXPlugIn->m_pVectorJSProgram[JS_PROG_INDEX(i,0)] = pJSProg[JS_PROG_INDEX(i,0)];
        m_pRAFXPlugIn->m_pVectorJSProgram[JS_PROG_INDEX(i,1)] = pJSProg[JS_PROG_INDEX(i,1)];
        m_pRAFXPlugIn->m_pVectorJSProgram[JS_PROG_INDEX(i,2)] = pJSProg[JS_PROG_INDEX(i,2)];
        m_pRAFXPlugIn->m_pVectorJSProgram[JS_PROG_INDEX(i,3)] = pJSProg[JS_PROG_INDEX(i,3)];
        m_pRAFXPlugIn->m_pVectorJSProgram[JS_PROG_INDEX(i,4)] = pJSProg[JS_PROG_INDEX(i,4)];
        m_pRAFXPlugIn->m_pVectorJSProgram[JS_PROG_INDEX(i,5)] = pJSProg[JS_PROG_INDEX(i,5)];
        m_pRAFXPlugIn->m_pVectorJSProgram[JS_PROG_INDEX(i,6)] = pJSProg[JS_PROG_INDEX(i,6)];
    }

    return noErr;
}


#ifdef synthplugin
// --- Note On Event handler
OSStatus RackAFX2AUFilter::StartNote(MusicDeviceInstrumentID      inInstrument,
                                     MusicDeviceGroupID           inGroupID,
                                     NoteInstanceID*              outNoteInstanceID,
                                     UInt32                       inOffsetSampleFrame,
                                     const MusicDeviceNoteParams& inParams)
{
    if(m_pMIDIEventList)
    {
        MIDI_EVENT event = {0};
        event.uMessage = NOTE_ON;
        event.uChannel = (UINT)inGroupID;
        event.uData1 = (UINT)inParams.mPitch;
        event.uData2 = (UINT)inParams.mVelocity;
        event.uSampleOffset = inOffsetSampleFrame;
        m_pMIDIEventList->addEvent(event);
    }
    return noErr;
}

// --- Note Off handler
OSStatus RackAFX2AUFilter::StopNote(MusicDeviceGroupID inGroupID,
                                    NoteInstanceID     inNoteInstanceID,
                                    UInt32             inOffsetSampleFrame)
{
    if(m_pMIDIEventList)
    {
        MIDI_EVENT event = {0};
        event.uMessage = NOTE_OFF;
        event.uChannel = (UINT)inGroupID;
        event.uData1 = (UINT)inNoteInstanceID;
        event.uData2 = 0;
        event.uSampleOffset = inOffsetSampleFrame;
        m_pMIDIEventList->addEvent(event);
    }
    return noErr;
}

#elif defined(midifxplugin)
OSStatus RackAFX2AUFilter::HandleNoteOn(UInt8 	inChannel,
                                        UInt8 	inNoteNumber,
                                        UInt8 	inVelocity,
                                        UInt32  inStartFrame)
{
    if(m_pMIDIEventList)
    {
        MIDI_EVENT event = {0};
        event.uMessage = NOTE_ON;
        event.uChannel = (UINT)inChannel;
        event.uData1 = (UINT)inNoteNumber;
        event.uData2 = (UINT)inVelocity;
        event.uSampleOffset = inStartFrame;
        m_pMIDIEventList->addEvent(event);
    }
    return noErr;
}

OSStatus RackAFX2AUFilter::HandleNoteOff(UInt8 	inChannel,
                                         UInt8 	inNoteNumber,
                                         UInt8 	inVelocity,
                                         UInt32 inStartFrame)
{
    if(m_pMIDIEventList)
    {
        MIDI_EVENT event = {0};
        event.uMessage = NOTE_OFF;
        event.uChannel = (UINT)inChannel;
        event.uData1 = (UINT)inNoteNumber;
        event.uData2 = inVelocity;
        event.uSampleOffset = inStartFrame;
        m_pMIDIEventList->addEvent(event);
    }
    return noErr;
}
#endif

#if defined(synthplugin) || defined(midifxplugin)
// -- Pitch Bend handler
OSStatus RackAFX2AUFilter::HandlePitchWheel(UInt8  inChannel,
                                            UInt8  inPitch1,
                                            UInt8  inPitch2,
                                            UInt32 inStartFrame)
{
    // --- convert 14-bit concatentaion of inPitch1, inPitch2
    if(m_pMIDIEventList)
    {
        int nActualPitchBendValue = (int) ((inPitch1 & 0x7F) | ((inPitch2 & 0x7F) << 7));
        float fNormalizedPitchBendValue = (float) (nActualPitchBendValue - 0x2000) / (float) (0x2000);

        MIDI_EVENT event = {0};
        event.uMessage = PITCH_BEND;
        event.uChannel = (UINT)inChannel;
        event.nPitchBendValue = nActualPitchBendValue;
        event.fNormalizedPitchBendValue = fNormalizedPitchBendValue;
        event.uSampleOffset = inStartFrame;
        m_pMIDIEventList->addEvent(event);
    }
    return noErr;
}


/*
 // --- NOTE: Logic hooks the Volume and Pan controls
 // --- But since MIDI CC 7 and 10 (volume and pan respectively) are reserved by the main channel strip controls,
 //     it's best to use MIDI CC 11 (expression) to automate volume effects
 //     http://www.soundonsound.com/sos/apr08/articles/logictech_0408.htm
 //
 On some plugins and instruments, CC#11 does nothing but control volume. On other plugins/instruments, CC#11 is programmed to control volume and timbre (brightness) simultaneously. This is a feature of the programming of the plugin or instrument and not an inherent quality of CC#11 data. In such a case, higher CC#11 values make a sound both louder and brighter, and vice versa. If in fact your instruments respond to CC#11 only with a change in volume then you might as well not try and fight city hall: use CC#11 as your volume control.
 */
// --- CC handler
OSStatus RackAFX2AUFilter::HandleControlChange(UInt8  inChannel,
                                               UInt8  inController,
                                               UInt8  inValue,
                                               UInt32 inStartFrame)
{
    if(m_pMIDIEventList)
    {
        MIDI_EVENT event = {0};
        event.uSampleOffset = inStartFrame;

        if(inController == MOD_WHEEL)
        {
            event.uMessage = MOD_WHEEL;
            event.uChannel = (UINT)inChannel;
            event.uData1 = (UINT)inValue;
            m_pMIDIEventList->addEvent(event);
        }
        else if(inController == ALL_NOTES_OFF)
        {
            event.uMessage = ALL_NOTES_OFF;
            event.uChannel = (UINT)inChannel;
            event.uData1 = 0;
            event.uData2 = 0;
            m_pMIDIEventList->addEvent(event);
        }
        else if(m_pRAFXPlugIn->m_bWantAllMIDIMessages)
        {
            event.uMessage = CONTROL_CHANGE;
            event.uChannel = (UINT)inChannel;
            event.uData1 = (UINT)inController;
            event.uData2 = (UINT)inValue;
            m_pMIDIEventList->addEvent(event);
        }
    }
    return noErr;
}


OSStatus RackAFX2AUFilter::HandleMidiEvent(UInt8  status,
                                           UInt8  channel,
                                           UInt8  data1,
                                           UInt8  data2,
                                           UInt32 inStartFrame)
{
    if(m_pMIDIEventList)
    {
        MIDI_EVENT event = {0};
        if(m_pRAFXPlugIn->m_bWantAllMIDIMessages)
        {
            event.uMessage = (UINT)status;
            event.uChannel = (UINT)channel;
            event.uData1 = (UINT)data1;
            event.uData2 = (UINT)data2;
            event.uSampleOffset = inStartFrame;
            m_pMIDIEventList->addEvent(event);
        }
    }

    // --- call base class to do its thing
#if defined (midifxplugin)
    return AUMIDIEffectBase::HandleMidiEvent(status, channel, data1, data2, inStartFrame);
#else
   	return AUMIDIBase::HandleMidiEvent(status, channel, data1, data2, inStartFrame);
#endif
}
#endif

// #endif
//} // namespace VSTGUI4








