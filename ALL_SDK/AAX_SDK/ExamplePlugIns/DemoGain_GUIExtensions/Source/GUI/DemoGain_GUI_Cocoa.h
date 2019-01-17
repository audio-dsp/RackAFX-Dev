/*================================================================================================*/
/*
 *	Copyright 2009-2015 by Avid Technology, Inc.
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
 *	\file   DemoGain_GUI_Cocoa.h
 *
  *	\brief	DemoGain_GUI class definition.
 */ 
/*================================================================================================*/
#pragma once
#ifndef DEMOGAIN_GUI_COCOA_H
#define DEMOGAIN_GUI_COCOA_H

#include "AAX_CEffectGUI_Cocoa.h"

#ifdef __OBJC__
#import <Cocoa/Cocoa.h>
#import "DemoGain_ViewController.h"
#else
class DemoGain_ViewController;
#endif

//==============================================================================
class DemoGain_GUI : public AAX_CEffectGUI_Cocoa
{
public:
	static AAX_IEffectGUI *		AAX_CALLBACK Create ();

	DemoGain_GUI();
	virtual ~DemoGain_GUI();

	virtual void				CreateViewContents ();
	virtual AAX_Result			ParameterUpdated ( AAX_CParamID iParameterID );
	virtual	AAX_Result			TimerWakeup();

protected:
	virtual void				CreateViewContainer (); // AAX_CEffectGUI_Cocoa

private:
	DemoGain_ViewController	*	mDemoGainViewController;
};

#endif
