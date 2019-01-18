#ifndef __cknobwp__
#define __cknobwp__
#include "vstgui/lib/controls/cknob.h"
#include "vstgui/lib/cframe.h"
#include <vector>

namespace VSTGUI {

/* ------------------------------------------------------
     CKnobWP
     Custom VSTGUI Object by Will Pirkle
     Created with RackAFX(TM) Plugin Development Software
     www.willpirkle.com
 -------------------------------------------------------*/
class CKnobWP : public CAnimKnob
{
public:
	CKnobWP(const CRect& size, IControlListener* listener, int32_t tag, int32_t subPixmaps,
			CCoord heightOfOneImage, CBitmap* background, const CPoint &offset,
			bool bSwitchKnob = false);

	virtual void draw (CDrawContext* pContext) override;
   
    CMouseEventResult onMouseUp(CPoint& where, const CButtonState& buttons) override;
    CMouseEventResult onMouseDown(CPoint& where, const CButtonState& buttons) override;
	virtual CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
    bool checkDefaultValue (CButtonState button) override;
    
	virtual void valueChanged() override;
	void setSwitchMax(float f){maxControlValue = f;}
    bool isSwitchKnob(){return aaxKnob;}
    
    void setAAXKnob(bool b){aaxKnob = b;}
    bool isAAXKnob(){return aaxKnob;}

protected:
	bool switchKnob;
    bool aaxKnob;
	float maxControlValue;
	virtual ~CKnobWP(void);
    
private:
    CPoint firstPoint;
    CPoint lastPoint;
    float  startValue;
    float  fEntryState;
    float  range;
    float  coef;
    CButtonState   oldButton;
    bool   modeLinear;

};
}

#endif
