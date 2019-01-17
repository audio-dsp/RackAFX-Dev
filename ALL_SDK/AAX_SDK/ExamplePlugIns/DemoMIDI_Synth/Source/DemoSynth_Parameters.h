/*================================================================================================*/
/*
 *	Copyright 2012-2015 by Avid Technology, Inc.
 *	All rights reserved.
 *
 *	CONFIDENTIAL: This document contains confidential information.  Do not 
 *	read or examine this document unless you are an Avid Technology employee
 *	or have signed a non-disclosure agreement with Avid Technology which protects
 *	the	confidentiality of this document.  DO NOT DISCLOSE ANY INFORMATION 
 *	CONTAINED IN THIS DOCUMENT TO ANY THIRD-PARTY WITHOUT THE PRIOR WRITTEN	CONSENT 
 *	OF Avid Technology, INC.
 */

/**  
 *	\file   DemoSynth_Parameters.h
 *
 *	\brief  DemoSynth_Parameters class declaration.
 *
 *  \author Andriy Goshko
 */ 
/*================================================================================================*/ 
#pragma once
#ifndef DEMOSYNTH_PARAMETERS_H
#define DEMOSYNTH_PARAMETERS_H

// AAX Includes
#include "AAX_CMonolithicParameters.h"
#include "AAX_CMutex.h"

class AAX_IParameter;
class CSimpleSynth;

//==============================================================================
class DemoSynth_Parameters : public AAX_CMonolithicParameters
{
public:
	DemoSynth_Parameters (void);
	virtual ~DemoSynth_Parameters (void);
	
	static AAX_CEffectParameters *AAX_CALLBACK	Create();
	
public:
	//Overrides from AAX_CMonolithicParameters
	virtual AAX_Result	EffectInit();
	virtual AAX_Result	UpdateParameterNormalizedValue(AAX_CParamID iParamID, double aValue, AAX_EUpdateSource inSource);
	virtual void		RenderAudio(AAX_SInstrumentRenderInfo* ioRenderInfo, const TParamValPair* inSynchronizedParamValues[], int32_t inNumSynchronizedParamValues);
		
private:
	void				InitializeSynthesizerState(CSimpleSynth& ioSynth);
	void				UpdateCoefficients(const TParamValPair* inSynchronizedParamValues[], int32_t inNumSynchronizedParamValues);
	void				ProcessInputMIDI(CSimpleSynth& ioSynth, uint32_t inSample, const AAX_CMidiPacket*& ioPacketPtr, uint32_t& ioPacketsRemaining);
	
private:
	CSimpleSynth*		mSynthesizer;
};

#endif
