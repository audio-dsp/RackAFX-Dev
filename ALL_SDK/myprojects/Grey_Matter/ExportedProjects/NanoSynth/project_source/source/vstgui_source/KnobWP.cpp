#include "KnobWP.h"
#include "vstgui/lib/cbitmap.h"
#include <cmath>
#pragma warning (disable : 4244) // conversion from 'int' to 'float', possible loss of data for knob/slider switch views (this is what we want!)

/* ------------------------------------------------------
     CKnobWP
     Custom VSTGUI Object by Will Pirkle
     Created with RackAFX(TM) Plugin Development Software
     www.willpirkle.com
 -------------------------------------------------------*/

namespace VSTGUI {
#if TARGET_OS_IPHONE
    static const float kCKnobRange = 300.f;
#else
    static const float kCKnobRange = 200.f;
#endif

CKnobWP::CKnobWP (const CRect& size, IControlListener* listener, int32_t tag, int32_t subPixmaps, CCoord heightOfOneImage, CBitmap* background, const CPoint &offset, bool bSwitchKnob)
: CAnimKnob (size, listener, tag, subPixmaps, heightOfOneImage, background, offset)
, switchKnob(bSwitchKnob)
{
	maxControlValue = 1.0;
    aaxKnob = false;
}
CKnobWP::~CKnobWP(void)
{

}

void CKnobWP::draw(CDrawContext* pContext)
{
	if(getDrawBackground ())
	{
		CPoint where (0, 0);

		// --- mormalize to the switch for clicky behaviour
		if(switchKnob)
		{
			value *= maxControlValue;
            value = int(value);// + 0.5f);
			value /= maxControlValue;
		}

		if(value >= 0.f && heightOfOneImage > 0.)
		{
			CCoord tmp = heightOfOneImage * (getNumSubPixmaps () - 1);
			if (bInverseBitmap)
				where.y = floor ((1. - value) * tmp);
			else
				where.y = floor (value * tmp);
			where.y -= (int32_t)where.y % (int32_t)heightOfOneImage;
		}

		// --- draw it
		getDrawBackground()->draw(pContext, getViewSize(), where);

	}

	setDirty (false);
}
    
bool CKnobWP::checkDefaultValue (CButtonState button)
{
    int32_t modder = isAAXKnob() ? kAlt : kDefaultValueModifier;
    
#if TARGET_OS_IPHONE
    if (button.isDoubleClick ())
#else
        if (button.isLeftButton () && button.getModifierState () == modder)
#endif
        {
            float defValue = getDefaultValue ();
            if (defValue != getValue ())
            {
                // begin of edit parameter
                beginEdit ();
                
                setValue (defValue);
                valueChanged ();
                
                // end of edit parameter
                endEdit ();
            }
            return true;
        }
    return false;
}

CMouseEventResult CKnobWP::onMouseDown(CPoint& where, const CButtonState& buttons)
{
    if (!buttons.isLeftButton ())
        return kMouseEventNotHandled;
    
    beginEdit ();
 
    if(checkDefaultValue(buttons))
    {
        endEdit ();
        return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
    }
    
    firstPoint = where;
    lastPoint (-1, -1);
    startValue = getOldValue ();
    
    modeLinear = false;
    fEntryState = value;
    range = kCKnobRange;
    coef = (getMax () - getMin ()) / range;
    oldButton = buttons;
    
    int32_t mode    = kCircularMode;
    int32_t newMode = getFrame()->getKnobMode();
    int32_t zoomer = isAAXKnob() ? kControl : kZoomModifier;
    
    if(kLinearMode == newMode)
    {
        if(!(buttons & kAlt))
            mode = newMode;
    }
    else if(buttons & kAlt)
    {
        mode = kLinearMode;
    }
    
    if (mode == kLinearMode)
    {
        if (buttons & zoomer)
            range *= zoomFactor;
        lastPoint = where;
        modeLinear = true;
        coef = (getMax () - getMin ()) / range;
    }
    else
    {
        CPoint where2 (where);
        where2.offset (-getViewSize ().left, -getViewSize ().top);
        startValue = valueFromPoint (where2);
        lastPoint = where;
    }
    
    return kMouseEventHandled;
}

CMouseEventResult CKnobWP::onMouseUp(CPoint& where, const CButtonState& buttons)
{
    endEdit ();
    return kMouseEventHandled;
}

CMouseEventResult CKnobWP::onMouseMoved (CPoint& where, const CButtonState& buttons)
{
    if(isEditing())
    {
        float middle = (getMax () - getMin ()) * 0.5f;
        
        int32_t zoomer = isAAXKnob() ? kControl : kZoomModifier;

        if (where != lastPoint)
        {
            lastPoint = where;
            if (modeLinear)
            {
                CCoord diff = (firstPoint.y - where.y) + (where.x - firstPoint.x);
                if (buttons != oldButton)
                {
                    range = kCKnobRange;
                    if (buttons & zoomer)
                        range *= zoomFactor;
                    
                    float coef2 = (getMax () - getMin ()) / range;
                    fEntryState += (float)(diff * (coef - coef2));
                    coef = coef2;
                    oldButton = buttons;
                }
                value = (float)(fEntryState + diff * coef);
                bounceValue ();
            }
            else
            {
                where.offset (-getViewSize ().left, -getViewSize ().top);
                value = valueFromPoint (where);
                if (startValue - value > middle)
                    value = getMax ();
                else if (value - startValue > middle)
                    value = getMin ();
                else
                    startValue = value;
            }
            if (value != getOldValue ())
                valueChanged ();
            if (isDirty ())
                invalid ();
        }
        return kMouseEventHandled;
    }
    return kMouseEventNotHandled;
}

void CKnobWP::valueChanged()
{
	CControl::valueChanged();
}

}