/*================================================================================================*/
/*
 *	Copyright 2013-2016 by Avid Technology, Inc.
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
 *  \file AAX_CEffectGUI_VST.h
 *
 */
/*================================================================================================*/

#ifndef AAX_CEFFECTGUI_VST_H
#define AAX_CEFFECTGUI_VST_H

#include "AAX_CEffectGUI.h"

#include "vstgui.h"

//==============================================================================
class VSTGUI_ContentView : public VSTGUI::CFrame
{
	// Change this type depending on the VSTGUI SDK being used
	typedef VSTGUI::IDataPackage TDragContainer;
	
public:
	VSTGUI_ContentView(void *pSystemWindow, AAX_IEffectParameters* pParams = NULL, VSTGUI::VSTGUIEditorInterface *pEditor = NULL);
	virtual ~VSTGUI_ContentView();
	
	void setParameters (AAX_IEffectParameters *inParameters);
	AAX_IEffectParameters * getParameters ();

	virtual bool setPosition (VSTGUI::CCoord x, VSTGUI::CCoord y) VSTGUI_OVERRIDE_VMETHOD;

	virtual bool setSize (VSTGUI::CCoord width, VSTGUI::CCoord height) VSTGUI_OVERRIDE_VMETHOD;
	virtual bool getSize (VSTGUI::CRect *pSize) const VSTGUI_OVERRIDE_VMETHOD;
	virtual bool getSize (VSTGUI::CRect &pSize) const VSTGUI_OVERRIDE_VMETHOD;

protected:
	// VSTGUI 4 CFrame overrides
	virtual bool platformOnKeyDown (VstKeyCode& keyCode);

	// VSTGUI 3 CFrame overrides
	virtual bool onDrop (TDragContainer* drag, const VSTGUI::CPoint& where) VSTGUI_OVERRIDE_VMETHOD;
	virtual void onDragEnter (TDragContainer* drag, const VSTGUI::CPoint& where) VSTGUI_OVERRIDE_VMETHOD;
	virtual void onDragLeave (TDragContainer* drag, const VSTGUI::CPoint& where) VSTGUI_OVERRIDE_VMETHOD;
	virtual void onDragMove (TDragContainer* drag, const VSTGUI::CPoint& where) VSTGUI_OVERRIDE_VMETHOD;
	
private:
	AAX_IEffectParameters	* mEffectParameters;
};

//==============================================================================
class AAX_CEffectGUI_VST : public AAX_CEffectGUI, public VSTGUI::VSTGUIEditorInterface
{
public:
	AAX_CEffectGUI_VST ();
	virtual ~AAX_CEffectGUI_VST ();
	
protected:
	virtual void	CreateViewContainer ();
	virtual void	DeleteViewContainer ();
	
	virtual AAX_Result	GetViewSize ( AAX_Point * oEffectViewSize ) const;
	virtual AAX_Result	Draw ( AAX_Rect * iDrawRect );
	
	virtual void	CreateEffectView (void *inSystemWindow);

protected:
	VSTGUI::CFrame*		mViewComponent;
};

//==============================================================================
// CAAXButtonState
// Helper class combining representations of the AAX modifier key mask and
// the VSTGUI::CButtonState mask. Includes logic to fix missing modifier
// key states that are removed by host filtering, e.g. by the host key hook.
class AAX_CVSTGUIButtonState
{
public:
	AAX_CVSTGUIButtonState(const VSTGUI::CButtonState& inButtonState, AAX_IViewContainer* inViewContainer);

	const VSTGUI::CButtonState& AsVST() const { return mButtonState; }
	uint32_t AsAAX() const { return mModifiers; }

private:
	uint32_t mModifiers;
	VSTGUI::CButtonState mButtonState;

private:
	AAX_CVSTGUIButtonState(); // Unimplemented
};

#endif // AAX_CEFFECTGUI_VST_H
