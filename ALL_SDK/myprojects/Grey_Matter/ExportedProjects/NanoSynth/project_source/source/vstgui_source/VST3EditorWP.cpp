#include "VST3EditorWP.h"
// #include "vstgui/plugin-bindings/vst3editor.cpp"

namespace VSTGUI {

    //-----------------------------------------------------------------------------
    class ParameterChangeListener : public Steinberg::FObject
    {
    public:
        ParameterChangeListener (Steinberg::Vst::EditController* editController, Steinberg::Vst::Parameter* parameter, CControl* control)
        : editController (editController)
        , parameter (parameter)
        {
            if (parameter)
            {
                parameter->addRef ();
                parameter->addDependent (this);
            }
            addControl (control);
            if (parameter)
                parameter->changed ();
        }
        
        ~ParameterChangeListener ()
        {
            if (parameter)
            {
                parameter->removeDependent (this);
                parameter->release ();
            }
            for (const auto& c : controls)
                c->forget ();
        }
        
        void addControl (CControl* control)
        {
            if (containsControl (control))
                return;
            control->remember ();
            controls.push_back (control);
            Steinberg::Vst::ParamValue value = 0.;
            if (parameter)
            {
                value = editController->getParamNormalized (getParameterID ());
            }
            else
            {
                CControl* control = controls.front ();
                if (control)
                    value = control->getValueNormalized ();
            }
            CParamDisplay* display = dynamic_cast<CParamDisplay*> (control);
            if (display)
                display->setValueToStringFunction([this](float value, char utf8String[256], CParamDisplay* display) {
                    return convertValueToString (value, utf8String);
                });
            
            if (parameter)
                parameter->deferUpdate ();
            else
                updateControlValue (value);
        }
        
        void removeControl (CControl* control)
        {
            for (const auto& c : controls)
            {
                if (c == control)
                {
                    controls.remove (control);
                    control->forget ();
                    return;
                }
            }
        }
        
        bool containsControl (CControl* control)
        {
            return std::find (controls.begin (), controls.end (), control) != controls.end ();
        }
        
        void PLUGIN_API update (FUnknown* changedUnknown, Steinberg::int32 message)
        {
            if (message == IDependent::kChanged && parameter)
            {
                updateControlValue (editController->getParamNormalized (getParameterID ()));
            }
        }
        
        Steinberg::Vst::ParamID getParameterID ()
        {
            if (parameter)
                return parameter->getInfo ().id;
            CControl* control = controls.front ();
            if (control)
                return static_cast<Steinberg::Vst::ParamID> (control->getTag ());
            return 0xFFFFFFFF;
        }
        
        void beginEdit ()
        {
            if (parameter)
                editController->beginEdit (getParameterID ());
        }
        
        void endEdit ()
        {
            if (parameter)
                editController->endEdit (getParameterID ());
        }
        
        void performEdit (Steinberg::Vst::ParamValue value)
        {
            if (parameter)
            {
                auto id = getParameterID ();
                if (editController->setParamNormalized (id, value) == Steinberg::kResultTrue)
                    editController->performEdit (id, editController->getParamNormalized (id));
            }
            else
            {
                updateControlValue (value);
            }
        }
        Steinberg::Vst::Parameter* getParameter () const { return parameter; }
        
    protected:
        bool convertValueToString (float value, char utf8String[256])
        {
            if (parameter)
            {
                Steinberg::Vst::String128 utf16Str;
                if (parameter && parameter->getInfo ().stepCount)
                {
                    // convert back to normalized value
                    value = (float)editController->plainParamToNormalized (getParameterID (), (Steinberg::Vst::ParamValue)value);
                }
                editController->getParamStringByValue (getParameterID (), value, utf16Str);
                Steinberg::String utf8Str (utf16Str);
                utf8Str.toMultiByte (Steinberg::kCP_Utf8);
                utf8Str.copyTo8 (utf8String, 0, 256);
                return true;
            }
            return false;
        }
        
        void updateControlValue (Steinberg::Vst::ParamValue value)
        {
            bool mouseEnabled = true;
            bool isStepCount = false;
            Steinberg::Vst::ParamValue defaultValue = 0.5;
            float minValue = 0.f;
            float maxValue = 1.f;
            if (parameter)
            {
                defaultValue = parameter->getInfo ().defaultNormalizedValue;
                if (parameter->getInfo ().flags & Steinberg::Vst::ParameterInfo::kIsReadOnly)
                    mouseEnabled = false;
                if (parameter->getInfo ().stepCount)
                {
                    isStepCount = true;
                    value = parameter->toPlain (value);
                    defaultValue = parameter->toPlain (defaultValue);
                    minValue = (float)parameter->toPlain ((Steinberg::Vst::ParamValue)minValue);
                    maxValue = (float)parameter->toPlain ((Steinberg::Vst::ParamValue)maxValue);
                }
            }
            for (const auto& c : controls)
            {
                c->setMouseEnabled (mouseEnabled);
                if (parameter)
                {
                    c->setDefaultValue ((float)defaultValue);
                    c->setMin (minValue);
                    c->setMax (maxValue);
                }
                CTextLabel* label = dynamic_cast<CTextLabel*>(c);
                if (label)
                {
                    Steinberg::Vst::ParamValue normValue = value;
                    if (isStepCount)
                    {
                        normValue = parameter->toNormalized (value);
                    }
                    Steinberg::Vst::String128 utf16Str;
                    if (editController->getParamStringByValue (getParameterID (), normValue, utf16Str) != Steinberg::kResultTrue)
                        continue;
                    Steinberg::String utf8Str (utf16Str);
                    utf8Str.toMultiByte (Steinberg::kCP_Utf8);
                    label->setText (utf8Str.text8 ());
                }
                else
                {
                    if (isStepCount)
                    {
                        c->setMin (minValue);
                        c->setMax (maxValue);
                        COptionMenu* optMenu = dynamic_cast<COptionMenu*>(c);
                        if (optMenu)
                        {
                            optMenu->removeAllEntry ();
                            for (Steinberg::int32 i = 0; i <= parameter->getInfo ().stepCount; i++)
                            {
                                Steinberg::Vst::String128 utf16Str;
                                editController->getParamStringByValue (getParameterID (), (Steinberg::Vst::ParamValue)i / (Steinberg::Vst::ParamValue)parameter->getInfo ().stepCount, utf16Str);
                                Steinberg::String utf8Str (utf16Str);
                                utf8Str.toMultiByte (Steinberg::kCP_Utf8);
                                optMenu->addEntry (utf8Str.text8 ());
                            }
                            c->setValue ((float)value - minValue);
                        }
                        else
                            c->setValue ((float)value);
                    }
                    else
                        c->setValueNormalized ((float)value);
                }
                c->invalid ();
            }
        }
        Steinberg::Vst::EditController* editController;
        Steinberg::Vst::Parameter* parameter;
        
        typedef std::list<CControl*> ControlList;
        ControlList controls;
    };
}

namespace Steinberg {
namespace Vst {
namespace Sock2VST3 {

VST3EditorWP::VST3EditorWP(Steinberg::Vst::EditController* controller, UTF8StringPtr templateName, UTF8StringPtr xmlFile)
: VST3Editor(controller, templateName, xmlFile)
{
	setIdleRate(GUI_IDLE_UPDATE_INTERVAL_MSEC);
	m_uKnobMode = kHostChoice; // --- v6.6
	m_pPlugIn = NULL;
}

// --- v6.6 addition
VSTGUI_INT32 VST3EditorWP::getKnobMode() const
{
	switch(m_uKnobMode)
	{
		case kHostChoice:
		{
			switch(EditController::getHostKnobMode())
			{
				case kRelativCircularMode: return VSTGUI::kRelativCircularMode;
				case kLinearMode: return VSTGUI::kLinearMode;
				case kCircularMode: return VSTGUI::kCircularMode;
			}
			break;
		}

		case kRelativCircularMode: return VSTGUI::kRelativCircularMode;
		case kLinearMode: return VSTGUI::kLinearMode;
		case kCircularMode: return VSTGUI::kCircularMode;
	}

	return VSTGUI::kLinearMode; // RAFX Default!
}

void VST3EditorWP::valueChanged(CControl* pControl)
{
	ParameterChangeListener* pcl = getParameterChangeListener (pControl->getTag ());
	if(pcl)
	{
		Steinberg::Vst::ParamValue value = pControl->getValueNormalized ();
		CTextEdit* textEdit = dynamic_cast<CTextEdit*> (pControl);
		if (textEdit && pcl->getParameter ())
		{
			Steinberg::String str (textEdit->getText ());
			str.toWideString (Steinberg::kCP_Utf8);
			if (getController ()->getParamValueByString (pcl->getParameterID (), (Steinberg::Vst::TChar*)str.text16 (), value) != Steinberg::kResultTrue)
			{
				pcl->update (0, kChanged);
				return;
			}
		}
		// --- this does the value change edit
		pcl->performEdit(value);
	}

	if(m_pPlugIn)
	{		
		Steinberg::Vst::ParamValue value = pControl->getValueNormalized ();
		Steinberg::Vst::ParamValue actualValue = getController()->normalizedParamToPlain(pControl->getTag(), value);

		CUICtrl* pUICtrl = m_pPlugIn->getUICtrlByListIndex(pControl->getTag());
		if(pUICtrl)
		{
			checkSendUpdateGUI(pUICtrl->uControlId, actualValue, false);
		}
	}
}

// --- v6.8: this is the new threadsafe way of handling updates to the GUI
//           NOTE: you should try to only use this for UINT or int-based controls
//				   if you need to link continuous controls, you should do so in 
//				   the RackAFX GUI Designer, or using the Advanced GUI API to 
//				   create your own custom control-sets, see: www.willpirkle.com
//				   for info on the Advanced GUI API
void VST3EditorWP::checkSendUpdateGUI(int nControlId, float fValue, bool bLoadingPreset)
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
				int32_t tag = getGUI_UICtrlIndexByControlID(pParam->uControlId);
				if(tag >= 0)
				{
					Steinberg::Vst::ParamValue normalizedValue = getController()->plainParamToNormalized(tag, pParam->fActualValue);
					getController()->setParamNormalized(tag, normalizedValue); // updates GUI
					getController()->performEdit(tag, normalizedValue); // changes variable
				}
			}
        }
        
        // --- plugin needs to clean because it allocted from its address space
        m_pPlugIn->clearUpdateGUIParameters(updateGUIParameters);
    }
}

// --- the only reason for existence is for timed notifications
CMessageResult VST3EditorWP::notify(CBaseObject* sender, IdStringPtr message)
{
	if (message == CVSTGUITimer::kMsgTimer && getController())
	{
		getController()->receiveText("VSTGUITimerPing");

		// --- this allows meters to fall-off for clients that do not update when
		//     no audio is playing; otherwize meters freeze when audio stops.
		getFrame()->invalid();
	}

	bool bRecreateView = doCreateView;

	CMessageResult result = VST3Editor::notify(sender, message);

	if(bRecreateView && getController())
		getController()->receiveText("RecreateView");
 	
	return result;
}

// --- RAFX v6.6 --- for GUI control from plugin 
CBitmap* VST3EditorWP::getBitmap(UTF8StringPtr name)
{
	CBitmap* bitmap = description->getBitmap(name);
	return bitmap;
}

UTF8StringPtr VST3EditorWP::lookupBitmapName(const CBitmap* bitmap)
{
	UTF8StringPtr name =  description->lookupBitmapName(bitmap);
	return name;
}

// --- This was added to fix a bug in Reaper (anything prior to v5.24) where the GUI did not show up
//
// --- This was fixed in Reaper 5.24Pre1 however I am leaving the return true statement
//     in case it pops up again or other hosts have the same issue; if so, then comment out
//     the base class call and un-comment the return true statement
bool VST3EditorWP::beforeSizeChange (const CRect& newSize, const CRect& oldSize)
{
	// --- just call base class
	//return VST3Editor::beforeSizeChange(newSize, oldSize);

	// --- if you are using an old version of Reaper, return TRUE instead of the above base class call
	return true;
}

}}}
