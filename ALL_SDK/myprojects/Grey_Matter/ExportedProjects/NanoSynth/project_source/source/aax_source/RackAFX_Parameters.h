#pragma once
#ifndef RACKAFX_PARAMETERS_H
#define RACKAFX_PARAMETERS_H

#include "AAX_CEffectParameters.h"
#pragma warning(disable : 4985)	// --- 'ceil': attributes not present on previous declaration NOTE: for VS2008 only, see the google for more info

#include "AAX_CAtomicQueue.h"
#include "AAX_IParameter.h"
#include "AAX_IMIDINode.h"
#include "AAX_IString.h"
#include "AAX_IEffectDescriptor.h"
#include "AAX_IComponentDescriptor.h"
#include "AAX_IPropertyMap.h"

#include <set>
#include <list>
#include <utility>

// --- the RackAFX plugin
#include "NanoSynth.h"

// --- defines
//#define WANT_SIDECHAIN
#define IS_SYNTH
//#define USE_CUSTOM_GUI

// --- see AAX_ITransport.h before enabling this: performance hits!
// #define ENABLE_EXTRA_HOST_INFO

// --- set by RAFX
const AAX_CTypeID cRAFX_ManufactureID =			'5687';
const AAX_CTypeID cRAFX_ProductID =				'5678';
const AAX_CTypeID cRAFX_TypeID_MonoNative =		'RxMN';
const AAX_CTypeID cRAFX_TypeID_StereoNative =	'RxST';
const unsigned int RAFX_AAX_Category = AAX_ePlugInCategory_HWGenerators;

// --- meters JAN
const unsigned int cMeterTapCount = 2;
const unsigned int cGRMeterL = 0;
const unsigned int cGRMeterR = 1;

const AAX_CTypeID cRAFX_MeterID[cMeterTapCount] = {'grML','grMR'};
const AAX_CTypeID RACKAFX_PLUGIN_ID = 0;

// --- setup context struct
class RackAFX_Parameters;

// --- struct for our parameter object which owns the plugin instance
struct rackAFXCustomData
{
	CPlugIn* pRackAFX_Plugin;
    int* pMeterMap;
};

struct rackAFXPrivateData
{
	RackAFX_Parameters*	pRackAFX_Parameters;
};

// --- processing struct
struct RackAFXAlgorithmStruct
{
	// --- audio
	rackAFXPrivateData*	pPrivateData;			// --- the RAFX Monolithic Parameters
	float**				ppInputs;				// --- inputs buffer
	float**				ppOutputs;				// --- outputs buffer
	int32_t*			pBufferLength;			// --- buffer size (per channel)
    float**             ppMeters;				// --- Meter taps // JAN

	#ifdef WANT_SIDECHAIN
	int32_t*			pSidechainChannel;		// --- sidechain channel pointer
	#endif

	// --- MIDI
	AAX_IMIDINode*		mInputNode;             // --- MIDI input node -> plugin
    AAX_IMIDINode*      mTransportNode;         // --- for getting info about the host BPM, etc...

	// --- params
	int64_t*			mCurrentStateNum;		// --- state value
};

#define kMaxAdditionalMIDINodes 15
#define kMaxAuxOutputStems 32
#define kSynchronizedParameterQueueSize 25 // set with RAFX [Make AAX]

// --- our dirty parameter list
typedef std::pair<AAX_CParamID const, const AAX_IParameterValue*> TParamValPair;
class AAXMIDIEventList;

//==============================================================================
class RackAFX_Parameters : public AAX_CEffectParameters
{
public:
	RackAFX_Parameters ();
	virtual ~RackAFX_Parameters(){if(m_pRAFXPlugIn) delete m_pRAFXPlugIn; DeleteUsedParameterChanges();}

	// --- creation function
	static AAX_CEffectParameters* AAX_CALLBACK Create();

	// --- grap param changes and add to defer list
	virtual AAX_Result UpdateParameterNormalizedValue (AAX_CParamID iParameterID, double iValue, AAX_EUpdateSource iSource );

	// --- see AAX_CMonolithicParameters in SDK
	void AddSynchronizedParameter(const AAX_IParameter& inParameter);

public:
	// --- Overrides from AAX_CEffectParameters
	virtual AAX_Result EffectInit();
	virtual AAX_Result ResetFieldData (AAX_CFieldIndex inFieldIndex, void * oData, uint32_t iDataSize) const;
	virtual AAX_Result GenerateCoefficients();
	virtual AAX_Result TimerWakeup();

	// --- static functions for processing and description; can not be distributed
	static void AAX_CALLBACK StaticRenderAudio(RackAFXAlgorithmStruct* const inInstancesBegin[], const void* inInstancesEnd);
	static AAX_Result StaticDescribe(AAX_IComponentDescriptor& outDesc);

	// --- data transfer with GUI
	virtual AAX_Result GetCustomData(AAX_CTypeID iDataBlockID, uint32_t iDataSize, void* oData, uint32_t* oDataWritten) const;


	// --- RackAFX plugin functions
	//
	// --- do the processing
	void ProcessAudio(RackAFXAlgorithmStruct* ioRenderInfo, const TParamValPair* inSynchronizedParamValues[], int32_t inNumSynchronizedParamValues);
	bool ProcessMIDI(uint32_t sampleIndex, const AAX_CMidiPacket*& ioPacketPtr, uint32_t& midiBuffersize, bool bProcessAllMIDI = false);

	// --- for MIDI over buffers (currently not used)
	int getNextMIDIBufferTimestamp(int nStartAfter, const AAX_CMidiPacket*& ioPacketPtr, uint32_t& midiBuffersize);

	// --- update dirty parameters
	void UpdatePluginParameters(const TParamValPair* inSynchronizedParamValues[], int32_t inNumSynchronizedParamValues);

    // --- outbound parameter set: meters
    void updateOutboundAAXParameters();

	// old VST2 function for safe strncpy()
	inline char* vst_strncpy(char* dst, const char* src, size_t maxLen)
	{
		char* result = strncpy(dst, src, maxLen);
		dst[maxLen] = 0;
		return result;
	}

	inline char* getEnumString(char* string, int index)
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

				// --- special support for 2-state switches
				if(strcmp(pType, "SWITCH_OFF") == 0)
				{
					delete [] pType;
					return "OFF";
				}
				else if(strcmp(pType, "SWITCH_ON") == 0)
				{
					delete [] pType;
					return "ON";
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

	// --- see AAX_CMonolithicParameters in SDK
	struct SParamValList
	{
		// Using 4x the preset queue size: the buffer must be large enough to accommodate the maximum
		// number of updates that we expect to be queued between/before executions of the render callback.
		// The maximum queuing that will likely ever occur is during a preset change (i.e. a call to
		// SetChunk()), in which updates to all parameters may be queued in the same state frame. It is
		// possible that the host would call SetChunk() on the plug-in more than once before the render
		// callback executes, but probably not more than 2-3x. Therefore 4x seems like a safe upper limit
		// for the capacity of this buffer.
		static const int32_t sCap = 4*kSynchronizedParameterQueueSize;

		TParamValPair* mElem[sCap];
		int32_t mSize;

		SParamValList()
		{
			Clear();
		}

		void Add(TParamValPair* inElem)
		{
			AAX_ASSERT(sCap > mSize);
			if (sCap > mSize)
			{
				mElem[mSize++] = inElem;
			}
		}

		void Append(const SParamValList& inOther)
		{
			AAX_ASSERT(sCap >= mSize + inOther.mSize);
			for (int32_t i = 0; i < inOther.mSize; ++i)
			{
				Add(inOther.mElem[i]);
			}
		}

		void Append(const std::list<TParamValPair*>& inOther)
		{
			AAX_ASSERT(sCap >= mSize + (int64_t)inOther.size());
			for (std::list<TParamValPair*>::const_iterator iter = inOther.begin(); iter != inOther.end(); ++iter)
			{
				Add(*iter);
			}
		}

		void Merge(AAX_IPointerQueue<TParamValPair>& inOther)
		{
			do
			{
				TParamValPair* const val = inOther.Pop();
				if (NULL == val) { break; }
				Add(val);
			} while (1);
		}

		void Clear() ///< Zeroes out the mElem array; does not destroy any elements
		{
			std::memset(mElem, 0x0, sizeof(mElem));
			mSize = 0;
		}
	};

	// --- see AAX_CMonolithicParameters in SDK
	typedef std::set<const AAX_IParameter*> TParamSet;
	typedef std::pair<int64_t, std::list<TParamValPair*> > TNumberedParamStateList;
	typedef AAX_CAtomicQueue<TNumberedParamStateList, 256> TNumberedStateListQueue;
	typedef AAX_CAtomicQueue<const TParamValPair, 16*kSynchronizedParameterQueueSize> TParamValPairQueue;
	SParamValList GetUpdatesForState(int64_t inTargetStateNum);
	void DeleteUsedParameterChanges();

private:
	std::set<std::string>					mSynchronizedParameters;
	int64_t									mStateCounter;
	TParamSet								mDirtyParameters;
	TNumberedStateListQueue 				mQueuedParameterChanges;
	TNumberedStateListQueue 				mFinishedParameterChanges; // Parameter changes ready for deletion
	TParamValPairQueue						mFinishedParameterValues; // Parameter values ready for deletion

// --- RackAFX stuff
private:
	rackAFXCustomData m_CustomData;
	CNanoSynth* m_pRAFXPlugIn;
	bool m_bSoftBypass;

    // --- for outbound meter mapping
    int m_nMeterMap[MAX_USER_METERS];
    int* m_pMeterMap;
    int getNextMeterMapSlot();
    void clearMeterMap();

    // --- for sample accurate MIDI events with buffer processing
    AAXMIDIEventList* m_pMidiEventList;
};

// --- event list for sample accurate midi events when using the VSTBuffer Processing function instead of processAudioFrame()
class AAXMIDIEventList : public IMidiEventList
{
protected:
    RackAFX_Parameters* m_pPlugIn;
    uint32_t m_MidiBuffersize;
    const AAX_CMidiPacket* m_pCurrentMidiPacket;

public:
    AAXMIDIEventList(RackAFX_Parameters* pPlugIn);
    virtual ~AAXMIDIEventList();
    void clearIncomingMIDIBuffer();
    void setIncomingMIDIBuffer(const AAX_CMidiPacket* ioPacketPtr, uint32_t midiBuffersize);

    // --- IMidiEventList
    unsigned int getEventCount();
    bool fireMidiEvent(unsigned int uSampleOffset);
};

#endif


