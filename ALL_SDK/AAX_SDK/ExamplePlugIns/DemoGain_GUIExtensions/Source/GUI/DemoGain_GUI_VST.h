/*================================================================================================*/
/*
 *	Copyright 2009-2015, 2018 by Avid Technology, Inc.
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
 *	\file   DemoGain_GUI_VST.h
 *
 */ 
/*================================================================================================*/
#pragma once
#ifndef DEMOGAIN_GUI_JUCE_H
#define DEMOGAIN_GUI_JUCE_H

#include "AAX_CEffectGUI_VST.h"

class DemoGain_SliderVST;

enum EDemoGain_ControlIndex {
	eControl_Bypass = 0,
	eControl_GainSlider,
	eControl_GainText
};

class DemoGain_View : public VSTGUI_ContentView, public VSTGUI::IControlListener
{
public: ///////////////////////////// constructor/destructor
	DemoGain_View(void *pSystemWindow, AAX_IEffectParameters* pParams, VSTGUI::VSTGUIEditorInterface *pEditor, AAX_IViewContainer* inViewContainer);
	virtual ~DemoGain_View();

public: ///////////////////////////// DemoGain_View
	static void LoadViewResources();
	
	void updateGain();
	void highliteGain(AAX_CBoolean iIsHighlighted, AAX_EHighlightColor iColor);

public: ///////////////////////////// CControlListener
	virtual void valueChanged (VSTGUI::CControl* pControl);
	virtual void controlBeginEdit (VSTGUI::CControl* pControl);
	virtual void controlEndEdit (VSTGUI::CControl* pControl);
	
private: //////////////////////////// DemoGain_View
	static std::auto_ptr<VSTGUI::CBitmap>	m_hPIBackground;
	static std::auto_ptr<VSTGUI::CBitmap>	m_hSliderBack;					
	static std::auto_ptr<VSTGUI::CBitmap>	m_hSliderHandle;
	static std::auto_ptr<VSTGUI::CBitmap>	m_hSliderHandleRed;
	static std::auto_ptr<VSTGUI::CBitmap>	m_hSliderHandleBlue;
	static std::auto_ptr<VSTGUI::CBitmap>	m_hSliderHandleGreen;
	static std::auto_ptr<VSTGUI::CBitmap>	m_hSliderHandleYellow;

	DemoGain_SliderVST*	m_hSliderCtrl;
	VSTGUI::CTextEdit* m_hGainText;
	
	AAX_IViewContainer* mViewContainer;
};

//==============================================================================
class DemoGain_GUI : public AAX_CEffectGUI_VST
{
public:
	static AAX_IEffectGUI *		AAX_CALLBACK Create ( void );

	DemoGain_GUI();
	virtual ~DemoGain_GUI() {};
	
protected:
	virtual void			CreateViewContents ();
	virtual void			CreateEffectView (void *inSystemWindow);
	virtual AAX_Result		ParameterUpdated ( AAX_CParamID iParameterID );

	virtual AAX_Result		SetControlHighlightInfo (AAX_CParamID iParameterID, AAX_CBoolean iIsHighlighted, AAX_EHighlightColor iColor);
		
	AAX_CBoolean	m_bSliderIsHighlighted;
	AAX_EHighlightColor	m_SliderColor;			// current slider handle color
};


#endif // DEMOGAIN_GUI_JUCE_H
