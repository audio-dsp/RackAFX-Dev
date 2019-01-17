#pragma once

#ifndef RACKAFX_GUI_H
#define RACKAFX_GUI_H

#include "AAX_CEffectGUI.h"
#include "vstgui/vstgui.h" // needed for UNICODE define only
#include "RackAFX_Parameters.h"
#include "RafxVSTEditor.h"

const UINT FILTER_CONTROL_USER_VSTGUI_VARIABLE = 106;

class RackAFX_GUI : public AAX_CEffectGUI
{
public:
	RackAFX_GUI(void);
	~RackAFX_GUI(void);

	static AAX_IEffectGUI* AAX_CALLBACK Create( void );

	// --- AAX_CEffectGUI overrides
	virtual void CreateViewContainer();
	virtual void DeleteViewContainer();
	virtual void CreateViewContents();
	virtual AAX_Result ParameterUpdated(const char* iParameterID);
	virtual AAX_Result Draw(AAX_Rect* iDrawRect);
	virtual AAX_Result TimerWakeup(void);

	// --- for sizing the GUI
	virtual AAX_Result GetViewSize(AAX_Point* oEffectViewSize) const;
	int m_nWidth;
	int m_nHeight;

	// --- GUI helpers
    UINT* createControlMap(CPlugIn* pRAFXPlugIn, int& nControlCount);
    int getGUIControlCount(CPlugIn* pRAFXPlugIn);


protected:
	rackAFXCustomData m_CustomData;
    CRafxVSTEditor m_RafxVSTGUI;
    bool m_bPureCustomGUI;
};

#endif