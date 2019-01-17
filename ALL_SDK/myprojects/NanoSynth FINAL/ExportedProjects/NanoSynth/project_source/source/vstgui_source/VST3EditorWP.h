#pragma once
#include "vstgui/plugin-bindings/vst3editor.h"
#include "plugin.h"
#include <vector>

namespace Steinberg {
namespace Vst {
namespace Sock2VST3 {

#define GUI_IDLE_UPDATE_INTERVAL_MSEC 50
const unsigned int kHostChoice = 3;

class VST3EditorWP : public VST3Editor
{
public:
	VST3EditorWP(Steinberg::Vst::EditController* controller, UTF8StringPtr templateName, UTF8StringPtr xmlFile);
	CMessageResult notify(CBaseObject* sender, IdStringPtr message) override;

	// --- RAFX v6.8 changed updateGUI to safe method	
	virtual void valueChanged(CControl* pControl);

	// --- RAFX v6.6 adds knob mode
	VSTGUI_INT32 getKnobMode() const;
	void setKnobMode(VSTGUI_INT32 n){m_uKnobMode = n;}

	// --- RAFX v6.6 adds GUI customization 
	CBitmap* getBitmap(UTF8StringPtr name);
	UTF8StringPtr lookupBitmapName(const CBitmap* bitmap);

	// --- RAFX v6.8 changes the update GUI paradigm
	void setPlugIn(CPlugIn* pPlugIn)
	{
		m_pPlugIn = pPlugIn;
	    if(m_pPlugIn)
		{
			// --- make a copy of the control list for calculations and other non-write stuff
			int nParams = m_pPlugIn->getControlCount();
			for(int i=0; i<nParams; i++)
			{
				CUICtrl* p = m_pPlugIn->getUICtrlByListIndex(i);
				if(p)
				{
					m_GUI_UIControlList.append(*p);
				}
			}
		}
	}
	void checkSendUpdateGUI(int nControlId, float fValue, bool bLoadingPreset);

	// --- this is the fix for Reaper 5.12 breaking with VSTGUI4 see implementation
	bool beforeSizeChange (const CRect& newSize, const CRect& oldSize) override;
	    
	CUIControlList m_GUI_UIControlList;

	inline int getGUI_UICtrlIndexByControlID(UINT uID)
    {
		if(!m_pPlugIn) return -1;

		int nCount = m_GUI_UIControlList.count();
        for(int i=0; i<nCount; i++)
        {
            CUICtrl* p = m_GUI_UIControlList.getAt(i);
            if(p->uControlId == uID)
                return i;
        }
        return -1;
    }

protected:
	VSTGUI_INT32 m_uKnobMode;
	CPlugIn* m_pPlugIn;
};
}}}
