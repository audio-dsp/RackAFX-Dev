
#include "RackAFX_Parameters.h"
#include "AAX_CLinearTaperDelegate.h"
#include "AAX_CRafxLogTaperDelegate.h"
#include "AAX_CVoltOctaveTaperDelegate.h"
#include "AAX_CNumberDisplayDelegate.h"
#include "AAX_CDecibelDisplayDelegateDecorator.h"
#include "AAX_CBinaryTaperDelegate.h"
#include "AAX_CBinaryDisplayDelegate.h"
#include "AAX_CUnitDisplayDelegateDecorator.h"
#include "AAX_CStringDisplayDelegate.h"
#include "AAX_MIDIUtilities.h"
#include "AAX_Assert.h"
#include "AAX_CMutex.h" // --- not currently used

// --- RackAFX constants
const UINT FILTER_CONTROL_USER_VARIABLE				= 105;
const UINT FILTER_CONTROL_USER_VSTGUI_VARIABLE		= 106;
const UINT LATENCY_IN_SAMPLES						= 0;
const unsigned char MOD_WHEEL_DATA = 0x01;
const unsigned char PITCH_BEND_MSG = 0xE0;
const unsigned char CONTROL_CHANGE_MSG = 0xB0;


// --- creation function
AAX_CEffectParameters * AAX_CALLBACK RackAFX_Parameters::Create()
{
	return new RackAFX_Parameters();
}

RackAFX_Parameters::RackAFX_Parameters() :
	AAX_CEffectParameters()
{
	m_pRAFXPlugIn = NULL;
	m_pRAFXPlugIn = new CNanoSynth();
	m_bSoftBypass = false;
    m_pMeterMap = &m_nMeterMap[0]; // needed for const issue in GetCustomData( ) below...
}

AAX_Result RackAFX_Parameters::GetCustomData(AAX_CTypeID iDataBlockID, uint32_t iDataSize, void* oData, uint32_t* oDataWritten) const
{
    if(iDataBlockID == RACKAFX_PLUGIN_ID)
    {
        rackAFXCustomData* pData = (rackAFXCustomData*)oData;
        pData->pRackAFX_Plugin = m_pRAFXPlugIn;
        pData->pMeterMap = m_pMeterMap;

        *oDataWritten = sizeof(rackAFXCustomData*);
        return AAX_SUCCESS;
    }
    return AAX_ERROR_UNIMPLEMENTED;
}

// --- init the plugin parameters
AAX_Result RackAFX_Parameters::EffectInit()
{
    // --- bypass
    AAX_CString bypassID = cDefaultMasterBypassID;
    AAX_IParameter* masterBypass = new AAX_CParameter<bool>(
                                                            bypassID.CString(), AAX_CString("Master Bypass"), false,
                                                            AAX_CBinaryTaperDelegate<bool>(),
                                                            AAX_CBinaryDisplayDelegate<bool>("bypass", "on"), true);
    masterBypass->SetNumberOfSteps( 2 );
    masterBypass->SetType( AAX_eParameterType_Discrete );
    mParameterManager.AddParameter(masterBypass);
    AddSynchronizedParameter(*masterBypass);

    // --- Init parameters
    if(m_pRAFXPlugIn)
    {
        // --- chance to change the default control values
        m_pRAFXPlugIn->initialize();

        // --- sets -1 in all map slots
        clearMeterMap();

        // --- Sample Rate/Latency
        AAX_CSampleRate sampleRate = 44100;
        Controller()->GetSampleRate(&sampleRate);
        Controller()->SetSignalLatency(m_pRAFXPlugIn->m_fPlugInEx[LATENCY_IN_SAMPLES]);

        // --- init plugin
        m_pRAFXPlugIn->m_nSampleRate = (int)sampleRate;
        m_pRAFXPlugIn->prepareForPlay();

        // --- set pointer for GUI
        m_CustomData.pRackAFX_Plugin = m_pRAFXPlugIn;

        // --- iterate
        int nParams = m_pRAFXPlugIn->getControlCount();
        int nIndexer = 0; // not needed right now

        // --- iterate and set
        for(int i = 0; i < nParams; i++)
        {
            CUICtrl* pUICtrl = m_pRAFXPlugIn->getUICtrlByListIndex(i);

            if(pUICtrl)
            {
                float fDefault = 0.0;
                std::string sUnit = " ";
                sUnit.append(pUICtrl->cControlUnits);
                if(pUICtrl->uUserDataType == nonData)
                    sUnit = "";

                std::stringstream strAAX_ID; // use control index as "Unique ID"
                strAAX_ID << i + 1;

                // --- get the current value as default
                fDefault = m_pRAFXPlugIn->getParameterValue(i);

                // --- only for custmom GUIs
                if(pUICtrl->uControlType == FILTER_CONTROL_LED_METER)
                {
                    if(m_pRAFXPlugIn->m_bUseCustomVSTGUI)
                    {
                        AAX_CParameter<float>* param = new AAX_CParameter<float>(
                                                                                 strAAX_ID.str().c_str(),
                                                                                 AAX_CString(pUICtrl->cControlName),
                                                                                 fDefault,
                                                                                 AAX_CLinearTaperDelegate<float>(0.0, 1.0),
                                                                                 AAX_CUnitDisplayDelegateDecorator<float>(AAX_CNumberDisplayDelegate<float>(), sUnit.c_str()), false); // false = no automation
                        param->SetNumberOfSteps(128);
                        param->SetType(AAX_eParameterType_Continuous);
                        mParameterManager.AddParameter(param);
                        AddSynchronizedParameter(*param);
                        nIndexer++;

                        int nIndex = this->getNextMeterMapSlot();
                        if(nIndex >= 0)
                            m_nMeterMap[nIndex] = i; // location in parameters of meter (param - 1);
                    }
                }
                else if(pUICtrl->uControlType == FILTER_CONTROL_CONTINUOUSLY_VARIABLE ||
                        pUICtrl->uControlType == FILTER_CONTROL_RADIO_SWITCH_VARIABLE ||
                        pUICtrl->uControlType == FILTER_CONTROL_USER_VSTGUI_VARIABLE ||
                        pUICtrl->uControlType == FILTER_CONTROL_COMBO_VARIABLE)
                {
                    switch(pUICtrl->uUserDataType)
                    {
                        case floatData:
                        case doubleData:
                        {
                            if(pUICtrl->bLogSlider)
                            {
                                AAX_CParameter<float>* param = new AAX_CParameter<float>(strAAX_ID.str().c_str(),
                                                                                         AAX_CString(pUICtrl->cControlName),
                                                                                         fDefault,
                                                                                         AAX_CRafxLogTaperDelegate<float>(pUICtrl->fUserDisplayDataLoLimit, pUICtrl->fUserDisplayDataHiLimit),
                                                                                         AAX_CUnitDisplayDelegateDecorator<float>(AAX_CNumberDisplayDelegate<float>(), sUnit.c_str()), true);
                                param->SetNumberOfSteps(128);
                                param->SetType(AAX_eParameterType_Continuous);
                                mParameterManager.AddParameter(param);
                                AddSynchronizedParameter(*param);
                                nIndexer++;

                                break;
                            }
                            else if(pUICtrl->bExpSlider)
                            {
                                AAX_CParameter<float>* param = new AAX_CParameter<float>(
                                                                                         strAAX_ID.str().c_str(),
                                                                                         AAX_CString(pUICtrl->cControlName),
                                                                                         fDefault,
                                                                                         AAX_CVoltOctaveTaperDelegate<float>(pUICtrl->fUserDisplayDataLoLimit, pUICtrl->fUserDisplayDataHiLimit),
                                                                                         AAX_CUnitDisplayDelegateDecorator<float>(AAX_CNumberDisplayDelegate<float>(), sUnit.c_str()), true);
                                param->SetNumberOfSteps(128);
                                param->SetType(AAX_eParameterType_Continuous);
                                mParameterManager.AddParameter(param);
                                AddSynchronizedParameter(*param);
                                nIndexer++;

                                break;
                            }
                            else
                            {
                                AAX_CParameter<float>* param = new AAX_CParameter<float>(strAAX_ID.str().c_str(),
                                                                                         AAX_CString(pUICtrl->cControlName),
                                                                                         fDefault,
                                                                                         AAX_CLinearTaperDelegate<float>(pUICtrl->fUserDisplayDataLoLimit, pUICtrl->fUserDisplayDataHiLimit),
                                                                                         AAX_CUnitDisplayDelegateDecorator<float>(AAX_CNumberDisplayDelegate<float>(), sUnit.c_str()), true);
                                param->SetNumberOfSteps(128);
                                param->SetType(AAX_eParameterType_Continuous);
                                mParameterManager.AddParameter(param);
                                AddSynchronizedParameter(*param);
                                nIndexer++;

                                break;
                            }
                        }

                        case intData:
                        {
                            if(pUICtrl->bLogSlider)
                            {
                                AAX_CParameter<int32_t>* param = new AAX_CParameter<int32_t>(strAAX_ID.str().c_str(),
                                                                                             AAX_CString(pUICtrl->cControlName),
                                                                                             fDefault,
                                                                                             AAX_CRafxLogTaperDelegate<int32_t>(pUICtrl->fUserDisplayDataLoLimit, pUICtrl->fUserDisplayDataHiLimit),
                                                                                             AAX_CUnitDisplayDelegateDecorator<int32_t>(AAX_CNumberDisplayDelegate<int32_t>(), sUnit.c_str()), true);
                                param->SetNumberOfSteps(128);
                                param->SetType(AAX_eParameterType_Continuous);
                                mParameterManager.AddParameter(param);
                                AddSynchronizedParameter(*param);
                                nIndexer++;

                                break;
                            }
                            else if(pUICtrl->bExpSlider)
                            {
                                AAX_CParameter<int32_t>* param = new AAX_CParameter<int32_t>(strAAX_ID.str().c_str(),
                                                                                             AAX_CString(pUICtrl->cControlName),
                                                                                             fDefault,
                                                                                             AAX_CVoltOctaveTaperDelegate<int32_t>(pUICtrl->fUserDisplayDataLoLimit, pUICtrl->fUserDisplayDataHiLimit),
                                                                                             AAX_CUnitDisplayDelegateDecorator<int32_t>(AAX_CNumberDisplayDelegate<int32_t>(), sUnit.c_str()), true);
                                param->SetNumberOfSteps(128);
                                param->SetType(AAX_eParameterType_Continuous);
                                mParameterManager.AddParameter(param);
                                AddSynchronizedParameter(*param);
                                nIndexer++;

                                break;
                            }
                            else
                            {
                                AAX_CParameter<int32_t>* param = new AAX_CParameter<int32_t>(strAAX_ID.str().c_str(),
                                                                                             AAX_CString(pUICtrl->cControlName),
                                                                                             fDefault,
                                                                                             AAX_CLinearTaperDelegate<int32_t>(pUICtrl->fUserDisplayDataLoLimit, pUICtrl->fUserDisplayDataHiLimit),
                                                                                             AAX_CUnitDisplayDelegateDecorator<int32_t>(AAX_CNumberDisplayDelegate<int32_t>(), sUnit.c_str()), true);
                                param->SetNumberOfSteps(128);
                                param->SetType(AAX_eParameterType_Continuous);
                                mParameterManager.AddParameter(param);
                                AddSynchronizedParameter(*param);
                                nIndexer++;

                                break;
                            }
                        }

                        case UINTData:
                        {
                            int nEnumStrings = pUICtrl->fUserDisplayDataHiLimit + 1;
                            std::map<int32_t, AAX_CString> enumStrings;

                            for (int j=0; j<nEnumStrings; j++)
                            {
                                const char* text = getEnumString(pUICtrl->cEnumeratedList, j);
                                enumStrings.insert(std::pair<int32_t, AAX_CString>(j, AAX_CString(text)) );
                            }

							// NOTE: this linear taper uses the hidden RealPrecision of 1 to force integer rounding: AAX_CLinearTaperDelegate<int32_t,1> the ",1" see the AAX_CLinearTaperDelegate declaration
                            AAX_CParameter<int32_t>* param = new AAX_CParameter<int32_t>(strAAX_ID.str().c_str(),
                            															 AAX_CString(pUICtrl->cControlName),
                            															 fDefault,
                                                                                         AAX_CLinearTaperDelegate<int32_t,1>(pUICtrl->fUserDisplayDataLoLimit, pUICtrl->fUserDisplayDataHiLimit),
                                                                                         AAX_CStringDisplayDelegate<int32_t>(enumStrings),
                                                                                         true);

                            param->SetNumberOfSteps(pUICtrl->fUserDisplayDataHiLimit+1); // NOTE: #of steps = fUserDisplayDataHiLimit + 1
                            param->SetType(AAX_eParameterType_Discrete);
                            mParameterManager.AddParameter(param);
                            AddSynchronizedParameter(*param);
                            nIndexer++;

                            break;
                        }

                        default:
                            break;
                    }
                }
            }// if pUICtrl
        } // for(int i = 0; i < nParams; i++)

        // --- vector joystick controls (only available with custom GUI)
        if(m_pRAFXPlugIn->m_bUseCustomVSTGUI)
        {
            float fDefault = 0.0;
            std::string sUnit = "";
            std::stringstream strAAX_IDJSX;

            strAAX_IDJSX << nParams + vectorJoystickX_Offset + 1;
            AAX_CParameter<float>* JSXParam = new AAX_CParameter<float>(strAAX_IDJSX.str().c_str(),
                                                                        AAX_CString("Vector JS X"),
                                                                        fDefault,
                                                                        AAX_CLinearTaperDelegate<float>(-1.0, 1.0),
                                                                        AAX_CUnitDisplayDelegateDecorator<float>(AAX_CNumberDisplayDelegate<float>(), sUnit.c_str()),
                                                                        true);
            JSXParam->SetNumberOfSteps(128);
            JSXParam->SetType(AAX_eParameterType_Continuous);
            mParameterManager.AddParameter(JSXParam);
            AddSynchronizedParameter(*JSXParam);
            nIndexer++;

            std::stringstream strAAX_IDJSY;
            strAAX_IDJSY << nParams + vectorJoystickY_Offset + 1;
            AAX_CParameter<float>* JSYParam = new AAX_CParameter<float>(strAAX_IDJSY.str().c_str(),
                                                                        AAX_CString("Vector JS Y"),
                                                                        fDefault,
                                                                        AAX_CLinearTaperDelegate<float>(-1.0, 1.0),
                                                                        AAX_CUnitDisplayDelegateDecorator<float>(AAX_CNumberDisplayDelegate<float>(), sUnit.c_str()),
                                                                        true);
            JSYParam->SetNumberOfSteps(128);
            JSYParam->SetType(AAX_eParameterType_Continuous);
            mParameterManager.AddParameter(JSYParam);
            AddSynchronizedParameter(*JSYParam);
            nIndexer++;
        }

    } // if m_pRAFXPlugIn

    // --- sample accurate MIDI events when using processVSTBuffer() -- processAudioFrame() is *already* sample accurate
    m_pMidiEventList = new AAXMIDIEventList(this);
    PROCESS_INFO processInfo = { 0 };
    processInfo.pIMidiEventList = m_pMidiEventList;

    // --- hand it to the plugin
    m_pRAFXPlugIn->processRackAFXMessage(midiEventList, processInfo);

    return AAX_SUCCESS;
}

// --- called once per buffer process operation to update any parameters that changed during the buffer fill
void RackAFX_Parameters::UpdatePluginParameters(const TParamValPair* inSynchronizedParamValues[], int32_t inNumSynchronizedParamValues)
{
    if(!m_pRAFXPlugIn) return;
    int nParams = m_pRAFXPlugIn->getControlCount();

    for(int32_t i = 0; i < inNumSynchronizedParamValues; ++i)
    {
        AAX_CParamID const iParameterID = inSynchronizedParamValues[i]->first;
        const AAX_IParameter* const param = mParameterManager.GetParameterByID(iParameterID);
        int n = atoi(iParameterID) - 1; // --- first param is ALWAYS bypass

        float fNormalizedValue = param->GetNormalizedValue();
        float fFloatValue = 0.f;
        int32_t nIntValue = 0;
        bool bsuccess = param->GetValueAsFloat(&fFloatValue);

        if(n < nParams)
        {
            // --- NOTE: using this method of passing actual values so that log and volt/octave tapers work properly
            //           and sync'ed to GUI
            //
            // --- float/double
            if(bsuccess)
                m_pRAFXPlugIn->setParameterValue(n, fFloatValue);
            else
            {
                // --- int/UINT
                bsuccess = param->GetValueAsInt32(&nIntValue);
                if(bsuccess)
                    m_pRAFXPlugIn->setParameterValue(n, (float)nIntValue);
            }
        }
        else if(n == nParams + vectorJoystickX_Offset)
        {
            m_pRAFXPlugIn->setVectorJSXValue(2.0f*fNormalizedValue - 1.0f);
        }
        else if(n == nParams + vectorJoystickY_Offset)
        {
            m_pRAFXPlugIn->setVectorJSYValue(2.0f*fNormalizedValue - 1.0f);
        }
    }

}


// --- currently not used
int RackAFX_Parameters::getNextMIDIBufferTimestamp(int nStartAfter, const AAX_CMidiPacket*& ioPacketPtr, uint32_t& midiBuffersize)
{
	for(int i=0; i<midiBuffersize; i++)
	{
		if(ioPacketPtr->mTimestamp > nStartAfter)
			return ioPacketPtr->mTimestamp;

		ioPacketPtr++;
	}

	return -1;
}

// --- process per-sample MIDI data
bool RackAFX_Parameters::ProcessMIDI(uint32_t sampleIndex, const AAX_CMidiPacket*& ioPacketPtr, uint32_t& midiBuffersize, bool bProcessAllMIDI)
{
    bool bProcessedMIDIEvent = false;
    if(!m_pRAFXPlugIn) return bProcessedMIDIEvent;

    while( (midiBuffersize > 0) && (NULL != ioPacketPtr) && ((ioPacketPtr->mTimestamp <= sampleIndex) || bProcessAllMIDI))
	{
	   	if(ioPacketPtr->mTimestamp == sampleIndex || bProcessAllMIDI)
    	{
			const uint8_t uMessage = (ioPacketPtr->mData[0] & 0xF0);	// message
			const uint8_t uChannel = (ioPacketPtr->mData[0] & 0x0F);	// channel

			if(AAX::IsNoteOn(ioPacketPtr))
			{
				m_pRAFXPlugIn->midiNoteOn((unsigned int)uChannel, (unsigned int)ioPacketPtr->mData[1], (unsigned int)ioPacketPtr->mData[2]);
				bProcessedMIDIEvent = true;
			}
			else if(AAX::IsNoteOff(ioPacketPtr))
			{
				m_pRAFXPlugIn->midiNoteOff((unsigned int)uChannel, (unsigned int)ioPacketPtr->mData[1], (unsigned int)ioPacketPtr->mData[2], false);
				bProcessedMIDIEvent = true;
			}
			else if(AAX::IsAllNotesOff(ioPacketPtr))
			{
				m_pRAFXPlugIn->midiNoteOff((unsigned int)uChannel, 0, 0, true);
				bProcessedMIDIEvent = true;
			}
			else if(uMessage == CONTROL_CHANGE_MSG && (unsigned int)ioPacketPtr->mData[1] == MOD_WHEEL_DATA)
			{
				m_pRAFXPlugIn->midiModWheel((unsigned int)uChannel, (unsigned int)ioPacketPtr->mData[2]);
				bProcessedMIDIEvent = true;
			}
			else if(uMessage == PITCH_BEND_MSG)
			{
				UINT uLSB7 = (unsigned int)ioPacketPtr->mData[1];
				UINT uMSB7 = (unsigned int)ioPacketPtr->mData[2];

				unsigned short shValue = (unsigned short)uLSB7;  // 0xxx xxxx
				unsigned short shMSPart = (unsigned short)uMSB7; // 0yyy yyyy
				unsigned short shMSPartShift = shMSPart<<7;

				shValue = shValue | shMSPartShift;
				int nPitchBend = (int)shValue - 8192.0;

				int nNormPB = nPitchBend;
				if(nNormPB == -8192)
					nNormPB = -8191;

				float fPitchBend = (float)nNormPB/8191.0; // -1.0 -> 1.0
				m_pRAFXPlugIn->midiPitchBend((unsigned int)uChannel, nPitchBend, fPitchBend);
				bProcessedMIDIEvent = true;
			}
			else if(m_pRAFXPlugIn->m_bWantAllMIDIMessages)
			{
				m_pRAFXPlugIn->midiMessage( (unsigned int)uChannel,
											(unsigned int)uMessage,
											(unsigned int)ioPacketPtr->mData[1],
											(unsigned int)ioPacketPtr->mData[2] );
				bProcessedMIDIEvent = true;
			}
		}
		++ioPacketPtr;
		--midiBuffersize;
	}

    return bProcessedMIDIEvent;
}


// --- the process function
void RackAFX_Parameters::ProcessAudio(RackAFXAlgorithmStruct* ioRenderInfo, const TParamValPair* inSynchronizedParamValues[], int32_t inNumSynchronizedParamValues)
{
    // --- global param update for dirty params
    UpdatePluginParameters(inSynchronizedParamValues, inNumSynchronizedParamValues);

    // --- MIDI input (all RAFX plugins can have MIDI in)
    AAX_IMIDINode* const midiInNode = ioRenderInfo->mInputNode;
    AAX_IMIDINode* const transportNode = ioRenderInfo->mTransportNode;
    AAX_CMidiStream* const midiBuffer = midiInNode->GetNodeBuffer();
    const AAX_CMidiPacket* currentMidiPacket = midiBuffer->mBuffer;
    uint32_t midiBufferSize = midiBuffer->mBufferSize;

    AAX_ITransport* const midiTransport = transportNode->GetTransport();

    // --- audio input
    const int32_t buffersize = *(ioRenderInfo->pBufferLength);
    AAX_EStemFormat inputStemFormat = AAX_eStemFormat_None;
    AAX_EStemFormat outputStemFormat = AAX_eStemFormat_None;
    Controller()->GetInputStemFormat(&inputStemFormat);
    Controller()->GetOutputStemFormat(&outputStemFormat);
    int32_t nLatencySamples = 0;

    Controller()->GetHybridSignalLatency(&nLatencySamples);

    const int32_t numChannelsIn = AAX_STEM_FORMAT_CHANNEL_COUNT(inputStemFormat);
    const int32_t numChannelsOut = AAX_STEM_FORMAT_CHANNEL_COUNT(outputStemFormat);

    // --- host information
    double dTempo = 120.0;
    int64_t nAbsoluteSampleLocation = 0;
    int64_t nTimelineStartLocation = 0;
    bool bIsPlaying = false;
    int32_t nTSNumerator = 0;
    int32_t nTSDenominator = 0;
    double dSampleInterval = 44100.0;
    bool bLooping = false;
    int64_t nLoopStartTick = 0;
    int64_t nLoopEndTick = 0;
    int64_t nTickPosition = 0;

    // --- check flag
    midiTransport->IsTransportPlaying(&bIsPlaying);
    midiTransport->GetCurrentTempo(&dTempo);
    midiTransport->GetCurrentNativeSampleLocation(&nAbsoluteSampleLocation); // notoriously incorrect
    midiTransport->GetTimelineSelectionStartPosition(&nTimelineStartLocation);
    midiTransport->GetCurrentMeter(&nTSNumerator, &nTSDenominator);
    midiTransport->GetCurrentTickPosition(&nTickPosition);
    midiTransport->GetCurrentLoopPosition (&bLooping, &nLoopStartTick, &nLoopEndTick);

    // --- see the documentation in AAX_ITransport.h about these - if you decide to use them, they need to be relocated to a non-realtime thread!
#ifdef ENABLE_EXTRA_HOST_INFO // by default this is NOT defined because of the performance hit you take with these extra functions; use at your own risk and/or relocate to a non-realtime thread
    int32_t nBars = 0;
    int32_t nBeats = 0;
    int64_t nDisplayTicks = 0;
    int64_t nCustomTickPosition = 0;

    // --- There is a minor performance cost associated with using this API in Pro Tools. It should NOT be used excessively without need
    midiTransport->GetBarBeatPosition(&nBars, &nBeats, &nDisplayTicks, nAbsoluteSampleLocation);

    // --- There is a minor performance cost associated with using this API in Pro Tools. It should NOT be used excessively without need
    midiTransport->GetCustomTickPosition(&nCustomTickPosition, nAbsoluteSampleLocation);
#endif

    HOST_INFO hostInfo = {0};
    hostInfo.dBPM = dTempo;
    hostInfo.fTimeSigNumerator = (float)nTSNumerator;
    hostInfo.uTimeSigDenomintor = (unsigned int)nTSDenominator;
    hostInfo.uAbsoluteFrameBufferIndex = (unsigned int)nAbsoluteSampleLocation;
    hostInfo.bIsPlayingAAX = bIsPlaying;
    hostInfo.nTickPosition = nTickPosition;	// --- "Tick" is represented here as 1/960000 of a quarter note
    hostInfo.bLooping = bLooping;
    hostInfo.nLoopStartTick = nLoopStartTick;
    hostInfo.nLoopEndTick =nLoopEndTick;

    PROCESS_INFO processInfo = {0};
    processInfo.nNumParams = -1; // not valid for this message
    processInfo.pHostInfo = &hostInfo;

    float* const AAX_RESTRICT pMeters = *ioRenderInfo->ppMeters;

#ifdef WANT_SIDECHAIN
    const int32_t sidechainChannel = *ioRenderInfo->pSidechainChannel;
#else
    const int32_t sidechainChannel = 0;
#endif

    float* sideChainInput = 0;

    // --- sidechain info
    audioProcessData auxInputProcessData;
    auxInputProcessData.uInputBus = 1;
    auxInputProcessData.bInputEnabled = false;
    auxInputProcessData.uNumInputChannels = 1; // AAX is mono SC only
    auxInputProcessData.uBufferSize = 0;
    auxInputProcessData.pFrameInputBuffer = NULL;
    auxInputProcessData.pRAFXInputBuffer = NULL;
    auxInputProcessData.ppVSTInputBuffer = NULL;

    // --- for RackAFX processAudioFrame()
    float inputs[2] = {0};
    float outputs[2] = {0};

    if(!m_pRAFXPlugIn || (m_bSoftBypass || (numChannelsIn != 1 && numChannelsIn != 2)))
    {
        for (int ch = 0; ch < numChannelsIn; ch++)
        {
            const float* const AAX_RESTRICT pInputBuffer = ioRenderInfo->ppInputs[ch];
            float* const AAX_RESTRICT pOutputBuffer = ioRenderInfo->ppOutputs[ch];

            for (int t = 0; t < buffersize; t ++)
            {
                pOutputBuffer[t] = pInputBuffer[t];
            }
        }
        return;
    }

    // --- for sample accurate MIDI with processVSTBuffer()
    if(m_pMidiEventList)
        m_pMidiEventList->setIncomingMIDIBuffer(currentMidiPacket, midiBufferSize);

    // --- ok to set now
    dSampleInterval = 1.0/(double)m_pRAFXPlugIn->m_nSampleRate;
    hostInfo.dAbsoluteFrameBufferTime = (double)nAbsoluteSampleLocation*dSampleInterval;

    // --- set host info for top of buffer
    m_pRAFXPlugIn->processRackAFXMessage(updateHostInfo, processInfo);

    if(m_pRAFXPlugIn->m_uPlugInEx[ENABLE_SIDECHAIN_VSTAU])
    {
        if(sidechainChannel)
        {
            auxInputProcessData.bInputEnabled = true;
            auxInputProcessData.uBufferSize = buffersize;
            sideChainInput = ioRenderInfo->ppInputs[sidechainChannel];
        }
        else // JAN FIXX
        {
            auxInputProcessData.bInputEnabled = false;
            auxInputProcessData.uBufferSize = 0;
            sideChainInput = NULL;
        }
    }

    if(m_pRAFXPlugIn->m_bWantVSTBuffers)
    {
        if(sideChainInput)
        {
            auxInputProcessData.ppVSTInputBuffer = &sideChainInput; //** to sidechain
            m_pRAFXPlugIn->processAuxInputBus(&auxInputProcessData);
        }

        // --- process VST Buffer
        m_pRAFXPlugIn->processVSTAudioBuffer(&ioRenderInfo->ppInputs[0], &ioRenderInfo->ppOutputs[0], numChannelsIn, buffersize);

        // --- for sample accurate MIDI with processVSTBuffer()
        if(m_pMidiEventList)
            m_pMidiEventList->clearIncomingMIDIBuffer(); // no ovehead for this

        // --- METER EXAMPLE: loop over outut buffer to find max value for this buffer-run JAN
        float* const AAX_RESTRICT pOutputBuffer = ioRenderInfo->ppOutputs[0];
        for (int t = 0; t < buffersize; t++)
        {
            // --- gain reduction meters
            pMeters[cGRMeterL] = (std::max)(fabsf(pOutputBuffer[t]), pMeters[cGRMeterL]);
            pMeters[cGRMeterR] = pMeters[cGRMeterR];
        }
        if(numChannelsOut == 2)
        {
            float* const AAX_RESTRICT pOutputBuffer = ioRenderInfo->ppOutputs[1];
            for (int t = 0; t < buffersize; t++)
            {
                // --- gain reduction meters
                pMeters[cGRMeterR] = (std::max)(fabsf(pOutputBuffer[t]), pMeters[cGRMeterR]);
            }
        }

   		// --- update outbound parameters
    	updateOutboundAAXParameters();

        return;
    }
    else
    {
        if(numChannelsIn == 1)
        {
            const float* const AAX_RESTRICT pInputBuffer = ioRenderInfo->ppInputs[0];
            float* const AAX_RESTRICT pOutputBuffer = ioRenderInfo->ppOutputs[0];

            for (int t = 0; t < buffersize; t ++)
            {
                inputs[0] =  pInputBuffer[t];
                outputs[0] = 0.0;
                if(sideChainInput)
                {
                    auxInputProcessData.pFrameInputBuffer = &sideChainInput[t]; //* to sidechain
                    m_pRAFXPlugIn->processAuxInputBus(&auxInputProcessData);
                }

                // --- process MIDI events for this sample offset
                ProcessMIDI(t, currentMidiPacket, midiBufferSize);

                // --- process RAFX Frame
                m_pRAFXPlugIn->processAudioFrame(&inputs[0], &outputs[0], numChannelsIn, numChannelsOut);

                // --- write out
                pOutputBuffer[t] = outputs[0];
            }

            // --- METER EXAMPLE: loop over outut buffer to find max value for this buffer-run JAN
            for (int t = 0; t < buffersize; t++)
            {
                // --- gain reduction meters
                pMeters[cGRMeterL] = (std::max)(fabsf(pOutputBuffer[t]), pMeters[cGRMeterL]);
                pMeters[cGRMeterR] = pMeters[cGRMeterR];
            }
        }
        else if(numChannelsIn == 2)
        {
            const float* const AAX_RESTRICT pLeftInputBuffer = ioRenderInfo->ppInputs[0];
            float* const AAX_RESTRICT pLeftOutputBuffer = ioRenderInfo->ppOutputs[0];

            const float* const AAX_RESTRICT pRightInputBuffer = ioRenderInfo->ppInputs[1];
            float* const AAX_RESTRICT pRightOutputBuffer = ioRenderInfo->ppOutputs[1];

            for (int t = 0; t < buffersize; t ++)
            {
                // --- setup RAFX frames
                inputs[0] =  pLeftInputBuffer[t];
                inputs[1] =  pRightInputBuffer[t];

                outputs[0] = 0.0;
                outputs[1] = 0.0;

                if(m_pRAFXPlugIn->m_uPlugInEx[ENABLE_SIDECHAIN_VSTAU])// JAN FIXX
                {
                    auxInputProcessData.pFrameInputBuffer = &sideChainInput[t]; //** to sidechain
                    m_pRAFXPlugIn->processAuxInputBus(&auxInputProcessData);
                }

                // --- process MIDI events for this sample offset
                ProcessMIDI(t, currentMidiPacket, midiBufferSize);

                // --- process RAFX Frame
                m_pRAFXPlugIn->processAudioFrame(&inputs[0], &outputs[0], numChannelsIn, numChannelsOut);

                // --- write out
                pLeftOutputBuffer[t] = outputs[0];
                pRightOutputBuffer[t] = outputs[1];
            }

            // --- METER EXAMPLE: loop over outut buffer to find max value for this buffer-run JAN
            for (int t = 0; t < buffersize; t++)
            {
                pMeters[cGRMeterL] = (std::max)(fabsf(pLeftOutputBuffer[t]), pMeters[cGRMeterL]);
                pMeters[cGRMeterR] = (std::max)(fabsf(pRightOutputBuffer[t]), pMeters[cGRMeterR]);
            }

            hostInfo.uAbsoluteFrameBufferIndex++;
            hostInfo.dAbsoluteFrameBufferTime = (double)hostInfo.uAbsoluteFrameBufferIndex/(double)m_pRAFXPlugIn->m_nSampleRate;

            // --- set host info for bottom of this frame
            m_pRAFXPlugIn->processRackAFXMessage(updateHostInfo, processInfo);
        }
    }

    // --- update outbound parameters
    updateOutboundAAXParameters();
}

// --- the render callback; static
void AAX_CALLBACK RackAFX_Parameters::StaticRenderAudio(RackAFXAlgorithmStruct* const	inInstancesBegin [], const void* inInstancesEnd)
{
	for (RackAFXAlgorithmStruct * const * instanceRenderInfoPtr = inInstancesBegin; instanceRenderInfoPtr != inInstancesEnd; ++instanceRenderInfoPtr)
	{
		rackAFXPrivateData* privateData = (*instanceRenderInfoPtr)->pPrivateData;
		if(privateData != 0)
		{
			// --- Grab the object pointer from the Context
			RackAFX_Parameters*	parameters = privateData->pRackAFX_Parameters;
			if (parameters != 0)
			{
				// --- Update synchronized parameters to the target state num and get a queue of parameter value changes
				SParamValList paramValList = parameters->GetUpdatesForState(*(*instanceRenderInfoPtr)->mCurrentStateNum);
				parameters->ProcessAudio(*instanceRenderInfoPtr, (const TParamValPair**)paramValList.mElem, paramValList.mSize);

				// --- Queue the parameter value pairs for later deletion
				for (int32_t i = 0; i < paramValList.mSize; ++i)
				{
					const AAX_IContainer::EStatus pushResult = parameters->mFinishedParameterValues.Push(paramValList.mElem[i]);
					AAX_ASSERT(AAX_IContainer::eStatus_Success == pushResult);
				}
			}
		}
	}
}

// --- static description
AAX_Result RackAFX_Parameters::StaticDescribe(AAX_IComponentDescriptor& outDesc)
{
	AAX_Result err = AAX_SUCCESS;

	// --- AUDIO
	err |= outDesc.AddAudioIn(AAX_FIELD_INDEX (RackAFXAlgorithmStruct, ppInputs));
	err |= outDesc.AddAudioOut(AAX_FIELD_INDEX (RackAFXAlgorithmStruct, ppOutputs));
	err |= outDesc.AddAudioBufferLength( AAX_FIELD_INDEX (RackAFXAlgorithmStruct, pBufferLength) );

	// --- MIDI
	err |= outDesc.AddMIDINode(AAX_FIELD_INDEX(RackAFXAlgorithmStruct, mInputNode), AAX_eMIDINodeType_LocalInput, "NanoSynth MIDI In", 0xffff); // 0xffff = mask
    err |= outDesc.AddMIDINode(AAX_FIELD_INDEX(RackAFXAlgorithmStruct, mTransportNode), AAX_eMIDINodeType_Transport, "Transport", 0xffff); // 0xffff = mask

	// --- private data
	err |= outDesc.AddPrivateData(AAX_FIELD_INDEX(RackAFXAlgorithmStruct, pPrivateData), sizeof(rackAFXPrivateData), AAX_ePrivateDataOptions_DefaultOptions);
	err |= outDesc.AddDataInPort(AAX_FIELD_INDEX(RackAFXAlgorithmStruct, mCurrentStateNum), sizeof(uint64_t));

    // --- meters JAN
    err |= outDesc.AddMeters (AAX_FIELD_INDEX(RackAFXAlgorithmStruct, ppMeters), cRAFX_MeterID, cMeterTapCount);
    AAX_ASSERT (err == AAX_SUCCESS);

	// --- setup properties
	AAX_IPropertyMap* properties = outDesc.NewPropertyMap();
	AAX_ASSERT(properties);
	if(!properties) return err;

	// --- depends on RAFX
#ifndef USE_CUSTOM_GUI
	properties->AddProperty(AAX_eProperty_UsesClientGUI, true); 	// Register for auto-GUI
#endif

    // --- initial latency setting
    properties->AddProperty(AAX_eProperty_LatencyContribution, 0);

	// --- if sidechaining...
	#ifdef WANT_SIDECHAIN
		err |= outDesc.AddSideChainIn(AAX_FIELD_INDEX(RackAFXAlgorithmStruct, pSidechainChannel));
		err |= properties->AddProperty(AAX_eProperty_SupportsSideChainInput, true );
	#endif

    // --- transport node
    err |= properties->AddProperty(AAX_eProperty_UsesTransport, true);

	return err;
}

// --- this is called when parameters change; add dirty params to list
AAX_Result RackAFX_Parameters::GenerateCoefficients()
{
	AAX_Result result = AAX_CEffectParameters::GenerateCoefficients();
	if (AAX_SUCCESS != result) { return result; }

	const int64_t stateNum = mStateCounter++;

	// --- Check for dirty parameters
	TNumberedParamStateList::second_type paramStateList;
	while (false == mDirtyParameters.empty())
	{
		TParamSet::iterator paramIter = mDirtyParameters.begin();
		const AAX_IParameter* const param = *paramIter;
		mDirtyParameters.erase(param);
		if (NULL != param)
		{
			paramStateList.push_back(new TParamValPair(param->Identifier(), param->CloneValue()));
		}
	}

	if (false == paramStateList.empty())
	{
		TNumberedParamStateList* const numberedParamState = new TNumberedParamStateList(std::make_pair(stateNum, paramStateList));
		{
			const AAX_IContainer::EStatus pushResult = mQueuedParameterChanges.Push(numberedParamState);
			AAX_ASSERT(AAX_IContainer::eStatus_Success == pushResult);
		}
	}

	result = Controller()->PostPacket(AAX_FIELD_INDEX(RackAFXAlgorithmStruct, mCurrentStateNum), &stateNum, sizeof(int64_t));

	return result;
}

// --- see AAX_CMonolithicParameters in SDK
void RackAFX_Parameters::AddSynchronizedParameter(const AAX_IParameter& inParameter)
{
	mSynchronizedParameters.insert(inParameter.Identifier());
	AAX_ASSERT(inParameter.Automatable());
	AAX_ASSERT(kSynchronizedParameterQueueSize >= mSynchronizedParameters.size());
}

// --- called for param changes
AAX_Result RackAFX_Parameters::UpdateParameterNormalizedValue (AAX_CParamID iParameterID, double iValue, AAX_EUpdateSource iSource )
{
	AAX_Result result = AAX_CEffectParameters::UpdateParameterNormalizedValue(iParameterID, iValue, iSource);
	if (AAX_SUCCESS != result) { return result; }

	if(AAX::IsParameterIDEqual("MasterBypassID", iParameterID))
	{
		m_bSoftBypass = iValue;
		return AAX_SUCCESS;
	}

	const AAX_IParameter* const param = mParameterManager.GetParameterByID(iParameterID);

	// --- add to dirty param list
	if ((param) && (0 < mSynchronizedParameters.count(iParameterID)))
	{
		mDirtyParameters.insert(param);
	}

	return result;
}

// --- setup our private data
AAX_Result RackAFX_Parameters::ResetFieldData (AAX_CFieldIndex inFieldIndex, void * oData, uint32_t iDataSize) const
{
	if (inFieldIndex == AAX_FIELD_INDEX(RackAFXAlgorithmStruct, pPrivateData) )
	{
		AAX_ASSERT( iDataSize == sizeof(rackAFXPrivateData) );
		memset(oData, 0, iDataSize);

		rackAFXPrivateData* privateData = static_cast <rackAFXPrivateData*> (oData);
		privateData->pRackAFX_Parameters = (RackAFX_Parameters*) this;
		return AAX_SUCCESS;
	}

	return AAX_CEffectParameters::ResetFieldData(inFieldIndex, oData, iDataSize);
}

// --- see AAX_CMonolithicParameters in SDK
RackAFX_Parameters::SParamValList RackAFX_Parameters::GetUpdatesForState(int64_t inTargetStateNum)
{
	SParamValList paramValList;
	TNumberedStateListQueue stateLists;

	for(TNumberedParamStateList* numberedStateList = mQueuedParameterChanges.Peek();
		 // Condition
		 (NULL != numberedStateList) && // there is an element in the queue
		 (	(numberedStateList->first <= inTargetStateNum) || // next queued state is before or equal to target state
		  ((-0xFFFF > inTargetStateNum) && (0xFFFF < numberedStateList->first)) // target state counter has wrapped around
		  );

		 // Increment
		 numberedStateList = mQueuedParameterChanges.Peek()
		 )
	{
		// We'll use this state, so pop it from the queue
		const TNumberedParamStateList* const poppedPair = mQueuedParameterChanges.Pop();
		AAX_ASSERT(poppedPair == numberedStateList); // We can trust that this will match because there is only one thread calling Pop() on mQueuedParameterChanges
		const AAX_IContainer::EStatus pushResult = stateLists.Push(numberedStateList);
		AAX_ASSERT(AAX_IContainer::eStatus_Success == pushResult);
		numberedStateList = mQueuedParameterChanges.Peek();
	}

	// Transfer all parameter states into the single SParamValList
	for (TNumberedParamStateList* numberedStateList = stateLists.Pop(); NULL != numberedStateList; numberedStateList = stateLists.Pop())
	{
		paramValList.Append(numberedStateList->second);
		numberedStateList->second.clear(); // Ownership of all elements has been transferred to paramValList

		// Queue the now-empty container for later deletion
		mFinishedParameterChanges.Push(numberedStateList);
	}

	return paramValList;
}

// --- see AAX_CMonolithicParameters in SDK
void RackAFX_Parameters::DeleteUsedParameterChanges()
{
	// Deep-delete all elements from the used parameter change queue
	for (TNumberedParamStateList* numberedStateList = mFinishedParameterChanges.Pop(); NULL != numberedStateList; numberedStateList = mFinishedParameterChanges.Pop())
	{
		TNumberedParamStateList::second_type& curStateList = numberedStateList->second;
		for (std::list<TParamValPair*>::const_iterator iter = curStateList.begin(); iter != curStateList.end(); ++iter)
		{
			if (*iter) { delete *iter; }
		}
		delete numberedStateList;
	}

	// Delete all used parameter values
	for (const TParamValPair* paramVal = mFinishedParameterValues.Pop(); NULL != paramVal; paramVal = mFinishedParameterValues.Pop())
	{
		if (paramVal) { delete paramVal; }
	}
}

// --- timer to clean out old params
AAX_Result RackAFX_Parameters::TimerWakeup()
{
	DeleteUsedParameterChanges();
	return AAX_CEffectParameters::TimerWakeup();
}

// --- meter map of param indices for outbound meter updates
void RackAFX_Parameters::clearMeterMap()
{
    // --- clear map
    for(int i=0; i<MAX_USER_METERS; i++)
        m_nMeterMap[i] = -1; // -1 = no meter mapped
}

// --- next non(-1) slot available
int RackAFX_Parameters::getNextMeterMapSlot()
{
    for(int i=0; i<MAX_USER_METERS; i++)
    {
        if(m_nMeterMap[i] == -1)
            return i;
    }
    return -1; // no slots left, will never happen...
}

// --- threadsafe update of outbound parameters
void RackAFX_Parameters::updateOutboundAAXParameters()
{
    for(int i=0; i<MAX_USER_METERS; i++)
    {
        if(m_nMeterMap[i]>= 0)
        {
            CUICtrl* pUICtrl = m_pRAFXPlugIn->getUICtrlByListIndex(m_nMeterMap[i]);
            if(!pUICtrl || !pUICtrl->m_pCurrentMeterValue) continue; // should never happen

            // --- update the meter value
            std::stringstream str;
            str << m_nMeterMap[i] + 1;
            SetParameterNormalizedValue(str.str().c_str(),*pUICtrl->m_pCurrentMeterValue);
        }
    }
}

// --- MIDI Event List stuff
AAXMIDIEventList::AAXMIDIEventList(RackAFX_Parameters* pPlugIn)
{
    m_pPlugIn = pPlugIn;
    clearIncomingMIDIBuffer();
}

AAXMIDIEventList::~AAXMIDIEventList()
{

}

// --- IMidiEventList
unsigned int AAXMIDIEventList::getEventCount()
{
    if(m_pCurrentMidiPacket) return m_MidiBuffersize;
    else return 0;
}

bool AAXMIDIEventList::fireMidiEvent(unsigned int uSampleOffset)
{
    if(m_pPlugIn)
        m_pPlugIn->ProcessMIDI(uSampleOffset, m_pCurrentMidiPacket, m_MidiBuffersize);
    return true;
}

void AAXMIDIEventList::clearIncomingMIDIBuffer()
{
    m_pCurrentMidiPacket = NULL;
    m_MidiBuffersize = 0;
}

void AAXMIDIEventList::setIncomingMIDIBuffer(const AAX_CMidiPacket* ioPacketPtr, uint32_t midiBuffersize)
{
    m_pCurrentMidiPacket = ioPacketPtr;
    m_MidiBuffersize = midiBuffersize;
}



