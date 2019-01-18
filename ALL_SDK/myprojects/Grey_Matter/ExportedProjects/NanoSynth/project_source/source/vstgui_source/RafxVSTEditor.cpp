#include "RafxVSTEditor.h"
#include "KnobWP.h"
#include "SliderWP.h"
#include "KickButtonWP.h"
#if MAC
#include <dlfcn.h>
#include <AssertMacros.h>

// --- for au event granularity/interval; these are recommended defaults
const Float32 AUEVENT_GRANULARITY = 0.05;
const Float32 AUEVENT_INTERVAL = 0.05;
#endif

namespace VSTGUI{

/* ------------------------------------------------------
     CRafxVSTEditor
     Custom VSTGUI Object by Will Pirkle
     Created with RackAFX(TM) Plugin Development Software
     www.willpirkle.com
 -------------------------------------------------------*/

#if MAC
void* gBundleRef = 0;
static int openCount = 0;

static void CreateVSTGUIBundleRef();
static void ReleaseVSTGUIBundleRef();

#ifdef AUPLUGIN
    enum {kBeginEdit, kEndEdit, kValueChanged, kNumEventTypes};

// --- this is called when presets change for us to synch GUI
void EventListenerDispatcher(void *inRefCon, void *inObject, const AudioUnitEvent *inEvent, UInt64 inHostTime, Float32 inValue)
{
    CRafxVSTEditor *pEditor = (CRafxVSTEditor*)inRefCon;
    if(!pEditor) return;

    // --- kAudioUnitEvent_ParameterValueChange
    if(inEvent->mEventType == kAudioUnitEvent_ParameterValueChange)
    {
        // --- update the GUI control
        if(pEditor)
            pEditor->dispatchControlChange(inEvent->mArgument.mParameter.mParameterID, inValue, true);
    }
}
#endif

// --- BundleRef Functions for VSTGUI4 only ------------------------------------------------------------------------
void CreateVSTGUIBundleRef()
{
    openCount++;
    if(gBundleRef)
    {
        CFRetain(gBundleRef);
        return;
    }
#if TARGET_OS_IPHONE
    gBundleRef = CFBundleGetMainBundle();
    CFRetain(gBundleRef);
#else
    Dl_info info;
    if(dladdr((const void*)CreateVSTGUIBundleRef, &info))
    {
        if(info.dli_fname)
        {
            string name;
            name.assign (info.dli_fname);
            for (int i = 0; i < 3; i++)
            {
                int delPos = name.find_last_of('/');
                if (delPos == -1)
                {
                    fprintf (stdout, "Could not determine bundle location.\n");
                    return; // unexpected
                }
                name.erase (delPos, name.length () - delPos);
            }
            CFURLRef bundleUrl = CFURLCreateFromFileSystemRepresentation (0, (const UInt8*)name.c_str(), name.length (), true);
            if (bundleUrl)
            {
                gBundleRef = CFBundleCreate (0, bundleUrl);
                CFRelease (bundleUrl);
            }
        }
    }
#endif
}

void ReleaseVSTGUIBundleRef()
{
    openCount--;
    if(gBundleRef)
        CFRelease (gBundleRef);
    if(openCount == 0)
        gBundleRef = 0;
}
#endif
// --- END BundleRef Functions for VSTGUI4 only --------------------------------------------------------------------


CRafxVSTEditor::CRafxVSTEditor() : CControlListener(), VSTGUI::VSTGUIEditorInterface(), VSTGUI::CBaseObject()
{
    m_pPlugIn = NULL; // OK if null
#ifdef AUPLUGIN
    m_AU = NULL;
#endif
#ifdef AAXPLUGIN
    m_pAAXParameters = NULL;
	aaxViewContainer = NULL;
#endif
    m_pControlMap = NULL;
    m_nControlCount = 0;
    m_pGUIControls = NULL;

    m_bInitialized = false; // currently unused; reserved for future one-time-init flag

    m_ppTextEditControls = NULL;
    m_nTextEditCount = 0;

    m_ppKnobControls = NULL;
    m_nAnimKnobCount = 0;

    m_ppSliderControls = NULL;
    m_nSliderCount = 0;

    m_ppKickButtonControls = NULL;
    m_nKickButtonCount = 0;

    m_ppOnOffButtonControls = NULL;
    m_nOnOffButtonCount = 0;

    m_ppVerticalSwitchControls = NULL;
    m_nVerticalSwitchCount = 0;

    m_ppHorizontalSwitchControls = NULL;
	m_nHorizontalSwitchCount = 0;

    m_ppSegmentButtons = NULL;
     m_nSegmentButtonCount = 0;

    m_ppVuMeterControls = NULL;
    m_nVuMeterCount = 0;

    m_ppXYPads = NULL;
    m_nXYPadCount = 0;

    m_ppViewContainers = NULL;
    m_nViewContainerCount = 0;

    m_ppOptionMenuControls = NULL;
    m_nOptionMenuCount = 0;

    m_ppTaggedTextLabels = NULL;
    m_nTaggedTextLabelCount = 0;

    m_ppViews = NULL;
	m_nViewCount = 0;

    m_ppTextLabels = NULL;
    m_nTextLabelCount = 0;

    m_fJS_X = 0;
    m_fJS_Y = 0;

    bClosing = false;

    // --- RAFX v6.6
    m_uKnobAction = kHostChoice; // RAFX Default is linear

    // --- RAFX v6.8
    m_bAppendUnits = true;

#if MAC
    // --- for VSTGUI
    CreateVSTGUIBundleRef();
#endif

    // create a timer used for idle update: will call notify method
    timer = new CVSTGUITimer (dynamic_cast<CBaseObject*>(this));

}
CRafxVSTEditor::~CRafxVSTEditor()
{
    if(timer)
		timer->forget();
#if MAC
    ReleaseVSTGUIBundleRef();
#endif

    m_bInitialized = false;
}


/*
enum CKnobMode
{
	kCircularMode = 0,
	kRelativCircularMode,
	kLinearMode
};*/
int32_t CRafxVSTEditor::getKnobMode() const
{
	// --- RAFX v6.6
	if(m_uKnobAction == kHostChoice)
		return kLinearMode; // RAFX DEFAULT MODE!
	else if(m_uKnobAction == kLinearMode)
		return kLinearMode;
	else if(m_uKnobAction == kRelativCircularMode)
		return kRelativCircularMode;
	else if(m_uKnobAction == kCircularMode)
		return kCircularMode;

	return kLinearMode;
}

void CRafxVSTEditor::getSize(float& width, float& height)
{
    CRect rect = frame->getViewSize();
    width = rect.getWidth();
    height = rect.getHeight();
}

CMessageResult CRafxVSTEditor::notify(CBaseObject* /*sender*/, const char* message)
{
    if(message == CVSTGUITimer::kMsgTimer && m_pPlugIn)
    {
        if(frame)
            idle();

        // --- timer ping
        VSTGUI_VIEW_INFO info;
        info.message = GUI_TIMER_PING;
        m_pPlugIn->showGUI((void*)&info);

        return kMessageNotified;
    }

    return kMessageUnknown;
}


VSTGUI::CNinePartTiledBitmap* CRafxVSTEditor::loadTiledBitmap(const CResourceDescription& desc, CCoord left, CCoord top, CCoord right, CCoord bottom)
{
    VSTGUI::CNinePartTiledBitmap* pBM = NULL;
#ifdef VSTGUI_43
    pBM = new VSTGUI::CNinePartTiledBitmap(desc, (CNinePartTiledDescription(left, top, right, bottom)));
#else
    pBM = new VSTGUI::CNinePartTiledBitmap(desc, (CNinePartTiledBitmap::PartOffsets(left, top, right, bottom)));
#endif
    return pBM;
}

VSTGUI::CBitmap* CRafxVSTEditor::loadBitmap(const CResourceDescription& desc)
{
    VSTGUI::CBitmap* pBM = new VSTGUI::CBitmap(desc);
    return pBM;
}

//-----------------------------------------------------------------------------------
#ifdef AUPLUGIN
    bool CRafxVSTEditor::open(void* pHwnd, CPlugIn* pPlugIn, char* pXMLFile, AudioUnit inAU)
#else
    bool CRafxVSTEditor::open(void* pHwnd, CPlugIn* pPlugIn, void* pXMLResource, DWORD xmlSize, char* pXMLFile)
#endif
{
    if(!pHwnd) return false;

    // --- save the plugin
    setPlugIn(pPlugIn);

    // --- save the AU
#ifdef AUPLUGIN
    m_AU = inAU;
#endif

#if MAC
    if(pXMLFile)
    {
        // --- open the file from path
        if(!m_XMLParser.loadXMLFile(pXMLFile))
            return false;
    }
#else
    if(pXMLResource)
    {
        if(!loadXMLResource(pXMLResource, xmlSize))
        {
            return false;
        }
    }
#endif

    // --- the knob action attribute is custom
    xml_node anode;
    const char* action = m_XMLParser.getCustomAttribute("KnobAction", anode);
    if(action)
        m_uKnobAction = atoi(action);

    // --- create the frame; find the editor template
    int nEditorViewCount = 0;
    pugi::xml_node node = m_XMLParser.getTemplateInfo("Editor", nEditorViewCount);
    if(!node)
        return false;

    // --- have editor and count so start building
    //
    int w, h = 0;
    // --- get editor size
    const char_t* size = m_XMLParser.getTemplateAttribute("Editor", "size");
    string sSize(size);

    if(!getXYFromString(sSize, w, h))
        return false;

    //-- first we create the frame with a size of x, y and set the background to white
    CRect frameSize(0, 0, w, h);
    m_pRafxFrame = new CFrame(frameSize, this);
    
    m_pRafxFrame->open(pHwnd, kNSView);

    m_pRafxFrame->registerKeyboardHook(&m_KeyHooker);
    m_pRafxFrame->registerMouseObserver((IMouseObserver*)this);

    // --- not sure if needed
    m_pRafxFrame->kDirtyCallAlwaysOnMainThread = false;

    const char_t* backColor = m_XMLParser.getTemplateAttribute("Editor", "background-color");
    string sBackColorName(backColor);
    CColor backCColor;

    if(isBuiltInColor(sBackColorName, backCColor))
        m_pRafxFrame->setBackgroundColor(backCColor);
    else
    {
        // do we have it?
        if(m_XMLParser.hasColor(backColor))
        {
            const char_t* rgba = m_XMLParser.getColorAttribute(backColor, "rgba");
            string sRGBA(rgba);

            int r, g, b, a = 255;
            getRGBAFromString(sRGBA, r, g, b, a);
            m_pRafxFrame->setBackgroundColor(CColor(r, g, b, a));
        }
    }

    // --- background bitmap (if there is one)
    const char_t* bitmap = m_XMLParser.getTemplateAttribute("Editor", "bitmap");
    if(strlen(bitmap) > 0)
    {
        setCurrentBackBitmap(bitmap, m_pRafxFrame);
    }

    // --- create arrays
    m_ppTextEditControls = new CTextEdit*[m_nTextEditCount];
    m_ppKnobControls = new CAnimKnob*[m_nAnimKnobCount];
    m_ppSliderControls = new CSlider*[m_nSliderCount];
    m_ppKickButtonControls = new CKickButton*[m_nKickButtonCount];
    m_ppOnOffButtonControls = new COnOffButton*[m_nOnOffButtonCount];
    m_ppVerticalSwitchControls = new CVerticalSwitch*[m_nVerticalSwitchCount];
   	m_ppHorizontalSwitchControls = new CHorizontalSwitch*[m_nHorizontalSwitchCount];
    m_ppVuMeterControls = new CVuMeter*[m_nVuMeterCount];
    m_ppXYPads = new CXYPad*[m_nXYPadCount];
    m_ppOptionMenuControls = new COptionMenu*[m_nOptionMenuCount];
    m_ppViewContainers = new CViewContainer*[m_nViewContainerCount];
    m_ppTextLabels = new CTextLabel*[m_nTextLabelCount];
	m_ppViews = new CView*[m_nViewCount];
    m_ppSegmentButtons = new CSegmentButton*[m_nSegmentButtonCount];
    m_ppTaggedTextLabels = new CTextLabel*[m_nTaggedTextLabelCount];

    // --- clear arrays
    memset(m_ppTextEditControls, 0, sizeof(CTextEdit*)*m_nTextEditCount);
    memset(m_ppKnobControls, 0, sizeof(CAnimKnob*)*m_nAnimKnobCount);
    memset(m_ppSliderControls, 0, sizeof(CSlider*)*m_nSliderCount);
    memset(m_ppKickButtonControls, 0, sizeof(CKickButton*)*m_nKickButtonCount);
    memset(m_ppOnOffButtonControls, 0, sizeof(COnOffButton*)*m_nOnOffButtonCount);
    memset(m_ppVerticalSwitchControls, 0, sizeof(CVerticalSwitch*)*m_nVerticalSwitchCount);
    memset(m_ppHorizontalSwitchControls, 0, sizeof(CHorizontalSwitch*)*m_nHorizontalSwitchCount);
    memset(m_ppVuMeterControls, 0, sizeof(CVuMeterWP*)*m_nVuMeterCount);
    memset(m_ppXYPads, 0, sizeof(CXYPad*)*m_nXYPadCount);
    memset(m_ppOptionMenuControls, 0, sizeof(COptionMenu*)*m_nOptionMenuCount);
    memset(m_ppViewContainers, 0, sizeof(CViewContainer*)*m_nViewContainerCount);
    memset(m_ppTextLabels, 0, sizeof(CTextLabel*)*m_nTextLabelCount);
	memset(m_ppViews, 0, sizeof(CView*)*m_nViewCount);
    memset(m_ppSegmentButtons, 0, sizeof(CSegmentButton*)*m_nSegmentButtonCount);

    // --- create subviews
    for(int i=0; i<nEditorViewCount; i++)
    {
        VIEW_DESC* pViewDesc = m_XMLParser.getTemplateSubViewDesc("Editor", i);
        if(pViewDesc)
        {
            createSubView(m_pRafxFrame, pViewDesc->viewNode, true);
            delete pViewDesc;
        }
    }

    // --- for tagging labels
    gatherTaggedTextLabels();

    //-- set the member frame to our frame
    frame = m_pRafxFrame;
    frame->enableTooltips(true);
    if(timer)
	{
        timer->setFireTime(METER_UPDATE_INTERVAL_MSEC);
        timer->start();
    }

    m_bInitialized = true;

    return true;
}


//-----------------------------------------------------------------------------------
void CRafxVSTEditor::close()
{
    if(!frame) return;

    bClosing = true;

    frame->unregisterKeyboardHook(&m_KeyHooker);
    frame->unregisterMouseObserver((IMouseObserver*)this);

    // --- delete our copy
    if(m_pControlMap)
        delete [] m_pControlMap;
    m_pControlMap = NULL;

    // --- delete our uicontrol copied objects
    int nCount = m_GUI_UIControlList.count();
    for(int j=nCount-1; j>=0; j--)
    {
        CUICtrl* p = m_GUI_UIControlList.getAt(j);
        m_GUI_UIControlList.del(*p);
    }

    // --- clear map
    for(int i=0; i<MAX_USER_METERS; i++)
        m_nMeterMap[i] = -1; // -1 = no meter mapped

    // --- delete pointers BUT NOT OBJECTS; they will be deleted in frame->forget()
    // --- arrays - remember to add to CalculateFrameSize() when you addd morearrays
    if(m_ppTextLabels)
        delete [] m_ppTextLabels;

    if(m_ppTaggedTextLabels)
        delete [] m_ppTaggedTextLabels;

    if(m_ppTextEditControls)
        delete [] m_ppTextEditControls;

    if(m_ppKnobControls)
        delete [] m_ppKnobControls;

    if(m_ppSliderControls)
        delete [] m_ppSliderControls;

    if(m_ppKickButtonControls)
        delete [] m_ppKickButtonControls;

    if(m_ppOnOffButtonControls)
        delete [] m_ppOnOffButtonControls;

    if(m_ppVerticalSwitchControls)
        delete [] m_ppVerticalSwitchControls;

    if(m_ppHorizontalSwitchControls)
		delete [] m_ppHorizontalSwitchControls;

    if(m_ppVuMeterControls)
        delete [] m_ppVuMeterControls;

    if(m_ppXYPads)
        delete [] m_ppXYPads;

    if(m_ppOptionMenuControls)
        delete [] m_ppOptionMenuControls;

    if(m_ppViewContainers)
        delete [] m_ppViewContainers;

    if(m_ppSegmentButtons)
        delete [] m_ppSegmentButtons;

    if(m_ppViews)
        delete [] m_ppViews;

    m_ppViews = NULL;
    m_ppTextLabels = NULL;
    m_ppTaggedTextLabels = NULL;
    m_ppTextEditControls = NULL;
    m_ppKnobControls = NULL;
    m_ppSliderControls = NULL;
    m_ppKickButtonControls = NULL;
    m_ppOnOffButtonControls = NULL;
    m_ppVerticalSwitchControls = NULL;
    m_ppHorizontalSwitchControls = NULL;
    m_ppVuMeterControls = NULL;
    m_ppXYPads = NULL;
    m_ppOptionMenuControls = NULL;
    m_ppViewContainers = NULL;
    m_ppSegmentButtons = NULL;

   	m_nViewCount = 0;
    m_nTextLabelCount = 0;
    m_nTaggedTextLabelCount = 0;
    m_nTextEditCount = 0;
	m_nAnimKnobCount = 0;
	m_nSliderCount = 0;
	m_nKickButtonCount = 0;
	m_nOnOffButtonCount = 0;
	m_nVerticalSwitchCount = 0;
	m_nHorizontalSwitchCount = 0;
	m_nVuMeterCount = 0;
	m_nXYPadCount = 0;
	m_nOptionMenuCount = 0;
	m_nViewContainerCount = 0;
    m_nSegmentButtonCount = 0;

#ifdef AUPLUGIN
   	if (AUEventListener) __Verify_noErr(AUListenerDispose(AUEventListener));
    AUEventListener = NULL;
    m_AU = NULL;
#endif


    if(m_pGUIControls)
    {
        for(int j=0; j<m_nControlCount; j++)
        {
            if(m_pGUIControls[j].pLabel)
                delete [] m_pGUIControls[j].pLabel;
            if(m_pGUIControls[j].pEdit)
                delete [] m_pGUIControls[j].pEdit;
            if(m_pGUIControls[j].pKnob)
                delete [] m_pGUIControls[j].pKnob;
            if(m_pGUIControls[j].pSlider)
                delete [] m_pGUIControls[j].pSlider;
            if(m_pGUIControls[j].pVSwitch)
                delete [] m_pGUIControls[j].pVSwitch;
            if(m_pGUIControls[j].pHSwitch)
                delete [] m_pGUIControls[j].pHSwitch;
            if(m_pGUIControls[j].pOOB)
                delete [] m_pGUIControls[j].pOOB;
            if(m_pGUIControls[j].pMeter)
                delete [] m_pGUIControls[j].pMeter;
            if(m_pGUIControls[j].pKickButton)
                delete [] m_pGUIControls[j].pKickButton;
            if(m_pGUIControls[j].pOptionMenu)
                delete [] m_pGUIControls[j].pOptionMenu;
            if(m_pGUIControls[j].pXYPad)
                delete [] m_pGUIControls[j].pXYPad;
            if(m_pGUIControls[j].pSegButton)
                delete [] m_pGUIControls[j].pSegButton;
            if(m_pGUIControls[j].pControl)
                delete [] m_pGUIControls[j].pControl;
        }
        delete [] m_pGUIControls;
    }

    m_pGUIControls = NULL;

 	if(timer)
		timer->stop();

	// --- v6.6 Custom
	if(m_pPlugIn)
	{
		// --- fill in the struct
		guiInfoStruct.message = GUI_WILL_CLOSE;
		guiInfoStruct.customViewName = "";
		guiInfoStruct.subControllerName = "";
		guiInfoStruct.editor = (void*)this;
		guiInfoStruct.listener = NULL;

        m_pPlugIn->showGUI((void*)(&guiInfoStruct));
	}

    //-- on close we need to delete the frame object.
    //-- once again we make sure that the member frame variable is set to zero before we delete it
    //-- so that calls to setParameter won't crash.
    CFrame* oldFrame = frame;
    frame = 0;
    oldFrame->forget();
    bClosing = false;
}

void CRafxVSTEditor::idle()
{
    if(bClosing) return;

    // --- update meters (thread safe)
    if(m_ppVuMeterControls && m_pControlMap && m_pPlugIn)
    {
        for(int i=0; i<m_nVuMeterCount; i++)
        {
            CVuMeter* pControl = m_ppVuMeterControls[i];
            if(pControl)
            {
                for(int i=0; i<MAX_USER_METERS; i++)
                {
                    if(m_nMeterMap[i]>= 0 && pControl->getTag() == m_nMeterMap[i])
                    {
#ifdef AUPLUGIN
                        Float32 paramValue = 0.0;
                        if(AudioUnitGetParameter(m_AU, m_nMeterMap[i], kAudioUnitScope_Global, 0, &paramValue) == noErr)
                        {
                            pControl->setValue(paramValue);
                            pControl->invalid();
                        }
#endif

#ifdef AAXPLUGIN
                        double dParam = 0.0;
                        if(m_pAAXParameters)
                        {
                            std::stringstream str;
                            str << m_nMeterMap[i] + 1;
                            m_pAAXParameters->GetParameterNormalizedValue(str.str().c_str(), &dParam);
                            pControl->setValue(dParam);
                            pControl->invalid();
                        }
#endif
                    }
                }
            }
        }
    }

    // --- update frame - important; this updates all children
    if(frame)
        frame->idle();
}


void CRafxVSTEditor::gatherTaggedTextLabels()
{
    m_nTaggedTextLabelCount = 0;
    for(int i=0; i<m_nTextLabelCount; i++)
    {
        CTextLabel* p = m_ppTextLabels[i];
        if(p->getTag() >= 0)
            m_nTaggedTextLabelCount++;
    }

    if(m_ppTaggedTextLabels)
        delete [] m_ppTaggedTextLabels;

    m_ppTaggedTextLabels = new CTextLabel*[m_nTaggedTextLabelCount];

    int m = 0;
    for(int i=0; i<m_nTextLabelCount; i++)
    {
        CTextLabel* p = m_ppTextLabels[i];
        if(p->getTag() >= 0)
            m_ppTaggedTextLabels[m++] = p;
    }
}

// --- only for controls embedded in the frame
bool CRafxVSTEditor::createSubView(CViewContainer* pParentView, pugi::xml_node viewNode, bool bFrameSubView)
{
    const char_t* classname = m_XMLParser.getAttribute(viewNode, "class");
    string sClassName = string(classname);

 	// --- currently only supporting CViewContainer && UIViewSwitchContainer to hold sub-views
	if(sClassName == "CViewContainer" || sClassName == "UIViewSwitchContainer")
	{
		// --- get bitmap first
		CBitmap* backBitmap = getLoadBitmap(viewNode, "bitmap");

		CViewContainer* pVC = sClassName == "CViewContainer" ? createViewContainer(viewNode, backBitmap) : createUISwitchView(viewNode, backBitmap, true, bFrameSubView);

		if(!isRAFXSwitchView(pParentView)) // otherwise check
			pParentView->addView(pVC);

		if(sClassName == "UIViewSwitchContainer")
			((CRAFXSwitchView*)pVC)->setCurrentViewIndex(0);

        // --- is this a template VC or stand-alone?
        const char_t* templatename = m_XMLParser.getAttribute(viewNode, "template");
        if(strlen(templatename) > 0)
        {
            int count = 0;
            pugi::xml_node templateNode = m_XMLParser.getTemplateInfo(templatename, count);
            for(int i=0; i<count; i++)
            {
                if(pParentView == frame)
                    createSubView(pVC, m_XMLParser.getViewSubViewNode(templateNode, i), false);
                else
                    createSubView(pVC, m_XMLParser.getViewSubViewNode(templateNode, i), true);
            }
        }
        else // called for ViewContainers inside of ViewConatiners
        {
            int count = m_XMLParser.getSubViewCount(viewNode);
            for(int i=0; i<count; i++)
            {
                createSubView(pVC, m_XMLParser.getViewSubViewNode(viewNode, i), true);
            }
        }

        if(backBitmap)
            backBitmap->forget();
    }

    if(sClassName == "CView")
	{
		// --- get bitmap first
		CBitmap* viewBitmap = getLoadBitmap(viewNode, "bitmap");
		CView* pView = createView(viewNode, viewBitmap, true, bFrameSubView);

		// --- add it
		pParentView->addView(pView);

		// --- forget bitmap
		if(viewBitmap)
			viewBitmap->forget();
	}

    if(sClassName == "CTextLabel")
    {
        // ---										  addingNew, standalone)
        CTextLabel* pLabel = createTextLabel(viewNode, true, bFrameSubView); // if bFrameSubView, standAlone = true
        pParentView->addView(pLabel);
    }

    if(sClassName == "CTextEdit")
    {
        CTextEdit* pEdit = createTextEdit(viewNode, true, bFrameSubView);
        pParentView->addView(pEdit);
    }

    if(sClassName == "COnOffButton")
    {
        // --- get bitmap first
        CBitmap* buttBitmap = getLoadBitmap(viewNode, "bitmap");
        COnOffButton* pButt = createOnOffButton(viewNode, buttBitmap, true, bFrameSubView);

        // --- add it
        pParentView->addView(pButt);

        // --- forget bitmap
        if(buttBitmap)
            buttBitmap->forget();
    }

    if(sClassName == "CKickButton")
    {
        // --- get bitmap first
        CBitmap* buttBitmap = getLoadBitmap(viewNode, "bitmap");
        CKickButton* pButt = createKickButton(viewNode, buttBitmap, true, bFrameSubView);

        // --- add it
        pParentView->addView(pButt);

        // --- forget bitmap
        if(buttBitmap)
            buttBitmap->forget();
    }

    if(sClassName == "CVerticalSwitch")
    {
        // --- get bitmap first
        CBitmap* switchBitmap = getLoadBitmap(viewNode, "bitmap");
        CVerticalSwitch* pSwitch = createVerticalSwitch(viewNode, switchBitmap);

        // --- add it
        pParentView->addView(pSwitch);

        // --- forget bitmap
        if(switchBitmap)
            switchBitmap->forget();
    }

    if(sClassName == "CHorizontalSwitch")
	{
		// --- get bitmap first
		CBitmap* switchBitmap = getLoadBitmap(viewNode, "bitmap");
		CHorizontalSwitch* pSwitch = createHorizontalSwitch(viewNode, switchBitmap);

		// --- add it
		if(pParentView)
			pParentView->addView(pSwitch);

		// --- forget bitmap
		if(switchBitmap)
			switchBitmap->forget();
	}

    if(sClassName == "CSegmentButton")
    {
        // ---										        addingNew, standalone)
        CSegmentButton* pSB = createSegmentButton(viewNode, true);//, bFrameSubView); // if bFrameSubView, standAlone = true
        pParentView->addView(pSB);
    }

    if(sClassName == "COptionMenu")
    {
        // --- get bitmap first
        CBitmap* backBitmap = getLoadBitmap(viewNode, "bitmap");
        COptionMenu* pOM = createOptionMenu(viewNode, backBitmap, true, bFrameSubView);

        // --- add it
        pParentView->addView(pOM);

        // --- forget bitmap
        if(backBitmap)
            backBitmap->forget();
    }

    if(sClassName == "CXYPad")
    {
        // --- get bitmap first
        CBitmap* backBitmap = getLoadBitmap(viewNode, "bitmap");
        CXYPad* pPad = createXYPad(viewNode, backBitmap, true, bFrameSubView);

        // --- add it
        pParentView->addView(pPad);

        // --- forget bitmap
        if(backBitmap)
            backBitmap->forget();
    }

    if(sClassName == "CAnimKnob")
    {
        // --- get bitmap first
        CBitmap* knobBitmap = getLoadBitmap(viewNode, "bitmap");
        CAnimKnob* pKnob = createAnimKnob(viewNode, knobBitmap, true, bFrameSubView);

        // --- add it
        pParentView->addView(pKnob);

        // --- forget bitmap
        if(knobBitmap)
            knobBitmap->forget();
    }

    if(sClassName == "CSlider")
    {
        // --- get bitmap first
        CBitmap* grooveBitmap = getLoadBitmap(viewNode, "bitmap");
        CBitmap* handleBitmap = getLoadBitmap(viewNode, "handle-bitmap");

        CSlider* pSlider = createSlider(viewNode, handleBitmap, grooveBitmap, true, bFrameSubView);
        pSlider->setValue(0.0);

        // --- add it
        pParentView->addView(pSlider);
        pSlider->invalid();

        // --- forget bitmaps
        if(grooveBitmap)
            grooveBitmap->forget();
        if(handleBitmap)
            handleBitmap->forget();
    }

    if(sClassName == "CVuMeter")
    {
        // --- get bitmap first
        CBitmap* onBitmap = getLoadBitmap(viewNode, "bitmap");
        CBitmap* offBitmap = getLoadBitmap(viewNode, "off-bitmap");

        CVuMeter* pMeter = createMeter(viewNode, onBitmap, offBitmap, true, bFrameSubView);

        // --- add it
        pParentView->addView(pMeter);

        // --- forget bitmaps
        if(onBitmap)
            onBitmap->forget();
        if(offBitmap)
            offBitmap->forget();
    }
    return true;
}
CViewContainer* CRafxVSTEditor::createUISwitchViewSubView(CViewContainer* pParentView, pugi::xml_node viewNode)
{
    const char_t* classname = m_XMLParser.getAttribute(viewNode, "class");
    string sClassName = string(classname);

    CViewContainer* pVC = NULL;
    bool bFrameSubView = false;

    // --- currently supporting CViewContainer && UIViewSwitchContainer to hold sub-views
    if(sClassName == "CViewContainer" ||  sClassName == "UIViewSwitchContainer")
    {
        // --- get bitmap first
        CBitmap* backBitmap = getLoadBitmap(viewNode, "bitmap");

        // --- may need to set that last true to false, check that -------------------------------------------------------------------------|
        pVC = sClassName == "CViewContainer" ? createViewContainer(viewNode, backBitmap) : createUISwitchView(viewNode, backBitmap, true, false);
        if(pParentView)
            pParentView->addView(pVC);

        // --- is this a template VC or stand-alone?
        const char_t* templatename = m_XMLParser.getAttribute(viewNode, "template");
        if(strlen(templatename) > 0)
        {
            int count = 0;
            pugi::xml_node templateNode = m_XMLParser.getTemplateInfo(templatename, count);
            for(int i=0; i<count; i++)
            {
                createUISwitchViewSubView(pVC, m_XMLParser.getViewSubViewNode(templateNode, i));
            }
        }
        else // I don't think this will get called ever? --  YES for ViewContainers inside of ViewConatiners
        {
            int count = m_XMLParser.getSubViewCount(viewNode);
            for(int i=0; i<count; i++)
            {
                createUISwitchViewSubView(pVC, m_XMLParser.getViewSubViewNode(viewNode, i));
            }
        }

        if(backBitmap)
            backBitmap->forget();
    }

    if(sClassName == "CView")
    {
        // --- get bitmap first
        CBitmap* viewBitmap = getLoadBitmap(viewNode, "bitmap");
        CView* pView = createView(viewNode, viewBitmap, true, bFrameSubView);

        // --- add it
        if(pParentView)
            pParentView->addView(pView);

        // --- forget bitmap
        if(viewBitmap)
            viewBitmap->forget();
    }

    // --- text labels:
    if(sClassName == "CTextLabel")
    {
        // ---										  addingNew, standalone)
        CTextLabel* pLabel = createTextLabel(viewNode, true, bFrameSubView); // if bFrameSubView, standAlone = true
        if(pParentView)
            pParentView->addView(pLabel);
    }
    if(sClassName == "CTextEdit")
    {
        CTextEdit* pEdit = createTextEdit(viewNode, true, bFrameSubView);
        if(pParentView)
            pParentView->addView(pEdit);
    }

    if(sClassName == "COnOffButton")
    {
        // --- get bitmap first
        CBitmap* buttBitmap = getLoadBitmap(viewNode, "bitmap");
        COnOffButton* pButt = createOnOffButton(viewNode, buttBitmap, true, bFrameSubView);

        // --- add it
        if(pParentView)
            pParentView->addView(pButt);

        // --- forget bitmap
        if(buttBitmap)
            buttBitmap->forget();
    }

    if(sClassName == "CKickButton")
    {
        // --- get bitmap first
        CBitmap* buttBitmap = getLoadBitmap(viewNode, "bitmap");
        CKickButton* pButt = createKickButton(viewNode, buttBitmap, true, bFrameSubView);

        // --- add it
        if(pParentView)
            pParentView->addView(pButt);

        // --- forget bitmap
        if(buttBitmap)
            buttBitmap->forget();
    }

    if(sClassName == "CVerticalSwitch")
    {
        // --- get bitmap first
        CBitmap* switchBitmap = getLoadBitmap(viewNode, "bitmap");
        CVerticalSwitch* pSwitch = createVerticalSwitch(viewNode, switchBitmap);

        // --- add it
        if(pParentView)
            pParentView->addView(pSwitch);

        // --- forget bitmap
        if(switchBitmap)
            switchBitmap->forget();
    }

    if(sClassName == "CHorizontalSwitch")
    {
        // --- get bitmap first
        CBitmap* switchBitmap = getLoadBitmap(viewNode, "bitmap");
        CHorizontalSwitch* pSwitch = createHorizontalSwitch(viewNode, switchBitmap);

        // --- add it
        if(pParentView)
            pParentView->addView(pSwitch);

        // --- forget bitmap
        if(switchBitmap)
            switchBitmap->forget();
    }

  	if(sClassName == "CSegmentButton")
    {
        // ---										        addingNew, standalone)
        CSegmentButton* pSB = createSegmentButton(viewNode, true);//, bFrameSubView); // if bFrameSubView, standAlone = true
        pParentView->addView(pSB);
    }

    if(sClassName == "COptionMenu")
    {
        // --- get bitmap first
        CBitmap* backBitmap = getLoadBitmap(viewNode, "bitmap");
        COptionMenu* pOM = createOptionMenu(viewNode, backBitmap, true, bFrameSubView);

        // --- add it
        if(pParentView)
            pParentView->addView(pOM);

        // --- forget bitmap
        if(backBitmap)
            backBitmap->forget();
    }

    if(sClassName == "CXYPad")
    {
        // --- get bitmap first
        CBitmap* backBitmap = getLoadBitmap(viewNode, "bitmap");
        CXYPad* pPad = createXYPad(viewNode, backBitmap, true, bFrameSubView);

        // --- add it
        if(pParentView)
            pParentView->addView(pPad);

        // --- forget bitmap
        if(backBitmap)
            backBitmap->forget();
    }

    if(sClassName == "CAnimKnob")
    {
        // --- get bitmap first
        CBitmap* knobBitmap = getLoadBitmap(viewNode, "bitmap");
        CAnimKnob* pKnob = createAnimKnob(viewNode, knobBitmap, true, bFrameSubView);

        // --- add it
        if(pParentView)
            pParentView->addView(pKnob);

        // --- forget bitmap
        if(knobBitmap)
            knobBitmap->forget();
    }

    if(sClassName == "CSlider")
    {
        // --- get bitmap first
        CBitmap* grooveBitmap = getLoadBitmap(viewNode, "bitmap");
        CBitmap* handleBitmap = getLoadBitmap(viewNode, "handle-bitmap");

        CSlider* pSlider = createSlider(viewNode, handleBitmap, grooveBitmap, true, bFrameSubView);
        pSlider->setValue(0.0);

        // --- add it
        if(pParentView)
            pParentView->addView(pSlider);

        pSlider->invalid();

        // --- forget bitmaps
        if(grooveBitmap)
            grooveBitmap->forget();
        if(handleBitmap)
            handleBitmap->forget();
    }

    if(sClassName == "CVuMeter")
    {
        // --- get bitmap first
        CBitmap* onBitmap = getLoadBitmap(viewNode, "bitmap");
        CBitmap* offBitmap = getLoadBitmap(viewNode, "off-bitmap");

        CVuMeter* pMeter = createMeter(viewNode, onBitmap, offBitmap, true, bFrameSubView);

        // --- add it
        if(pParentView)
            pParentView->addView(pMeter);

        // --- forget bitmaps
        if(onBitmap)
            onBitmap->forget();
        if(offBitmap)
            offBitmap->forget();
    }
    return pVC;
}


CView* CRafxVSTEditor::createView(pugi::xml_node node, CBitmap* backgroundBitmap, bool bAddingNew, bool bStandAlone)
{
    CView* pVC = NULL;
    CView* pView = checkCreateCustomView(node);

    if(pView)
        pVC = pView;
    else
    {
        pVC = new CView(getRectFromNode(node));

        if(backgroundBitmap)
            pVC->setBackground(backgroundBitmap);
        else
            pVC->setAlphaValue(1);

        // --- CView
        const char_t* transp = m_XMLParser.getAttribute(node, "transparent");
        if(strlen(transp) > 0)
        {
            if(strcmp(transp, "true") == 0)
                pVC->setTransparency(true);
            else
                pVC->setTransparency(false);
        }

        const char_t* mouseE = m_XMLParser.getAttribute(node, "mouse-enabled");
        if(strlen(mouseE) > 0)
        {
            if(strcmp(mouseE, "true") == 0)
                pVC->setMouseEnabled(true);
            else
                pVC->setMouseEnabled(false);
        }
        // --- stash the XML Node on the CView object
		pVC->setAttribute(RAFX_XML_NODE, sizeof(pugi::xml_node*), (void*)&node);
    }

    if(bAddingNew)
    {
        // --- save control; requires destroy/create
        CView** temp = new CView*[m_nViewCount + 1];
        for(int i=0; i<m_nViewCount; i++)
            temp[i] = m_ppViews[i];

        // --- add new one
        temp[m_nViewCount] = pVC;
        m_nViewCount++;

        if(m_ppViews)
            delete m_ppViews;
        m_ppViews = temp;
    }

    return pVC;
}

CViewContainer* CRafxVSTEditor::createViewContainer(pugi::xml_node node, CBitmap* backgroundBitmap, bool bAddingNew, bool bStandAlone)
{
   	CViewContainer* pVC = NULL;
	CView* pView = checkCreateCustomView(node);

	if(pView)
		pVC = (CViewContainer*)pView;
	else
	{
        pVC = new CViewContainer(getRectFromNode(node));
        if(backgroundBitmap)
            pVC->setBackground(backgroundBitmap);

        const char_t* backcolor = m_XMLParser.getAttribute(node, "background-color");
        if(strlen(backcolor) > 0)
        {
            string sBackColorName(backcolor);

            // --- get the color
            CColor backCColor = getCColor(backcolor);
            pVC->setBackgroundColor(backCColor);
        }

        const char_t* backcolorDS = m_XMLParser.getAttribute(node, "background-color-draw-style");
        if(strlen(backcolorDS) > 0)
        {
            if(strcmp(backcolorDS, "filled and stroked") == 0)
                pVC->setBackgroundColorDrawStyle(kDrawFilledAndStroked);
            else if(strcmp(backcolorDS, "filled") == 0)
                pVC->setBackgroundColorDrawStyle(kDrawFilled);
            else if(strcmp(backcolorDS, "stroked") == 0)
                pVC->setBackgroundColorDrawStyle(kDrawStroked);
        }

        // --- CView
        const char_t* transp = m_XMLParser.getAttribute(node, "transparent");
        if(strlen(transp) > 0)
        {
            if(strcmp(transp, "true") == 0)
                pVC->setTransparency(true);
            else
                pVC->setTransparency(false);
        }

        const char_t* mouseE = m_XMLParser.getAttribute(node, "mouse-enabled");
        if(strlen(mouseE) > 0)
        {
            if(strcmp(mouseE, "true") == 0)
                pVC->setMouseEnabled(true);
            else
                pVC->setMouseEnabled(false);
        }
        // --- stash the XML Node on the CView object
		pVC->setAttribute(RAFX_XML_NODE, sizeof(pugi::xml_node*), (void*)&node);
    }

    if(bAddingNew)
    {
        // --- save control; requires destroy/create
        CViewContainer** temp = new CViewContainer*[m_nViewContainerCount + 1];
        for(int i=0; i<m_nViewContainerCount; i++)
            temp[i] = m_ppViewContainers[i];

        // --- add new one
        temp[m_nViewContainerCount] = pVC;
        m_nViewContainerCount++;

        if(m_ppViewContainers)
            delete m_ppViewContainers;
        m_ppViewContainers = temp;
    }

    return pVC;
}

CViewContainer* CRafxVSTEditor::createUISwitchView(pugi::xml_node node, CBitmap* backgroundBitmap,
                                                   bool bAddingNew, bool bStandAlone)
{
    CViewContainer* pVC = NULL;
    CView* pView = checkCreateCustomView(node);

    if(pView)
        pVC = (CViewContainer*)pView;
    else
    {
        pVC = new CRAFXSwitchView(getRectFromNode(node));

        if(backgroundBitmap)
            pVC->setBackground(backgroundBitmap);

        const char_t* backcolor = m_XMLParser.getAttribute(node, "background-color");
        if(strlen(backcolor) > 0)
        {
            string sBackColorName(backcolor);

            // --- get the color
            CColor backCColor = getCColor(backcolor);
            pVC->setBackgroundColor(backCColor);
        }

        const char_t* backcolorDS = m_XMLParser.getAttribute(node, "background-color-draw-style");
        if(strlen(backcolorDS) > 0)
        {
            if(strcmp(backcolorDS, "filled and stroked") == 0)
                pVC->setBackgroundColorDrawStyle(kDrawFilledAndStroked);
            else if(strcmp(backcolorDS, "filled") == 0)
                pVC->setBackgroundColorDrawStyle(kDrawFilled);
            else if(strcmp(backcolorDS, "stroked") == 0)
                pVC->setBackgroundColorDrawStyle(kDrawStroked);
        }

        // --- CView
        const char_t* transp = m_XMLParser.getAttribute(node, "transparent");
        if(strlen(transp) > 0)
        {
            if(strcmp(transp, "true") == 0)
                pVC->setTransparency(true);
            else
                pVC->setTransparency(false);
        }

        const char_t* mouseE = m_XMLParser.getAttribute(node, "mouse-enabled");
        if(strlen(mouseE) > 0)
        {
            if(strcmp(mouseE, "true") == 0)
                pVC->setMouseEnabled(true);
            else
                pVC->setMouseEnabled(false);
        }

        // --- tag
        const char_t* ctagName = m_XMLParser.getAttribute(node, "template-switch-control");
        const char_t* ctag = m_XMLParser.getControlTagAttribute(ctagName, "tag");
        int32_t tag = -1;
        if(strlen(ctag) > 0)
            tag = atoi(ctag);

        ((CRAFXSwitchView*)pVC)->setControllerTag(tag);

        const char_t* animationTime = m_XMLParser.getAttribute(node, "animation-time");
        int32_t time = 120;
        if(strlen(animationTime) > 0)
        {
            time = atoi(animationTime);
            ((CRAFXSwitchView*)pVC)->setAnimationTime(time);
        }

        // --- iterate and create subviews
        const char_t* subViews = m_XMLParser.getAttribute(node, "template-names");
        if(strlen(subViews) > 0)// && !m_bGUIDesignMode)
        {
            // --- enums
            bool bWorking = true;
            int nIndex = 0;
            while(bWorking)
            {
                char* templateName = getEnumString((char*)subViews, nIndex);
                if(!templateName)
                    bWorking = false;
                else
                {
                    // --- create view - my way is non-dynamic, but you get all the new RAFX GUI API properly
                    xml_node templateNode = m_XMLParser.getTemplateNode(templateName);
                    if(!templateNode.empty())
                    {
                        // --- view containers only!
                        const char_t* classname = m_XMLParser.getAttribute(templateNode, "class");
                        string sClassName = string(classname);

                        CViewContainer* pSubContainer = createUISwitchViewSubView(NULL, templateNode);

                        if(pSubContainer)
                            ((CRAFXSwitchView*)pVC)->addSubViewContainer(pSubContainer); // this will add, then hide, the container
                    }
                    nIndex++;
                }
            }
        }
        // --- stash the XML Node on the CView object
		pVC->setAttribute(RAFX_XML_NODE, sizeof(pugi::xml_node*), (void*)&node);
    }

    if(bAddingNew)
    {
        // --- save control; requires destroy/create
        CViewContainer** temp = new CViewContainer*[m_nViewContainerCount + 1];
        for(int i=0; i<m_nViewContainerCount; i++)
            temp[i] = m_ppViewContainers[i];

        // --- add new one
        temp[m_nViewContainerCount] = pVC;
        m_nViewContainerCount++;

        if(m_ppViewContainers)
            delete m_ppViewContainers;
        m_ppViewContainers = temp;
    }

    return pVC;
}


CFontDesc* CRafxVSTEditor::createFontDescFromFontNode(pugi::xml_node node)
{
    const char_t* font = m_XMLParser.getAttribute(node, "font-name");
    if(strlen(font) <= 0)
        return new CFontDesc();

    CFontDesc* fontDesc = new CFontDesc(font);

    const char_t* size = m_XMLParser.getFontAttribute(font, "size");
    if(strlen(size) > 0)
    {
        string ccoord(size);
        const CCoord fontsize = ::atof(ccoord.c_str());
        fontDesc->setSize(fontsize);
    }

    const char_t* bold = m_XMLParser.getFontAttribute(font, "bold");
    if(strlen(bold) > 0)
    {
        if(strcmp(bold, "true") == 0)
            fontDesc->setStyle(fontDesc->getStyle() | kBoldFace);
        else
            fontDesc->setStyle(fontDesc->getStyle() & ~kBoldFace);
    }

    const char_t* ital = m_XMLParser.getFontAttribute(font, "italic");
    if(strlen(ital) > 0)
    {
        if(strcmp(ital, "true") == 0)
            fontDesc->setStyle(fontDesc->getStyle() | kItalicFace);
        else
            fontDesc->setStyle(fontDesc->getStyle() & ~kBoldFace);
    }

    const char_t* strike = m_XMLParser.getFontAttribute(font, "strike-through");
    if(strlen(strike) > 0)
    {
        if(strcmp(strike, "true") == 0)
            fontDesc->setStyle(fontDesc->getStyle() | kStrikethroughFace);
        else
            fontDesc->setStyle(fontDesc->getStyle() & ~kStrikethroughFace);
    }

    const char_t* underline = m_XMLParser.getFontAttribute(font, "underline");
    if(strlen(underline) > 0)
    {
        if(strcmp(underline, "true") == 0)
            fontDesc->setStyle(fontDesc->getStyle() | kUnderlineFace);
        else
            fontDesc->setStyle(fontDesc->getStyle() & ~kUnderlineFace);
    }

    return fontDesc;

}

CFontDesc* CRafxVSTEditor::createFontDesc(pugi::xml_node node)
{
    const char_t* font = m_XMLParser.getAttribute(node, "font");
    if(strlen(font) <= 0)
        return new CFontDesc();

    const char_t* fontname = m_XMLParser.getFontAttribute(font, "font-name");
    string str(fontname);
    if(strlen(fontname) <= 0)
    {
        const CFontRef builtInFont = getBuiltInFont(font);
        if(builtInFont)
        {
            return builtInFont;
        }
    }

    CFontDesc* fontDesc = new CFontDesc(fontname);

    const char_t* size = m_XMLParser.getFontAttribute(font, "size");
    if(strlen(size) > 0)
    {
        string ccoord(size);
        const CCoord fontsize = ::atof(ccoord.c_str());
        fontDesc->setSize(fontsize);
    }

    const char_t* bold = m_XMLParser.getFontAttribute(font, "bold");
    if(strlen(bold) > 0)
    {
        if(strcmp(bold, "true") == 0)
            fontDesc->setStyle(kBoldFace);
    }

    const char_t* ital = m_XMLParser.getFontAttribute(font, "italic");
    if(strlen(ital) > 0)
    {
        if(strcmp(ital, "true") == 0)
            fontDesc->setStyle(fontDesc->getStyle() | kItalicFace);
    }

    const char_t* strike = m_XMLParser.getFontAttribute(font, "strike-through");
    if(strlen(strike) > 0)
    {
        if(strcmp(strike, "true") == 0)
            fontDesc->setStyle(fontDesc->getStyle() | kStrikethroughFace);
    }

    const char_t* underline = m_XMLParser.getFontAttribute(font, "underline");
    if(strlen(underline) > 0)
    {
        if(strcmp(underline, "true") == 0)
            fontDesc->setStyle(fontDesc->getStyle() | kUnderlineFace);
    }

    return fontDesc;
}


void CRafxVSTEditor::parseTextLabel(CTextLabel* pLabel, pugi::xml_node node)
{
    const char_t* title = m_XMLParser.getAttribute(node, "title");
    if(strlen(title) > 0)
        pLabel->setText(title);

    const char_t* ctagName = m_XMLParser.getAttribute(node, "control-tag");

	// --- zero indexed number
	const char_t* ctag = m_XMLParser.getControlTagAttribute(ctagName, "tag");
	int32_t tag = -1;
	if(strlen(ctag) > 0)
	{
		tag = atoi(ctag);
		pLabel->setTag(tag);
	}

    // --- now set supported attributes
    //
    // --- CParamDisplay
    const char_t* backcolor = m_XMLParser.getAttribute(node, "back-color");
    if(strlen(backcolor) > 0)
    {
        string sBackColorName(backcolor);

        // --- get the color
        CColor backCColor = getCColor(backcolor);
        pLabel->setBackColor(backCColor);
    }

    // --- font stuff
    CFontDesc* fontDesc = createFontDesc(node);
    fontDesc->getPlatformFont();

    pLabel->setFont(fontDesc);

    const char_t* antia = m_XMLParser.getAttribute(node, "font-antialias");
    if(strlen(antia) > 0)
    {
        if(strcmp(antia, "true") == 0)
            pLabel->setAntialias(true);
        else
            pLabel->setAntialias(false);
    }

    const char_t* fcolor = m_XMLParser.getAttribute(node, "font-color");
    if(strlen(fcolor) > 0)
    {
        CColor fCColor = getCColor(fcolor);
        pLabel->setFontColor(fCColor);
    }

    const char_t* frmcolor = m_XMLParser.getAttribute(node, "frame-color");
    if(strlen(frmcolor) > 0)
    {
        CColor frmCColor = getCColor(frmcolor);
        pLabel->setFrameColor(frmCColor);
    }

    const char_t* frmwidth = m_XMLParser.getAttribute(node, "frame-width");
    if(strlen(frmwidth) > 0)
    {
        const CCoord frmWidth = ::atof(frmwidth);
        pLabel->setFrameWidth(frmWidth);
    }

    const char_t* rrr = m_XMLParser.getAttribute(node, "round-rect-radius");
    if(strlen(rrr) > 0)
    {
        const CCoord rrradius = ::atof(rrr);
        pLabel->setRoundRectRadius(rrradius);
    }

    const char_t* shadcolor = m_XMLParser.getAttribute(node, "shadow-color");
    if(strlen(shadcolor) > 0)
    {
        CColor shadCColor = getCColor(shadcolor);
        pLabel->setShadowColor(shadCColor);
    }

    const char_t* align = m_XMLParser.getAttribute(node, "text-alignment");
    if(strlen(align) > 0)
    {
        if(strcmp(align, "left") == 0)
            pLabel->setHoriAlign(kLeftText);
        else if(strcmp(align, "center") == 0)
            pLabel->setHoriAlign(kCenterText);
        else if(strcmp(align, "right") == 0)
            pLabel->setHoriAlign(kRightText);
    }

    const char_t* inset = m_XMLParser.getAttribute(node, "text-inset");
    if(strlen(inset) > 0)
    {
        const CPoint insetPt = getCPointFromString(inset);
        pLabel->setTextInset(insetPt);
    }

    // --- currently not supported
    //const char_t* precis = m_XMLParser.getAttribute(node, "value-precision");
    //pLabel->setPrecision(precis);

    // --- CControl
    const char_t* backoffset = m_XMLParser.getAttribute(node, "background-offset");
    if(strlen(backoffset) > 0)
    {
        const CPoint point = getCPointFromString(backoffset);
        pLabel->setBackOffset(point);
    }

    const char_t* minValue = m_XMLParser.getAttribute(node, "min-value");
    if(strlen(minValue) > 0)
        pLabel->setMin(::atof(minValue));

    const char_t* maxValue = m_XMLParser.getAttribute(node, "max-value");
    if(strlen(maxValue) > 0)
        pLabel->setMax(::atof(maxValue));

    const char_t* wiv = m_XMLParser.getAttribute(node, "wheel-inc-value");
    if(strlen(wiv) > 0)
        pLabel->setWheelInc(::atof(wiv));

    // --- style shit
    const char_t* sty1 = m_XMLParser.getAttribute(node, "style-3D-in");
    if(strlen(sty1) > 0)
    {
        if(strcmp(sty1, "true") == 0)
            pLabel->setStyle(pLabel->getStyle() | k3DIn);
        else
            pLabel->setStyle(pLabel->getStyle() & ~k3DIn);
    }

    const char_t* sty2 = m_XMLParser.getAttribute(node, "style-3D-out");
    if(strlen(sty2) > 0)
    {
        if(strcmp(sty2, "true") == 0)
            pLabel->setStyle(pLabel->getStyle() | k3DOut);
        else
            pLabel->setStyle(pLabel->getStyle() & ~k3DOut);
    }

    const char_t* sty3 = m_XMLParser.getAttribute(node, "style-no-draw");
    if(strlen(sty3) > 0)
    {
        if(strcmp(sty3, "true") == 0)
            pLabel->setStyle(pLabel->getStyle() | kNoDrawStyle);
        else
            pLabel->setStyle(pLabel->getStyle() & ~kNoDrawStyle);
    }

    const char_t* sty4 = m_XMLParser.getAttribute(node, "style-no-text");
    if(strlen(sty4) > 0)
    {
        if(strcmp(sty4, "true") == 0)
            pLabel->setStyle(pLabel->getStyle() | kNoTextStyle);
        else
            pLabel->setStyle(pLabel->getStyle() & ~kNoTextStyle);
    }

    const char_t* sty5 = m_XMLParser.getAttribute(node, "style-round-rect");
    if(strlen(sty5) > 0)
    {
        if(strcmp(sty5, "true") == 0)
            pLabel->setStyle(pLabel->getStyle() | kRoundRectStyle);
        else
            pLabel->setStyle(pLabel->getStyle() & ~kRoundRectStyle);
    }

    const char_t* sty6 = m_XMLParser.getAttribute(node, "style-shadow-text");
    if(strlen(sty6) > 0)
    {
        if(strcmp(sty6, "true") == 0)
            pLabel->setStyle(pLabel->getStyle() | kShadowText);
        else
            pLabel->setStyle(pLabel->getStyle() & ~kShadowText);
    }

    const char_t* sty7 = m_XMLParser.getAttribute(node, "style-no-frame");
    if(strlen(sty7) > 0)
    {
        if(strcmp(sty7, "true") == 0)
            pLabel->setStyle(pLabel->getStyle() | kNoFrame);
        else
            pLabel->setStyle(pLabel->getStyle() & ~kNoFrame);
    }

    // --- CView
    const char_t* transp = m_XMLParser.getAttribute(node, "transparent");
    if(strlen(transp) > 0)
    {
        if(strcmp(transp, "true") == 0)
            pLabel->setTransparency(true);
        else
            pLabel->setTransparency(false);
    }

    const char_t* mouseE = m_XMLParser.getAttribute(node, "mouse-enabled");
    if(strlen(mouseE) > 0)
    {
        if(strcmp(mouseE, "true") == 0)
            pLabel->setMouseEnabled(true);
        else
            pLabel->setMouseEnabled(false);
    }
}

CTextLabel* CRafxVSTEditor::createTextLabel(pugi::xml_node node, bool bAddingNew, bool bStandAlone)
{
	CTextLabel* pLabel = NULL;
	CView* pView = checkCreateCustomView(node);
	if(pView)
		pLabel = (CTextLabel*)pView;
	else
	{
		pLabel = new CTextLabel(getRectFromNode(node)); //, title, 0, 0);
		parseTextLabel(pLabel, node);

        // --- stash the XML Node on the CView object
		pLabel->setAttribute(RAFX_XML_NODE, sizeof(pugi::xml_node*), (void*)&node);
	}

    if(bAddingNew)
    {
        // --- save control; requires destroy/create
        CTextLabel** temp = new CTextLabel*[m_nTextLabelCount + 1];
        for(int i=0; i<m_nTextLabelCount; i++)
            temp[i] = m_ppTextLabels[i];

        // --- add new one
        temp[m_nTextLabelCount] = pLabel;
        m_nTextLabelCount++;

        if(m_ppTextLabels)
            delete m_ppTextLabels;
        m_ppTextLabels = temp;
    }

    return pLabel;
}

CTextEdit* CRafxVSTEditor::createTextEdit(pugi::xml_node node, bool bAddingNew, bool bStandAlone)
{
 	CTextEdit* pEdit = NULL;
	CView* pView = checkCreateCustomView(node);

	// --- tag
	const char_t* ctagName = m_XMLParser.getAttribute(node, "control-tag");

	// --- zero indexed number
	const char_t* ctag = m_XMLParser.getControlTagAttribute(ctagName, "tag");
	int32_t tag = -1;
	if(strlen(ctag) > 0)
		tag = atoi(ctag);

	if(pView)
		pEdit = (CTextEdit*)pView;
	else
	{
		pEdit = new CTextEdit(getRectFromNode(node), this, tag);

		// --- CTextLabel stuff (base class of TextEdit)
		parseTextLabel(pEdit, node);

		// --- CControl
		const char_t* minValue = m_XMLParser.getAttribute(node, "min-value");
		if(strlen(minValue) > 0)
			pEdit->setMin(::atof(minValue));

		const char_t* maxValue = m_XMLParser.getAttribute(node, "max-value");
		if(strlen(maxValue) > 0)
			pEdit->setMax(::atof(maxValue));

		const char_t* defValue = m_XMLParser.getAttribute(node, "default-value");
		if(strlen(defValue) > 0)
			pEdit->setDefaultValue(::atof(defValue));

		// --- tooltip
		const char_t* tooltipText = m_XMLParser.getAttribute(node, "tooltip");
		if(strlen(tooltipText) > 0)
			pEdit->setAttribute(kCViewTooltipAttribute, strlen (tooltipText)+1, tooltipText);

		// --- CTextEdit
		const char_t* itc = m_XMLParser.getAttribute(node, "immediate-text-change");
		if(strlen(itc) > 0)
		{
			if(strcmp(itc, "true") == 0)
				pEdit->setImmediateTextChange(true);
			else
				pEdit->setImmediateTextChange(false);
		}
		// --- stash the XML Node on the CView object
		pEdit->setAttribute(RAFX_XML_NODE, sizeof(pugi::xml_node*), (void*)&node);
	}

    if(bAddingNew)
    {
        // --- save control; requires destroy/create
        CTextEdit** temp = new CTextEdit*[m_nTextEditCount + 1];
        for(int i=0; i<m_nTextEditCount; i++)
            temp[i] = m_ppTextEditControls[i];

        // --- add new one
        temp[m_nTextEditCount] = pEdit;
        m_nTextEditCount++;

        if(m_ppTextEditControls)
            delete m_ppTextEditControls;
        m_ppTextEditControls = temp;
    }

	return pEdit;

}

CXYPad* CRafxVSTEditor::createXYPad(pugi::xml_node node, CBitmap* backgroundBitmap, bool bAddingNew, bool bStandAlone)
{

 	CXYPad* pPad = NULL;
	CView* pView = checkCreateCustomView(node);

	if(pView)
		pPad = (CXYPad*)pView;
	else
	{
		pPad = new CXYPadWP(getRectFromNode(node));

		pPad->setListener(this);
		if(backgroundBitmap)
			pPad->setBackground(backgroundBitmap);

		// --- tag
		const char_t* ctagName = m_XMLParser.getAttribute(node, "control-tag");

		if(strcmp(ctagName, "JOYSTICK") == 0)
			((CXYPadWP*)pPad)->m_bIsJoystickPad = true;
		else
			((CXYPadWP*)pPad)->m_bIsJoystickPad = false;

		// --- zero indexed number
		const char_t* ctag = m_XMLParser.getControlTagAttribute(ctagName, "tag");
		int32_t tag = -1;
		if(strlen(ctag) > 0)
			tag = atoi(ctag);

		pPad->setTag(tag);

		const char_t* ctagNameX = m_XMLParser.getAttribute(node, "control-tagX");
		const char_t* ctagX = m_XMLParser.getControlTagAttribute(ctagNameX, "tag");
		tag = -1;
		if(strlen(ctagX) > 0)
			tag = atoi(ctagX);
		else
			tag = JOYSTICK_X;

		((CXYPadWP*)pPad)->setTagX(tag);

		const char_t* ctagNameY = m_XMLParser.getAttribute(node, "control-tagY");
		const char_t* ctagY = m_XMLParser.getControlTagAttribute(ctagNameY, "tag");
		tag = -1;
		if(strlen(ctagY) > 0)
			tag = atoi(ctagY);
		else
			tag = JOYSTICK_Y;

		((CXYPadWP*)pPad)->setTagY(tag);

		// --- CParamDisplay
		const char_t* backcolor = m_XMLParser.getAttribute(node, "back-color");
		if(strlen(backcolor) > 0)
		{
			string sBackColorName(backcolor);

			// --- get the color
			CColor backCColor = getCColor(backcolor);
			pPad->setBackColor(backCColor);
		}

		// --- font stuff
		CFontDesc* fontDesc = createFontDesc(node);
		fontDesc->getPlatformFont();

		pPad->setFont(fontDesc);

		const char_t* antia = m_XMLParser.getAttribute(node, "font-antialias");
		if(strlen(antia) > 0)
		{
			if(strcmp(antia, "true") == 0)
				pPad->setAntialias(true);
			else
				pPad->setAntialias(false);
		}

		const char_t* fcolor = m_XMLParser.getAttribute(node, "font-color");
		if(strlen(fcolor) > 0)
		{
			CColor fCColor = getCColor(fcolor);
			pPad->setFontColor(fCColor);
		}

		const char_t* frmcolor = m_XMLParser.getAttribute(node, "frame-color");
		if(strlen(frmcolor) > 0)
		{
			CColor frmCColor = getCColor(frmcolor);
			pPad->setFrameColor(frmCColor);
		}

		const char_t* frmwidth = m_XMLParser.getAttribute(node, "frame-width");
		if(strlen(frmwidth) > 0)
		{
			const CCoord frmWidth = ::atof(frmwidth);
			pPad->setFrameWidth(frmWidth);
		}

		const char_t* rrr = m_XMLParser.getAttribute(node, "round-rect-radius");
		if(strlen(rrr) > 0)
		{
			const CCoord rrradius = ::atof(rrr);
			pPad->setRoundRectRadius(rrradius);
		}

		const char_t* shadcolor = m_XMLParser.getAttribute(node, "shadow-color");
		if(strlen(shadcolor) > 0)
		{
			CColor shadCColor = getCColor(shadcolor);
			pPad->setShadowColor(shadCColor);
		}

		// --- style shit
		const char_t* sty1 = m_XMLParser.getAttribute(node, "style-3D-in");
		if(strlen(sty1) > 0)
		{
			if(strcmp(sty1, "true") == 0)
				pPad->setStyle(pPad->getStyle() | k3DIn);
			else
				pPad->setStyle(pPad->getStyle() & ~k3DIn);
		}

		const char_t* sty2 = m_XMLParser.getAttribute(node, "style-3D-out");
		if(strlen(sty2) > 0)
		{
			if(strcmp(sty2, "true") == 0)
				pPad->setStyle(pPad->getStyle() | k3DOut);
			else
				pPad->setStyle(pPad->getStyle() & ~k3DOut);
		}

		const char_t* sty3 = m_XMLParser.getAttribute(node, "style-no-draw");
		if(strlen(sty3) > 0)
		{
			if(strcmp(sty3, "true") == 0)
				pPad->setStyle(pPad->getStyle() | kNoDrawStyle);
			else
			{
				pPad->setStyle(pPad->getStyle() & ~kNoDrawStyle);
			}
		}

		const char_t* sty4 = m_XMLParser.getAttribute(node, "style-no-text");
		if(strlen(sty4) > 0)
		{
			if(strcmp(sty4, "true") == 0)
				pPad->setStyle(pPad->getStyle() | kNoTextStyle);
			else
				pPad->setStyle(pPad->getStyle() & ~kNoTextStyle);
		}

		const char_t* sty5 = m_XMLParser.getAttribute(node, "style-round-rect");
		if(strlen(sty5) > 0)
		{
			if(strcmp(sty5, "true") == 0)
				pPad->setStyle(pPad->getStyle() | kRoundRectStyle);
			else
				pPad->setStyle(pPad->getStyle() & ~kRoundRectStyle);
		}

		const char_t* sty6 = m_XMLParser.getAttribute(node, "style-shadow-text");
		if(strlen(sty6) > 0)
		{
			if(strcmp(sty6, "true") == 0)
				pPad->setStyle(pPad->getStyle() | kShadowText);
			else
				pPad->setStyle(pPad->getStyle() & ~kShadowText);
		}

		const char_t* sty7 = m_XMLParser.getAttribute(node, "style-no-frame");
		if(strlen(sty7) > 0)
		{
			if(strcmp(sty7, "true") == 0)
				pPad->setStyle(pPad->getStyle() | kNoFrame);
			else
				pPad->setStyle(pPad->getStyle() & ~kNoFrame);
		}

		// --- CControl
		const char_t* backoffset = m_XMLParser.getAttribute(node, "background-offset");
		if(strlen(backoffset) > 0)
		{
			const CPoint point = getCPointFromString(backoffset);
			pPad->setBackOffset(point);
		}

		const char_t* align = m_XMLParser.getAttribute(node, "text-alignment");
		if(strlen(align) > 0)
		{
			if(strcmp(align, "left") == 0)
				pPad->setHoriAlign(kLeftText);
			else if(strcmp(align, "center") == 0)
				pPad->setHoriAlign(kCenterText);
			else if(strcmp(align, "right") == 0)
				pPad->setHoriAlign(kRightText);
		}

		const char_t* inset = m_XMLParser.getAttribute(node, "text-inset");
		if(strlen(inset) > 0)
		{
			const CPoint insetPt = getCPointFromString(inset);
			pPad->setTextInset(insetPt);
		}

		const char_t* wiv = m_XMLParser.getAttribute(node, "wheel-inc-value");
		if(strlen(wiv) > 0)
			pPad->setWheelInc(::atof(wiv));

		// --- CView
		const char_t* transp = m_XMLParser.getAttribute(node, "transparent");
		if(strlen(transp) > 0)
		{
			if(strcmp(transp, "true") == 0)
				pPad->setTransparency(true);
			else
				pPad->setTransparency(false);
		}

		const char_t* mouseE = m_XMLParser.getAttribute(node, "mouse-enabled");
		if(strlen(mouseE) > 0)
		{
			if(strcmp(mouseE, "true") == 0)
				pPad->setMouseEnabled(true);
			else
				pPad->setMouseEnabled(false);
		}
		// --- stash the XML Node on the CView object
		pPad->setAttribute(RAFX_XML_NODE, sizeof(pugi::xml_node*), (void*)&node);
	}

	pPad->invalid();

    if(bAddingNew)
    {
        // --- save control; requires destroy/create
        CXYPad** temp = new CXYPad*[m_nXYPadCount + 1];
        for(int i=0; i<m_nXYPadCount; i++)
            temp[i] = m_ppXYPads[i];

        // --- add new one
        temp[m_nXYPadCount] = pPad;
        m_nXYPadCount++;

        if(m_ppXYPads)
            delete m_ppXYPads;
        m_ppXYPads = temp;
    }

	return pPad;

}

COptionMenu* CRafxVSTEditor::createOptionMenu(pugi::xml_node node, CBitmap* backgroundBitmap, bool bAddingNew, bool bStandAlone)
{
 	// --- tag
	const char_t* ctagName = m_XMLParser.getAttribute(node, "control-tag");

	// --- zero indexed number
	const char_t* ctag = m_XMLParser.getControlTagAttribute(ctagName, "tag");
	int32_t tag = -1;
	if(strlen(ctag) > 0)
		tag = atoi(ctag);

	COptionMenu* pOM = NULL;
	CView* pView = checkCreateCustomView(node);

	if(pView)
		pOM = (COptionMenu*)pView;
	else
	{
		pOM = new COptionMenu(getRectFromNode(node), this, tag, backgroundBitmap, NULL);

		// --- OM specific
		const char_t* checkStyle = m_XMLParser.getAttribute(node, "menu-check-style");
		if(strlen(checkStyle) > 0)
		{
			if(strcmp(checkStyle, "true") == 0)
				pOM->setStyle(pOM->getStyle() | kCheckStyle);
			else
				pOM->setStyle(pOM->getStyle() & ~kCheckStyle);
		}

		const char_t* popupStyle = m_XMLParser.getAttribute(node, "menu-popup-style");
		if(strlen(popupStyle) > 0)
		{
			if(strcmp(popupStyle, "true") == 0)
				pOM->setStyle(pOM->getStyle() | kPopupStyle);
			else
				pOM->setStyle(pOM->getStyle() & ~kPopupStyle);
		}

		// --- CParamDisplay
		const char_t* backcolor = m_XMLParser.getAttribute(node, "back-color");
		if(strlen(backcolor) > 0)
		{
			string sBackColorName(backcolor);

			// --- get the color
			CColor backCColor = getCColor(backcolor);
			pOM->setBackColor(backCColor);
		}

		// --- font stuff
		CFontDesc* fontDesc = createFontDesc(node);
		fontDesc->getPlatformFont();

		pOM->setFont(fontDesc);

		const char_t* antia = m_XMLParser.getAttribute(node, "font-antialias");
		if(strlen(antia) > 0)
		{
			if(strcmp(antia, "true") == 0)
				pOM->setAntialias(true);
			else
				pOM->setAntialias(false);
		}

		const char_t* fcolor = m_XMLParser.getAttribute(node, "font-color");
		if(strlen(fcolor) > 0)
		{
			CColor fCColor = getCColor(fcolor);
			pOM->setFontColor(fCColor);
		}

		const char_t* frmcolor = m_XMLParser.getAttribute(node, "frame-color");
		if(strlen(frmcolor) > 0)
		{
			CColor frmCColor = getCColor(frmcolor);
			pOM->setFrameColor(frmCColor);
		}

		const char_t* frmwidth = m_XMLParser.getAttribute(node, "frame-width");
		if(strlen(frmwidth) > 0)
		{
			const CCoord frmWidth = ::atof(frmwidth);
			pOM->setFrameWidth(frmWidth);
		}

		const char_t* rrr = m_XMLParser.getAttribute(node, "round-rect-radius");
		if(strlen(rrr) > 0)
		{
			const CCoord rrradius = ::atof(rrr);
			pOM->setRoundRectRadius(rrradius);
		}

		const char_t* shadcolor = m_XMLParser.getAttribute(node, "shadow-color");
		if(strlen(shadcolor) > 0)
		{
			CColor shadCColor = getCColor(shadcolor);
			pOM->setShadowColor(shadCColor);
		}

		const char_t* align = m_XMLParser.getAttribute(node, "text-alignment");
		if(strlen(align) > 0)
		{
			if(strcmp(align, "left") == 0)
				pOM->setHoriAlign(kLeftText);
			else if(strcmp(align, "center") == 0)
				pOM->setHoriAlign(kCenterText);
			else if(strcmp(align, "right") == 0)
				pOM->setHoriAlign(kRightText);
		}

		const char_t* inset = m_XMLParser.getAttribute(node, "text-inset");
		if(strlen(inset) > 0)
		{
			const CPoint insetPt = getCPointFromString(inset);
			pOM->setTextInset(insetPt);
		}

		// --- currently not supported
		//const char_t* precis = m_XMLParser.getAttribute(node, "value-precision");
		//pLabel->setPrecision(precis);

		// --- CControl
		const char_t* backoffset = m_XMLParser.getAttribute(node, "background-offset");
		if(strlen(backoffset) > 0)
		{
			const CPoint point = getCPointFromString(backoffset);
			pOM->setBackOffset(point);
		}

		const char_t* minValue = m_XMLParser.getAttribute(node, "min-value");
		if(strlen(minValue) > 0)
			pOM->setMin(::atof(minValue));

		const char_t* maxValue = m_XMLParser.getAttribute(node, "max-value");
		if(strlen(maxValue) > 0)
			pOM->setMax(::atof(maxValue));

		const char_t* defValue = m_XMLParser.getAttribute(node, "default-value");
		if(strlen(defValue) > 0)
			pOM->setDefaultValue(::atof(defValue));

		const char_t* wiv = m_XMLParser.getAttribute(node, "wheel-inc-value");
		if(strlen(wiv) > 0)
			pOM->setWheelInc(::atof(wiv));

		// --- style shit
		const char_t* sty1 = m_XMLParser.getAttribute(node, "style-3D-in");
		if(strlen(sty1) > 0)
		{
			if(strcmp(sty1, "true") == 0)
				pOM->setStyle(pOM->getStyle() | k3DIn);
			else
				pOM->setStyle(pOM->getStyle() & ~k3DIn);
		}

		const char_t* sty2 = m_XMLParser.getAttribute(node, "style-3D-out");
		if(strlen(sty2) > 0)
		{
			if(strcmp(sty2, "true") == 0)
				pOM->setStyle(pOM->getStyle() | k3DOut);
			else
				pOM->setStyle(pOM->getStyle() & ~k3DOut);
		}

		const char_t* sty3 = m_XMLParser.getAttribute(node, "style-no-draw");
		if(strlen(sty3) > 0)
		{
			if(strcmp(sty3, "true") == 0)
				pOM->setStyle(pOM->getStyle() | kNoDrawStyle);
			else
				pOM->setStyle(pOM->getStyle() & ~kNoDrawStyle);
		}

		const char_t* sty4 = m_XMLParser.getAttribute(node, "style-no-text");
		if(strlen(sty4) > 0)
		{
			if(strcmp(sty4, "true") == 0)
				pOM->setStyle(pOM->getStyle() | kNoTextStyle);
			else
				pOM->setStyle(pOM->getStyle() & ~kNoTextStyle);
		}

		const char_t* sty5 = m_XMLParser.getAttribute(node, "style-round-rect");
		if(strlen(sty5) > 0)
		{
			if(strcmp(sty5, "true") == 0)
				pOM->setStyle(pOM->getStyle() | kRoundRectStyle);
			else
				pOM->setStyle(pOM->getStyle() & ~kRoundRectStyle);
		}

		const char_t* sty6 = m_XMLParser.getAttribute(node, "style-shadow-text");
		if(strlen(sty6) > 0)
		{
			if(strcmp(sty6, "true") == 0)
				pOM->setStyle(pOM->getStyle() | kShadowText);
			else
				pOM->setStyle(pOM->getStyle() & ~kShadowText);
		}

		const char_t* sty7 = m_XMLParser.getAttribute(node, "style-no-frame");
		if(strlen(sty7) > 0)
		{
			if(strcmp(sty7, "true") == 0)
				pOM->setStyle(pOM->getStyle() | kNoFrame);
			else
				pOM->setStyle(pOM->getStyle() & ~kNoFrame);
		}

		const char_t* tit = m_XMLParser.getAttribute(node, "title");
		if(strlen(tit) > 0)
		{
			pOM->addEntry(tit, 0);
			pOM->setCurrent(0);
		}


		// --- CView
		const char_t* transp = m_XMLParser.getAttribute(node, "transparent");
		if(strlen(transp) > 0)
		{
			if(strcmp(transp, "true") == 0)
				pOM->setTransparency(true);
			else
				pOM->setTransparency(false);
		}

		const char_t* mouseE = m_XMLParser.getAttribute(node, "mouse-enabled");
		if(strlen(mouseE) > 0)
		{
			if(strcmp(mouseE, "true") == 0)
				pOM->setMouseEnabled(true);
			else
				pOM->setMouseEnabled(false);
		}
 		// --- stash the XML Node on the CView object
		pOM->setAttribute(RAFX_XML_NODE, sizeof(pugi::xml_node*), (void*)&node);
    }

    if(bAddingNew)
    {
        // --- save control; requires destroy/create
        COptionMenu** temp = new COptionMenu*[m_nOptionMenuCount + 1];
        for(int i=0; i<m_nOptionMenuCount; i++)
            temp[i] = m_ppOptionMenuControls[i];

        // --- add new one
        temp[m_nOptionMenuCount] = pOM;
        m_nOptionMenuCount++;

        if(m_ppOptionMenuControls)
            delete m_ppOptionMenuControls;
        m_ppOptionMenuControls = temp;
    }

	return pOM;
}

CKickButton* CRafxVSTEditor::createKickButton(pugi::xml_node node, CBitmap* backgroundBitmap, bool bAddingNew, bool bStandAlone)
{
  	// --- tag
	const char_t* ctagName = m_XMLParser.getAttribute(node, "control-tag");

	// --- zero indexed number
	const char_t* ctag = m_XMLParser.getControlTagAttribute(ctagName, "tag");
	int32_t tag = -1;
	if(strlen(ctag) > 0)
		tag = atoi(ctag);

	const char_t* htOneImage = m_XMLParser.getAttribute(node, "height-of-one-image");
//	CCoord htImage = ::atof(htOneImage);

	const char_t* backoffset = m_XMLParser.getAttribute(node, "background-offset");
	const CPoint point = getCPointFromString(backoffset);

	CKickButton* pButt = NULL;
	CView* pView = checkCreateCustomView(node);

	if(pView)
		pButt = (VSTGUI::CKickButton*)pView;
	else
	{
		pButt = new CKickButtonWP(getRectFromNode(node), this, tag, backgroundBitmap, point);

        const char_t* cvn = m_XMLParser.getAttribute(node, "custom-view-name");
		if(strlen(cvn) > 0)
		{
			if(strcmp(cvn, "RafxKickButtonDU") == 0)
				((CKickButtonWP*)pButt)->setMouseMode(mouseUpAndDown);
			else if(strcmp(cvn, "RafxKickButtonD") == 0)
				((CKickButtonWP*)pButt)->setMouseMode(mouseDown);
			else if(strcmp(cvn, "RafxKickButtonU") == 0)
				((CKickButtonWP*)pButt)->setMouseMode(mouseUp);
			else
				((CKickButtonWP*)pButt)->setMouseMode(mouseUp); // old
		}
		else
			((CKickButtonWP*)pButt)->setMouseMode(mouseUp); // old

		const char_t* subPix = m_XMLParser.getAttribute(node, "sub-pixmaps");
		if(strlen(subPix) > 0)
			pButt->setNumSubPixmaps(atoi(subPix));

		// --- CControl
		const char_t* minValue = m_XMLParser.getAttribute(node, "min-value");
		if(strlen(minValue) > 0)
			pButt->setMin(::atof(minValue));

		const char_t* maxValue = m_XMLParser.getAttribute(node, "max-value");
		if(strlen(maxValue) > 0)
			pButt->setMax(::atof(maxValue));

		const char_t* defValue = m_XMLParser.getAttribute(node, "default-value");
		if(strlen(defValue) > 0)
			pButt->setDefaultValue(::atof(defValue));

		const char_t* wiv = m_XMLParser.getAttribute(node, "wheel-inc-value");
		if(strlen(wiv) > 0)
			pButt->setWheelInc(::atof(wiv));

		// --- CView
		const char_t* transp = m_XMLParser.getAttribute(node, "transparent");
		if(strlen(transp) > 0)
		{
			if(strcmp(transp, "true") == 0)
				pButt->setTransparency(true);
			else
				pButt->setTransparency(false);
		}

		const char_t* mouseE = m_XMLParser.getAttribute(node, "mouse-enabled");
		if(strlen(mouseE) > 0)
		{
			if(strcmp(mouseE, "true") == 0)
				pButt->setMouseEnabled(true);
			else
				pButt->setMouseEnabled(false);
		}
		// --- stash the XML Node on the CView object
		pButt->setAttribute(RAFX_XML_NODE, sizeof(pugi::xml_node*), (void*)&node);
	}

    if(bAddingNew)
    {
        // --- save control; requires destroy/create
        CKickButton** temp = new CKickButton*[m_nKickButtonCount + 1];
        for(int i=0; i<m_nKickButtonCount; i++)
            temp[i] = m_ppKickButtonControls[i];

        // --- add new one
        temp[m_nKickButtonCount] = pButt;
        m_nKickButtonCount++;

        if(m_ppKickButtonControls)
            delete m_ppKickButtonControls;
        m_ppKickButtonControls = temp;
    }

	// --- this is needed for standalone controls to show bitmaps, sometimes not sure why
	const char_t* sizee = m_XMLParser.getAttribute(node, "size");
	updateViewSize(pButt,sizee);

	return pButt;
}

CVuMeter* CRafxVSTEditor::createMeter(pugi::xml_node node, CBitmap* onBitmap, CBitmap* offBitmap, bool bAddingNew, bool bStandAlone)
{
	const char_t* numLED = m_XMLParser.getAttribute(node, "num-led");

	int32_t nbLed = -1;
	if(strlen(numLED) > 0)
		nbLed = atoi(numLED);

	const char_t* orient = m_XMLParser.getAttribute(node, "orientation");
	const char_t* custView = m_XMLParser.getAttribute(node, "custom-view-name");
	bool bInverted  = false;
	bool bAnalogVU  = false;
	float fHtOneImage = 0.0;
	float fZeroDBFrame = 0.0;

	// --- analog VU meters use CAnimKnob attributes
	//     embedded in the custom view name
	if(strlen(custView) > 0)
	{
		string sCustView(custView);
		string sInvertedMeter("InvertedMeterView");
		string sInvertedAnalogMeter("InvertedAnalogMeterView");
		string sAnalogMeter("AnalogMeterView");

		int n = sCustView.find(sInvertedMeter);
		int m = sCustView.find(sInvertedAnalogMeter);
		int b = sCustView.find(sAnalogMeter);

		// --- sInvertedMeter
		if(n >= 0)
		{
			bInverted = true;
			bAnalogVU = false;
		}
		else if(m >= 0) // sInvertedAnalogMeter
		{
			bInverted = true;
			bAnalogVU = true;

			// --- decode hieght one image and zero db frame
			int nX = sCustView.find("_H");
			int nY = sCustView.find("_Z");
			int len = sCustView.length();
			string sH = sCustView.substr(nX + 2, nY - 2 - nX);
			string sZ = sCustView.substr(nY + 2, len - 2 - nY);
			const char* hoi = sH.c_str();
			const char* zdbf = sZ.c_str();

			fHtOneImage = atof(hoi);
			fZeroDBFrame = atof(zdbf);
		}
		else if(b >= 0) // sAnalogMeter
		{
			bInverted = false;
			bAnalogVU = true;

			// --- decode hieght one image and zero db frame
			int nX = sCustView.find("_H");
			int nY = sCustView.find("_Z");
			int len = sCustView.length();
			string sH = sCustView.substr(nX + 2, nY - 2 - nX);
			string sZ = sCustView.substr(nY + 2, len - 2 - nY);

			const char* hoi = sH.c_str();
			const char* zdbf = sZ.c_str();

			fHtOneImage = atof(hoi);
			fZeroDBFrame = atof(zdbf);
		}
	}

	CVuMeter* pMeter = NULL;
	CView* pView = checkCreateCustomView(node);
	if(pView)
		pMeter = (CVuMeter*)pView;
	else
	{
		if(strcmp(orient, "vertical") == 0)
		{
			pMeter = new CVuMeterWP(getRectFromNode(node), onBitmap, offBitmap, nbLed, bInverted, bAnalogVU, kVertical);
		}
		else
		{
			pMeter = new CVuMeterWP(getRectFromNode(node), onBitmap, offBitmap, nbLed, bInverted, bAnalogVU, kHorizontal);
		}

		if(bAnalogVU)
		{
			if(isCVuMeterWP(pMeter))
			{
				((CVuMeterWP*)pMeter)->setImageCount(atof(numLED));
				((CVuMeterWP*)pMeter)->setHtOneImage(fHtOneImage);
				((CVuMeterWP*)pMeter)->setZero_dB_Frame(fZeroDBFrame);
			}
		}

		// --- CControl
		const char_t* minValue = m_XMLParser.getAttribute(node, "min-value");
		if(strlen(minValue) > 0)
			pMeter->setMin(::atof(minValue));

		const char_t* maxValue = m_XMLParser.getAttribute(node, "max-value");
		if(strlen(maxValue) > 0)
			pMeter->setMax(::atof(maxValue));

		const char_t* defValue = m_XMLParser.getAttribute(node, "default-value");
		if(strlen(defValue) > 0)
			pMeter->setDefaultValue(::atof(defValue));

		const char_t* backoffset = m_XMLParser.getAttribute(node, "background-offset");
		if(strlen(backoffset) > 0)
		{
			const CPoint point = getCPointFromString(backoffset);
			pMeter->setBackOffset(point);
		}

		// --- tag
		const char_t* ctagName = m_XMLParser.getAttribute(node, "control-tag");
		if(strlen(ctagName) > 0)
		{
			// --- zero indexed number
			const char_t* ctag = m_XMLParser.getControlTagAttribute(ctagName, "tag");
			int32_t tag = -1;
			if(strlen(ctag) > 0)
				tag = atoi(ctag);

			pMeter->setTag(tag);
		}

		const char_t* wiv = m_XMLParser.getAttribute(node, "wheel-inc-value");
		if(strlen(wiv) > 0)
			pMeter->setWheelInc(::atof(wiv));

		// --- CView
		const char_t* transp = m_XMLParser.getAttribute(node, "transparent");
		if(strlen(transp) > 0)
		{
			if(strcmp(transp, "true") == 0)
				pMeter->setTransparency(true);
			else
				pMeter->setTransparency(false);
		}

		const char_t* mouseE = m_XMLParser.getAttribute(node, "mouse-enabled");
		if(strlen(mouseE) > 0)
		{
			if(strcmp(mouseE, "true") == 0)
				pMeter->setMouseEnabled(true);
			else
				pMeter->setMouseEnabled(false);
		}

		const char_t* stepVal = m_XMLParser.getAttribute(node, "decrease-step-value");
		if(strlen(stepVal) > 0)
			pMeter->setDecreaseStepValue(atof(stepVal));

        // --- stash the XML Node on the CView object
		pMeter->setAttribute(RAFX_XML_NODE, sizeof(pugi::xml_node*), (void*)&node);
    }

    if(bAddingNew)
    {
        // --- save control; requires destroy/create
        CVuMeter** temp = new CVuMeter*[m_nVuMeterCount + 1];
        for(int i=0; i<m_nVuMeterCount; i++)
            temp[i] = m_ppVuMeterControls[i];

        // --- add new one
        temp[m_nVuMeterCount] = pMeter;
        m_nVuMeterCount++;

        if(m_ppVuMeterControls)
            delete m_ppVuMeterControls;
        m_ppVuMeterControls = temp;
    }

	return pMeter;
}

COnOffButton* CRafxVSTEditor::createOnOffButton(pugi::xml_node node, CBitmap* bitmap, bool bAddingNew, bool bStandAlone)
{
 	// --- tag
	const char_t* ctagName = m_XMLParser.getAttribute(node, "control-tag");

	// --- zero indexed number
	const char_t* ctag = m_XMLParser.getControlTagAttribute(ctagName, "tag");
	int32_t tag = -1;
	if(strlen(ctag) > 0)
		tag = atoi(ctag);

	COnOffButton* pButt = NULL;
	CView* pView = checkCreateCustomView(node);

	if(pView)
		pButt = (COnOffButton*)pView;
	else
	{
		pButt = new COnOffButton(getRectFromNode(node), this, tag, bitmap);

		const char_t* backoffset = m_XMLParser.getAttribute(node, "background-offset");
		if(strlen(backoffset) > 0)
		{
			const CPoint point = getCPointFromString(backoffset);
			pButt->setBackOffset(point);
		}

		// --- CControl
		const char_t* minValue = m_XMLParser.getAttribute(node, "min-value");
		if(strlen(minValue) > 0)
			pButt->setMin(::atof(minValue));

		const char_t* maxValue = m_XMLParser.getAttribute(node, "max-value");
		if(strlen(maxValue) > 0)
			pButt->setMax(::atof(maxValue));

		const char_t* defValue = m_XMLParser.getAttribute(node, "default-value");
		if(strlen(defValue) > 0)
			pButt->setDefaultValue(::atof(defValue));

		const char_t* wiv = m_XMLParser.getAttribute(node, "wheel-inc-value");
		if(strlen(wiv) > 0)
			pButt->setWheelInc(::atof(wiv));

		// --- CView
		const char_t* transp = m_XMLParser.getAttribute(node, "transparent");
		if(strlen(transp) > 0)
		{
			if(strcmp(transp, "true") == 0)
				pButt->setTransparency(true);
			else
				pButt->setTransparency(false);
		}

		const char_t* mouseE = m_XMLParser.getAttribute(node, "mouse-enabled");
		if(strlen(mouseE) > 0)
		{
			if(strcmp(mouseE, "true") == 0)
				pButt->setMouseEnabled(true);
			else
				pButt->setMouseEnabled(false);
		}

		// --- stash the XML Node on the CView object
		pButt->setAttribute(RAFX_XML_NODE, sizeof(pugi::xml_node*), (void*)&node);
	}

    if(bAddingNew)
    {
        // --- save control; requires destroy/create
        COnOffButton** temp = new COnOffButton*[m_nOnOffButtonCount + 1];
        for(int i=0; i<m_nOnOffButtonCount; i++)
            temp[i] = m_ppOnOffButtonControls[i];

        // --- add new one
        temp[m_nOnOffButtonCount] = pButt;
        m_nOnOffButtonCount++;

        if(m_ppOnOffButtonControls)
            delete m_ppOnOffButtonControls;
        m_ppOnOffButtonControls = temp;
    }

	// --- this is needed for standalone controls to show bitmaps, sometimes not sure why
	const char_t* sizee = m_XMLParser.getAttribute(node, "size");
	updateViewSize(pButt,sizee);

	return pButt;
}

CHorizontalSwitch* CRafxVSTEditor::createHorizontalSwitch(pugi::xml_node node, CBitmap* backgroundBitmap, bool bAddingNew)
{
    // --- tag
    const char_t* ctagName = m_XMLParser.getAttribute(node, "control-tag");

    // --- zero indexed number
    const char_t* ctag = m_XMLParser.getControlTagAttribute(ctagName, "tag");
    int32_t tag = -1;
    if(strlen(ctag) > 0)
        tag = atoi(ctag);

    const char_t* backoffset = m_XMLParser.getAttribute(node, "background-offset");
    const CPoint point = getCPointFromString(backoffset);

    const char_t* subpix = m_XMLParser.getAttribute(node, "sub-pixmaps");
    int32_t subPixmaps = 0;
    if(strlen(subpix) > 0)
        subPixmaps = atoi(subpix);

    const char_t* htOneImage = m_XMLParser.getAttribute(node, "height-of-one-image");
    CCoord htImage = ::atof(htOneImage);

    CHorizontalSwitch* pSwitch = NULL;
    CView* pView = checkCreateCustomView(node);

    if(pView)
        pSwitch = (CHorizontalSwitch*)pView;
    else
    {
        pSwitch = new CHorizontalSwitch(getRectFromNode(node), this, tag, subPixmaps, htImage, 1, backgroundBitmap, point);

        // NOTE: iMaxPositions is not supported on VSTGUI end of things, def to 1

        // --- CControl
        const char_t* minValue = m_XMLParser.getAttribute(node, "min-value");
        if(strlen(minValue) > 0)
            pSwitch->setMin(::atof(minValue));

        const char_t* maxValue = m_XMLParser.getAttribute(node, "max-value");
        if(strlen(maxValue) > 0)
            pSwitch->setMax(::atof(maxValue));

        const char_t* defValue = m_XMLParser.getAttribute(node, "default-value");
        if(strlen(defValue) > 0)
            pSwitch->setDefaultValue(::atof(defValue));

        const char_t* wiv = m_XMLParser.getAttribute(node, "wheel-inc-value");
        if(strlen(wiv) > 0)
            pSwitch->setWheelInc(::atof(wiv));

        // --- CView
        const char_t* transp = m_XMLParser.getAttribute(node, "transparent");
        if(strlen(transp) > 0)
        {
            if(strcmp(transp, "true") == 0)
                pSwitch->setTransparency(true);
            else
                pSwitch->setTransparency(false);
        }

        const char_t* mouseE = m_XMLParser.getAttribute(node, "mouse-enabled");
        if(strlen(mouseE) > 0)
        {
            if(strcmp(mouseE, "true") == 0)
                pSwitch->setMouseEnabled(true);
            else
                pSwitch->setMouseEnabled(false);
        }

		// --- stash the XML Node on the CView object
		pSwitch->setAttribute(RAFX_XML_NODE, sizeof(pugi::xml_node*), (void*)&node);
	}

    if(bAddingNew)
    {
        // --- save control; requires destroy/create
        CHorizontalSwitch** temp = new CHorizontalSwitch*[m_nHorizontalSwitchCount + 1];
        for(int i=0; i<m_nHorizontalSwitchCount; i++)
            temp[i] = m_ppHorizontalSwitchControls[i];

        // --- add new one
        temp[m_nHorizontalSwitchCount] = pSwitch;
        m_nHorizontalSwitchCount++;

        if(m_ppHorizontalSwitchControls)
            delete m_ppHorizontalSwitchControls;
        m_ppHorizontalSwitchControls = temp;
    }

    return pSwitch;
}

CVerticalSwitch* CRafxVSTEditor::createVerticalSwitch(pugi::xml_node node, CBitmap* backgroundBitmap, bool bAddingNew, bool bStandAlone)
{
 	// --- tag
	const char_t* ctagName = m_XMLParser.getAttribute(node, "control-tag");

	// --- zero indexed number
	const char_t* ctag = m_XMLParser.getControlTagAttribute(ctagName, "tag");
	int32_t tag = -1;
	if(strlen(ctag) > 0)
		tag = atoi(ctag);

	const char_t* backoffset = m_XMLParser.getAttribute(node, "background-offset");
	const CPoint point = getCPointFromString(backoffset);

	const char_t* subpix = m_XMLParser.getAttribute(node, "sub-pixmaps");
	int32_t subPixmaps = 0;
	if(strlen(subpix) > 0)
		subPixmaps = atoi(subpix);

	const char_t* htOneImage = m_XMLParser.getAttribute(node, "height-of-one-image");
	CCoord htImage = ::atof(htOneImage);

	CVerticalSwitch* pSwitch = NULL;
	CView* pView = checkCreateCustomView(node);

	if(pView)
		pSwitch = (CVerticalSwitch*)pView;
	else
	{
		pSwitch = new CVerticalSwitch(getRectFromNode(node), this, tag, subPixmaps, htImage, 1, backgroundBitmap, point);

		// NOTE: iMaxPositions is not supported on VSTGUI end of things, def to 1
		// CVerticalSwitch* pSwitch = new CVerticalSwitch(getRectFromNode(node), this, tag, subPixmaps, htImage, 1, backgroundBitmap, point);

		// --- CControl
		const char_t* minValue = m_XMLParser.getAttribute(node, "min-value");
		if(strlen(minValue) > 0)
			pSwitch->setMin(::atof(minValue));

		const char_t* maxValue = m_XMLParser.getAttribute(node, "max-value");
		if(strlen(maxValue) > 0)
			pSwitch->setMax(::atof(maxValue));

		const char_t* defValue = m_XMLParser.getAttribute(node, "default-value");
		if(strlen(defValue) > 0)
			pSwitch->setDefaultValue(::atof(defValue));

		const char_t* wiv = m_XMLParser.getAttribute(node, "wheel-inc-value");
		if(strlen(wiv) > 0)
			pSwitch->setWheelInc(::atof(wiv));

		// --- CView
		const char_t* transp = m_XMLParser.getAttribute(node, "transparent");
		if(strlen(transp) > 0)
		{
			if(strcmp(transp, "true") == 0)
				pSwitch->setTransparency(true);
			else
				pSwitch->setTransparency(false);
		}

		const char_t* mouseE = m_XMLParser.getAttribute(node, "mouse-enabled");
		if(strlen(mouseE) > 0)
		{
			if(strcmp(mouseE, "true") == 0)
				pSwitch->setMouseEnabled(true);
			else
				pSwitch->setMouseEnabled(false);
		}

        // --- stash the XML Node on the CView object
		pSwitch->setAttribute(RAFX_XML_NODE, sizeof(pugi::xml_node*), (void*)&node);
	}

    if(bAddingNew)
    {
        // --- save control; requires destroy/create
        CVerticalSwitch** temp = new CVerticalSwitch*[m_nVerticalSwitchCount + 1];
        for(int i=0; i<m_nVerticalSwitchCount; i++)
            temp[i] = m_ppVerticalSwitchControls[i];

        // --- add new one
        temp[m_nVerticalSwitchCount] = pSwitch;
        m_nVerticalSwitchCount++;

        if(m_ppVerticalSwitchControls)
            delete m_ppVerticalSwitchControls;
        m_ppVerticalSwitchControls = temp;
    }

	return pSwitch;
}

CSegmentButton* CRafxVSTEditor::createSegmentButton(pugi::xml_node node, bool bAddingNew)
{
    // --- tag
    const char_t* ctagName = m_XMLParser.getAttribute(node, "control-tag");

    // --- zero indexed number
    const char_t* ctag = m_XMLParser.getControlTagAttribute(ctagName, "tag");
    int32_t tag = -1;
    if(strlen(ctag) > 0)
        tag = atoi(ctag);

    const char_t* backoffset = m_XMLParser.getAttribute(node, "background-offset");
    const CPoint point = getCPointFromString(backoffset);

    CSegmentButton* pSwitch = NULL;
    CView* pView = checkCreateCustomView(node);

    if(pView)
        pSwitch = (CSegmentButton*)pView;
    else
    {
        pSwitch = new CSegmentButton(getRectFromNode(node), this, tag);

        // --- CSegmentButton
        // --- font stuff
        CFontDesc* fontDesc = createFontDesc(node);
        fontDesc->getPlatformFont();

        pSwitch->setFont(fontDesc);

        const char_t* fcolor = m_XMLParser.getAttribute(node, "text-color");
        if(strlen(fcolor) > 0)
        {
            CColor fCColor = getCColor(fcolor);
            pSwitch->setTextColor(fCColor);
        }

        const char_t* fcolor2 = m_XMLParser.getAttribute(node, "text-color-highlighted");
        if(strlen(fcolor2) > 0)
        {
            CColor fCColor = getCColor(fcolor2);
            pSwitch->setTextColorHighlighted(fCColor);
        }

        const char_t* frmcolor = m_XMLParser.getAttribute(node, "frame-color");
        if(strlen(frmcolor) > 0)
        {
            CColor frmCColor = getCColor(frmcolor);
            pSwitch->setFrameColor(frmCColor);
        }

        const char_t* frmwidth = m_XMLParser.getAttribute(node, "frame-width");
        if(strlen(frmwidth) > 0)
        {
            const CCoord frmWidth = ::atof(frmwidth);
            pSwitch->setFrameWidth(frmWidth);
        }

        const char_t* rrr = m_XMLParser.getAttribute(node, "round-radius");
        if(strlen(rrr) > 0)
        {
            const CCoord rrradius = ::atof(rrr);
            pSwitch->setRoundRadius(rrradius);
        }

        const char_t* align = m_XMLParser.getAttribute(node, "text-alignment");
        if(strlen(align) > 0)
        {
            if(strcmp(align, "left") == 0)
                pSwitch->setTextAlignment(kLeftText);
            else if(strcmp(align, "center") == 0)
                pSwitch->setTextAlignment(kCenterText);
            else if(strcmp(align, "right") == 0)
                pSwitch->setTextAlignment(kRightText);
        }

        const char_t* sty2 = m_XMLParser.getAttribute(node, "style");
        if(strlen(sty2) > 0)
        {
            if(strcmp(sty2, "horizontal") == 0)
                pSwitch->setStyle(CSegmentButton::kHorizontal);
            else
                pSwitch->setStyle(CSegmentButton::kVertical);
        }

        const char_t* gradientNormal = m_XMLParser.getAttribute(node, "gradient");
        const char_t* gradientHighlight = m_XMLParser.getAttribute(node, "gradient-highlighted");
        if(strlen(gradientNormal) > 0 && strlen(gradientHighlight) > 0)
        {
            // --- gradients
            const char_t* startRGBA = m_XMLParser.getGradientStartRGBA(gradientNormal);
            const char_t* stopRGBA = m_XMLParser.getGradientStopRGBA(gradientNormal);
            const char_t* startRGBA_HL = m_XMLParser.getGradientStartRGBA(gradientHighlight);
            const char_t* stopRGBA_HL = m_XMLParser.getGradientStopRGBA(gradientHighlight);
            if(strlen(startRGBA) > 0 && strlen(stopRGBA) > 0 && strlen(startRGBA_HL) > 0 && strlen(stopRGBA_HL) > 0)
            {
                CColor startColor = getCColorFromRGBA(startRGBA);
                CColor stopColor = getCColorFromRGBA(stopRGBA);
                CGradient* pG1 = CGradient::create(0.0, 1.0, startColor, stopColor);

                CColor startColor_HL = getCColorFromRGBA(startRGBA_HL);
                CColor stopColor_HL = getCColorFromRGBA(stopRGBA_HL);
                CGradient* pG2 = CGradient::create(0.0, 1.0, startColor_HL, stopColor_HL);

                pSwitch->setGradient(pG1);
                pSwitch->setGradientHighlighted(pG2);
                if(pG1) pG1->forget();
                if(pG2) pG2->forget();
            }
        }

        // --- CControl
        const char_t* minValue = m_XMLParser.getAttribute(node, "min-value");
        if(strlen(minValue) > 0)
            pSwitch->setMin(::atof(minValue));

        const char_t* maxValue = m_XMLParser.getAttribute(node, "max-value");
        if(strlen(maxValue) > 0)
            pSwitch->setMax(::atof(maxValue));

        const char_t* defValue = m_XMLParser.getAttribute(node, "default-value");
        if(strlen(defValue) > 0)
            pSwitch->setDefaultValue(::atof(defValue));

        const char_t* wiv = m_XMLParser.getAttribute(node, "wheel-inc-value");
        if(strlen(wiv) > 0)
            pSwitch->setWheelInc(::atof(wiv));

        // --- segments
        const char_t* segments = m_XMLParser.getAttribute(node, "segment-names");
        if(strlen(segments) > 0)
        {
            for(int i=0; i <= atoi(maxValue); i++)
            {
                char* segName = getEnumString((char*)segments, i);
                if(segName)
                {
                    if(strlen(segName) > 0)
                    {
                        CSegmentButton::Segment seg;
                        seg.name = segName;
                        pSwitch->addSegment(seg);
                    }
                }
            }
        }

        // --- CView
        const char_t* transp = m_XMLParser.getAttribute(node, "transparent");
        if(strlen(transp) > 0)
        {
            if(strcmp(transp, "true") == 0)
                pSwitch->setTransparency(true);
            else
                pSwitch->setTransparency(false);
        }

        const char_t* mouseE = m_XMLParser.getAttribute(node, "mouse-enabled");
        if(strlen(mouseE) > 0)
        {
            if(strcmp(mouseE, "true") == 0)
                pSwitch->setMouseEnabled(true);
            else
                pSwitch->setMouseEnabled(false);
        }

        // --- stash the XML Node on the CView object
        pSwitch->setAttribute(RAFX_XML_NODE, sizeof(pugi::xml_node*), (void*)&node);
    }

    if(bAddingNew)
    {
        // --- save control; requires destroy/create
        CSegmentButton** temp = new CSegmentButton*[m_nSegmentButtonCount + 1];
        for(int i=0; i<m_nSegmentButtonCount; i++)
            temp[i] = m_ppSegmentButtons[i];

        // --- add new one
        temp[m_nSegmentButtonCount] = pSwitch;
        m_nSegmentButtonCount++;

        if(m_ppSegmentButtons)
            delete m_ppSegmentButtons;
        m_ppSegmentButtons = temp;
    }

    return pSwitch;
}

CSlider* CRafxVSTEditor::createSlider(pugi::xml_node node, CBitmap* handleBitmap, CBitmap* grooveBitmap, bool bAddingNew, bool bStandAlone)
{
  	// --- tag
	const char_t* ctagName = m_XMLParser.getAttribute(node, "control-tag");

	// --- zero indexed number
	const char_t* ctag = m_XMLParser.getControlTagAttribute(ctagName, "tag");
	int32_t tag = -1;
	if(strlen(ctag) > 0)
		tag = atoi(ctag);

	const char_t* minValue = m_XMLParser.getAttribute(node, "min-value");
	const char_t* maxValue = m_XMLParser.getAttribute(node, "max-value");
	const char_t* orient = m_XMLParser.getAttribute(node, "orientation");
	const char_t* backoffset = m_XMLParser.getAttribute(node, "background-offset");
	//const char_t* backoffset = m_XMLParser.getAttribute(node, "handle-offset");
	const CPoint point = getCPointFromString(backoffset);

	const char_t* sizee = m_XMLParser.getAttribute(node, "size");
	string sSizee(sizee);
	int w, nSlotHeight =0;
	getXYFromString(sSizee, w, nSlotHeight);

	CSlider* pSlider = NULL;
	double handleHt = handleBitmap->getHeight();

	const char_t* customView = m_XMLParser.getAttribute(node, "custom-view-name");

	CView* pView = checkCreateCustomView(node);
	if(pView)
		pSlider = (CSlider*)pView;
	else
	{
		if(strcmp(orient, "vertical") == 0)
		{
			pSlider = new CVerticalSliderWP(getRectFromNode(node), this, tag, handleHt, nSlotHeight + 2, handleBitmap, grooveBitmap, point, kBottom);
#ifdef AAXPLUGIN
            ((CVerticalSliderWP*)pSlider)->setAAXSlider(true);
#endif

			if(strcmp(customView, "SliderSwitchView") == 0)
				((CVerticalSliderWP*)pSlider)->setSwitchSlider(true);
		}
		else
		{
			pSlider = new CHorizontalSliderWP(getRectFromNode(node), this, tag, atoi(minValue), atoi(maxValue), handleBitmap, grooveBitmap, point, kLeft);
#ifdef AAXPLUGIN
            ((CHorizontalSliderWP*)pSlider)->setAAXSlider(true);
#endif

			if(strcmp(customView, "SliderSwitchView") == 0)
				((CHorizontalSliderWP*)pSlider)->setSwitchSlider(true);
		}

		// --- CSlider
		const char_t* bitmapOffset = m_XMLParser.getAttribute(node, "bitmap-offset");
		if(strlen(bitmapOffset) > 0)
		{
			const CPoint pointBMO = getCPointFromString(bitmapOffset);
			pSlider->setOffset(pointBMO);
		}

		const char_t* handleOffset = m_XMLParser.getAttribute(node, "handle-offset");
		if(strlen(handleOffset) > 0)
		{
			const CPoint pointHO = getCPointFromString(handleOffset);
			pSlider->setOffsetHandle(pointHO);
		}

		const char_t* transHandle = m_XMLParser.getAttribute(node, "transparent-handle");
		if(strlen(transHandle) > 0)
		{
			if(strcmp(transHandle, "true") == 0)
				pSlider->setDrawTransparentHandle(true);
			else
				pSlider->setDrawTransparentHandle(false);
		}

		const char_t* mode = m_XMLParser.getAttribute(node, "mode");
		if(strlen(mode) > 0)
		{
			if(strcmp(mode, "free click") == 0)
				pSlider->setMode(VSTGUI::CSlider::kFreeClickMode);
			else if(strcmp(mode, "touch") == 0)
				pSlider->setMode(VSTGUI::CSlider::kTouchMode);
			else if(strcmp(mode, "relative touch") == 0)
				pSlider->setMode(VSTGUI::CSlider::kRelativeTouchMode);
		}

		// --- I don't know how to handle this one - can't find it's home
		const char_t* revOrient = m_XMLParser.getAttribute(node, "reverse-orientation");
		// --- add...

		const char_t* frameColor = m_XMLParser.getAttribute(node, "draw-frame-color");
		if(strlen(frameColor) > 0)
		{
			CColor frameCColor = getCColor(frameColor);
			pSlider->setFrameColor(frameCColor);
		}

		const char_t* backColor = m_XMLParser.getAttribute(node, "draw-back-color");
		if(strlen(backColor) > 0)
		{
			CColor backCColor = getCColor(backColor);
			pSlider->setBackColor(backCColor);
		}

		const char_t* valColor = m_XMLParser.getAttribute(node, "draw-value-color");
		if(strlen(valColor) > 0)
		{
			CColor valCColor = getCColor(valColor);
			pSlider->setValueColor(valCColor);
		}

		const char_t* drawBack = m_XMLParser.getAttribute(node, "draw-back");
		if(strlen(drawBack) > 0)
		{
			if(strcmp(drawBack, "true") == 0)
				pSlider->setDrawStyle(pSlider->getDrawStyle() | VSTGUI::CSlider::kDrawBack);
		}

		const char_t* drawFrame = m_XMLParser.getAttribute(node, "draw-frame");
		if(strlen(drawFrame) > 0)
		{
			if(strcmp(drawFrame, "true") == 0)
				pSlider->setDrawStyle(pSlider->getDrawStyle() | VSTGUI::CSlider::kDrawFrame);
		}

		const char_t* drawVal = m_XMLParser.getAttribute(node, "draw-value");
		if(strlen(drawVal) > 0)
		{
			if(strcmp(drawVal, "true") == 0)
				pSlider->setDrawStyle(pSlider->getDrawStyle() | VSTGUI::CSlider::kDrawValue);
		}

		const char_t* drawValCent = m_XMLParser.getAttribute(node, "draw-value-from-center");
		if(strlen(drawValCent) > 0)
		{
			if(strcmp(drawValCent, "true") == 0)
				pSlider->setDrawStyle(pSlider->getDrawStyle() | VSTGUI::CSlider::kDrawValueFromCenter);
		}

		const char_t* drawValInv = m_XMLParser.getAttribute(node, "draw-value-inverted");
		if(strlen(drawValInv) > 0)
		{
			if(strcmp(drawValInv, "true") == 0)
				pSlider->setDrawStyle(pSlider->getDrawStyle() | VSTGUI::CSlider::kDrawInverted);
		}

		const char_t* zf = m_XMLParser.getAttribute(node, "zoom-factor");
		if(strlen(zf) > 0)
			pSlider->setZoomFactor(atof(zf));

		const char_t* wiv = m_XMLParser.getAttribute(node, "wheel-inc-value");
		if(strlen(wiv) > 0)
			pSlider->setWheelInc(::atof(wiv));

		// --- CControl
		const char_t* defValue = m_XMLParser.getAttribute(node, "default-value");
		if(strlen(defValue) > 0)
			pSlider->setDefaultValue(::atof(defValue));

		const char_t* tooltipText = m_XMLParser.getAttribute(node, "tooltip");
		if(strlen(tooltipText) > 0)
			pSlider->setAttribute(kCViewTooltipAttribute, strlen (tooltipText)+1, tooltipText);

		// --- CView
		const char_t* transp = m_XMLParser.getAttribute(node, "transparent");
		if(strlen(transp) > 0)
		{
			if(strcmp(transp, "true") == 0)
				pSlider->setTransparency(true);
			else
				pSlider->setTransparency(false);
		}

		const char_t* mouseE = m_XMLParser.getAttribute(node, "mouse-enabled");
		if(strlen(mouseE) > 0)
		{
			if(strcmp(mouseE, "true") == 0)
				pSlider->setMouseEnabled(true);
			else
				pSlider->setMouseEnabled(false);
		}

        // --- stash the XML Node on the CView object
		pSlider->setAttribute(RAFX_XML_NODE, sizeof(pugi::xml_node*), (void*)&node);
	}

    if(bAddingNew)
    {
        // --- save control; requires destroy/create
        CSlider** temp = new CSlider*[m_nSliderCount + 1];
        for(int i=0; i<m_nSliderCount; i++)
            temp[i] = m_ppSliderControls[i];

        // --- add new one
        temp[m_nSliderCount] = pSlider;
        m_nSliderCount++;

        if(m_ppSliderControls)
            delete m_ppSliderControls;
        m_ppSliderControls = temp;
    }

	// --- this is needed for standalone controls to show handle, not sure why
	updateViewSize(pSlider,sizee);

	return pSlider;
}

CAnimKnob* CRafxVSTEditor::createAnimKnob(pugi::xml_node node, CBitmap* bitmap, bool bAddingNew, bool bStandAlone)
{
 	// --- tag
	const char_t* ctagName = m_XMLParser.getAttribute(node, "control-tag");

	// --- zero indexed number
	const char_t* ctag = m_XMLParser.getControlTagAttribute(ctagName, "tag");
	int32_t tag = -1;
	if(strlen(ctag) > 0)
		tag = atoi(ctag);

	const char_t* subpix = m_XMLParser.getAttribute(node, "sub-pixmaps");
	int32_t subPixmaps = 0;
	if(strlen(subpix) > 0)
		subPixmaps = atoi(subpix);

	const char_t* htOneImage = m_XMLParser.getAttribute(node, "height-of-one-image");
	CCoord htImage = ::atof(htOneImage);

	const char_t* backoffset = m_XMLParser.getAttribute(node, "background-offset");
	const CPoint point = getCPointFromString(backoffset);

	const char_t* customView = m_XMLParser.getAttribute(node, "custom-view-name");
	bool bSwitch = false;

	if(strcmp(customView, "KnobSwitchView") == 0)
		bSwitch = true;

	CAnimKnob* pKnob = NULL;
	CView* pView = checkCreateCustomView(node);

	if(pView)
	{
		pKnob = (VSTGUI::CAnimKnob*)pView;
	}
	else
	{
		pKnob = new CKnobWP(getRectFromNode(node), this, tag, subPixmaps, htImage, bitmap, point, bSwitch);

#ifdef AAXPLUGIN
        ((CKnobWP*)pKnob)->setAAXKnob(true);
#endif
        
		// --- CKnob
		const char_t* arange = m_XMLParser.getAttribute(node, "angle-range");
		if(strlen(arange) > 0)
			pKnob->setRangeAngle(k2PI*atof(arange)/360.0);

		const char_t* astart = m_XMLParser.getAttribute(node, "angle-start");
		if(strlen(astart) > 0)
			pKnob->setStartAngle(k2PI*atof(astart)/360.0);

		const char_t* zf = m_XMLParser.getAttribute(node, "zoom-factor");
		if(strlen(zf) > 0)
			pKnob->setZoomFactor(atof(zf));

		const char_t* inset = m_XMLParser.getAttribute(node, "value-inset");
		if(strlen(inset) > 0)
			pKnob->setInsetValue(::atof(inset));

		const char_t* coronaColor = m_XMLParser.getAttribute(node, "corona-color");
		if(strlen(coronaColor) > 0)
		{
			CColor coronaCColor = getCColor(coronaColor);
			pKnob->setCoronaColor(coronaCColor);
		}

		const char_t* cdd = m_XMLParser.getAttribute(node, "corona-dash-dot");
		if(strlen(cdd) > 0)
		{
			if(strcmp(cdd, "true") == 0)
				pKnob->setDrawStyle(pKnob->getDrawStyle() | VSTGUI::CKnob::kCoronaLineDashDot);
		}

		const char_t* cdraw = m_XMLParser.getAttribute(node, "corona-drawing");
		if(strlen(cdraw) > 0)
		{
			if(strcmp(cdraw, "true") == 0)
				pKnob->setDrawStyle(pKnob->getDrawStyle() | VSTGUI::CKnob::kCoronaDrawing);
		}

		const char_t* cfc = m_XMLParser.getAttribute(node, "corona-from-center");
		if(strlen(cdraw) > 0)
		{
			if(strcmp(cfc, "true") == 0)
				pKnob->setDrawStyle(pKnob->getDrawStyle() | VSTGUI::CKnob::kCoronaFromCenter);
		}

		const char_t* cinv = m_XMLParser.getAttribute(node, "corona-inverted");
		if(strlen(cinv) > 0)
		{
			if(strcmp(cinv, "true") == 0)
				pKnob->setDrawStyle(pKnob->getDrawStyle() | VSTGUI::CKnob::kCoronaInverted);
		}

		const char_t* coutline = m_XMLParser.getAttribute(node, "corona-outline");
		if(strlen(coutline) > 0)
		{
			if(strcmp(coutline, "true") == 0)
				pKnob->setDrawStyle(pKnob->getDrawStyle() | VSTGUI::CKnob::kCoronaOutline);
		}

		const char_t* cdrawing = m_XMLParser.getAttribute(node, "circle-drawing");
		if(strlen(cdrawing) > 0)
		{
			if(strcmp(cdrawing, "true") == 0)
				pKnob->setDrawStyle(pKnob->getDrawStyle() | VSTGUI::CKnob::kCoronaDrawing);
		}

		const char_t* corInset = m_XMLParser.getAttribute(node, "corona-inset");
		if(strlen(corInset) > 0)
			pKnob->setCoronaInset(::atof(corInset));


		// --- Knobs always have this min/max; for RAFX switch knobs, the derived class handles non-normalized values
		pKnob->setMin(0.0);
		pKnob->setMax(1.0);

		const char_t* defValue = m_XMLParser.getAttribute(node, "default-value");
		if(strlen(defValue) > 0)
			pKnob->setDefaultValue(::atof(defValue));

		const char_t* Hcolor = m_XMLParser.getAttribute(node, "handle-color");
		if(strlen(Hcolor) > 0)
		{
			CColor HCColor = getCColor(Hcolor);
			pKnob->setColorHandle(HCColor);
		}

		const char_t* HScolor = m_XMLParser.getAttribute(node, "handle-shadow-color");
		if(strlen(HScolor) > 0)
		{
			CColor HSCColor = getCColor(HScolor);
			pKnob->setColorShadowHandle(HSCColor);
		}

		const char_t* handlWid = m_XMLParser.getAttribute(node, "handle-line-width");
		if(strlen(handlWid) > 0)
		{
			CCoord handlWidCoord = ::atof(handlWid);
			pKnob->setHandleLineWidth(handlWidCoord);
		}

		const char_t* wiv = m_XMLParser.getAttribute(node, "wheel-inc-value");
		if(strlen(wiv) > 0)
			pKnob->setWheelInc(::atof(wiv));

		const char_t* tooltipText = m_XMLParser.getAttribute(node, "tooltip");
		if(strlen(tooltipText) > 0)
			pKnob->setAttribute(kCViewTooltipAttribute, strlen (tooltipText)+1, tooltipText);

		// --- CView
		const char_t* transp = m_XMLParser.getAttribute(node, "transparent");
		if(strlen(transp) > 0)
		{
			if(strcmp(transp, "true") == 0)
				pKnob->setTransparency(true);
			else
				pKnob->setTransparency(false);
		}

		const char_t* mouseE = m_XMLParser.getAttribute(node, "mouse-enabled");
		if(strlen(mouseE) > 0)
		{
			if(strcmp(mouseE, "true") == 0)
				pKnob->setMouseEnabled(true);
			else
				pKnob->setMouseEnabled(false);
		}
		// --- stash the XML Node on the CView object
		pKnob->setAttribute(RAFX_XML_NODE, sizeof(pugi::xml_node*), (void*)&node);
	}

    if(bAddingNew)
    {
        // --- save control; requires destroy/create
        CAnimKnob** temp = new CAnimKnob*[m_nAnimKnobCount + 1];
        for(int i=0; i<m_nAnimKnobCount; i++)
            temp[i] = m_ppKnobControls[i];

        // --- add new one
        temp[m_nAnimKnobCount] = pKnob;
        m_nAnimKnobCount++;

        if(m_ppKnobControls)
            delete m_ppKnobControls;
        m_ppKnobControls = temp;
    }

	return pKnob;
}


void CRafxVSTEditor::initPluginCustomGUI()
{
    if(!frame) return;
    if(!m_pPlugIn) return;

    // --- fill in the struct
    guiInfoStruct.message = GUI_DID_OPEN;
    guiInfoStruct.customViewName = "";
    guiInfoStruct.subControllerName = "";
    guiInfoStruct.editor = (void*)this;
    guiInfoStruct.listener = NULL;

    m_pPlugIn->showGUI((void*)(&guiInfoStruct));
}

void CRafxVSTEditor::syncTabViews(CControl* pTabControl, int nControlTag, int nControlValue)
{
    if(m_ppHorizontalSwitchControls)
    {
        for(int i=0; i<m_nHorizontalSwitchCount; i++)
        {
            CHorizontalSwitch* pControl = m_ppHorizontalSwitchControls[i]; //->setValue(pControl->getValue());
            if(pControl)
            {
                if(pControl->getTag() == nControlTag && pTabControl != pControl)
                {
                    pControl->setValue(nControlValue);
                    pControl->invalid();
                }
            }
        }
    }
    if(m_ppVerticalSwitchControls)
    {
        for(int i=0; i<m_nVerticalSwitchCount; i++)
        {
            CVerticalSwitch* pControl = m_ppVerticalSwitchControls[i];
            if(pControl)
            {
                if(pControl->getTag() == nControlTag && pTabControl != pControl)
                {
                    pControl->setValue(nControlValue);
                    pControl->invalid();
                }
            }
        }
    }
    if(m_ppOptionMenuControls)
    {
        for(int i=0; i<m_nOptionMenuCount; i++)
        {
            COptionMenu* pControl = m_ppOptionMenuControls[i];
            if(pControl)
            {
                if(pControl->getTag() == nControlTag && pTabControl != pControl)
                {
                    pControl->setValue(nControlValue);
                    pControl->invalid();
                }
            }
        }
    }
   	if(m_ppSegmentButtons)
    {
        for(int i=0; i<m_nSegmentButtonCount; i++)
        {
            CSegmentButton* pControl = m_ppSegmentButtons[i];
            if(pControl)
            {
                if(pControl->getTag() == nControlTag && pTabControl != pControl)
                {
                    pControl->setValue(nControlValue);
                    pControl->invalid();
                }
            }
        }
    }
}

void CRafxVSTEditor::initControls(bool bSetListener)
{
	if(!frame)
		return;

	if(!m_pPlugIn)
		return frame->onActivate(true);

	if(!m_pControlMap)
		return frame->onActivate(true);

#ifdef AUPLUGIN
    // --- setup event listener
    if(m_AU && bSetListener)
    {
        // --- create the event listener and tell it the name of our Dispatcher function
        //     EventListenerDispatcher
        __Verify_noErr(AUEventListenerCreate(EventListenerDispatcher, this,
                                           CFRunLoopGetCurrent(), kCFRunLoopDefaultMode,
                                           AUEVENT_GRANULARITY, AUEVENT_INTERVAL,
                                           &AUEventListener));

        // --- setup automation handlers
        //
        //     see also: controlBeginEdit() -> begin gesture message sent to host
        //               controlEndEdit()   -> end gesture message sent to host
        //               valueChanged()     -> param change message sent to host
        for(int i=0; i< kNumEventTypes; i++)
        {
            // --- start with first control 0
            AudioUnitEvent auEvent;
            
            // --- parameter 0
            AudioUnitParameter parameter = {m_AU, 0, kAudioUnitScope_Global, 0};
            
            // --- set param & add it
            auEvent.mArgument.mParameter = parameter;
            if(i == kBeginEdit)
                auEvent.mEventType = kAudioUnitEvent_BeginParameterChangeGesture;
            else if(i == kEndEdit)
                auEvent.mEventType = kAudioUnitEvent_EndParameterChangeGesture;
            else if(i == kValueChanged)
                auEvent.mEventType = kAudioUnitEvent_ParameterValueChange;
            
            __Verify_noErr(AUEventListenerAddEventType(AUEventListener, this, &auEvent));
            
            // --- add the rest of the params
            int nParams = m_GUI_UIControlList.count() + numAddtlParams;
            for(int i=1; i<nParams; i++)
            {
                auEvent.mArgument.mParameter.mParameterID = i;
                __Verify_noErr(AUEventListenerAddEventType(AUEventListener, this, &auEvent));
            }
        }
    }
#endif

    // --- option menus require manual addition of menu items!
    if(m_ppOptionMenuControls)
    {
        for(int i=0; i<m_nOptionMenuCount; i++)
        {
            COptionMenu* pControl = m_ppOptionMenuControls[i];
            if(pControl)
            {
                if(pControl->getTag() < 0)
                    continue;

                int32_t nTag = pControl->getTag();
                if(nTag < TAB_CTRL_0)
                {
                    // OM's value is 0->string_count-1
                    int nControlIndex = m_XMLParser.getTagIndex(pControl->getTag());
                    int nPlugInControlIndex = m_pControlMap[nControlIndex];
                    CUICtrl* pUICtrl = m_pPlugIn->getUICtrlByListIndex(nPlugInControlIndex);

                    if(pUICtrl)
                    {
                        // --- clear
                        pControl->removeAllEntry();

                        if(pUICtrl->uUserDataType == UINTData)
                        {
                            bool bWorking = true;
                            int m = 0;
                            while(bWorking)
                            {
                                char* pEnum;
                                pEnum = getEnumString(pUICtrl->cEnumeratedList, m);
                                if(!pEnum)
                                    bWorking = false;
                                else
                                {
                                    pControl->addEntry(pEnum, m);
                                    delete [] pEnum;
                                    m++;
                                }
                            }

                            // --- set max
                            int n = pControl->getNbEntries();
                            if(n <= 0)
                            {
                                pControl->addEntry("-n/a-");
                                pControl->setMax(0);
                                pControl->setValue(0);
                            }

                            // --- init
                            pControl->setValue(0);
                        }
                        else
                        {
                            pControl->addEntry("min");
                            pControl->addEntry("max");
                            pControl->setMax(1);
                            pControl->setValue(0);
                        }
                    }
                }
             }
        }
    }

    // --- center trackpads to give valid starting point (may be altered by RestoreState() on load
    if(m_ppXYPads)
    {
        for(int i=0; i<m_nXYPadCount; i++)
        {
            if(m_ppXYPads[i]->getTag() == JOYSTICK && isCXYPadWP(m_ppXYPads[i]))
            {
                int nTagX = ((CXYPadWP*)m_ppXYPads[i])->getTagX();
                int nTagY = ((CXYPadWP*)m_ppXYPads[i])->getTagY();

                if(nTagX == JOYSTICK_X && nTagY == JOYSTICK_Y)
                {
                    float fx = (m_pPlugIn->m_JS_XCtrl.fJoystickValue + 1.0)/2.0;
                    float fy = (m_pPlugIn->m_JS_YCtrl.fJoystickValue + 1.0)/(2.0);

                    m_ppXYPads[i]->setValue(m_ppXYPads[i]->calculateValue(fx, fy));
                    m_ppXYPads[i]->invalid();
                }
                else
                {
                    CXYPadWP* pPad = (CXYPadWP*)m_ppXYPads[i];
                    if(pPad->m_bIsJoystickPad)
                    {
                        m_ppXYPads[i]->setValue(m_ppXYPads[i]->calculateValue(0.5, 0.5));
                        //m_ppXYPads[i]->invalid(); not needed here
                    }
                }
            }
        }
    }

    // --- setup knobswitch and sliderswitches (read only)
    if(m_ppSliderControls)
    {
        for(int i=0; i<m_nSliderCount; i++)
        {
            CSlider* pControl = m_ppSliderControls[i]; //->setValue(pControl->getValue());
            if(pControl)
            {
                if(pControl->getTag() < 0)
                    continue;

                int32_t nTag = pControl->getTag();
                int nControlIndex = m_XMLParser.getTagIndex(nTag);
                int nPlugInControlIndex = m_pControlMap[nControlIndex];
                CUICtrl* pCtrl = m_GUI_UIControlList.getAt(nPlugInControlIndex);

                if(pCtrl)
                {
                    pControl->setDefaultValue(getNormalizedValue(getDefaultValue(pCtrl), pCtrl));

                    if(pCtrl->uUserDataType == UINTData)
                    {
                        float fEnums = (float)getNumEnums(pCtrl->cEnumeratedList);

                        CVerticalSliderWP* vertSlider = dynamic_cast<CVerticalSliderWP*>(pControl);
                        if(vertSlider)
                            vertSlider->setSwitchMax(fEnums - 1.0);

                        CHorizontalSliderWP* horizSlider = dynamic_cast<CHorizontalSliderWP*>(pControl);
                        if(horizSlider)
                            horizSlider->setSwitchMax(fEnums - 1.0);
                    }
                }
            }
        }
    }
    if(m_ppKnobControls)
    {
        // --- knobs
        for(int i=0; i<m_nAnimKnobCount; i++)
        {
            CAnimKnob* pControl = m_ppKnobControls[i]; //->setValue(pControl->getValue());
            if(pControl)
            {
                if(pControl->getTag() < 0)
                    continue;

                int32_t nTag = pControl->getTag();
                int nControlIndex = m_XMLParser.getTagIndex(nTag);
                int nPlugInControlIndex = m_pControlMap[nControlIndex];
                CUICtrl* pCtrl = m_GUI_UIControlList.getAt(nPlugInControlIndex);
                if(pCtrl)
                {
                    pControl->setDefaultValue(getNormalizedValue(getDefaultValue(pCtrl), pCtrl));
                                              
                    if(pCtrl->uUserDataType == UINTData)
                    {
                        float fEnums = (float)getNumEnums(pCtrl->cEnumeratedList);

                        CKnobWP* knob = dynamic_cast<CKnobWP*>(pControl);
                        if(knob)
                            ((CKnobWP*)pControl)->setSwitchMax(fEnums - 1.0);
                    }
                }
            }
        }
    }

    // --- this just sets up the detector, does not update with variable
    if(m_ppVuMeterControls)
    {
        for(int i=0; i<m_nVuMeterCount; i++)
        {
            CVuMeter* pControl = m_ppVuMeterControls[i];
            if(pControl)
            {
                if(pControl->getTag() < 0)
                    continue;

                int nControlIndex = m_XMLParser.getTagIndex(pControl->getTag());
                int nPlugInControlIndex = m_pControlMap[nControlIndex];
                CUICtrl* pUICtrl = m_GUI_UIControlList.getAt(nPlugInControlIndex);
                if(pUICtrl)
                {
                    float fSampleRate = 1.0/(METER_UPDATE_INTERVAL_MSEC*0.001);

                    if(isCVuMeterWP(pControl))
                        ((CVuMeterWP*)pControl)->initDetector(fSampleRate, pUICtrl->fMeterAttack_ms, pUICtrl->fMeterRelease_ms, true, pUICtrl->uDetectorMode, pUICtrl->bLogMeter);
                }
            }
        }
    }

#ifdef AUPLUGIN
    // --- Threadsafe Sync to Global Variables
    // get the parameter values
    Float32 paramValue;
    int nParams = m_GUI_UIControlList.count() + numAddtlParams;
    for(int i=0; i<nParams; i++)
    {
        if(AudioUnitGetParameter(m_AU, i, kAudioUnitScope_Global, 0, &paramValue) != noErr)
            return;

        // --- update controls
        dispatchControlChange(i, paramValue);
    }
#endif

#ifdef AAXPLUGIN
    // --- threadsafe sync to AAX parameters
    /* --- NOTE: this is redundant; the controls are
                 automatically initialized by the framework
                 when the view container is loaded.
    if(m_pAAXParameters)
    {
        int32_t nParams;
        m_pAAXParameters->GetNumberOfParameters(&nParams);
        double dParam = 0.0;
        for(int i=0; i<nParams; i++)
        {
            std::stringstream str;
            str << i+1;
            m_pAAXParameters->GetParameterNormalizedValue(str.str().c_str(), &dParam);
            updateGUIControlsAAX(i, (float)dParam, false);
        }
    }*/
#endif

    // --- turn on
	frame->onActivate(true);
}


//-----------------------------------------------------------------------------------
void CRafxVSTEditor::valueChanged(CControl* pControl)
{
	if(!m_pPlugIn) return;

	int32_t nTag = pControl->getTag();
	if(nTag == ASSIGNBUTTON_1)
	{
		m_pPlugIn->userInterfaceChange(50); // NOTE: no underlying variable is being changed here!
		return;
	}
	else if(nTag == ASSIGNBUTTON_2)
	{
		m_pPlugIn->userInterfaceChange(51); // NOTE: no underlying variable is being changed here!
		return;
	}
	else if(nTag == ASSIGNBUTTON_3)
	{
		m_pPlugIn->userInterfaceChange(52); // NOTE: no underlying variable is being changed here!
		return;
	}

    if(!m_pControlMap) return;
    CUICtrl* pCtrl = NULL;

	if(nTag == JOYSTICK)
	{
		// --- find this control
		CXYPad* pPad = findJoystickXYPad(pControl);

		if(pPad && isCXYPadWP(pPad))
		{
			if(((CXYPadWP*)pPad)->getTagX() == JOYSTICK_X && ((CXYPadWP*)pPad)->getTagY() == JOYSTICK_Y)
			{
				float x, y = 0;

				pPad->calculateXY(pPad->getValue(), x, y);
				m_fJS_X = 2.0*x - 1.0;
				m_fJS_Y = -2.0*y + 1.0;
                int nParams = m_GUI_UIControlList.count();
#ifdef AUPLUGIN
                // --- threadsafe atomic writes
                AudioUnitParameter paramJSX = {m_AU, static_cast<AudioUnitParameterID>(nParams + vectorJoystickX_Offset), kAudioUnitScope_Global, 0};
                AUParameterSet(AUEventListener, pControl, &paramJSX, m_fJS_X, 0);

                AudioUnitParameter paramJSY = {m_AU, static_cast<AudioUnitParameterID>(nParams + vectorJoystickY_Offset), kAudioUnitScope_Global, 0};
                AUParameterSet(AUEventListener, pControl, &paramJSY, m_fJS_Y, 0);
#endif

#ifdef AAXPLUGIN
                y = -1.0*y + 1.0;
                setAAXParameterFromGUIControl(NULL, NULL, nParams + vectorJoystickX_Offset, x);
                setAAXParameterFromGUIControl(NULL, NULL, nParams + vectorJoystickY_Offset, y);
#endif
                return;
			}
		}
	}
	else if(nTag == TRACKPAD)
	{
		// --- find this control
		CXYPad* pPad = findJoystickXYPad(pControl);

		if(pPad && isCXYPadWP(pPad))
		{
			int nControlIndex = m_XMLParser.getTagIndex(((CXYPadWP*)pPad)->getTagX());
			int nPlugInControlIndex = m_pControlMap[nControlIndex];
			float x, y = 0;

			((CXYPadWP*)pPad)->calculateXY(pPad->getValue(), x, y);
			y = -1.0*y + 1.0;

			// --- get the control for re-broadcast of some types
            pCtrl = m_GUI_UIControlList.getAt(nPlugInControlIndex);
#ifdef AUPLUGIN
            // --- set param and disptatch
            setAUEventFromGUIControl(pControl, pCtrl, nPlugInControlIndex, x);
#endif

#ifdef AAXPLUGIN
            float fNormalizedControlValue = x;
            setAAXParameterFromGUIControl(pControl, pCtrl, nPlugInControlIndex, fNormalizedControlValue);
#endif
			nControlIndex = m_XMLParser.getTagIndex(((CXYPadWP*)pPad)->getTagY());
			nPlugInControlIndex = m_pControlMap[nControlIndex];

            // --- get the control for re-broadcast of some types
            pCtrl = m_GUI_UIControlList.getAt(nPlugInControlIndex);

#ifdef AUPLUGIN
            // --- set param and disptatch
            setAUEventFromGUIControl(pControl, pCtrl, nPlugInControlIndex, y);
#endif

#ifdef AAXPLUGIN
            fNormalizedControlValue = y;
            setAAXParameterFromGUIControl(pControl, pCtrl, nPlugInControlIndex, fNormalizedControlValue);
#endif
            return;
        }
	}
	else if(nTag >= TAB_CTRL_0 && nTag <= TAB_CTRL_31)
	{
		float fActualControlValue = pControl->getValue();
        doTabControlSwitch(nTag, fActualControlValue); // does view switch
        syncTabViews(pControl, nTag, fActualControlValue); // because these are not global params
		return;
	}

    // --- everything else
	int nControlIndex = m_XMLParser.getTagIndex(nTag);
	int nPlugInControlIndex = m_pControlMap[nControlIndex];

	// --- get the control for re-broadcast of some types
	pCtrl = m_GUI_UIControlList.getAt(nPlugInControlIndex);

    // --- normalized
    float fNormalizedControlValue = 0.0;

    // --- check text edits
   if(isTextEditControl(pControl))
    {
        // --- self update, calc string index if needed, returns 0->1 value to update other controls
        fNormalizedControlValue = updateEditControl(pControl, pCtrl); // v6.6 changes in this function
    }
    else
        fNormalizedControlValue = pControl->getValueNormalized();

#ifdef AUPLUGIN
    // --- set param and disptatch
    setAUEventFromGUIControl(pControl, pCtrl, nPlugInControlIndex, fNormalizedControlValue);
#endif

#ifdef AAXPLUGIN
    setAAXParameterFromGUIControl(pControl, pCtrl, nPlugInControlIndex, fNormalizedControlValue);
#endif

}

void CRafxVSTEditor::checkSendUpdateGUI(int nControlId, float fValue, bool bLoadingPreset)
{
    if(!m_pPlugIn) return;
    CLinkedList<GUI_PARAMETER> updateGUIParameters;

    // --- ask plugin for updates
    if(m_pPlugIn->checkUpdateGUI(nControlId, fValue, updateGUIParameters, bLoadingPreset))
    {
        for(int i = 0; i < updateGUIParameters.count(); i++)
        {
            GUI_PARAMETER* pParam = updateGUIParameters.getAt(i);
            if(pParam)
            {
#ifdef AUPLUGIN
                // --- threadsafe atomic write
                AudioUnitParameter param = {m_AU, static_cast<AudioUnitParameterID>(getGUI_UICtrlIndexByControlID(pParam->uControlId)), kAudioUnitScope_Global, 0};
                AUParameterSet(AUEventListener, AUEventListener, &param, pParam->fActualValue, 0);
#endif

#ifdef AAXPLUGIN
                int nIndex = getGUI_UICtrlIndexByControlID(pParam->uControlId);
                CUICtrl* pUICtrl = getGUI_UICtrlByControlID(pParam->uControlId);
                if(pUICtrl)
                {
                    std::stringstream str;
                    str << nIndex+1;
                    AAX_IParameter* oParameter;
                    AAX_Result result = m_pAAXParameters->GetParameter(str.str().c_str(), &oParameter);
                    if(AAX_SUCCESS == result)
                    {
                        if(pUICtrl->uUserDataType == floatData || pUICtrl->uUserDataType == doubleData)
                            oParameter->SetValueWithFloat(pParam->fActualValue);
                        else if(pUICtrl->uUserDataType == intData || pUICtrl->uUserDataType == UINTData)
                            oParameter->SetValueWithInt32((int32_t)pParam->fActualValue);
                    }
                }
#endif
            }
        }

        // --- plugin needs to clean because it allocted from its address space
        m_pPlugIn->clearUpdateGUIParameters(updateGUIParameters);
    }
}

const char* CRafxVSTEditor::getEnumString(const char* sTag)
{
    if(!m_pPlugIn) return NULL;

    CUICtrl* pCtrl;
    int nControlIndex = m_XMLParser.getTagIndex(sTag);
    int nPlugInControlIndex = m_pControlMap[nControlIndex];

    pCtrl = m_GUI_UIControlList.getAt(nPlugInControlIndex);
    if(pCtrl)
    {
        const char* cEnum(pCtrl->cEnumeratedList);
        return cEnum;
    }

    return NULL;
}
// --- needed to do this to make the object construct with proper vtable, June 2015
int32_t CRafxVSTEditor::controlModifierClicked (VSTGUI::CControl* pControl, VSTGUI::CButtonState button)//;// { return 0; }	///< return 1 if you want the control to not handle it, otherwise 0
{
    return 0;
}

void CRafxVSTEditor::controlBeginEdit (VSTGUI::CControl* pControl)//;// {}
{
#ifdef AAXPLUGIN
    if(m_pAAXParameters )
    {
        int nTag = pControl->getTag();
        std::stringstream str;
        str << nTag+1;
        m_pAAXParameters->TouchParameter(str.str().c_str());
    }
#endif
    
#ifdef AUPLUGIN
    // --- automation handlers
    AudioUnitEvent auEvent;
    auEvent.mArgument.mParameter.mAudioUnit = m_AU;
    auEvent.mArgument.mParameter.mParameterID = pControl->getTag();
    auEvent.mArgument.mParameter.mScope = kAudioUnitScope_Global;
    auEvent.mArgument.mParameter.mElement = 0;
    auEvent.mEventType = kAudioUnitEvent_BeginParameterChangeGesture;
    AUEventListenerNotify (AUEventListener, NULL, &auEvent);
#endif
    
    return;
}

void CRafxVSTEditor::controlEndEdit (VSTGUI::CControl* pControl)//;// {}
{
#ifdef AAXPLUGIN// #ifdef RAFX_AAX_PLUGIN
    if(m_pAAXParameters )
    {
        int nTag = pControl->getTag();
        std::stringstream str;
        str << nTag+1;
        m_pAAXParameters->ReleaseParameter(str.str().c_str());
    }
#endif
    
#ifdef AUPLUGIN
    // --- automation handlers
    AudioUnitEvent auEvent;
    auEvent.mArgument.mParameter.mAudioUnit = m_AU;
    auEvent.mArgument.mParameter.mParameterID = pControl->getTag();
    
    auEvent.mArgument.mParameter.mScope = kAudioUnitScope_Global;
    auEvent.mArgument.mParameter.mElement = 0;
    auEvent.mEventType = kAudioUnitEvent_EndParameterChangeGesture;
    AUEventListenerNotify (AUEventListener, NULL, &auEvent);
#endif

    return;
}

void CRafxVSTEditor::controlTagWillChange (VSTGUI::CControl* pControl)//;// {}
{
    return;
}

void CRafxVSTEditor::controlTagDidChange (VSTGUI::CControl* pControl)//;// {}
{
    return;
}


CMouseEventResult CRafxVSTEditor::onMouseDown(CFrame* frame, const CPoint& where, const CButtonState& buttons)
{
    CMouseEventResult result = kMouseEventNotHandled;
    
    int controlID = getControlID_WithMouseCoords(where);
    if(sendAAXMouseDown(controlID, buttons) == kMouseEventHandled)
        return kMouseEventHandled;
 
    return result;
}


CMouseEventResult CRafxVSTEditor::onMouseMoved(CFrame* frame, const CPoint& where, const CButtonState& buttons)
{
    CMouseEventResult result = kMouseEventNotHandled;
    
    int controlID = getControlID_WithMouseCoords(where);
    if(sendAAXMouseMoved(controlID, buttons, where) == kMouseEventHandled)
        return kMouseEventHandled;
    
    return result;
}
    
/*  For AAX/ProTools:
 This handles several ProTools-specific mouse events;
 * ctrl-alt-cmd: enable automation
 * alt:          set control to default value (VSTGUI uses ctrl and this overrides it)
 */
CMouseEventResult CRafxVSTEditor::sendAAXMouseDown(int controlID, const CButtonState& buttons)
{
#ifdef AAXPLUGIN
    VSTGUI::CMouseEventResult result = VSTGUI::kMouseEventNotHandled;
    const AAX_CVSTGUIButtonState aax_buttons(buttons, aaxViewContainer);
    
    if (aaxViewContainer && controlID >= 0)
    {
        std::stringstream str;
        str << controlID + 1;
        
        if (aaxViewContainer->HandleParameterMouseDown(str.str().c_str(), aax_buttons.AsAAX() ) == AAX_SUCCESS)
        {
            result = kMouseEventHandled;
        }
    }
    return result;
    
#endif
    return kMouseEventNotHandled;
}

CMouseEventResult CRafxVSTEditor::sendAAXMouseMoved(int controlID, const CButtonState& buttons, const CPoint& where)
{
#ifdef AAXPLUGIN
    VSTGUI::CMouseEventResult result = VSTGUI::kMouseEventNotHandled;
    const AAX_CVSTGUIButtonState aax_buttons(buttons, aaxViewContainer);
    
    if (aaxViewContainer && controlID >= 0)
    {
        std::stringstream str;
        str << controlID + 1;
        
        if (aaxViewContainer->HandleParameterMouseDrag(str.str().c_str(), aax_buttons.AsAAX() ) == AAX_SUCCESS)
        {
            result = kMouseEventHandled;
        }
    }
    
    return result;
    
#endif
    return kMouseEventNotHandled;
}
#if DEBUG
char CRafxVSTEditor::controlModifierClicked (VSTGUI::CControl* pControl, long button)//;// { return 0; }
{
    return 0;
}
#endif



}
