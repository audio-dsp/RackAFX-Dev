#include "RackAFX_GUI.h"

#if _WINDOWS
#include <windows.h>

#if defined AAXPLUGIN && !defined _WINDOWS && !defined _WINDLL
#include <CoreFoundation/CoreFoundation.h>
#endif

void* hInstance = NULL;
// *******************************************************************************
// ROUTINE:	DllMain
// *******************************************************************************
extern "C" BOOL WINAPI DllMain(HINSTANCE iInstance, DWORD iSelector, LPVOID iReserved)
{
	if(iSelector == DLL_PROCESS_ATTACH)
	{
		hInstance = (HINSTANCE)iInstance;
	}
	return true;
}
#endif

RackAFX_GUI::RackAFX_GUI(void)
{
	m_nWidth = 0;
	m_nHeight = 0;
	m_bPureCustomGUI = false;
}

RackAFX_GUI::~RackAFX_GUI(void)
{
}

AAX_IEffectGUI* AAX_CALLBACK RackAFX_GUI::Create()
{
	return new RackAFX_GUI;
}

void RackAFX_GUI::CreateViewContainer()
{
#if defined _WINDOWS || defined _WINDLL
	if(this->GetViewContainerType() == AAX_eViewContainer_Type_HWND)
	{
		m_CustomData.pRackAFX_Plugin = NULL;
		uint32_t oSize;
		m_nWidth = 0;
		m_nHeight = 0;

		AAX_Result success = GetEffectParameters()->GetCustomData(RACKAFX_PLUGIN_ID, sizeof(rackAFXCustomData*), &m_CustomData, &oSize);

		if(success != AAX_SUCCESS) return; // fail
		if(!m_CustomData.pRackAFX_Plugin) return; // fail
        if(!m_CustomData.pMeterMap) return; // fail

        // --- first see if they have a custom GUI
        void* createdCustomGUI = NULL;
        VSTGUI_VIEW_INFO info = {0};
        info.message = GUI_HAS_USER_CUSTOM;
        info.bHasUserCustomView = false; // this flag will be set if they do
        m_CustomData.pRackAFX_Plugin->showGUI((void*)&info);

        if(info.bHasUserCustomView)
        {
            info.message = GUI_USER_CUSTOM_OPEN;
            info.window = (void*)this->GetViewContainerPtr();
            info.hPlugInInstance = hInstance;
            info.hRAFXInstance = hInstance;
            info.pAAXParameters = this->GetEffectParameters();
            info.size.width = 0;
            info.size.height = 0;
            info.vstPlatformType = kHWND;
            info.bHasUserCustomView = true;

            createdCustomGUI = m_CustomData.pRackAFX_Plugin->showGUI((void*)&info);

            if(createdCustomGUI)
            {
                m_nWidth = info.size.width;
                m_nHeight = info.size.height;
                m_bPureCustomGUI = true;
                return;
            }
        }

        if(!createdCustomGUI)
        {
			DWORD xmlSize = 0;
			LPVOID lpXMLResource = NULL;

			HANDLE hRes = NULL;
			HRSRC hResInfo;
			hResInfo= ::FindResource((HMODULE)hInstance, L"RAFX.UIDESC", L"DATA"); //stresId is resource id

			if(hResInfo != NULL)
			{
				xmlSize = SizeofResource((HMODULE)hInstance, hResInfo);
				hRes = LoadResource((HMODULE)hInstance, hResInfo);
				if (hRes != NULL)
				{
					lpXMLResource = LockResource(hRes);
				}
			}

			bool success = m_RafxVSTGUI.open(this->GetViewContainerPtr(), m_CustomData.pRackAFX_Plugin, lpXMLResource, xmlSize);
			if(!success) return;

			float fWidth, fHeight = 0.f;
            m_RafxVSTGUI.getSize(fWidth, fHeight);
			m_nWidth = (int)fWidth;
			m_nHeight = (int)fHeight;

            m_RafxVSTGUI.initPluginCustomGUI();
			m_RafxVSTGUI.setAAXParameters(this->GetEffectParameters());

            // --- setup control maps
            int nControlCount = 0;
            UINT* pControlMap = createControlMap(m_CustomData.pRackAFX_Plugin, nControlCount);
            m_RafxVSTGUI.copyControlMap(pControlMap, nControlCount);
            m_RafxVSTGUI.copyMeterMap(m_CustomData.pMeterMap);
            m_RafxVSTGUI.initControls(false);
            m_RafxVSTGUI.setAAXViewContainer(this->GetViewContainer());

            return;
        }
    }

#else
    if(this->GetViewContainerType() == AAX_eViewContainer_Type_NSView)
    {
        m_CustomData.pRackAFX_Plugin = NULL;
        uint32_t oSize;
        m_nWidth = 0;
        m_nHeight = 0;

        AAX_Result success = GetEffectParameters()->GetCustomData(RACKAFX_PLUGIN_ID, sizeof(rackAFXCustomData*), &m_CustomData, &oSize);

        if(success != AAX_SUCCESS) return; // fail
        if(!m_CustomData.pRackAFX_Plugin) return; // fail
        if(!m_CustomData.pMeterMap) return; // fail

        // --- first see if they have a custom GUI
        void* createdCustomGUI = NULL;
        VSTGUI_VIEW_INFO info = {0};
        info.message = GUI_HAS_USER_CUSTOM;
        info.bHasUserCustomView = false; // this flag will be set if they do
        m_CustomData.pRackAFX_Plugin->showGUI((void*)&info);

        if(info.bHasUserCustomView)
        {
            info.message = GUI_USER_CUSTOM_OPEN;
            info.window = (void*)this->GetViewContainerPtr();
            info.hPlugInInstance = NULL;
            info.hRAFXInstance = NULL;
            info.pAAXParameters = this->GetEffectParameters();
            info.size.width = 0;
            info.size.height = 0;
            info.vstPlatformType = kNSView;
            info.bHasUserCustomView = true;

            createdCustomGUI = m_CustomData.pRackAFX_Plugin->showGUI((void*)&info);

            if(createdCustomGUI)
            {
                m_nWidth = info.size.width;
                m_nHeight = info.size.height;
                m_bPureCustomGUI = true;
                return;
            }
        }

        if(!createdCustomGUI)
        {
            // --- Look for a resource in the main bundle by name and type.
            CFBundleRef bundle = CFBundleGetBundleWithIdentifier( CFSTR("developer.aax.nanosynth") );
            if (bundle == NULL) return;

            // --- get .uidesc file
            CFURLRef bundleURL = CFBundleCopyResourceURL(bundle,CFSTR("RackAFX"),CFSTR("uidesc"),NULL);
            CFStringRef xmlPath = CFURLCopyFileSystemPath(bundleURL, kCFURLPOSIXPathStyle);
            int nSize = CFStringGetLength(xmlPath);
            char* path = new char[nSize+1];
            memset(path, 0, (nSize+1)*sizeof(char));
            CFStringGetFileSystemRepresentation(xmlPath, path, nSize+1);
            CFRelease(xmlPath);

            // --- create GUI
            bool success = m_RafxVSTGUI.open(this->GetViewContainerPtr(), m_CustomData.pRackAFX_Plugin, NULL, 0, path);
            if(!success) return;

            // --- save for host queries later
            float fWidth, fHeight = 0.f;
            m_RafxVSTGUI.getSize(fWidth, fHeight);
            m_nWidth = (int)fWidth;
            m_nHeight = (int)fHeight;

            m_RafxVSTGUI.initPluginCustomGUI();
            m_RafxVSTGUI.setAAXParameters(this->GetEffectParameters());

            // --- setup control maps
            int nControlCount = 0;
            UINT* pControlMap = createControlMap(m_CustomData.pRackAFX_Plugin, nControlCount);
            m_RafxVSTGUI.copyControlMap(pControlMap, nControlCount);
            m_RafxVSTGUI.copyMeterMap(m_CustomData.pMeterMap);
            m_RafxVSTGUI.initControls(false);
            m_RafxVSTGUI.setAAXViewContainer(this->GetViewContainer());
            return;
        }
    }

#endif
}

AAX_Result RackAFX_GUI::Draw(AAX_Rect* iDrawRect)
{
	return AAX_SUCCESS;
}

AAX_Result RackAFX_GUI::TimerWakeup(void)
{
	return AAX_SUCCESS;
}

AAX_Result RackAFX_GUI::ParameterUpdated(const char* iParameterID)
{
	AAX_IParameter* pParam = NULL;
	GetEffectParameters()->GetParameter(iParameterID, &pParam);

	if(pParam)
	{
		float fNormalizedValue = pParam->GetNormalizedValue();
		int nTag = atoi(iParameterID) - 1;
		m_RafxVSTGUI.updateGUIControlsAAX(nTag, fNormalizedValue);

	    // --- only for hand-rolled pure custom GUIs
        if(m_bPureCustomGUI && m_CustomData.pRackAFX_Plugin)
        {
            VSTGUI_VIEW_INFO info = {0};
            info.message = GUI_EXTERNAL_SET_NORM_VALUE;
            info.normalizedParameterValue = fNormalizedValue;
            info.parameterTag = nTag;
            m_CustomData.pRackAFX_Plugin->showGUI((void*)(&info));
        }
		return AAX_SUCCESS;
	}

	return AAX_ERROR_INVALID_PARAMETER_ID;
}

void RackAFX_GUI::DeleteViewContainer()
{
	if(!m_CustomData.pRackAFX_Plugin) return;

	VSTGUI_VIEW_INFO info;
	info.message = GUI_HAS_USER_CUSTOM;
	info.bHasUserCustomView = false; // this flag will be set if they do
	m_CustomData.pRackAFX_Plugin->showGUI((void*)&info);
	if(info.bHasUserCustomView)
	{
		info.message = GUI_USER_CUSTOM_CLOSE;
		m_CustomData.pRackAFX_Plugin->showGUI((void*)&info);
	}
	else
		m_RafxVSTGUI.close();
}

void RackAFX_GUI::CreateViewContents()
{
    // --- currently not used
}

AAX_Result RackAFX_GUI::GetViewSize(AAX_Point* oEffectViewSize) const
{
	oEffectViewSize->horz = (float)m_nWidth;
	oEffectViewSize->vert = (float)m_nHeight;

	return AAX_SUCCESS;
}

UINT* RackAFX_GUI::createControlMap(CPlugIn* pRAFXPlugIn, int& nControlCount)
{
    int nCount = getGUIControlCount(pRAFXPlugIn);
    UINT *pControlMap = nCount > 0 ? new UINT[nCount] : NULL;
    int m = 0;
    nControlCount = 0;
    int nControls = pRAFXPlugIn->getControlCount();
    for(int i = 0; i< nControls; i++)
    {
        CUICtrl* pUICtrl = pRAFXPlugIn->getUICtrlByListIndex(i);
        if(pUICtrl->uControlType != FILTER_CONTROL_CONTINUOUSLY_VARIABLE &&
           pUICtrl->uControlType != FILTER_CONTROL_COMBO_VARIABLE &&
           pUICtrl->uControlType != FILTER_CONTROL_USER_VSTGUI_VARIABLE &&
           pUICtrl->uControlType != FILTER_CONTROL_RADIO_SWITCH_VARIABLE &&
           pUICtrl->uControlType != FILTER_CONTROL_LED_METER)
            continue;

        // --- add the index
        if(pControlMap)
            pControlMap[m++] = i;
    }

    nControlCount = m;
    return pControlMap;
}

int RackAFX_GUI::getGUIControlCount(CPlugIn* pRAFXPlugIn)
{
    int nCount = 0;
    int nControls = pRAFXPlugIn->getControlCount();
    for(int i = 0; i< nControls; i++)
    {
        CUICtrl* pUICtrl = pRAFXPlugIn->getUICtrlByListIndex(i);
        if(pUICtrl->uControlType != FILTER_CONTROL_CONTINUOUSLY_VARIABLE &&
           pUICtrl->uControlType != FILTER_CONTROL_COMBO_VARIABLE &&
           pUICtrl->uControlType != FILTER_CONTROL_USER_VSTGUI_VARIABLE &&
           pUICtrl->uControlType != FILTER_CONTROL_RADIO_SWITCH_VARIABLE &&
           pUICtrl->uControlType != FILTER_CONTROL_LED_METER)
            continue;

        // --- inc
        nCount++;
    }
    return nCount;
}



