/*================================================================================================*/
/*
 *	Copyright 2013-2015, 2017 by Avid Technology, Inc.
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
 *  \file AAX_CEffectGUI_Juce.h
 *
 */
/*================================================================================================*/

#ifndef AAX_CEFFECTGUI_JUCE_H
#define AAX_CEFFECTGUI_JUCE_H

#include "juce.h"
#include "AAX_Enums.h"
#include "AAX_CEffectGUI.h"
#include "AAX_JuceCount.h"

//==============================================================================
class AAX_JuceContentView : public juce::Component
                          , public juce::DragAndDropContainer
{
public:
	AAX_JuceContentView ( const juce::Colour & inBackColour, AAX_CEffectGUI * inEffectGUI );
	virtual ~AAX_JuceContentView();

	void							SetEffectGUI ( AAX_CEffectGUI * inEffectGUI );
	AAX_CEffectGUI *				GetEffectGUI ();
	AAX_IEffectParameters *			GetParameters ();
	AAX_IController *				GetController ();
	AAX_IViewContainer *			GetViewContainer ();
	virtual void					SetViewContainer ( AAX_IViewContainer * inViewContainer ) { mViewContainer = inViewContainer; }
	
	enum ColourIds { backgroundColourId = 0x1005700 };
	
	const juce::Colour getBackgroundColour() const;
	void setBackgroundColour (const juce::Colour& newColour);
	void setBackgroundImage( juce::Image * inImage, bool inResize );
	
	virtual AAX_Result	setControlHighlightInfo ( AAX_CParamID inParameterID, bool inIsHighlighted, AAX_EHighlightColor inColor ) { return AAX_SUCCESS; }
	virtual AAX_Result	clearControlHighlightInfo () { return AAX_SUCCESS; }

	virtual	AAX_Result	TimerWakeup() { return AAX_SUCCESS; }
	virtual void		mouseMove ( const juce::MouseEvent & inMouseEvent );
	
protected:
	virtual void paint (juce::Graphics& g);
	
private:
	AAX_CEffectGUI *		mEffectGUI;
	AAX_IEffectParameters *	mEffectParameters;
	AAX_IController *		mController;
	AAX_IViewContainer *	mViewContainer;
	juce::Image *			mBackgroundImage;
};

//==============================================================================
class AAX_CEffectGUI_Juce : public AAX_CEffectGUI, public AAX_JuceCount
{
public:
	AAX_CEffectGUI_Juce ();
	virtual ~AAX_CEffectGUI_Juce ();
	
	virtual AAX_Result 	TimerWakeup();

	static bool			IncrementJuceUseCout ();
	static bool			DecrementJuceUseCout ();
	
protected:
	virtual void		CreateViewContainer ();
	virtual void		DeleteViewContainer ();

	virtual AAX_Result	GetViewSize ( AAX_Point * oEffectViewSize ) const;

	/* Create AAX_JuceContentView component
	 Overwrite this method if you want to create you own content view component
	 or add child components */
	virtual void		CreateViewContents ();
	
protected:
	AAX_JuceContentView*	mViewComponent;

public:	
	static juce::TooltipWindow*	sTooltipWindow;
};

#endif // AAX_CEFFECTGUI_JUCE_H
