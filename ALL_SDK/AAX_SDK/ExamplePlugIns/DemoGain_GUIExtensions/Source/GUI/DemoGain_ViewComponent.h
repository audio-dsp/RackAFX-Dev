/*================================================================================================*/
/*
 *	Copyright 2010-2015 by Avid Technology, Inc.
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
 *	\file   DemoGain_ViewComponent.h
 *
 *	\brief  DemoGain_ViewComponent class definition.
 */ 
/*================================================================================================*/
#pragma once
#ifndef DEMOGAIN_VIEWCOMPONENT_H
#define DEMOGAIN_VIEWCOMPONENT_H

#include "AAX_CEffectGUI_Juce.h"

class DemoGain_SliderJuce;

//==============================================================================
class DemoGain_ViewComponent	: public AAX_JuceContentView
								, public juce::SliderListener
								, public juce::LabelListener
								, public juce::ButtonListener
{
public:	
	DemoGain_ViewComponent ( AAX_CEffectGUI * inEffectGUI );
	virtual ~DemoGain_ViewComponent();
	
public: ///////////////////////////////////////////// DemoGain_ViewComponent
	virtual void UpdateGainParameter();
	virtual void UpdateGainDefaultValue();

public: ///////////////////////////////////////////// AAX_JuceContentView
	virtual	AAX_Result	setControlHighlightInfo (AAX_CParamID iParameterID, AAX_CBoolean iIsHighlighted, AAX_EHighlightColor iColor);
	virtual	AAX_Result	TimerWakeup();
	
	virtual void SetViewContainer ( AAX_IViewContainer * inViewContainer ); //TODO: notification hook would be better
	
protected:
	virtual void sliderDragStarted (juce::Slider* inSlider);
	virtual void sliderValueChanged (juce::Slider * inSlider);
	virtual void sliderDragEnded (juce::Slider* inSlider);
	virtual void labelTextChanged (juce::Label * labelThatHasChanged);
	virtual void buttonClicked (juce::Button* button);
	
private:
	DemoGain_SliderJuce		* mGainSlider;
	juce::Label		* mGainText;
	juce::Label		* mMonoText;
	juce::TextButton	* mResizeButton;
	juce::Label		* mInputMeterText;
	juce::Label		* mOutputMeterText;
	juce::Slider		* mInputMeter;
	juce::Slider 	* mOutputMeter;
};	

#endif
