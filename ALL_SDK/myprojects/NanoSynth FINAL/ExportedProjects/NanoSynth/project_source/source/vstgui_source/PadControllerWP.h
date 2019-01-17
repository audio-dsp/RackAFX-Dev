#pragma once
#include "vstgui/plugin-bindings/vst3padcontroller.h"

namespace VSTGUI{

class CPadControllerWP : public PadController
{
public:
	CPadControllerWP(IController* baseController, Steinberg::Vst::EditController* editController, Steinberg::Vst::Parameter* xParam, Steinberg::Vst::Parameter* yParam);
	~CPadControllerWP(void);

	void valueChanged (CControl* pControl) override;

};

}
