#ifndef __CRafxVSTEditor__
#define __CRafxVSTEditor__

#include "WPXMLParser.h"
#include "plugin.h"
#include "synthfunctions.h"
#include "VuMeterWP.h"
#include "XYPadWP.h"
#include "KnobWP.h"
#include "SliderWP.h"
#include "rafx2vstguimessages.h"
#include "guistructures.h"

#ifdef AAXPLUGIN
#include "vstgui/vstgui.h"
#include "vstgui/lib/platform/iplatformfont.h"
#endif
#ifdef AUPLUGIN
#include "vstgui/vstgui.h"
#include "vstgui/lib/platform/iplatformfont.h"
#endif
#include "RAFXSwitchView.h"
#include "vstgui4constants.h"
#include <sstream>
#include <functional>

// --- AAX support
#ifdef AAXPLUGIN
#include "AAX_CEffectParameters.h"
#include "AAX_IEffectParameters.h"
#include "AAXtoVSTGUIButtonState.h"
#endif

#ifdef AUPLUGIN
#import <CoreFoundation/CoreFoundation.h>
#import <AudioUnit/AudioUnit.h>
#import <AudioToolbox/AudioToolbox.h>
#endif

#include <cctype>
#include <locale>

using namespace std;
using namespace pugi;

namespace VSTGUI {

const UINT ASSIGNBUTTON_1 = 32768;
const UINT ASSIGNBUTTON_2 = 32769;
const UINT ASSIGNBUTTON_3 = 32770;
const UINT ALPHA_WHEEL    = 32771;
const UINT LCD_KNOB		  = 32772;
const UINT JOYSTICK_X	  = 32773;
const UINT JOYSTICK_Y	  = 32774;
const UINT JOYSTICK 	  = 32775;
const UINT TRACKPAD 	  = 32776;
const UINT LCD_TITLE 	  = 32777;
const UINT LCD_COUNT 	  = 32778;
const UINT TAB_CTRL_0 	  = 65536;
const UINT TAB_CTRL_1	  = 65537;
const UINT TAB_CTRL_2	  = 65538;
const UINT TAB_CTRL_3     = 65539;
const UINT TAB_CTRL_4     = 65540;
const UINT TAB_CTRL_5     = 65541;
const UINT TAB_CTRL_6     = 65542;
const UINT TAB_CTRL_7     = 65543;
const UINT TAB_CTRL_8     = 65544;
const UINT TAB_CTRL_9     = 65545;
const UINT TAB_CTRL_10    = 65546;
const UINT TAB_CTRL_11    = 65547;
const UINT TAB_CTRL_12    = 65548;
const UINT TAB_CTRL_13    = 65549;
const UINT TAB_CTRL_14    = 65550;
const UINT TAB_CTRL_15    = 65551;
const UINT TAB_CTRL_16    = 65552;
const UINT TAB_CTRL_17    = 65553;
const UINT TAB_CTRL_18    = 65554;
const UINT TAB_CTRL_19    = 65555;
const UINT TAB_CTRL_20    = 65556;
const UINT TAB_CTRL_21    = 65557;
const UINT TAB_CTRL_22    = 65558;
const UINT TAB_CTRL_23    = 65559;
const UINT TAB_CTRL_24    = 65560;
const UINT TAB_CTRL_25    = 65561;
const UINT TAB_CTRL_26    = 65562;
const UINT TAB_CTRL_27    = 65563;
const UINT TAB_CTRL_28    = 65564;
const UINT TAB_CTRL_29    = 65565;
const UINT TAB_CTRL_30    = 65566;
const UINT TAB_CTRL_31    = 65567;

// --- NODE
const UINT RAFX_XML_NODE = 'RXML';

// --- m_uPlugInEx[]
const UINT RAFX_VERSION = 0;
const UINT VSTGUI_VERSION = 1;
const UINT KNOB_MODE = 2;



typedef struct
{
    CTextLabel**		pLabel;
    int					nLabels;
    CTextEdit**			pEdit;
    int					nEdits;
    CAnimKnob**			pKnob;
    int					nKnobs;
    CSlider**			pSlider;
    int					nSliders;
    CVerticalSwitch**	pVSwitch;
    int					nVSwitches;
    CHorizontalSwitch**	pHSwitch;
    int					nHSwitches;
    COnOffButton**		pOOB;
    int					nOOBs;
    CVuMeter**			pMeter;
    int					nMeters;
    CKickButton**		pKickButton;
    int					nKickButtons;
    COptionMenu**		pOptionMenu;
    int					nOptionMenus;
    CXYPad**			pXYPad;
    int					nXYPads;
    CSegmentButton**	pSegButton;
    int					nSegButtons;
    CControl**			pControl;
    int					nControls;
} GUI_CONTROL_STRUCT;

// --- RAFX v6.6
const unsigned int kHostChoice = 3;

/* ------------------------------------------------------
 KeyboardHooker
 Custom VSTGUI Object by Will Pirkle
 Created with RackAFX(TM) Plugin Development Software
 www.willpirkle.com
 -------------------------------------------------------*/
class KeyboardHooker : public IKeyboardHook
{
    //< should return 1 if no further key down processing should apply, otherwise -1
    virtual int32_t onKeyDown (const VstKeyCode& code, CFrame* frame)
    {
        //VstKeyCode key(code);

        // --- do keydown processing based on code.character
        //     return 1 if handled, -1 if not handled
        return -1;
    }

    //< should return 1 if no further key up processing should apply, otherwise -1
    virtual int32_t onKeyUp (const VstKeyCode& code, CFrame* frame)
    {
        //VstKeyCode key(code);

        // --- do keyup processing based on code.character
        //     return 1 if handled, -1 if not handled
        return -1;
    }
};


/* ------------------------------------------------------
     CRafxVSTEditor
     Custom VSTGUI Object by Will Pirkle
     Created with RackAFX(TM) Plugin Development Software
     www.willpirkle.com
 -------------------------------------------------------*/
class CRafxVSTEditor : public CControlListener, public IMouseObserver, public VSTGUI::VSTGUIEditorInterface, public VSTGUI::CBaseObject
{
public:
	CRafxVSTEditor();
    ~CRafxVSTEditor();
	CWPXMLParser m_XMLParser;
    CFrame* m_pRafxFrame;
  	KeyboardHooker m_KeyHooker;
    CVSTGUITimer* timer;
   	CMessageResult notify(CBaseObject* sender, const char* message) override;

    // --- AU ONLY
    void getSize(float& width, float& height);
    bool m_bInitialized;
    // --- End AU Only

    // --- AAX Only
    #ifdef AAXPLUGIN
    AAX_IEffectParameters* m_pAAXParameters;
    void setAAXParameters(AAX_IEffectParameters* inParameters){m_pAAXParameters = inParameters;}
    void setAAXViewContainer(AAX_IViewContainer* _aaxViewContainer){ aaxViewContainer = _aaxViewContainer;}
    #endif
    // --- End AAX Only

 	bool bitmapExistsInResources(const char* bitmapname);

    // --- open/close
#ifdef AUPLUGIN
    bool open(void* pHwnd, CPlugIn* pPlugIn, char* pXMLFile, AudioUnit inAU);
#else // AAX
    bool open(void* pHwnd, CPlugIn* pPlugIn, void* pXMLResource, DWORD xmlSize, char* pXMLFile = NULL);
#endif

    void close();
	bool bClosing;
	virtual void idle();

	// --- lin/circ
	UINT m_uKnobAction;
	virtual int32_t getKnobMode() const override;

	// --- from CControlListener
	void valueChanged(CControl* pControl) override;
   	virtual int32_t controlModifierClicked (VSTGUI::CControl* pControl, VSTGUI::CButtonState button)  override;// { return 0; }	///< return 1 if you want the control to not handle it, otherwise 0
	virtual void controlBeginEdit (VSTGUI::CControl* pControl) override;// {}
	virtual void controlEndEdit (VSTGUI::CControl* pControl) override;// {}
	virtual void controlTagWillChange (VSTGUI::CControl* pControl) override;// {}
	virtual void controlTagDidChange (VSTGUI::CControl* pControl) override;// {}
#if DEBUG
	virtual char controlModifierClicked (VSTGUI::CControl* pControl, long button);// { return 0; }
#endif

    //---  IMouseObserver
    void onMouseEntered(CView* view, CFrame* frame) override {}
    void onMouseExited(CView* view, CFrame* frame) override {}
    CMouseEventResult onMouseMoved(CFrame* frame, const CPoint& where, const CButtonState& buttons) override;
    CMouseEventResult onMouseDown(CFrame* frame, const CPoint& where, const CButtonState& buttons) override;

    // --- AAX keystrokes
    CMouseEventResult sendAAXMouseDown(int controlID, const CButtonState& buttons);
    CMouseEventResult sendAAXMouseMoved(int controlID, const CButtonState& buttons, const CPoint& where);

	inline bool loadXMLResource(void* pXML, DWORD size)
	{
		return m_XMLParser.loadXMLResource(pXML, size);
	}
	// -- v6.8
    CUIControlList m_GUI_UIControlList; // read-only control list for log/volt-octave calulations and other non-write stuff, thread-safe
    inline int getGUI_UICtrlIndexByControlID(UINT uID)
    {
        int nCount = m_GUI_UIControlList.count();
        for(int i=0; i<nCount; i++)
        {
            CUICtrl* p = m_GUI_UIControlList.getAt(i);
            if(p->uControlId == uID)
                return i;
        }
        return -1;
    }

    inline CUICtrl* getGUI_UICtrlByControlID(UINT uID)
    {
        int nCount = m_GUI_UIControlList.count();
        for(int i=0; i<nCount; i++)
        {
            CUICtrl* p = m_GUI_UIControlList.getAt(i);
            if(p->uControlId == uID)
                return p;
        }
        return NULL;
    }

    void checkSendUpdateGUI(int nControlId, float fValue, bool bLoadingPreset);
    bool m_bAppendUnits;

    // --- sync to global variables
	void initControls(bool bSetListener = true);

    // --- VSTGUI4 creation, loading, etc...
	CBitmap* loadBitmap(const CResourceDescription& desc);
	CNinePartTiledBitmap* loadTiledBitmap(const CResourceDescription& desc, CCoord left, CCoord top, CCoord right, CCoord bottom);

	bool createSubView(CViewContainer* pParentView, pugi::xml_node viewNode, bool bFrameSubView); // framesubview = not part of anothe vc part of frame vc
	bool deleteSubView(CViewContainer* pParentView, pugi::xml_node viewNode, pugi::xml_node parentNode); // framesubview = not part of anothe vc part of frame vc

	CTextLabel* createTextLabel(pugi::xml_node node, bool bAddingNew = true, bool bStandAlone = false);
	void parseTextLabel(CTextLabel* pLabel, pugi::xml_node node);
	CTextEdit* createTextEdit(pugi::xml_node node, bool bAddingNew = true, bool bStandAlone = false);
	CFontDesc* createFontDesc(pugi::xml_node node);
	CFontDesc* createFontDescFromFontNode(pugi::xml_node node);
	CAnimKnob* createAnimKnob(pugi::xml_node node, CBitmap* bitmap, bool bAddingNew = true, bool bStandAlone = false);
	CSlider* createSlider(pugi::xml_node node, CBitmap* handleBitmap, CBitmap* grooveBitmap, bool bAddingNew = true, bool bStandAlone = false);
	CVerticalSwitch* createVerticalSwitch(pugi::xml_node node, CBitmap* backgroundBitmap, bool bAddingNew = true, bool bStandAlone = false);
    CHorizontalSwitch* createHorizontalSwitch(pugi::xml_node node, CBitmap* backgroundBitmap, bool bAddingNew = true);
    COnOffButton* createOnOffButton(pugi::xml_node node, CBitmap* bitmap, bool bAddingNew = true, bool bStandAlone = false);
	CVuMeter* createMeter(pugi::xml_node node, CBitmap* onBitmap, CBitmap* offBitmap, bool bAddingNew = true, bool bStandAlone = false);
	CKickButton* createKickButton(pugi::xml_node node, CBitmap* backgroundBitmap, bool bAddingNew = true, bool bStandAlone = false);
	COptionMenu* createOptionMenu(pugi::xml_node node, CBitmap* backgroundBitmap, bool bAddingNew = true, bool bStandAlone = false);
	CXYPad* createXYPad(pugi::xml_node node, CBitmap* backgroundBitmap, bool bAddingNew = true, bool bStandAlone = false);
	CViewContainer* createViewContainer(pugi::xml_node node, CBitmap* backgroundBitmap, bool bAddingNew = true, bool bStandAlone = false);
	CView* createView(pugi::xml_node node, CBitmap* backgroundBitmap, bool bAddingNew = true, bool bStandAlone = false);
	CViewContainer* createUISwitchView(pugi::xml_node node, CBitmap* backgroundBitmap, bool bAddingNew = true, bool bStandAlone = false);
    CViewContainer* createUISwitchViewSubView(CViewContainer* pParentView, pugi::xml_node viewNode);
    CSegmentButton* createSegmentButton(pugi::xml_node node, bool bAddingNew = true); // VSTGUI_43 <--- add to AU wrapper!

    // --- for fast lookup of controls
    GUI_CONTROL_STRUCT* m_pGUIControls;

    // --- set the plugin, make a copy of the parameters
	void setPlugIn(CPlugIn* pPlugIn)
    {
        m_pPlugIn = pPlugIn;

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

    inline bool controlAndContainerVisible(CControl* ctrl)
    {
        if(!ctrl) return false;
        
        bool stickyVisible = ctrl->isVisible();

        if(!stickyVisible)
            return false;
        
        // --- check parents
        CView* parent = ctrl->getParentView();
        while(parent)
        {
            stickyVisible = parent->isVisible();
            if(!stickyVisible)
                return false;
            
            parent = parent->getParentView();
        }
        return stickyVisible;
    }

    inline int mouseOverVisibleControl(const CPoint& where, CControl* ctrl)
    {
        CPoint mousePoint = where;
        
        if(ctrl && controlAndContainerVisible(ctrl))
        {
            CPoint point = ctrl->frameToLocal(mousePoint);
            CRect rect = ctrl->getViewSize();
            if(rect.pointInside(point))
            {
                int tag = ctrl->getTag();
                if(tag >= 32768 && tag <= 32778)
                    return -1;
                else if(tag >= 65536 && tag <= 65567)
                    return -1;
                else
                    return tag;
            }
        }
        return -1;
    }

    inline int getControlID_WithMouseCoords(const CPoint& where)
    {
        int controlID = -1;
        // --- first pass for counts
        for(int i=0; i<m_nTextLabelCount; i++)
        {
            controlID = mouseOverVisibleControl(where, m_ppTextLabels[i]);
            if(controlID != -1)
                return controlID;
        }
        for(int i=0; i<m_nTextEditCount; i++)
        {
            controlID = mouseOverVisibleControl(where, m_ppTextEditControls[i]);
            if(controlID != -1)
                return controlID;
        }
        for(int i=0; i<m_nAnimKnobCount; i++)
        {
            controlID = mouseOverVisibleControl(where, m_ppKnobControls[i]);
            if(controlID != -1)
                return controlID;
        }
        for(int i=0; i<m_nSliderCount; i++)
        {
            controlID = mouseOverVisibleControl(where, m_ppSliderControls[i]);
            if(controlID != -1)
                return controlID;
        }
        for(int i=0; i<m_nVerticalSwitchCount; i++)
        {
            controlID = mouseOverVisibleControl(where, m_ppVerticalSwitchControls[i]);
            if(controlID != -1)
                return controlID;
        }
        for(int i=0; i<m_nHorizontalSwitchCount; i++)
        {
            controlID = mouseOverVisibleControl(where, m_ppHorizontalSwitchControls[i]);
            if(controlID != -1)
                return controlID;
        }
        for(int i=0; i<m_nOnOffButtonCount; i++)
        {
            controlID = mouseOverVisibleControl(where, m_ppOnOffButtonControls[i]);
            if(controlID != -1)
                return controlID;
        }
        for(int i=0; i<m_nKickButtonCount; i++)
        {
            controlID = mouseOverVisibleControl(where, m_ppKickButtonControls[i]);
            if(controlID != -1)
                return controlID;
        }
        /*for(int i=0; i<m_nVuMeterCount; i++)
        {
            controlID = mouseOverVisibleControl(where, m_ppVuMeterControls[i]);
            if(controlID != -1)
                return controlID;
        }*/
        for(int i=0; i<m_nOptionMenuCount; i++)
        {
            controlID = mouseOverVisibleControl(where, m_ppOptionMenuControls[i]);
            if(controlID != -1)
                return controlID;
        }
        for(int i=0; i<m_nSegmentButtonCount; i++)
        {
            controlID = mouseOverVisibleControl(where, m_ppSegmentButtons[i]);
            if(controlID != -1)
                return controlID;
        }
        return -1;
     }


    // --- copy of map, we delete on close()
	inline void copyControlMap(UINT* pControlMap, int nControlCount)
    {
        if(m_pControlMap) delete [] m_pControlMap;

        if(!pControlMap)
        {
            m_pControlMap = NULL;
            return;
        }

        m_pControlMap = new UINT[nControlCount];
        memcpy(m_pControlMap, pControlMap, sizeof(UINT)*nControlCount);

        m_nControlCount = nControlCount;

        if(m_pGUIControls)
            delete [] m_pGUIControls;

        m_pGUIControls = new GUI_CONTROL_STRUCT[nControlCount];

        /*
         CTextLabel*		pLabel;
         CTextEdit*			pEdit;
         CAnimKnob*			pKnob;
         CSlider*			pSlider;
         CVerticalSwitch*	pVSwitch;
         CHorizontalSwitch*	pHSwitch;
         COnOffButton*		pOOB;
         CVuMeter*			pMeter;
         CKickButton*		pKickButton;
         COptionMenu*		pOptionMenu;
         CSegmentButton*	pSegButton;
         */
        // --- gather controls with like tags
        for(int j=0; j<nControlCount; j++)
        {
            // --- clear out
            m_pGUIControls[j].pLabel = NULL;
            m_pGUIControls[j].pEdit = NULL;
            m_pGUIControls[j].pKnob = NULL;
            m_pGUIControls[j].pSlider = NULL;
            m_pGUIControls[j].pVSwitch = NULL;
            m_pGUIControls[j].pHSwitch = NULL;
            m_pGUIControls[j].pOOB = NULL;
            m_pGUIControls[j].pMeter = NULL;
            m_pGUIControls[j].pKickButton = NULL;
            m_pGUIControls[j].pOptionMenu = NULL;
            m_pGUIControls[j].pXYPad = NULL;
            m_pGUIControls[j].pSegButton = NULL;
            m_pGUIControls[j].pControl = NULL;

            m_pGUIControls[j].nLabels = 0;
            m_pGUIControls[j].nEdits = 0;
            m_pGUIControls[j].nKnobs = 0;
            m_pGUIControls[j].nSliders = 0;
            m_pGUIControls[j].nVSwitches = 0;
            m_pGUIControls[j].nHSwitches = 0;
            m_pGUIControls[j].nOOBs = 0;
            m_pGUIControls[j].nMeters = 0;
            m_pGUIControls[j].nKickButtons = 0;
            m_pGUIControls[j].nOptionMenus = 0;
            m_pGUIControls[j].nXYPads = 0;
            m_pGUIControls[j].nSegButtons = 0;
            m_pGUIControls[j].nControls = 0;

            // --- first pass for counts
            for(int i=0; i<m_nTextLabelCount; i++)
            {
                CTextLabel* p = m_ppTextLabels[i];
                if(p->getTag() == j)
                    m_pGUIControls[j].nLabels++;
            }
            for(int i=0; i<m_nTextEditCount; i++)
            {
                CTextEdit* p = m_ppTextEditControls[i];
                if(p->getTag() == j)
                    m_pGUIControls[j].nEdits++;
            }
            for(int i=0; i<m_nAnimKnobCount; i++)
            {
                CAnimKnob* p = m_ppKnobControls[i];
                if(p->getTag() == j)
                    m_pGUIControls[j].nKnobs++;
            }
            for(int i=0; i<m_nSliderCount; i++)
            {
                CSlider* p = m_ppSliderControls[i];
                if(p->getTag() == j)
                    m_pGUIControls[j].nSliders++;
            }
            for(int i=0; i<m_nVerticalSwitchCount; i++)
            {
                CVerticalSwitch* p = m_ppVerticalSwitchControls[i];
                if(p->getTag() == j)
                    m_pGUIControls[j].nVSwitches++;
            }
            for(int i=0; i<m_nHorizontalSwitchCount; i++)
            {
                CHorizontalSwitch* p = m_ppHorizontalSwitchControls[i];
                if(p->getTag() == j)
                    m_pGUIControls[j].nHSwitches++;
            }
            for(int i=0; i<m_nOnOffButtonCount; i++)
            {
                COnOffButton* p = m_ppOnOffButtonControls[i];
                if(p->getTag() == j)
                    m_pGUIControls[j].nOOBs++;
            }
            for(int i=0; i<m_nKickButtonCount; i++)
            {
                CKickButton* p = m_ppKickButtonControls[i];
                if(p->getTag() == j)
                    m_pGUIControls[j].nKickButtons++;
            }
            for(int i=0; i<m_nVuMeterCount; i++)
            {
                CVuMeter* p = m_ppVuMeterControls[i];
                if(p->getTag() == j)
                    m_pGUIControls[j].nMeters++;
            }
            for(int i=0; i<m_nOptionMenuCount; i++)
            {
                COptionMenu* p = m_ppOptionMenuControls[i];
                if(p->getTag() == j)
                    m_pGUIControls[j].nOptionMenus++;
            }
            for(int i=0; i<m_nSegmentButtonCount; i++)
            {
                CSegmentButton* p = m_ppSegmentButtons[i];
                if(p->getTag() == j)
                    m_pGUIControls[j].nSegButtons++;
            }
            for(int i=0; i<m_nViewCount; i++)
            {
                CControl* c0 = dynamic_cast<CControl*>(m_ppViews[i]);
                if(c0)
                {
                    if(c0->getTag() == j)
                        m_pGUIControls[j].nControls++;
                }
            }
        }

        // --- second pass for pointers
        for(int j=0; j<nControlCount; j++)
        {
            if(m_pGUIControls[j].nLabels > 0)
            {
                int m = 0;
                m_pGUIControls[j].pLabel = new CTextLabel*[m_pGUIControls[j].nLabels];
                for(int i=0; i<m_nTextLabelCount; i++)
                {
                    CTextLabel* p = m_ppTextLabels[i];
                    if(p->getTag() == j)
                        m_pGUIControls[j].pLabel[m++] = p;
                }
            }
            if(m_pGUIControls[j].nEdits > 0)
            {
                int m = 0;
                m_pGUIControls[j].pEdit = new CTextEdit*[m_pGUIControls[j].nEdits];
                for(int i=0; i<m_nTextEditCount; i++)
                {
                    CTextEdit* p = m_ppTextEditControls[i];
                    if(p->getTag() == j)
                        m_pGUIControls[j].pEdit[m++] = p;
                }
            }
            if(m_pGUIControls[j].nKnobs > 0)
            {
                int m = 0;
                m_pGUIControls[j].pKnob = new CAnimKnob*[m_pGUIControls[j].nKnobs];
                for(int i=0; i<m_nAnimKnobCount; i++)
                {
                    CAnimKnob* p = m_ppKnobControls[i];
                    if(p->getTag() == j)
                        m_pGUIControls[j].pKnob[m++] = p;
                }
            }
            if(m_pGUIControls[j].nSliders > 0)
            {
                int m = 0;
                m_pGUIControls[j].pSlider = new CSlider*[m_pGUIControls[j].nSliders];
                for(int i=0; i<m_nSliderCount; i++)
                {
                    CSlider* p = m_ppSliderControls[i];
                    if(p->getTag() == j)
                        m_pGUIControls[j].pSlider[m++] = p;
                }
            }
            if(m_pGUIControls[j].nVSwitches > 0)
            {
                int m = 0;
                m_pGUIControls[j].pVSwitch = new CVerticalSwitch*[m_pGUIControls[j].nVSwitches];
                for(int i=0; i<m_nVerticalSwitchCount; i++)
                {
                    CVerticalSwitch* p = m_ppVerticalSwitchControls[i];
                    if(p->getTag() == j)
                        m_pGUIControls[j].pVSwitch[m++] = p;
                }
            }
            if(m_pGUIControls[j].nHSwitches > 0)
            {
                int m = 0;
                m_pGUIControls[j].pHSwitch = new CHorizontalSwitch*[m_pGUIControls[j].nHSwitches];
                for(int i=0; i<m_nHorizontalSwitchCount; i++)
                {
                    CHorizontalSwitch* p = m_ppHorizontalSwitchControls[i];
                    if(p->getTag() == j)
                        m_pGUIControls[j].pHSwitch[m++] = p;
                }
            }
            if(m_pGUIControls[j].nOOBs > 0)
            {
                int m = 0;
                m_pGUIControls[j].pOOB = new COnOffButton*[m_pGUIControls[j].nOOBs];
                for(int i=0; i<m_nOnOffButtonCount; i++)
                {
                    COnOffButton* p = m_ppOnOffButtonControls[i];
                    if(p->getTag() == j)
                        m_pGUIControls[j].pOOB[m++] = p;
                }
            }
            if(m_pGUIControls[j].nKickButtons > 0)
            {
                int m = 0;
                m_pGUIControls[j].pKickButton = new CKickButton*[m_pGUIControls[j].nKickButtons];
                for(int i=0; i<m_nKickButtonCount; i++)
                {
                    CKickButton* p = m_ppKickButtonControls[i];
                    if(p->getTag() == j)
                        m_pGUIControls[j].pKickButton[m++] = p;
                }
            }
            if(m_pGUIControls[j].nMeters > 0)
            {
                int m = 0;
                m_pGUIControls[j].pMeter = new CVuMeter*[m_pGUIControls[j].nMeters];
                for(int i=0; i<m_nVuMeterCount; i++)
                {
                    CVuMeter* p = m_ppVuMeterControls[i];
                    if(p->getTag() == j)
                        m_pGUIControls[j].pMeter[m++] = p;
                }
            }
            if(m_pGUIControls[j].nOptionMenus > 0)
            {
                int m = 0;
                m_pGUIControls[j].pOptionMenu = new COptionMenu*[m_pGUIControls[j].nOptionMenus];
                for(int i=0; i<m_nOptionMenuCount; i++)
                {
                    COptionMenu* p = m_ppOptionMenuControls[i];
                    if(p->getTag() == j)
                        m_pGUIControls[j].pOptionMenu[m++] = p;
                }
            }

            if(m_pGUIControls[j].nSegButtons > 0)
            {
                int m = 0;
                m_pGUIControls[j].pSegButton = new CSegmentButton*[m_pGUIControls[j].nSegButtons];
                for(int i=0; i<m_nSegmentButtonCount; i++)
                {
                    CSegmentButton* p = m_ppSegmentButtons[i];
                    if(p->getTag() == j)
                        m_pGUIControls[j].pSegButton[m++] = p;
                }
            }
            if(m_pGUIControls[j].nControls > 0)
            {
                int m = 0;
                m_pGUIControls[j].pControl = new CControl*[m_pGUIControls[j].nControls];
                for(int i=0; i<m_nViewCount; i++)
                {
                    CControl* c0 = dynamic_cast<CControl*>(m_ppViews[i]);
                    if(c0)
                    {
                        if(c0->getTag() == j)
                            m_pGUIControls[j].pControl[m++] = c0;
                    }
                }
            }
        }

    }

    // --- meter map for outbound params (meters)
    int m_nMeterMap[MAX_USER_METERS];
    void copyMeterMap(int* pMeterMap)
    {
        if(!pMeterMap) return;
        for(int i=0; i<MAX_USER_METERS; i++)
            m_nMeterMap[i] = pMeterMap[i];
    }

	// --------------------------------------------------------------------------------------------------
	const char* getEnumString(const char* sTag);

    inline bool setCurrentBackBitmap(const char* bitmap, CFrame* pFrame = NULL)
	{
		if(pFrame == NULL)
			pFrame = frame;

		if(strcmp(bitmap, "none") == 0)
		{
			pFrame->setBackground(NULL);
			pFrame->invalid();
			pFrame->idle();

			vstguiOutSize outSize;
			pugi::xml_node node;
			if(pFrame->getAttribute(RAFX_XML_NODE, sizeof(pugi::xml_node*), &node, outSize))
				m_XMLParser.setAttribute(node, "bitmap", "");

			return true;
		}

		CNinePartTiledBitmap* backgroundTiledBMP;
		CBitmap* backgroundBMP;

		// --- lookup the bitmap from the file
		if(m_XMLParser.hasBitmap(bitmap))
		{
			// --- get the bitmap file name (png)
			const char_t* bitmapFilename = m_XMLParser.getBitmapAttribute(bitmap, "path");

			if(m_XMLParser.isBitmapTiled(bitmap))
			{
				const char_t* offsets = m_XMLParser.getBitmapAttribute(bitmap, "nineparttiled-offsets");
				CCoord left, top, right, bottom = 0;
				getTiledOffsets(offsets, left, top, right, bottom);

				backgroundTiledBMP = loadTiledBitmap(bitmapFilename, left, top, right, bottom);
				pFrame->setBackground(backgroundTiledBMP);
				if(backgroundTiledBMP)
					backgroundTiledBMP->forget();
			}
			else
			{
				backgroundBMP = loadBitmap(bitmapFilename);
				pFrame->setBackground(backgroundBMP);
				if(backgroundBMP)
					backgroundBMP->forget();
			}

			vstguiOutSize outSize;
			pugi::xml_node node;
			if(pFrame->getAttribute(RAFX_XML_NODE, sizeof(pugi::xml_node*), &node, outSize))
				m_XMLParser.setAttribute(node, "bitmap", bitmap);

			return true;
		}

		return false;
	}

	// --- FONT
	inline CFontDesc* createFontDesc(const char* font, int nWeight,
									 bool bBold = false, bool bItalic = false,
									 bool bStrike = false, bool bUnder = false)
	{
		const char_t* fontname = m_XMLParser.getFontAttribute(font, "font-name");
		string str(fontname);
		CFontDesc* fontDesc = NULL;
		if(strlen(fontname) <= 0)
		{
			const CFontRef builtInFont = getBuiltInFont(font);
			if(builtInFont)
			{
				fontDesc = builtInFont;
			}
		}
		else
			fontDesc = new CFontDesc(fontname);

		//
		const char_t* fontSize = intToString(nWeight);
		if(strlen(fontSize) > 0)
		{
			string ccoord(fontSize);
			const CCoord fontsize = ::atof(ccoord.c_str());
			fontDesc->setSize(fontsize);
		}

		// --- bold
		if(bBold)
			fontDesc->setStyle(kBoldFace);

		// --- ital
		if(bItalic)
			fontDesc->setStyle(fontDesc->getStyle() | kItalicFace);

		if(bStrike)
			fontDesc->setStyle(fontDesc->getStyle() | kStrikethroughFace);

		if(bUnder)
			fontDesc->setStyle(fontDesc->getStyle() | kUnderlineFace);

		return fontDesc;
	}

	inline const CRect getRectFromNode(pugi::xml_node viewNode)
	{
		// --- make rect
		const char_t* origin = m_XMLParser.getAttribute(viewNode, "origin");
		string sOrigin = string(origin);

		int x, y = 0;
		if(!getXYFromString(sOrigin, x, y))
		{
			const CRect rect(0,0,0,0);
			return rect;
		}

		const char_t* size = m_XMLParser.getAttribute(viewNode, "size");
		string sSize = string(size);

		int w, h = 0;
		if(!getXYFromString(sSize, w, h))
		{
			const CRect rect(0,0,0,0);
			return rect;
		}
		const CRect rect(x, y, x+w, y+h);
		return rect;
	}
	inline bool getXYFromString(string str, int& x, int& y)
	{
		if(str.length() <= 0)
			return false;

		int nComma = str.find_first_of(',');
		if(nComma <= 0)
			return false;

		string sX = str.substr(0, nComma);
		string sY = str.substr(nComma+1);

		x = atoi(sX.c_str());
		y = atoi(sY.c_str());
		return true;
	}
	inline const CPoint getCPointFromString(string offset)
	{
		int x, y = 0;
		getXYFromString(offset, x, y);
		return CPoint(x,y);
	}
	inline bool getTiledOffsets(string str, CCoord& left, CCoord& top, CCoord& right, CCoord& bottom)
	{
		left = 0; top = 0; right = 0; bottom = 0;
		if(str.length() <= 0)
			return false;

		int nComma = str.find_first_of(',');
		if(nComma <= 0)
			return false;

		string sLeft = str.substr(0, nComma);
		string sRem1 = str.substr(nComma+1);

		nComma = sRem1.find_first_of(',');
		if(nComma <= 0)
			return false;

		string sTop = sRem1.substr(0, nComma);
		string sRem2 = sRem1.substr(nComma+1);

		nComma = sRem2.find_first_of(',');
		if(nComma <= 0)
			return false;

		string sRight = sRem2.substr(0, nComma);
		string sBottom = sRem2.substr(nComma+1);

		left = atof(sLeft.c_str());
		top = atof(sTop.c_str());
		right = atof(sRight.c_str());
		bottom = atof(sBottom.c_str());

		return true;
	}
	inline bool getRGBAFromString(string str, int& r, int& g, int& b, int& a)
	{
		if(str.length() <= 0)
			return false;

		string strAlpha = str.substr(7,2);
		stringstream ss1(strAlpha);
		ss1 >> hex >> a;

		string strR = str.substr(1,2);
		string strG = str.substr(3,2);
		string strB = str.substr(5,2);

		stringstream ss2(strR);
		ss2 >> hex >> r;
		stringstream ss3(strG);
		ss3 >> hex >> g;
		stringstream ss4(strB);
		ss4 >> hex >> b;
		return true;
	}
	inline bool isBuiltInColor(string str, CColor& builtInColor)
	{
		char c = str.at(0);
		if(c == '~')
		{
			if(strcmp(str.c_str(), "~ TransparentCColor") == 0)
			{
				builtInColor = kTransparentCColor; return true;
			}
			if(strcmp(str.c_str(), "~ BlackCColor") == 0)
			{
				builtInColor = kBlackCColor; return true;
			}
			if(strcmp(str.c_str(), "~ WhiteCColor") == 0)
			{
				builtInColor = kWhiteCColor; return true;
			}
			if(strcmp(str.c_str(), "~ GreyCColor") == 0)
			{
				builtInColor = kGreyCColor; return true;
			}
			if(strcmp(str.c_str(), "~ RedCColor") == 0)
			{
				builtInColor = kRedCColor; return true;
			}
			if(strcmp(str.c_str(), "~ GreenCColor") == 0)
			{
				builtInColor = kGreenCColor; return true;
			}
			if(strcmp(str.c_str(), "~ BlueCColor") == 0)
			{
				builtInColor = kBlueCColor; return true;
			}
			if(strcmp(str.c_str(), "~ YellowCColor") == 0)
			{
				builtInColor = kYellowCColor; return true;
			}
			if(strcmp(str.c_str(), "~ CyanCColor") == 0)
			{
				builtInColor = kCyanCColor; return true;
			}
			if(strcmp(str.c_str(), "~ MagentaCColor") == 0)
			{
				builtInColor = kMagentaCColor; return true;
			}
		}
		return false;
	}
	inline const CFontRef getBuiltInFont(string str)
	{
		char c = str.at(0);
		if(c == '~')
		{
			if(strcmp(str.c_str(), "~ SystemFont") == 0)
				return kSystemFont;
			if(strcmp(str.c_str(), "~ NormalFontVeryBig") == 0)
				return kNormalFontVeryBig;
			if(strcmp(str.c_str(), "~ NormalFontBig") == 0)
				return kNormalFontBig;
			if(strcmp(str.c_str(), "~ NormalFont") == 0)
				return kNormalFont;
			if(strcmp(str.c_str(), "~ NormalFontSmall") == 0)
				return kNormalFontSmall;
			if(strcmp(str.c_str(), "~ NormalFontSmaller") == 0)
				return kNormalFontSmaller;
			if(strcmp(str.c_str(), "~ NormalFontVerySmall") == 0)
				return kNormalFontVerySmall;
			if(strcmp(str.c_str(), "~ SymbolFont") == 0)
				return kSymbolFont;
		}
		return NULL;
	}
	inline CColor getCColor(const char_t* colorname)
	{
		if(strlen(colorname) <= 0)
			return CColor(0, 0, 0, 1);

		string sColorName(colorname);
		CColor theCColor;
		if(isBuiltInColor(sColorName, theCColor))
			return theCColor;
		else
		{
			// do we have it?
			if(m_XMLParser.hasColor(colorname))
			{
				const char_t* rgba = m_XMLParser.getColorAttribute(colorname, "rgba");
				string sRGBA(rgba);
				int r, g, b, a = 255;
				getRGBAFromString(sRGBA, r, g, b, a);
				return CColor(r, g, b, a);
			}
		}
		// --- may be empty/black
		return theCColor;
	}
	inline string COLORREFtoRGBAstring(DWORD cr)
	{
		char cResultR[32];
		char cResultG[32];
		char cResultB[32];
		BYTE r = GetRValue(cr);	/* get R, G, and B out of DWORD */
		BYTE g = GetGValue(cr);
		BYTE b = GetBValue(cr);
		sprintf(cResultR, "%X", r);
		sprintf(cResultG, "%X", g);
		sprintf(cResultB, "%X", b);
		string sR(cResultR);
		string sG(cResultG);
		string sB(cResultB);
		if(sR.size() == 1) sR = "0" + sR;
		if(sG.size() == 1) sG = "0" + sG;
		if(sB.size() == 1) sB = "0" + sB;
		string sResult = "#";
		sResult.append(sR);
		sResult.append(sG);
		sResult.append(sB);
		sResult.append("ff");

		return sResult;
	}

	inline CBitmap* getLoadBitmap(pugi::xml_node viewNode, const char_t* bmName = "bitmap")
	{
		const char_t* bitmapname = m_XMLParser.getAttribute(viewNode, bmName);
		const char_t* bitmapFilename = m_XMLParser.getBitmapAttribute(bitmapname, "path");

        if(strlen(bitmapname) <= 0)return NULL;
        if(strlen(bitmapFilename) <= 0)return NULL;

		CNinePartTiledBitmap* backgroundTiledBMP = NULL;
		CBitmap* backgroundBMP = NULL;

		if(m_XMLParser.isBitmapTiled(bitmapname))
		{
			const char_t* offsets = m_XMLParser.getBitmapAttribute(bitmapname, "nineparttiled-offsets");
			CCoord left, top, right, bottom = 0;
			getTiledOffsets(offsets, left, top, right, bottom);

			backgroundTiledBMP = loadTiledBitmap(bitmapFilename, left, top, right, bottom);
			return backgroundTiledBMP;
		}
		else
		{
			backgroundBMP = loadBitmap(bitmapFilename);
			return backgroundBMP;
		}

		return NULL;
	}

   	inline CBitmap* getLoadBitmap(const char_t* bmName)
	{
		const char_t* bitmapFilename = m_XMLParser.getBitmapAttribute(bmName, "path");

		CNinePartTiledBitmap* backgroundTiledBMP = NULL;
		CBitmap* backgroundBMP = NULL;

		if(m_XMLParser.isBitmapTiled(bmName))
		{
			const char_t* offsets = m_XMLParser.getBitmapAttribute(bmName, "nineparttiled-offsets");
			CCoord left, top, right, bottom = 0;
			getTiledOffsets(offsets, left, top, right, bottom);

			backgroundTiledBMP = loadTiledBitmap(bitmapFilename, left, top, right, bottom);
			return backgroundTiledBMP;
		}
		else
		{
			backgroundBMP = loadBitmap(bitmapFilename);
			return backgroundBMP;
		}

		return NULL;
	}

	inline int getEnumStringIndex(char* enumString, const char* testString)
	{
		string sEnumStr(enumString);
		string sTestStr(testString);
		int index = 0;
		bool bWorking = true;
		while(bWorking)
		{
			int nComma = sEnumStr.find_first_of(',');
			if(nComma <= 0)
			{
				if(sEnumStr == sTestStr)
					return index;

				bWorking = false;
			}
			else
			{
				string sL = sEnumStr.substr(0, nComma);
				sEnumStr = sEnumStr.substr(nComma+1);

				if(sL == sTestStr)
					return index;

				index++;
			}
		}

		return -1;
	}

   	inline int getNumEnums(char* string)
	{
		if(strlen(string) <= 0)
			return 0;
		std::string sTemp(string);
		std::string s(string);
		s.erase(std::remove(s.begin(), s.end(), ','), s.end());
		int nLen = sTemp.size() - s.size();
		return nLen+1;
	}

	inline char* getEnumString(char* string, int index)
	{
		int nLen = strlen(string);
		char* copyString = new char[nLen+1];

		strncpy(copyString, string, nLen);
        copyString[nLen] = '\0';

		for(int i=0; i<index+1; i++)
		{
			char * comma = ",";

			int j = strcspn (copyString,comma);

			if(i==index)
			{
				char* pType = new char[j+1];
				strncpy (pType, copyString, j);
				pType[j] = '\0';
				delete [] copyString;

				// special support for 2-state switches
				// (new in RAFX 5.4.14)
				if(strcmp(pType, "SWITCH_OFF") == 0)
				{
					delete [] pType;
					return "OFF";
				}
				else if(strcmp(pType, "SWITCH_ON") == 0)
				{
					delete [] pType;
					return "ON";
				}

				return pType;
			}
			else // remove it
			{
				char* pch = strchr(copyString,',');

				if(!pch)
				{
					delete [] copyString;
					return NULL;
				}

				int nLen = strlen(copyString);
				memcpy (copyString,copyString+j+1,nLen-j);
			}
		}

		delete [] copyString;
		return NULL;
	}

    #ifdef AAXPLUGIN
    // --- user must delete string
    inline char* createDisplayString(CUICtrl* pCtrl, float fNormalizedValue, unsigned int uPrecision)
    {
        if(!pCtrl) return "";
        char* pDisplayString = "";
        if(pCtrl->uUserDataType == intData)
            uPrecision = 0;

        float fDisplayVariable = calcDisplayVariable(pCtrl->fUserDisplayDataLoLimit, pCtrl->fUserDisplayDataHiLimit, fNormalizedValue);

        switch(pCtrl->uUserDataType)
        {
            case floatData:
            {
                pDisplayString = floatToString(fDisplayVariable, uPrecision);
                break;
            }
            case doubleData:
            {
                pDisplayString = doubleToString(fDisplayVariable, uPrecision);
                break;
            }
            case intData:
            {
                pDisplayString = intToString(fDisplayVariable);
                break;
            }
            case UINTData:
            {
                pDisplayString = getEnumString(pCtrl->cEnumeratedList, floor(fDisplayVariable + 0.5));
                break;
            }
            default:
                break;
        }

        return pDisplayString;
    }

    inline void setAAXParameterFromGUIControl(CControl* pControl, CUICtrl* pCtrl, int nPlugInControlIndex, float fNormalizedControlValue, bool bCheckUpdateGUI = true)
    {
        if(m_pAAXParameters)
        {
            if(pCtrl && pCtrl->uUserDataType == UINTData)
            {
                float fActualControlValue = getActualPluginParameterValue(pCtrl, pControl, fNormalizedControlValue);
                fNormalizedControlValue = calcSliderVariable(pCtrl->fUserDisplayDataLoLimit, pCtrl->fUserDisplayDataHiLimit, fActualControlValue);
                doTabControlSwitch(nPlugInControlIndex, fActualControlValue); // does view switch
            }

            // --- threadsafe atomic write
            std::stringstream str;
            str << nPlugInControlIndex+1;
            m_pAAXParameters->SetParameterNormalizedValue(str.str().c_str(),fNormalizedControlValue);

            // --- vector joystick does not use these
            if(pCtrl && pControl)
            {
                // --- get actual value
                float fActualControlValue = getActualPluginParameterValue(pCtrl, pControl, fNormalizedControlValue);

                // --- v6.8 threadsafe way to do updates from plugin
                if(bCheckUpdateGUI)
                    checkSendUpdateGUI(pCtrl->uControlId, fActualControlValue, false);
            }
        }
    }

    inline void updateGUIControlsAAX(int nTag, float fNormalizedValue)
    {
        if(!m_pControlMap) return;
        if(nTag < 0) return;
        int nParams = m_GUI_UIControlList.count();

        int nControlIndex = m_XMLParser.getTagIndex(nTag);
        int nPlugInControlIndex = m_pControlMap[nControlIndex];
        CUICtrl* pCtrl = m_GUI_UIControlList.getAt(nPlugInControlIndex); // note: using copy of list here; can't screw it up

        if(!pCtrl)
        {
            // --- check Vector Joystick
            for(int i=0; i<m_nXYPadCount; i++)
            {
                if(isCXYPadWP(m_ppXYPads[i]))
                {
                    if(((CXYPadWP*)m_ppXYPads[i])->m_bDraggingPuck)
                        continue;

                    if(((CXYPadWP*)m_ppXYPads[i])->getTagX() == JOYSTICK_X && ((CXYPadWP*)m_ppXYPads[i])->getTagY() == JOYSTICK_Y)
                    {
                        if(nTag == nParams + vectorJoystickX_Offset || nTag == nParams + vectorJoystickY_Offset)
                        {
                            // --- restore JS
                            float x, y;
                            m_ppXYPads[i]->calculateXY(m_ppXYPads[i]->getValue(), x, y);
                            y = -1.0*y + 1.0;

                            if(nTag == nParams + vectorJoystickX_Offset)
                                x = fNormalizedValue;
                            else
                                y = fNormalizedValue;

                            float val = m_ppXYPads[i]->calculateValue(x, y);
                            m_ppXYPads[i]->setValue(val);
                            m_ppXYPads[i]->invalid();
                        }
                    }
                }
            }
            return;
        }

        // --- v6.8
        string units(pCtrl->cControlUnits);
        trim(units);

        float fKnobSliderValue = fNormalizedValue;
        float fDisplayValue = fNormalizedValue;

        if(pCtrl->bLogSlider)
        {
            fDisplayValue = calcLogPluginValue(fDisplayValue);
        }
        else if(pCtrl->bExpSlider)
        {
            fDisplayValue = calcVoltOctavePluginValue(fDisplayValue, pCtrl);
        }

        if(m_pGUIControls && nTag < m_nControlCount)
        {
            GUI_CONTROL_STRUCT GUIControls = m_pGUIControls[nTag];

            if(GUIControls.pLabel && GUIControls.nLabels > 0)
            {
                for(int i=0; i<GUIControls.nLabels; i++)
                {
                    unsigned int uPrecision = GUIControls.pLabel[i]->getPrecision();
                    char* pDisplayString = createDisplayString(pCtrl, fDisplayValue, uPrecision);

                    GUIControls.pLabel[i]->setText(pDisplayString);
                    delete [] pDisplayString;

                    // --- RAFX v6.8
                    if(m_bAppendUnits)
                    {
                        string editString(GUIControls.pLabel[i]->getText());
                        editString.append(" ");
                        editString.append(units);
                        (GUIControls.pLabel[i])->setText(editString.c_str());
                    }

                    GUIControls.pLabel[i]->invalid();
                }
            }

            if(GUIControls.pEdit && GUIControls.nEdits > 0)
            {
                for(int i=0; i<GUIControls.nEdits; i++)
                {
                    unsigned int uPrecision = GUIControls.pEdit[i]->getPrecision();
                    char* pDisplayString = createDisplayString(pCtrl, fDisplayValue, uPrecision);

                    GUIControls.pEdit[i]->setValueNormalized(fNormalizedValue);
                    GUIControls.pEdit[i]->setText(pDisplayString);
                    delete [] pDisplayString;

                    // --- RAFX v6.8
                    if(m_bAppendUnits)
                    {
                        string editString(GUIControls.pEdit[i]->getText());
                        editString.append(" ");
                        editString.append(units);
                        (GUIControls.pEdit[i])->setText(editString.c_str());
                    }

                    GUIControls.pEdit[i]->invalid();
                }
            }

            // --- user views
            if(GUIControls.pControl && GUIControls.nControls > 0)
            {
                for(int i=0; i<GUIControls.nControls; i++)
                {
                    GUIControls.pControl[i]->setValue(fNormalizedValue);
                    GUIControls.pControl[i]->invalid();
                }
            }

            if(GUIControls.pKnob && GUIControls.nKnobs > 0)
            {
                for(int i=0; i<GUIControls.nKnobs; i++)
                {
                    // --- prevent flicker
                    if(isKnobWPControl(GUIControls.pKnob[i]) && GUIControls.pKnob[i]->isEditing())
                        continue;

                    GUIControls.pKnob[i]->setValue(fKnobSliderValue);
                    GUIControls.pKnob[i]->invalid();
                }
            }

            if(GUIControls.pSlider && GUIControls.nSliders > 0)
            {
                for(int i=0; i<GUIControls.nSliders; i++)
                {
                    // --- prevent flicker
                    if(isVerticalSliderWPControl(GUIControls.pSlider[i]) && GUIControls.pSlider[i]->isEditing())
                        continue;
                    if(isHorizontalSliderWPControl(GUIControls.pSlider[i]) && GUIControls.pSlider[i]->isEditing())
                        continue;

                    GUIControls.pSlider[i]->setValue(fKnobSliderValue);
                    GUIControls.pSlider[i]->invalid();
                }
            }

            if(GUIControls.pKickButton && GUIControls.nKickButtons > 0)
            {
                for(int i=0; i<GUIControls.nKickButtons; i++)
                {
                    GUIControls.pKickButton[i]->setValue(fNormalizedValue);
                    GUIControls.pKickButton[i]->invalid();
                }
            }

            if(GUIControls.pOOB && GUIControls.nOOBs > 0)
            {
                for(int i=0; i<GUIControls.nOOBs; i++)
                {
                    GUIControls.pOOB[i]->setValue(fNormalizedValue);
                    GUIControls.pOOB[i]->invalid();
                }
            }

            if(GUIControls.pVSwitch && GUIControls.nVSwitches > 0)
            {
                for(int i=0; i<GUIControls.nVSwitches; i++)
                {
                    float fV = fNormalizedValue*(float)(GUIControls.pVSwitch[i]->getMax());
                    GUIControls.pVSwitch[i]->setValue(floor(fV + 0.5));
                    GUIControls.pVSwitch[i]->invalid();

                    doTabControlSwitch(nTag, GUIControls.pVSwitch[i]->getValue()); // does view switch
                }
            }

            if(GUIControls.pHSwitch && GUIControls.nHSwitches > 0)
            {
                for(int i=0; i<GUIControls.nHSwitches; i++)
                {
                    float fV = fNormalizedValue*(float)(GUIControls.pHSwitch[i]->getMax());
                    GUIControls.pHSwitch[i]->setValue(floor(fV + 0.5));
                    GUIControls.pHSwitch[i]->invalid();

                    doTabControlSwitch(nTag, GUIControls.pHSwitch[i]->getValue()); // does view switch
                }
            }

            if(GUIControls.pSegButton && GUIControls.nSegButtons > 0)
            {
                for(int i=0; i<GUIControls.nSegButtons; i++)
                {
                    float fV = fNormalizedValue*(float)(GUIControls.pSegButton[i]->getMax());
                    GUIControls.pSegButton[i]->setValue(floor(fV + 0.5));
                    GUIControls.pSegButton[i]->invalid();

                    doTabControlSwitch(nTag, GUIControls.pSegButton[i]->getSelectedSegment()); // does view switch
                }
            }

            if(GUIControls.pMeter && GUIControls.nMeters > 0)
            {
                for(int i=0; i<GUIControls.nMeters; i++)
                {
                    GUIControls.pMeter[i]->setValue(fNormalizedValue);
                    GUIControls.pMeter[i]->invalid();
                }
            }

            if(GUIControls.pOptionMenu && GUIControls.nOptionMenus > 0)
            {
                for(int i=0; i<GUIControls.nOptionMenus; i++)
                {
                    float fV = fNormalizedValue*(float)(GUIControls.pOptionMenu[i]->getNbEntries() - 1);
                    GUIControls.pOptionMenu[i]->setCurrent(floor(fV + 0.5));
                    GUIControls.pOptionMenu[i]->invalid();

                    doTabControlSwitch(nTag, GUIControls.pOptionMenu[i]->getCurrentIndex()); // does view switch
                }
            }

            // --- handled the old fashioned way
            for(int i=0; i<m_nXYPadCount; i++)
            {
                if(isCXYPadWP(m_ppXYPads[i]))
                {
                    if(((CXYPadWP*)m_ppXYPads[i])->m_bDraggingPuck)
                        continue;

                    if(((CXYPadWP*)m_ppXYPads[i])->getTagX() == nTag || ((CXYPadWP*)m_ppXYPads[i])->getTagY() == nTag)
                    {
                        float x, y;
                        m_ppXYPads[i]->calculateXY(m_ppXYPads[i]->getValue(), x, y);
                        y = -1.0*y + 1.0;

                        if(((CXYPadWP*)m_ppXYPads[i])->getTagX() == nTag)
                            x = fNormalizedValue;
                        else
                            y = fNormalizedValue;

                        float val = m_ppXYPads[i]->calculateValue(x, y);
                        m_ppXYPads[i]->setValue(val);
                        m_ppXYPads[i]->invalid();
                    }
                }
            }
        }
    }
    #endif

    // --- uses acutal, not normalized value
    inline float calcVoltOctaveParameter(CUICtrl* pCtrl, float fValue)
    {
        float fRawValue = fValue;
        if(pCtrl->fUserDisplayDataLoLimit > 0)
        {
            double dOctaves = fastlog2(pCtrl->fUserDisplayDataHiLimit/pCtrl->fUserDisplayDataLoLimit);
            fRawValue = fastlog2(fValue/pCtrl->fUserDisplayDataLoLimit)/dOctaves;
        }
        return fRawValue;
    }

    inline float getActualValue(float fNormalizedValue, CUICtrl* pUICtrl, bool bWarpValue = false)
    {
        if(bWarpValue)
        {
            if(pUICtrl->bLogSlider && pUICtrl->uUserDataType != UINTData)
            {
                fNormalizedValue = calcLogPluginValue(fNormalizedValue);
            }
            else if(pUICtrl->bExpSlider && pUICtrl->uUserDataType != UINTData)
            {
                if(pUICtrl->fUserDisplayDataLoLimit > 0)
                {
                    fNormalizedValue = calcVoltOctavePluginValue(fNormalizedValue, pUICtrl);
                }
            }
        }

        float fActualValue = calcDisplayVariable(pUICtrl->fUserDisplayDataLoLimit,
                                                 pUICtrl->fUserDisplayDataHiLimit,
                                                 fNormalizedValue);

        return fActualValue;
    }

    inline float getDefaultValue(CUICtrl* pCtrl)
    {
        if(!pCtrl) return 0.f;
        
        if(pCtrl->uUserDataType == floatData)
            return pCtrl->fInitUserFloatValue;
        if(pCtrl->uUserDataType == doubleData)
            return pCtrl->fInitUserDoubleValue;
        if(pCtrl->uUserDataType == intData)
            return pCtrl->fInitUserIntValue;
        if(pCtrl->uUserDataType == UINTData)
            return pCtrl->fInitUserUINTValue;
        
        return 0.0;
    }

    inline float getNormalizedValue(float fActualControlValue, CUICtrl* pCtrl)
    {
        if(!pCtrl) return 0.f;
        
        float fNormalizedValue = calcSliderVariable(pCtrl->fUserDisplayDataLoLimit, pCtrl->fUserDisplayDataHiLimit, fActualControlValue);

        if(pCtrl->bLogSlider && pCtrl->uUserDataType != UINTData)
        {
            fNormalizedValue = calcLogParameter(fNormalizedValue);
        }
        else if(pCtrl->bExpSlider && pCtrl->uUserDataType != UINTData)
        {
            fNormalizedValue = calcVoltOctaveParameter(fActualControlValue, pCtrl);
        }
        return fNormalizedValue;
    }

    inline float applyParameterTaper(CUICtrl* pCtrl, float fActualControlValue, float fNormalizedValue)
    {
        if(pCtrl->bLogSlider && pCtrl->uUserDataType != UINTData)
        {
            fNormalizedValue = calcLogParameter(fNormalizedValue);
        }
        else if(pCtrl->bExpSlider && pCtrl->uUserDataType != UINTData)
        {
            fNormalizedValue = calcVoltOctaveParameter(fActualControlValue, pCtrl);
        }

        return fNormalizedValue;
    }

    // --- individual updaters
    inline void updateTextEdits(int nTag, float fActualControlValue)
    {

        for(int i=0; i<m_nTextEditCount; i++)
        {
            if(m_ppTextEditControls[i]->getTag() == nTag)
            {
                unsigned int uPrecision = m_ppTextEditControls[i]->getPrecision();

                int nControlIndex = m_XMLParser.getTagIndex(nTag);
                int nPlugInControlIndex = m_pControlMap[nControlIndex];

                // --- get the control for re-broadcast of some types
                CUICtrl* pCtrl = m_GUI_UIControlList.getAt(nPlugInControlIndex);

                if(!pCtrl) return;

                string units(pCtrl->cControlUnits);
                trim(units);

                if(pCtrl->uUserDataType == intData)
                    uPrecision = 0;

                if(pCtrl->uUserDataType == UINTData)
                {
                    char* pEnum;
                    pEnum = getEnumString(pCtrl->cEnumeratedList, (int)(fActualControlValue));
                    if(pEnum)
                        m_ppTextEditControls[i]->setText(pEnum);
                }
                else
                {
                    m_ppTextEditControls[i]->setText(floatToString(fActualControlValue, uPrecision));
                }

                // --- RAFX v6.8
                if(m_bAppendUnits)
                {
                    string editString(m_ppTextEditControls[i]->getText());
                    editString.append(" ");
                    editString.append(units);
                    (m_ppTextEditControls[i])->setText(editString.c_str());
                }
            }
        }
    }

    inline void updateKnobs(int nTag, float fActualControlValue)
    {
        for(int i=0; i<m_nAnimKnobCount; i++)
        {
            if(m_ppKnobControls[i]->getTag() == nTag)
            {
                int nControlIndex = m_XMLParser.getTagIndex(nTag);
                int nPlugInControlIndex = m_pControlMap[nControlIndex];

                // --- get the control for re-broadcast of some types
                CUICtrl* pCtrl = m_GUI_UIControlList.getAt(nPlugInControlIndex);
                if(!pCtrl) return;

                float fNormalizedValue = calcSliderVariable(pCtrl->fUserDisplayDataLoLimit, pCtrl->fUserDisplayDataHiLimit, fActualControlValue);

                if(pCtrl->bLogSlider && pCtrl->uUserDataType != UINTData)
                {
                    fNormalizedValue = calcLogParameter(fNormalizedValue);
                }
                else if(pCtrl->bExpSlider && pCtrl->uUserDataType != UINTData)
                {
                    fNormalizedValue = calcVoltOctaveParameter(fActualControlValue, pCtrl);
                }

                m_ppKnobControls[i]->setValueNormalized(fNormalizedValue);
                m_ppKnobControls[i]->invalid();
            }
        }
    }

    inline void updateSliders(int nTag, float fActualControlValue)
    {
        for(int i=0; i<m_nSliderCount; i++)
        {
            if(m_ppSliderControls[i]->getTag() == nTag)
            {
                int nControlIndex = m_XMLParser.getTagIndex(nTag);
                int nPlugInControlIndex = m_pControlMap[nControlIndex];

                // --- get the control for re-broadcast of some types
                CUICtrl* pCtrl = m_GUI_UIControlList.getAt(nPlugInControlIndex);
                if(!pCtrl) return;

                float fNormalizedValue = calcSliderVariable(pCtrl->fUserDisplayDataLoLimit, pCtrl->fUserDisplayDataHiLimit, fActualControlValue);

                if(pCtrl->bLogSlider && pCtrl->uUserDataType != UINTData)
                {
                    fNormalizedValue = calcLogParameter(fNormalizedValue);
                }
                else if(pCtrl->bExpSlider && pCtrl->uUserDataType != UINTData)
                {
                    fNormalizedValue = calcVoltOctaveParameter(fActualControlValue, pCtrl);
                }

                m_ppSliderControls[i]->setValue(fNormalizedValue);
                m_ppSliderControls[i]->invalid();
            }
        }
    }

    inline void updateTaggedTextLabels(int nTag, float fActualPluginValue)
    {
        for(int i=0; i<m_nTaggedTextLabelCount; i++)
        {
            if(m_ppTaggedTextLabels[i]->getTag() == nTag)
            {
                unsigned int uPrecision = m_ppTaggedTextLabels[i]->getPrecision();

                int nControlIndex = m_XMLParser.getTagIndex(nTag);
                int nPlugInControlIndex = m_pControlMap[nControlIndex];

                // --- get the control for re-broadcast of some types
                CUICtrl* pCtrl = m_GUI_UIControlList.getAt(nPlugInControlIndex);
                if(!pCtrl) return;

                string units(pCtrl->cControlUnits);
                trim(units);

                if(pCtrl->uUserDataType == intData)
                    uPrecision = 0;

                if(pCtrl->uUserDataType == UINTData)
                {
                    char* pEnum;

                    //pEnum = getEnumString(pCtrl->cEnumeratedList, floor(fActualControlValue + 0.5));
                    pEnum = getEnumString(pCtrl->cEnumeratedList, (int)(fActualPluginValue));
                    if(pEnum)
                        m_ppTaggedTextLabels[i]->setText(pEnum);
                }
                else
                {
                    m_ppTaggedTextLabels[i]->setText(floatToString(fActualPluginValue, uPrecision));
                }

                // --- RAFX v6.8
                if(m_bAppendUnits)
                {
                    string editString(m_ppTaggedTextLabels[i]->getText());
                    editString.append(" ");
                    editString.append(units);
                    (m_ppTaggedTextLabels[i])->setText(editString.c_str());
                }
            }
        }
    }

    inline void updateButtons(int nTag, float fActualPluginValue)
    {
        for(int i=0; i<m_nKickButtonCount; i++)
        {
            if(m_ppKickButtonControls[i]->getTag() == nTag)
            {
                m_ppKickButtonControls[i]->setValue(fActualPluginValue);
                m_ppKickButtonControls[i]->invalid();
            }
        }

        for(int i=0; i<m_nOnOffButtonCount; i++)
        {
            if(m_ppOnOffButtonControls[i]->getTag() == nTag)
            {
                m_ppOnOffButtonControls[i]->setValue(fActualPluginValue);
                m_ppOnOffButtonControls[i]->invalid();
            }
        }
    }

    inline void updateOptionMenus(int nTag, float fActualPluginValue)
    {
        for(int i=0; i<m_nOptionMenuCount; i++)
        {
            if(m_ppOptionMenuControls[i]->getTag() == nTag)
            {
                if(m_ppOptionMenuControls[i]->getNbEntries() > fActualPluginValue)
                {
                    m_ppOptionMenuControls[i]->setCurrent((int)fActualPluginValue);
                    m_ppOptionMenuControls[i]->invalid();

                    doTabControlSwitch(nTag, m_ppOptionMenuControls[i]->getCurrentIndex()); // does view switch
                }
            }
        }
    }

    inline void updateSegmentButtons(int nTag, float fActualPluginValue)
    {
        for(int i=0; i<m_nSegmentButtonCount; i++)
        {
            if(m_ppSegmentButtons[i]->getTag() == nTag)
            {
                m_ppSegmentButtons[i]->setValue((int)fActualPluginValue);
                m_ppSegmentButtons[i]->invalid();
                doTabControlSwitch(nTag, m_ppSegmentButtons[i]->getSelectedSegment()); // does view switch
            }
        }
    }

    inline void updateVertHorizSwitches(int nTag, float fActualPluginValue)
    {
        for(int i=0; i<m_nVerticalSwitchCount; i++)
        {
            if(m_ppVerticalSwitchControls[i]->getTag() == nTag)
            {
                m_ppVerticalSwitchControls[i]->setValue((int)fActualPluginValue);
                m_ppVerticalSwitchControls[i]->invalid();
                doTabControlSwitch(nTag, m_ppVerticalSwitchControls[i]->getValue()); // does view switch
            }
        }

        for(int i=0; i<m_nHorizontalSwitchCount; i++)
        {
            if(m_ppHorizontalSwitchControls[i]->getTag() == nTag)
            {
                m_ppHorizontalSwitchControls[i]->setValue((int)fActualPluginValue);
                m_ppHorizontalSwitchControls[i]->invalid();
                doTabControlSwitch(nTag, m_ppHorizontalSwitchControls[i]->getValue()); // does view switch
            }
        }
    }

    inline void updateXYPads(int nTag, float fActualPluginValue)
    {
        int nParams = m_GUI_UIControlList.count();

        for(int i=0; i<m_nXYPadCount; i++)
        {
            if(isCXYPadWP(m_ppXYPads[i]))
            {
                if(((CXYPadWP*)m_ppXYPads[i])->getTagX() == JOYSTICK_X && ((CXYPadWP*)m_ppXYPads[i])->getTagY() == JOYSTICK_Y && !((CXYPadWP*)m_ppXYPads[i])->m_bDraggingPuck)
                {
                    if(nTag == nParams + vectorJoystickX_Offset || nTag == nParams + vectorJoystickY_Offset)
                    {
                        float x = 0;
                        float y = 0;

                        m_ppXYPads[i]->calculateXY(m_ppXYPads[i]->getValue(), x, y);
                        y = -1.0*y + 1.0;

                        if(nTag == nParams + vectorJoystickX_Offset)
                            x = bipolarToUnipolar(fActualPluginValue);
                        else
                            y = bipolarToUnipolar(fActualPluginValue);

                        m_ppXYPads[i]->setValue(m_ppXYPads[i]->calculateValue(x, y));
                        m_ppXYPads[i]->invalid();
                    }
                }
                if(((CXYPadWP*)m_ppXYPads[i])->getTagX() == nTag || ((CXYPadWP*)m_ppXYPads[i])->getTagY() == nTag)
                {
                    CXYPadWP* pPad = (CXYPadWP*)m_ppXYPads[i];
                    if(pPad->m_bDraggingPuck)
                        continue;

                    int nControlIndex = -1;
                    if((((CXYPadWP*)m_ppXYPads[i])->getTagX() == nTag))
                        nControlIndex = m_XMLParser.getTagIndex(((CXYPadWP*)m_ppXYPads[i])->getTagX());
                    else
                        nControlIndex = m_XMLParser.getTagIndex(((CXYPadWP*)m_ppXYPads[i])->getTagY());

                    int nPlugInControlIndex = m_pControlMap[nControlIndex];

                    // --- get the control for re-broadcast of some types
                    CUICtrl* pCtrl = m_GUI_UIControlList.getAt(nPlugInControlIndex);
                    if(!pCtrl) return;

                    float x, y;
                    m_ppXYPads[i]->calculateXY(m_ppXYPads[i]->getValue(), x, y);
                    y = -1.0*y + 1.0;

                    if(((CXYPadWP*)m_ppXYPads[i])->getTagX() == nTag)
                    {
                        float newX = calcSliderVariable(pCtrl->fUserDisplayDataLoLimit, pCtrl->fUserDisplayDataHiLimit, fActualPluginValue);

                        // --- RAFX v6.6
                        if(pCtrl->bLogSlider)
                            newX = calcLogParameter(newX);
                        else if(pCtrl->bExpSlider)
                        {
                            newX = calcVoltOctaveParameter(newX, pCtrl);
                            newX = 1.0 + newX;
                        }

                        float val = m_ppXYPads[i]->calculateValue(newX, y);
                        m_ppXYPads[i]->setValue(val);
                        m_ppXYPads[i]->invalid();

                    }
                    else
                    {
                        float newY = calcSliderVariable(pCtrl->fUserDisplayDataLoLimit, pCtrl->fUserDisplayDataHiLimit, fActualPluginValue);
                        // --- RAFX v6.6
                        if(pCtrl->bLogSlider)
                            newY = calcLogParameter(newY);
                        else if(pCtrl->bExpSlider && newY > 0.0)
                        {
                            newY = calcVoltOctaveParameter(newY, pCtrl);
                            newY = 1.0 + newY;// - 1.0;
                        }

                        float val = m_ppXYPads[i]->calculateValue(x, newY);
                        m_ppXYPads[i]->setValue(val);
                        m_ppXYPads[i]->invalid();
                    }
                }
            }
        }
    }

    // --- main GUI control dispatcher, threadsafe
#ifdef AUPLUGIN
    inline void dispatchControlChange(int nTag, float fActualPluginValue, bool bFromEventListener = false)
    {
        // --- knobs
        updateKnobs(nTag, fActualPluginValue);

        // --- sliders
        updateSliders(nTag, fActualPluginValue);

        // --- edit controls
        updateTextEdits(nTag, fActualPluginValue);

        // --- tagged labels
        updateTaggedTextLabels(nTag, fActualPluginValue);

        // --- buttons (on/off and momentary)
        updateButtons(nTag, fActualPluginValue);

        // --- option menus
        updateOptionMenus(nTag, fActualPluginValue);

        // --- segment buttons
        updateSegmentButtons(nTag, fActualPluginValue);

        // --- vert/horiz switches
        updateVertHorizSwitches(nTag, fActualPluginValue);

        // --- XY/Joystick
        updateXYPads(nTag, fActualPluginValue);

        // --- check for update GUI when presets are loaded
        if(bFromEventListener)
        {
            Float32 paramValue;
            int nParams = m_GUI_UIControlList.count();
            if(nTag == nParams - 1)
            {
                for(int i=0; i<nParams; i++)
                {
                    if(AudioUnitGetParameter(m_AU, i, kAudioUnitScope_Global, 0, &paramValue) != noErr)
                        return;

                    CUICtrl* p = m_GUI_UIControlList.getAt(i);
                    if(p)
                    {
                        checkSendUpdateGUI(p->uControlId, paramValue, true);
                    }
                }
            }
        }
    }

    inline void setAUEventFromGUIControl(CControl* pControl, CUICtrl* pCtrl, int nPlugInControlIndex, float fNormalizedControlValue, bool bCheckUpdateGUI = true)
    {
        // --- get control normalized value
        float fNormalizedPluginValue = getPluginParameterValue(fNormalizedControlValue, pControl);

        // --- warp
        if(pCtrl)
        {
            if(pCtrl->bLogSlider)
                fNormalizedPluginValue = calcLogPluginValue(fNormalizedPluginValue);
            else if(pCtrl->bExpSlider)
            {
                fNormalizedPluginValue = calcVoltOctavePluginValue(fNormalizedPluginValue, pCtrl);
            }

            if(pCtrl->uUserDataType == UINTData)
            {
                float fActualControlValue = getActualPluginParameterValue(pCtrl, pControl, fNormalizedControlValue);
                fNormalizedControlValue = calcSliderVariable(pCtrl->fUserDisplayDataLoLimit, pCtrl->fUserDisplayDataHiLimit, fActualControlValue);
                doTabControlSwitch(nPlugInControlIndex, fActualControlValue); // does view switch
            }

            // --- get actual value
            float fActualControlValue = getActualPluginParameterValue(pCtrl, pControl, fNormalizedPluginValue);

            // --- threadsafe atomic write
            AudioUnitParameter param = {m_AU, static_cast<AudioUnitParameterID>(nPlugInControlIndex), kAudioUnitScope_Global, 0};
            AUParameterSet(AUEventListener, pControl, &param, fActualControlValue, 0);

            // --- v6.8 threadsafe way to do updates from plugin
            if(bCheckUpdateGUI)
                checkSendUpdateGUI(pCtrl->uControlId, fActualControlValue, false);
        }
    }
#endif

    inline void doTabControlSwitch(int nTag, float fActualPluginValue)
    {
        for(int i=0; i<m_nViewContainerCount; i++)
        {
            // --- switch view?
            if(isRAFXSwitchView(m_ppViewContainers[i]) && fActualPluginValue >= 0)
            {
                int nContTag = ((CRAFXSwitchView*)m_ppViewContainers[i])->getControllerTag();
                if(nContTag == nTag)
                    ((CRAFXSwitchView*)m_ppViewContainers[i])->setCurrentViewIndex((int)fActualPluginValue);
            }
        }
    }

	inline CXYPad* findJoystickXYPad(CControl* pControl)
	{
		for(int i=0; i<m_nXYPadCount; i++)
		{
			if(m_ppXYPads[i] == (CXYPadWP*)pControl)
				return m_ppXYPads[i];
		}
		return NULL;
	}

    inline float calcLogParameter(float fNormalizedParam)
	{
        //--- inverse convex transform
		return pow(10.0, (fNormalizedParam - 1) / (5.0 / 12.0));
    }

	inline float calcLogPluginValue(float fPluginValue)
	{
        // --- convex transform
        if (fPluginValue <= 0) return 0.0;
        return 1.0 + (5.0/12.0)*log10(fPluginValue);
	}

    inline float calcVoltOctaveParameter(float fPluginValue, CUICtrl* pCtrl)
    {
        float fRawValue = pCtrl->fUserDisplayDataLoLimit;
        if(fRawValue > 0)
        {
            double dOctaves = fastlog2(pCtrl->fUserDisplayDataHiLimit/pCtrl->fUserDisplayDataLoLimit);
            fRawValue = fastlog2(fPluginValue/pCtrl->fUserDisplayDataLoLimit)/dOctaves;
        }

        return fRawValue;
    }

    inline float calcVoltOctaveParameter(CUICtrl* pCtrl)
    {
        float fRawValue = pCtrl->fUserDisplayDataLoLimit;
        if(fRawValue > 0)
        {
            double dOctaves = fastlog2(pCtrl->fUserDisplayDataHiLimit/pCtrl->fUserDisplayDataLoLimit);
            if(pCtrl->uUserDataType == intData)
                fRawValue = fastlog2(*(pCtrl->m_pUserCookedIntData)/pCtrl->fUserDisplayDataLoLimit)/dOctaves;
            else if(pCtrl->uUserDataType == floatData)
                fRawValue = fastlog2(*(pCtrl->m_pUserCookedFloatData)/pCtrl->fUserDisplayDataLoLimit)/dOctaves;
            else if(pCtrl->uUserDataType == doubleData)
                fRawValue = fastlog2(*(pCtrl->m_pUserCookedDoubleData)/pCtrl->fUserDisplayDataLoLimit)/dOctaves;
            //else if(pCtrl->uUserDataType == UINTData)
            //    fRawValue = *(pCtrl->m_pUserCookedUINTData);
        }

        return fRawValue;
    }

	inline float calcVoltOctavePluginValue(float fPluginValue, CUICtrl* pCtrl)
	{
		if(pCtrl->uUserDataType == UINTData)
			return *(pCtrl->m_pUserCookedUINTData);

		double dOctaves = fastlog2(pCtrl->fUserDisplayDataHiLimit/pCtrl->fUserDisplayDataLoLimit);
		float fDisplay = pCtrl->fUserDisplayDataLoLimit*fastpow2(fPluginValue*dOctaves); //(m_fDisplayMax - m_fDisplayMin)*value + m_fDisplayMin; //m_fDisplayMin*fastpow2(value*dOctaves);
		float fDiff = pCtrl->fUserDisplayDataHiLimit - pCtrl->fUserDisplayDataLoLimit;
		return (fDisplay - pCtrl->fUserDisplayDataLoLimit)/fDiff;
	}

	// --- called when value changes in text edit; it needs to store the new stringindex value and update itself
	inline float updateEditControl(CControl* pControl, CUICtrl* pCtrl)
	{
        string units(pCtrl->cControlUnits);
        trim(units);

		const char* p = ((CTextEdit*)pControl)->getText();
        unsigned int uPrecision = ((CTextEdit*)pControl)->getPrecision();

		float fValue = 0.0;
		switch(pCtrl->uUserDataType)
		{
			case floatData:
			{
				float f = atof(p);
				if(f > pCtrl->fUserDisplayDataHiLimit) f = pCtrl->fUserDisplayDataHiLimit;
				if(f < pCtrl->fUserDisplayDataLoLimit) f = pCtrl->fUserDisplayDataLoLimit;
				p = floatToString(f,uPrecision);
				fValue = calcSliderVariable(pCtrl->fUserDisplayDataLoLimit, pCtrl->fUserDisplayDataHiLimit, f);

                // --- RAFX v6.6
				if(pCtrl->bLogSlider)
					fValue = calcLogParameter(fValue);
				else if(pCtrl->bExpSlider)
					fValue = calcVoltOctaveParameter(f, pCtrl);

                pControl->setValue(fValue);
				((CTextEdit*)pControl)->setText(p);
				break;
			}
			case doubleData:
			{
				float f = atof(p);
				if(f > pCtrl->fUserDisplayDataHiLimit) f = pCtrl->fUserDisplayDataHiLimit;
				if(f < pCtrl->fUserDisplayDataLoLimit) f = pCtrl->fUserDisplayDataLoLimit;
				p = floatToString(f,uPrecision);
				fValue = calcSliderVariable(pCtrl->fUserDisplayDataLoLimit, pCtrl->fUserDisplayDataHiLimit, f);

                // --- RAFX v6.6
				if(pCtrl->bLogSlider)
					fValue = calcLogParameter(fValue);
				else if(pCtrl->bExpSlider)
					fValue = calcVoltOctaveParameter(f, pCtrl);

                pControl->setValue(fValue);
				((CTextEdit*)pControl)->setText(p);
				break;
			}
			case intData:
			{
				int f = atoi(p);
				if(f > pCtrl->fUserDisplayDataHiLimit) f = pCtrl->fUserDisplayDataHiLimit;
				if(f < pCtrl->fUserDisplayDataLoLimit) f = pCtrl->fUserDisplayDataLoLimit;
				p = intToString(f);
				fValue = calcSliderVariable(pCtrl->fUserDisplayDataLoLimit, pCtrl->fUserDisplayDataHiLimit, f);

                // --- RAFX v6.6
				if(pCtrl->bLogSlider)
					fValue = calcLogParameter(fValue);
				else if(pCtrl->bExpSlider)
					fValue = calcVoltOctaveParameter(f, pCtrl);

                pControl->setValue(fValue);
				((CTextEdit*)pControl)->setText(p);
				break;
			}
			case UINTData:
			{
				string str(p);
				string list(pCtrl->cEnumeratedList);
				if(list.find(str) == -1)
				{
					fValue = calcSliderVariable(pCtrl->fUserDisplayDataLoLimit, pCtrl->fUserDisplayDataHiLimit, *(pCtrl->m_pUserCookedUINTData));
					pControl->setValue(fValue);

					char* pEnum;
					pEnum = getEnumString(pCtrl->cEnumeratedList, (int)(*(pCtrl->m_pUserCookedUINTData)));
					if(pEnum)
						((CTextEdit*)pControl)->setText(pEnum);
				}
				else
				{
					int t = getEnumStringIndex(pCtrl->cEnumeratedList, p);
					if(t < 0)
					{
						// this should never happen...
						char* pEnum;
						pEnum = getEnumString(pCtrl->cEnumeratedList, 0);
						if(pEnum)
							((CTextEdit*)pControl)->setText(pEnum);
						fValue = 0.0;
					}
					else
					{
						fValue = calcSliderVariable(pCtrl->fUserDisplayDataLoLimit, pCtrl->fUserDisplayDataHiLimit, (float)t);
						pControl->setValue(fValue);
						((CTextEdit*)pControl)->setText(str.c_str());
					}
				}

				break;
			}
			default:
				break;
		}

        // --- RAFX v6.8
        if(m_bAppendUnits)
        {
            string editString(((CTextEdit*)pControl)->getText());
            editString.append(" ");
            editString.append(units);
            ((CTextEdit*)pControl)->setText(editString.c_str());
        }

		return fValue;
	}
    inline bool isKnobControl(CControl* pControl)
	{
		CAnimKnob* control = dynamic_cast<CAnimKnob*>(pControl);
		if(control)
			return true;

		return false;
	}
    inline bool isKnobControl(CView* pControl)
    {
        CAnimKnob* control = dynamic_cast<CAnimKnob*>(pControl);
        if(control)
            return true;
        return false;
    }

    inline bool isKnobWPControl(CView* pControl)
    {
        CKnobWP* control = dynamic_cast<CKnobWP*>(pControl);
        if(control)
            return true;
        return false;
    }
    inline bool isSwitchKnobControl(CControl* pControl)
    {
        CKnobWP* control = dynamic_cast<CKnobWP*>(pControl);
        if(control)
        {
            if(control->isSwitchKnob())
                return true;
        }

        return false;
    }
	inline bool isSliderControl(CControl* pControl)
	{
		CSlider* control = dynamic_cast<CSlider*>(pControl);
		if(control)
			return true;

		return false;
	}
	inline bool isSliderControl(CView* pControl)
	{
		CSlider* control = dynamic_cast<CSlider*>(pControl);
		if(control)
			return true;
		return false;
	}
   	inline bool isSwitchSliderControl(CControl* pControl)
    {
        CVerticalSliderWP* control = dynamic_cast<CVerticalSliderWP*>(pControl);
        if(control)
        {
            if(control->isSwitchSlider())
                return true;
        }

        CHorizontalSliderWP* hcontrol = dynamic_cast<CHorizontalSliderWP*>(pControl);
        if(hcontrol)
        {
            if(hcontrol->isSwitchSlider())
                return true;
        }

        return false;
    }

    inline bool isSliderWPControl(CControl* pControl)
    {
        CVerticalSliderWP* control = dynamic_cast<CVerticalSliderWP*>(pControl);
        if(control)
            return true;

        CHorizontalSliderWP* hcontrol = dynamic_cast<CHorizontalSliderWP*>(pControl);
        if(hcontrol)
            return true;

        return false;
    }

    inline bool isVerticalSliderWPControl(CControl* pControl)
    {
        CVerticalSliderWP* control = dynamic_cast<CVerticalSliderWP*>(pControl);
        if(control)
            return true;

        return false;
    }

    inline bool isHorizontalSliderWPControl(CControl* pControl)
    {
        CHorizontalSliderWP* control = dynamic_cast<CHorizontalSliderWP*>(pControl);
        if(control)
            return true;

        return false;
    }

	inline bool isTextEditControl(CControl* pControl)
	{
		CTextEdit* control = dynamic_cast<CTextEdit*>(pControl);
		if(control)
			return true;

		return false;
	}
	inline bool isTextEditControl(CView* pControl)
	{
		CTextEdit* control = dynamic_cast<CTextEdit*>(pControl);
		if(control)
			return true;
		return false;
	}
	inline bool isOptionMenuControl(CControl* pControl)
	{
		COptionMenu* control = dynamic_cast<COptionMenu*>(pControl);
		if(control)
			return true;

		return false;
	}
	inline bool isOptionMenuControl(CView* pControl)
	{
		COptionMenu* control = dynamic_cast<COptionMenu*>(pControl);
		if(control)
			return true;
		return false;
	}
	inline bool isRadioButtonControl(CControl* pControl)
	{
		CVerticalSwitch* control = dynamic_cast<CVerticalSwitch*>(pControl);
		if(control)
			return true;

		return false;
	}
	inline bool isTextLabelControl(CControl* pControl)
	{
		CTextLabel* control = dynamic_cast<CTextLabel*>(pControl);
		if(control)
			return true;
		return false;
	}
	inline bool isTextLabelControl(CView* pControl)
	{
		CTextLabel* control = dynamic_cast<CTextLabel*>(pControl);
		if(control)
			return true;
		return false;
	}
	inline bool isKickButtonControl(CControl* pControl)
	{
		CKickButton* control = dynamic_cast<CKickButton*>(pControl);
		if(control)
			return true;
		return false;
	}
	inline bool isKickButtonControl(CView* pControl)
	{
		CKickButton* control = dynamic_cast<CKickButton*>(pControl);
		if(control)
			return true;
		return false;
	}
	inline bool isOnOffButtonControl(CControl* pControl)
	{
		COnOffButton* control = dynamic_cast<COnOffButton*>(pControl);
		if(control)
			return true;
		return false;
	}
	inline bool isOnOffButtonControl(CView* pControl)
	{
		COnOffButton* control = dynamic_cast<COnOffButton*>(pControl);
		if(control)
			return true;
		return false;
	}
	inline bool isVertSwitchControl(CControl* pControl)
	{
		CVerticalSwitch* control = dynamic_cast<CVerticalSwitch*>(pControl);
		if(control)
			return true;
		return false;
	}
	inline bool isVertSwitchControl(CView* pControl)
	{
		CVerticalSwitch* control = dynamic_cast<CVerticalSwitch*>(pControl);
		if(control)
			return true;
		return false;
	}
	inline bool isVuMeterControl(CControl* pControl)
	{
		CVuMeter* control = dynamic_cast<CVuMeter*>(pControl);
		if(control)
			return true;
		return false;
	}
	inline bool isVuMeterControl(CView* pControl)
	{
		CVuMeter* control = dynamic_cast<CVuMeter*>(pControl);
		if(control)
			return true;
		return false;
	}
	inline bool isXYPadControl(CControl* pControl)
	{
		CXYPad* control = dynamic_cast<CXYPad*>(pControl);
		if(control)
			return true;
		return false;
	}
	inline bool isXYPadControl(CView* pControl)
	{
		CXYPad* control = dynamic_cast<CXYPad*>(pControl);
		if(control)
			return true;
		return false;
	}
	inline bool isHorizSwitchControl(CControl* pControl)
	{
		CHorizontalSwitch* control = dynamic_cast<CHorizontalSwitch*>(pControl);
		if(control)
			return true;
		return false;
	}
	inline bool isHorizSwitchControl(CView* pControl)
	{
		CHorizontalSwitch* control = dynamic_cast<CHorizontalSwitch*>(pControl);
		if(control)
			return true;
		return false;
    }
    inline bool isSegmentButtonControl(CView* pControl)
    {
        CSegmentButton* control = dynamic_cast<CSegmentButton*>(pControl);
        if(control)
            return true;
        return false;
    }
	inline bool isViewContainer(CView* pControl)
	{
		CViewContainer* control = dynamic_cast<CViewContainer*>(pControl);
		if(control)
			return true;
		return false;
	}
	inline bool isCVuMeterWP(CControl* pControl)
	{
		CVuMeterWP* control = dynamic_cast<CVuMeterWP*>(pControl);
		if(control)
			return true;

		return false;
	}
	inline bool isCXYPadWP(CControl* pControl)
	{
		CXYPadWP* control = dynamic_cast<CXYPadWP*>(pControl);
		if(control)
			return true;

		return false;
	}
	inline bool isCXYPadWP(CXYPad* pControl)
	{
		CXYPadWP* control = dynamic_cast<CXYPadWP*>(pControl);
		if(control)
			return true;

		return false;
	}
    inline bool isRAFXSwitchView(CViewContainer* pCont)
    {
        CRAFXSwitchView* control = dynamic_cast<CRAFXSwitchView*>(pCont);
        if(control)
            return true;

        return false;
    }
   	inline float getPluginParameterValue(float fControlValue, CControl* pControl = NULL)
    {
        if(pControl)
        {
            if(isOptionMenuControl(pControl))
                return pControl->getValue()/((float)((COptionMenu*)pControl)->getNbEntries() - 1);
            else if(isVertSwitchControl(pControl) || isHorizSwitchControl(pControl) || isSegmentButtonControl(pControl))
                return pControl->getValue()/pControl->getMax();
            else if(isXYPadControl(pControl))
                return fControlValue; // ignore
            else
                return pControl->getValue();
        }

        return fControlValue;
    }
    // --- actual plugin version of variable
   	inline float getActualPluginParameterValue(CUICtrl* pCtrl, CControl* pControl, float fNormalizedControlValue)
    {
        if(!pControl) return 0.0;

        else if(isOptionMenuControl(pControl) || isVertSwitchControl(pControl) || isHorizSwitchControl(pControl) || isSegmentButtonControl(pControl))
            return pControl->getValue();
        else
        {
            float fValue = calcDisplayVariable(pCtrl->fUserDisplayDataLoLimit, pCtrl->fUserDisplayDataHiLimit, fNormalizedControlValue);
            if(pCtrl->uUserDataType == UINTData)
            {
                fValue = floor(fValue + 0.5);
            }
            return fValue;
        }

        return 0.0;
    }

	// trim from start
/*	static inline std::string &ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(isspace))));
        return s;
	}

	static inline std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int c) {return !std::isspace(c);});
    return s;
	}

	// trim from end
	static inline std::string &rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(isspace))).base(), s.end());
        return s;
	}

	// trim from both ends
	static inline std::string &trim(std::string &s) {
        return ltrim(rtrim(s));
	}*/
		// trim from start (in place)
	static inline void ltrim(std::string &s) {
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
			return !std::isspace(ch);
		}));
	}

	// trim from end (in place)
	static inline void rtrim(std::string &s) {
		s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
			return !std::isspace(ch);
		}).base(), s.end());
	}

	// trim from both ends (in place)
	static inline void trim(std::string &s) {
		ltrim(s);
		rtrim(s);
	}

	inline void inflateRect(CRect& rect, int x, int y)
	{
		rect.left -= x;
		rect.right += x;
		rect.top -= y;
		rect.bottom += y;
	}
	inline void updateViewSize(CView* pView, const char* pSizeString)
	{
		if(strlen(pSizeString) <= 0)
			return;

		pugi::xml_node node;

		CRect viewSize = pView->getViewSize();
		const CPoint origin2(viewSize.left, viewSize.top);
		const CPoint size2 = getCPointFromString(pSizeString);

		CRect newRect2(origin2, size2);
		pView->setViewSize(newRect2);
		pView->setMouseableArea(newRect2);
		pView->invalid();
	}

  	void syncTabViews(CControl* pTabControl, int nControlTag, int nControlValue);

	// --- simple GUI API stuff
	void initPluginCustomGUI();

	VSTGUI_VIEW_INFO guiInfoStruct;

	inline bool fillUIDescriptionMap(std::map<std::string,std::string>* map, pugi::xml_node node)
	{
		map->clear();
		int nCount = m_XMLParser.getAttributeCount(node);
		for(int i=0; i<nCount; i++)
		{
			const char_t* attributeName;
			const char_t* attributeValue;

			attributeName = m_XMLParser.getAttributeName(node, i);
			attributeValue = m_XMLParser.getAttributeValue(node, i);

			if(attributeName && attributeValue)
				map->insert(std::make_pair(attributeName, attributeValue));

		}
		return true;
	}

    inline CView* checkCreateCustomView(pugi::xml_node node, void* p = NULL)
	{
		CView* pView = NULL;
		const char_t* customView = m_XMLParser.getAttribute(node, "custom-view-name");
		const char_t* ctagName = m_XMLParser.getAttribute(node, "control-tag");
		const char_t* origin =  m_XMLParser.getAttribute(node, "origin");
		const char_t* size =  m_XMLParser.getAttribute(node, "size");
		const char_t* offset =  m_XMLParser.getAttribute(node, "background-offset");
		const char_t* bitmap =  m_XMLParser.getAttribute(node, "bitmap");
		const char_t* bitmap2 =  m_XMLParser.getAttribute(node, "handle-bitmap");
		const char_t* bitmap3 =  m_XMLParser.getAttribute(node, "off-bitmap");
		string sOffset(offset);

		CRect rect = getRectFromOriginSizeStrings(origin, size);
		CPoint offsetPoint = getCPointFromString(sOffset);

		const char_t* ctag = m_XMLParser.getControlTagAttribute(ctagName, "tag");
		int32_t tag = -1;
		if(strlen(ctag) > 0)
			tag = atoi(ctag);

		VSTGUI_VIEW_INFO* pInfoStruct = new VSTGUI_VIEW_INFO;

		if(m_pPlugIn && strlen(customView) > 0)
		{
			std::map<std::string,std::string> attributes;
			if(fillUIDescriptionMap(&attributes, node))
			{
				pInfoStruct->message = GUI_CUSTOMVIEW;
				pInfoStruct->hPlugInInstance = NULL;
				pInfoStruct->hRAFXInstance = NULL;
				pInfoStruct->size.width = 0; pInfoStruct->size.height = 0;
				pInfoStruct->window = NULL;

				// --- custom view only!
				pInfoStruct->customViewName = (char*)customView;
				pInfoStruct->customViewTag = tag;
				pInfoStruct->customViewRect.top = rect.top;
				pInfoStruct->customViewRect.bottom = rect.bottom;
				pInfoStruct->customViewRect.left = rect.left;
				pInfoStruct->customViewRect.right = rect.right;
				pInfoStruct->customViewOffset.x = offsetPoint.x;
				pInfoStruct->customViewOffset.y = offsetPoint.y;

				if(strlen(bitmap) > 0)
				{
					pInfoStruct->customViewBitmapName = addStrings((char*)bitmap, ".png");
				}
				else
					pInfoStruct->customViewBitmapName = "";

                pInfoStruct->customViewHandleBitmapName = "";
				pInfoStruct->customViewOffBitmapName = "";

				if(strlen(bitmap2) > 0)
				{
					pInfoStruct->customViewHandleBitmapName = addStrings((char*)bitmap2, ".png"); // bitmap2 is for slider handle or LEDMeter-off only
				}

				if(strlen(bitmap3) > 0)
				{
					pInfoStruct->customViewOffBitmapName = addStrings((char*)bitmap3, ".png");
				}

				const char_t* orientation =  m_XMLParser.getAttribute(node, "orientation");
				pInfoStruct->customViewOrientation = (char*)orientation;

				pInfoStruct->customViewBackColor = NULL;
				pInfoStruct->customViewFrameColor = NULL;
				pInfoStruct->customViewFontColor = NULL;

				const char_t* backcolor =  m_XMLParser.getAttribute(node, "back-color");
				if(strlen(backcolor) > 0)
				{
					CColor backCColor = getCColor(backcolor);
					pInfoStruct->customViewBackColor = (void*)&backCColor;
				}

				const char_t* framecolor =  m_XMLParser.getAttribute(node, "frame-color");
				if(strlen(framecolor) > 0)
				{
					CColor frameCColor = getCColor(framecolor);
					pInfoStruct->customViewFrameColor = (void*)&frameCColor;
				}

				const char_t* fontcolor =  m_XMLParser.getAttribute(node, "font-color");
				if(strlen(fontcolor) > 0)
				{
					CColor vontCColor = getCColor(fontcolor);
					pInfoStruct->customViewFontColor = (void*)&vontCColor;
				}

				pInfoStruct->customViewFrameWidth = 0;
				const char_t* framewidth =  m_XMLParser.getAttribute(node, "frame-width");
				if(strlen(framewidth) > 0)
					pInfoStruct->customViewFrameWidth = atoi(framewidth);

				pInfoStruct->customViewRoundRectRadius = 0;
				const char_t* rrradius =  m_XMLParser.getAttribute(node, "round-rect-radius");
				if(strlen(rrradius) > 0)
					pInfoStruct->customViewRoundRectRadius = atoi(rrradius);

				pInfoStruct->customViewStyleRoundRect = false;
				const char_t* styleRoundRect =  m_XMLParser.getAttribute(node, "style-round-rect");
				if(strlen(styleRoundRect) > 0)
					if(strcmp(styleRoundRect, "true") == 0) pInfoStruct->customViewStyleRoundRect = true;

				pInfoStruct->customViewStyleNoFrame = false;
				const char_t* styleNoFrame =  m_XMLParser.getAttribute(node, "style-no-frame");
				if(strlen(styleNoFrame) > 0)
					if(strcmp(styleNoFrame, "true") == 0) pInfoStruct->customViewStyleNoFrame = true;

				pInfoStruct->customViewHtOneImage = 0;
				const char_t* htOneImage =  m_XMLParser.getAttribute(node, "height-of-one-image");
				if(strlen(htOneImage) > 0)
					pInfoStruct->customViewHtOneImage = atoi(htOneImage);

				pInfoStruct->customViewSubPixmaps = 0;
				const char_t* subPixmaps =  m_XMLParser.getAttribute(node, "sub-pixmaps");
				if(strlen(subPixmaps) > 0)
					pInfoStruct->customViewSubPixmaps = atoi(subPixmaps);

				pInfoStruct->subControllerName = "";
				pInfoStruct->editor = (void*)this;

				if(p)
					pInfoStruct->listener = p;
				else
					pInfoStruct->listener = (void*)this;

				pView = (CView*)(m_pPlugIn->showGUI((void*)(pInfoStruct)));

				attributes.clear();
			}
		}
		return pView;
	}

    inline CColor getCColorFromRGBA(const char_t* rgba)
    {
        string sRGBA(rgba);
        int r, g, b, a = 255;
        getRGBAFromString(sRGBA, r, g, b, a);
        return CColor(r, g, b, a);
    }

	inline const CRect getRectFromOriginSizeStrings(const char_t* origin, const char_t* size)
	{
		// --- make rect
		string sOrigin = string(origin);

		int x, y = 0;
		if(!getXYFromString(sOrigin, x, y))
		{
			const CRect rect(0,0,0,0);
			return rect;
		}

		string sSize = string(size);

		int w, h = 0;
		if(!getXYFromString(sSize, w, h))
		{
			const CRect rect(0,0,0,0);
			return rect;
		}
		const CRect rect(x, y, x+w, y+h);
		return rect;
	}

protected:
	CPlugIn* m_pPlugIn;
#ifdef AUPLUGIN
    AudioUnit m_AU;
    AUEventListenerRef AUEventListener;
#endif

#ifdef AAXPLUGIN
AAX_IViewContainer* aaxViewContainer;
#endif

	UINT* m_pControlMap;
   	int m_nControlCount;

	// --- arrays - remember to add to CalculateFrameSize() when you addd morearrays
	CTextLabel** m_ppTextLabels;
	int m_nTextLabelCount;

    CTextLabel** m_ppTaggedTextLabels;
    int m_nTaggedTextLabelCount;
    void gatherTaggedTextLabels();

	CTextEdit** m_ppTextEditControls;
	int m_nTextEditCount;

	CAnimKnob** m_ppKnobControls;
	int m_nAnimKnobCount;

	CSlider** m_ppSliderControls;
	int m_nSliderCount;

	CKickButton** m_ppKickButtonControls;
	int m_nKickButtonCount;

	COnOffButton** m_ppOnOffButtonControls;
	int m_nOnOffButtonCount;

    COptionMenu** m_ppOptionMenuControls;
    int m_nOptionMenuCount;

	CVerticalSwitch** m_ppVerticalSwitchControls;
	int m_nVerticalSwitchCount;

    CHorizontalSwitch** m_ppHorizontalSwitchControls;
	int m_nHorizontalSwitchCount;

    CSegmentButton** m_ppSegmentButtons;
    int m_nSegmentButtonCount;

	CVuMeter** m_ppVuMeterControls;
	int m_nVuMeterCount;

	CXYPad** m_ppXYPads;
	int m_nXYPadCount;
	float m_fJS_X;
	float m_fJS_Y;

	CViewContainer** m_ppViewContainers;
	int m_nViewContainerCount;

	CView** m_ppViews;
	int m_nViewCount;
};

}

#endif // __CRafxVSTEditor__

