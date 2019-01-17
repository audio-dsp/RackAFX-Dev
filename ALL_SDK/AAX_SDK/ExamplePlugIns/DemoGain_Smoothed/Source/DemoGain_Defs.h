/*================================================================================================*/
/*
 *	Copyright 2008-2015, 2018 by Avid Technology, Inc.
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
 *	\file   DemoGain_Defs.h
 *
 *	\brief  Definitions shared by the DemoGain_Smoothed classes in one easy-to-maintain file.
 */ 
/*================================================================================================*/
#pragma once
#ifndef DEMOGAIN_DEFS_H
#define DEMOGAIN_DEFS_H

#include "AAX.h"			// for AAX_CTypeID

// Effect IDs
const AAX_CEffectID kEffectID_DemoGain =			"com.avid.aax.sdk.demogain.smoothed";

// Type, product, and relation IDs
const AAX_CTypeID cDemoGain_ManufactureID =			'AVID';
const AAX_CTypeID cDemoGain_ProductID =				'DmgS';
const AAX_CTypeID cDemoGain_PlugInID_Native =		'DGSR';
const AAX_CTypeID cDemoGain_PlugInID_AudioSuite =	'DGSA';
const AAX_CTypeID cDemoGain_PlugInID_TI =			'DGST';

const AAX_CTypeID cDemoGain_MeterID [2] =			{'mtrI','mtrO'};

#define DemoGain_GainID		"Gain"

#endif // DEMOGAIN_DEFS_H
