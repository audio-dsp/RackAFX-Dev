/*================================================================================================*/
/*
 *
 *	Copyright 2013-2017 by Avid Technology, Inc.
 *	All rights reserved.
 *	
 *	CONFIDENTIAL: This document contains confidential information.  Do not 
 *	read or examine this document unless you are an Avid Technology employee
 *	or have signed a non-disclosure agreement with Avid Technology which protects
 *	the	confidentiality of this document.  DO NOT DISCLOSE ANY INFORMATION 
 *	CONTAINED IN THIS DOCUMENT TO ANY THIRD-PARTY WITHOUT THE PRIOR WRITTEN	CONSENT 
 *	OF Avid Technology, INC.
 *
 */

/**  
 *	\file  AAX_VTransport.h
 *
 *	\brief Version-managed concrete Transport class
 *
 */ 
/*================================================================================================*/

#ifndef AAX_VTRANSPORT_H
#define AAX_VTRANSPORT_H

#include "AAX_ITransport.h"
#include "AAX_IACFTransport.h"
#include "ACFPtr.h"

/**
 *	\brief Version-managed concrete \ref AAX_ITransport class
 *
 */
class AAX_VTransport : public AAX_ITransport
{
public:
	AAX_VTransport( IACFUnknown* pUnknown );
	virtual ~AAX_VTransport();
	
	// Transport Information Getters
	// AAX_IACFTransport
	virtual	AAX_Result	GetCurrentTempo ( double* TempoBPM ) const AAX_OVERRIDE; ///< \copydoc AAX_ITransport::GetCurrentTempo()
	virtual	AAX_Result	GetCurrentMeter ( int32_t* MeterNumerator, int32_t* MeterDenominator ) const AAX_OVERRIDE; ///< \copydoc AAX_ITransport::GetCurrentMeter()
	virtual	AAX_Result	IsTransportPlaying ( bool* isPlaying ) const AAX_OVERRIDE; ///< \copydoc AAX_ITransport::IsTransportPlaying()
	virtual	AAX_Result	GetCurrentTickPosition ( int64_t* TickPosition ) const AAX_OVERRIDE; ///< \copydoc AAX_ITransport::GetCurrentTickPosition()
	virtual	AAX_Result	GetCurrentLoopPosition ( bool* bLooping, int64_t* LoopStartTick, int64_t* LoopEndTick ) const AAX_OVERRIDE; ///< \copydoc AAX_ITransport::GetCurrentLoopPosition()
	virtual	AAX_Result	GetCurrentNativeSampleLocation ( int64_t* SampleLocation ) const AAX_OVERRIDE; ///< \copydoc AAX_ITransport::GetCurrentNativeSampleLocation()
	virtual	AAX_Result	GetCustomTickPosition( int64_t* oTickPosition, int64_t iSampleLocation) const AAX_OVERRIDE; ///< \copydoc AAX_ITransport::GetCustomTickPosition()
	virtual	AAX_Result	GetBarBeatPosition(int32_t* Bars, int32_t* Beats, int64_t* DisplayTicks, int64_t SampleLocation) const AAX_OVERRIDE; ///< \copydoc AAX_ITransport::GetBarBeatPosition()
	virtual	AAX_Result	GetTicksPerQuarter ( uint32_t* ticks ) const AAX_OVERRIDE; ///< \copydoc AAX_ITransport::GetTicksPerQuarter()
	virtual	AAX_Result	GetCurrentTicksPerBeat ( uint32_t* ticks ) const AAX_OVERRIDE; ///< \copydoc AAX_ITransport::GetCurrentTicksPerBeat()

	// AAX_IACFTransport_V2
	virtual	AAX_Result	GetTimelineSelectionStartPosition ( int64_t* oSampleLocation ) const AAX_OVERRIDE; ///< \copydoc AAX_ITransport::GetTimelineSelectionStartPosition()
	virtual	AAX_Result	GetTimeCodeInfo( AAX_EFrameRate* oFrameRate, int32_t* oOffset ) const AAX_OVERRIDE; ///< \copydoc AAX_ITransport::GetTimeCodeInfo()
	virtual	AAX_Result	GetFeetFramesInfo( AAX_EFeetFramesRate* oFeetFramesRate, int64_t* oOffset ) const AAX_OVERRIDE; ///< \copydoc AAX_ITransport::GetFeetFramesInfo()
	virtual AAX_Result	IsMetronomeEnabled ( int32_t* isEnabled ) const AAX_OVERRIDE; ///< \copydoc AAX_ITransport::IsMetronomeEnabled()
	
private:
	ACFPtr<AAX_IACFTransport>		mITransport;
	ACFPtr<AAX_IACFTransport_V2>	mITransportV2;
};

#endif // AAX_VTRANSPORT_H

