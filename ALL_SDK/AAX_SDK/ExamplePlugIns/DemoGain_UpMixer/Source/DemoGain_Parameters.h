/*================================================================================================*/
/*
 *	Copyright 2009-2017 by Avid Technology, Inc.
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
 *	\file   DemoGain_Parameters.h
 *
 *	\brief  DemoGain_Parameters class declaration.
 */ 
/*================================================================================================*/ 

#pragma once
#ifndef DEMOGAIN_PARAMETERS_H
#define DEMOGAIN_PARAMETERS_H

#include "AAX_CEffectParameters.h"


class DemoGain_Parameters : public AAX_CEffectParameters
{
public:
	DemoGain_Parameters (void);
	virtual ~DemoGain_Parameters (void) {}
	
	// Create callback
	static AAX_CEffectParameters *AAX_CALLBACK Create();

public:
	//Overrides from AAX_CEffectParameters
	virtual AAX_Result EffectInit();

private:
	AAX_Result UpdatePacket_Gain(AAX_CPacket& ioPacket);
    AAX_Result UpdatePacket_Routing(AAX_CPacket& ioPacket);
	
private:
	uint16_t mNumInputChannels;
    uint16_t mNumOutputChannels;
};


#endif
