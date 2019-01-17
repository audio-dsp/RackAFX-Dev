/*================================================================================================*/
/*
 *	Copyright 2013-2015 by Avid Technology, Inc.
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
 *	\file   DemoGain_GUI_Juce.h
 *
 *	\brief	DemoGain_GUI class definition.
 */ 
/*================================================================================================*/
#pragma once
#ifndef DEMOGAIN_GUI_JUCE_H
#define DEMOGAIN_GUI_JUCE_H

#include "AAX_CEffectGUI_Juce.h"

class DemoGain_ViewComponent;

//==============================================================================
class DemoGain_GUI : public AAX_CEffectGUI_Juce
{
public:
	static AAX_IEffectGUI *		AAX_CALLBACK Create (void);

	DemoGain_GUI() : AAX_CEffectGUI_Juce(), mDemoGainComponent(NULL) {};
	virtual ~DemoGain_GUI() {};

	virtual void				CreateViewContents ();
	virtual AAX_Result			ParameterUpdated ( AAX_CParamID iParameterID );
	virtual AAX_Result			SetControlHighlightInfo (AAX_CParamID iParameterID, AAX_CBoolean iIsHighlighted, AAX_EHighlightColor iColor);

private:
	DemoGain_ViewComponent *	mDemoGainComponent;
};

#endif // DEMOGAIN_GUI_JUCE_H
