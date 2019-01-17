#pragma once
#include "vstgui/lib/controls/cbuttons.h"
#include "vstgui4constants.h"

namespace VSTGUI {
    
/* ------------------------------------------------------
 CKickButtonWP
 Custom VSTGUI Object by Will Pirkle
 Created with RackAFX(TM) Plugin Development Software
 www.willpirkle.com
 -------------------------------------------------------*/

enum {mouseUpAndDown, mouseUp, mouseDown};

class CKickButtonWP : public CKickButton
{
public:
	CKickButtonWP(const CRect& size, CControlListener* listener, int32_t tag, CBitmap* background, const CPoint& offset = CPoint (0, 0));
	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	virtual CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;

	void setMouseMode(unsigned int uMode){uMouseBehavior = uMode;}

private:
	float   fEntryState;
	unsigned int uMouseBehavior;
};

}
