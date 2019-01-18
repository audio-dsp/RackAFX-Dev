#include "PadControllerWP.h"

namespace VSTGUI{
CPadControllerWP::CPadControllerWP(IController* baseController, Steinberg::Vst::EditController* editController, Steinberg::Vst::Parameter* xParam, Steinberg::Vst::Parameter* yParam)
				 :  PadController(baseController, editController, xParam, yParam)
{
}

CPadControllerWP::~CPadControllerWP(void)
{
}

void CPadControllerWP::valueChanged (CControl* pControl)
{
	if (pControl == padControl)
	{
		float x, y;
		CXYPad::calculateXY (pControl->getValue (), x, y);
		
		// -- invert Y here
		//y = -1.0*y + 1.0;

		editController->performEdit (xParam->getInfo ().id, x);
		editController->setParamNormalized (xParam->getInfo ().id, x);
		editController->performEdit (yParam->getInfo ().id, y);
		editController->setParamNormalized (yParam->getInfo ().id, y);
	}
	else
	{
		DelegationController::valueChanged (pControl);
	}
}
}
