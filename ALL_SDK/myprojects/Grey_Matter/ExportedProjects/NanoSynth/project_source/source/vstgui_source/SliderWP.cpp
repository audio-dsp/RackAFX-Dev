#include "SliderWP.h"
#include "vstgui/lib/cbitmap.h"
#include "vstgui/lib/cdrawcontext.h"

#include <cmath>
#pragma warning (disable : 4244) // conversion from 'int' to 'float', possible loss of data for knob/slider switch views (this is what we want!)

/* ------------------------------------------------------
     CVerticalSliderWP, CHorizontalSliderWP
     Custom VSTGUI Objects by Will Pirkle
     Created with RackAFX(TM) Plugin Development Software
     www.willpirkle.com
 -------------------------------------------------------*/

namespace VSTGUI {

CVerticalSliderWP::CVerticalSliderWP(const CRect &rect, IControlListener* listener, int32_t tag, int32_t iMinPos, int32_t iMaxPos, CBitmap* handle, CBitmap* background, const CPoint& offset, const int32_t style)
: CVerticalSlider(rect, listener, tag, iMinPos, iMaxPos, handle, background, offset, style)
{
	switchSlider = false;
    aaxSlider = false;
	maxControlValue = 1.0;
}

CVerticalSliderWP::~CVerticalSliderWP()
{

}

//------------------------------------------------------------------------
void CVerticalSliderWP::draw (CDrawContext *pContext)
{
	CDrawContext* drawContext = pContext;

	// draw background
	if (getDrawBackground ())
	{
		CRect rect (0, 0, widthControl, heightControl);
		rect.offset (getViewSize ().left, getViewSize ().top);
		getDrawBackground ()->draw (drawContext, rect, offset);
	}

	if (drawStyle != 0)
	{
		pContext->setDrawMode (kAliasing);
		pContext->setLineStyle (kLineSolid);
		pContext->setLineWidth (1.);
		if (drawStyle & kDrawFrame || drawStyle & kDrawBack)
		{
			pContext->setFrameColor (frameColor);
			pContext->setFillColor (backColor);
			CDrawStyle d = kDrawFilled;
			if (drawStyle & kDrawFrame && drawStyle & kDrawBack)
				d = kDrawFilledAndStroked;
			else if (drawStyle & kDrawFrame)
				d = kDrawStroked;
			pContext->drawRect (getViewSize (), d);
		}
		pContext->setDrawMode (kAntiAliasing);
		if (drawStyle & kDrawValue)
		{
			CRect r (getViewSize ());
			if (drawStyle & kDrawFrame)
				r.inset (1., 1.);

			float drawValue = getValueNormalized ();

			if (drawStyle & kDrawValueFromCenter)
			{
				if (drawStyle & kDrawInverted)
					drawValue = 1.f - drawValue;
				if (getStyle () & kHorizontal)
				{
					CCoord width = r.getWidth ();
					r.right = r.left + r.getWidth () * drawValue;
					r.left += width / 2.;
					r.normalize ();
				}
				else
				{
					CCoord height = r.getHeight ();
					r.bottom = r.top + r.getHeight () * drawValue;
					r.top += height / 2.;
					r.normalize ();
				}
			}
			else
			{
				if (getStyle () & kHorizontal)
				{
					if (drawStyle & kDrawInverted)
						r.left = r.right - r.getWidth () * drawValue;
					else
						r.right = r.left + r.getWidth () * drawValue;
				}
				else
				{
					if (drawStyle & kDrawInverted)
						r.bottom = r.top + r.getHeight () * drawValue;
					else
						r.top = r.bottom - r.getHeight () * drawValue;
				}
			}
			pContext->setFillColor (valueColor);
			pContext->drawRect (r, kDrawFilled);
		}
	}

	if(pHandle)
	{
		// --- mormalize to the switch for clicky behaviour
		if(switchSlider)
		{
			value *= maxControlValue;
            value = int(value);// + 0.5f);
			value /= maxControlValue;
		}

		float normValue = getValueNormalized ();
		if (style & kRight || style & kBottom)
			normValue = 1.f - normValue;

		// calc new coords of slider
		CRect rectNew;
		if (style & kHorizontal)
		{
			rectNew.top    = offsetHandle.y;
			rectNew.bottom = rectNew.top + heightOfSlider;

			rectNew.left   = offsetHandle.x + floor (normValue * rangeHandle);
			rectNew.left   = (rectNew.left < minTmp) ? minTmp : rectNew.left;

			rectNew.right  = rectNew.left + widthOfSlider;
			rectNew.right  = (rectNew.right > maxTmp) ? maxTmp : rectNew.right;
		}
		else
		{
			rectNew.left   = offsetHandle.x;
			rectNew.right  = rectNew.left + widthOfSlider;

			rectNew.top    = offsetHandle.y + floor (normValue * rangeHandle);
			rectNew.top    = (rectNew.top < minTmp) ? minTmp : rectNew.top;

			rectNew.bottom = rectNew.top + heightOfSlider;
			rectNew.bottom = (rectNew.bottom > maxTmp) ? maxTmp : rectNew.bottom;
		}
		rectNew.offset (getViewSize ().left, getViewSize ().top);

		// draw slider at new position
		pHandle->draw (drawContext, rectNew);
	}

	setDirty (false);
}
    
bool CVerticalSliderWP::checkDefaultValue (CButtonState button)
{
    int32_t modder = isAAXSlider() ? kAlt : kDefaultValueModifier;
    
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

CMouseEventResult CVerticalSliderWP::onMouseDown(CPoint& where, const CButtonState& buttons)
{
    if (!(buttons & kLButton))
        return kMouseEventNotHandled;
    
    CRect handleRect;
    delta = calculateDelta (where, getMode () != kFreeClickMode ? &handleRect : 0);
    if (getMode () == kTouchMode && !handleRect.pointInside (where))
        return kMouseEventNotHandled;
    
    oldVal    = getMin () - 1;
    oldButton = buttons;
    
    if ((getMode () == kRelativeTouchMode && handleRect.pointInside (where)) || getMode () != kRelativeTouchMode)
    {
        if(checkDefaultValue(buttons))
        {
            return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
        }
    }
    
    startVal = getValue ();
    beginEdit ();
    mouseStartPoint = where;
   
    int32_t zoomer = isAAXSlider() ? kControl : kZoomModifier;

    if (buttons & zoomer)
        return kMouseEventHandled;
    
    return onMouseMoved (where, buttons);
}

CMouseEventResult CVerticalSliderWP::onMouseUp(CPoint& where, const CButtonState& buttons)
{
    oldButton = 0;
    endEdit ();
    return kMouseEventHandled;
}

CMouseEventResult CVerticalSliderWP::onMouseMoved (CPoint& where, const CButtonState& buttons)
{
    if (isEditing ())
    {
        if (buttons & kLButton)
        {
            if (oldVal == getMin () - 1)
                oldVal = (value - getMin ()) / getRange ();
            
            int32_t zoomer = isAAXSlider() ? kControl : kZoomModifier;

            if ((oldButton != buttons) && (buttons & zoomer))
            {
                oldVal = (value - getMin ()) / getRange ();
                oldButton = buttons;
            }
            else if (!(buttons & zoomer))
                oldVal = (value - getMin ()) / getRange ();
            
            float normValue;
            if (style & kHorizontal)
                normValue = (float)(where.x - delta) / (float)rangeHandle;
            else
                normValue = (float)(where.y - delta) / (float)rangeHandle;
            
            if (style & kRight || style & kBottom)
                normValue = 1.f - normValue;
            
            if (buttons & zoomer)
                normValue = oldVal + ((normValue - oldVal) / zoomFactor);
            
            setValueNormalized (normValue);
            
            if (isDirty ())
            {
                valueChanged ();
                invalid ();
            }
        }
        return kMouseEventHandled;
    }
    return kMouseEventNotHandled;
}
    
    
    
CHorizontalSliderWP::CHorizontalSliderWP (const CRect &rect, IControlListener* listener, int32_t tag, int32_t iMinPos, int32_t iMaxPos, CBitmap* handle, CBitmap* background, const CPoint& offset, const int32_t style)
: CHorizontalSlider(rect, listener, tag, iMinPos, iMaxPos, handle, background, offset, style)
{
	switchSlider = false;
    aaxSlider = false;
	maxControlValue = 1.0;
}
CHorizontalSliderWP::~CHorizontalSliderWP()
{

}

void CHorizontalSliderWP::draw (CDrawContext *pContext)
{
	CDrawContext* drawContext = pContext;

	// draw background
	if (getDrawBackground ())
	{
		CRect rect (0, 0, widthControl, heightControl);
		rect.offset (getViewSize ().left, getViewSize ().top);
		getDrawBackground ()->draw (drawContext, rect, offset);
	}

	if (drawStyle != 0)
	{
		pContext->setDrawMode (kAliasing);
		pContext->setLineStyle (kLineSolid);
		pContext->setLineWidth (1.);
		if (drawStyle & kDrawFrame || drawStyle & kDrawBack)
		{
			pContext->setFrameColor (frameColor);
			pContext->setFillColor (backColor);
			CDrawStyle d = kDrawFilled;
			if (drawStyle & kDrawFrame && drawStyle & kDrawBack)
				d = kDrawFilledAndStroked;
			else if (drawStyle & kDrawFrame)
				d = kDrawStroked;
			pContext->drawRect (getViewSize (), d);
		}
		pContext->setDrawMode (kAntiAliasing);
		if (drawStyle & kDrawValue)
		{
			CRect r (getViewSize ());
			if (drawStyle & kDrawFrame)
				r.inset (1., 1.);
			float drawValue = getValueNormalized ();
			if (drawStyle & kDrawValueFromCenter)
			{
				if (drawStyle & kDrawInverted)
					drawValue = 1.f - drawValue;
				if (getStyle () & kHorizontal)
				{
					CCoord width = r.getWidth ();
					r.right = r.left + r.getWidth () * drawValue;
					r.left += width / 2.;
					r.normalize ();
				}
				else
				{
					CCoord height = r.getHeight ();
					r.bottom = r.top + r.getHeight () * drawValue;
					r.top += height / 2.;
					r.normalize ();
				}
			}
			else
			{
				if (getStyle () & kHorizontal)
				{
					if (drawStyle & kDrawInverted)
						r.left = r.right - r.getWidth () * drawValue;
					else
						r.right = r.left + r.getWidth () * drawValue;
				}
				else
				{
					if (drawStyle & kDrawInverted)
						r.bottom = r.top + r.getHeight () * drawValue;
					else
						r.top = r.bottom - r.getHeight () * drawValue;
				}
			}
			pContext->setFillColor (valueColor);
			pContext->drawRect (r, kDrawFilled);
		}
	}

	if(pHandle)
	{
		// --- mormalize to the switch for clicky behaviour
		if(switchSlider)
		{
			value *= maxControlValue;
            value = int(value);// + 0.5f);
			value /= maxControlValue;
		}

		float normValue = getValueNormalized ();
		if (style & kRight || style & kBottom)
			normValue = 1.f - normValue;

		// calc new coords of slider
		CRect rectNew;
		if (style & kHorizontal)
		{
			rectNew.top    = offsetHandle.y;
			rectNew.bottom = rectNew.top + heightOfSlider;

			rectNew.left   = offsetHandle.x + floor (normValue * rangeHandle);
			rectNew.left   = (rectNew.left < minTmp) ? minTmp : rectNew.left;

			rectNew.right  = rectNew.left + widthOfSlider;
			rectNew.right  = (rectNew.right > maxTmp) ? maxTmp : rectNew.right;
		}
		else
		{
			rectNew.left   = offsetHandle.x;
			rectNew.right  = rectNew.left + widthOfSlider;

			rectNew.top    = offsetHandle.y + floor (normValue * rangeHandle);
			rectNew.top    = (rectNew.top < minTmp) ? minTmp : rectNew.top;

			rectNew.bottom = rectNew.top + heightOfSlider;
			rectNew.bottom = (rectNew.bottom > maxTmp) ? maxTmp : rectNew.bottom;
		}
		rectNew.offset (getViewSize ().left, getViewSize ().top);

		// draw slider at new position
		pHandle->draw (drawContext, rectNew);
	}

	setDirty (false);
}

bool CHorizontalSliderWP::checkDefaultValue (CButtonState button)
{
    int32_t modder = isAAXSlider() ? kAlt : kDefaultValueModifier;
    
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
    
CMouseEventResult CHorizontalSliderWP::onMouseDown(CPoint& where, const CButtonState& buttons)
{
    if (!(buttons & kLButton))
        return kMouseEventNotHandled;
    
    CRect handleRect;
    delta = calculateDelta (where, getMode () != kFreeClickMode ? &handleRect : 0);
    if (getMode () == kTouchMode && !handleRect.pointInside (where))
        return kMouseEventNotHandled;
    
    oldVal    = getMin () - 1;
    oldButton = buttons;
    
    if ((getMode () == kRelativeTouchMode && handleRect.pointInside (where)) || getMode () != kRelativeTouchMode)
    {
        if(checkDefaultValue(buttons))
        {
            return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
        }
    }
    
    startVal = getValue ();
    beginEdit ();
    mouseStartPoint = where;
    
    int32_t zoomer = isAAXSlider() ? kControl : kZoomModifier;
    
    if (buttons & zoomer)
        return kMouseEventHandled;
    
    return onMouseMoved (where, buttons);
}

CMouseEventResult CHorizontalSliderWP::onMouseUp(CPoint& where, const CButtonState& buttons)
{
    oldButton = 0;
    endEdit ();
    return kMouseEventHandled;
}

CMouseEventResult CHorizontalSliderWP::onMouseMoved (CPoint& where, const CButtonState& buttons)
{
    if (isEditing ())
    {
        if (buttons & kLButton)
        {
            if (oldVal == getMin () - 1)
                oldVal = (value - getMin ()) / getRange ();
            
            int32_t zoomer = isAAXSlider() ? kControl : kZoomModifier;
            
            if ((oldButton != buttons) && (buttons & zoomer))
            {
                oldVal = (value - getMin ()) / getRange ();
                oldButton = buttons;
            }
            else if (!(buttons & zoomer))
                oldVal = (value - getMin ()) / getRange ();
            
            float normValue;
            if (style & kHorizontal)
                normValue = (float)(where.x - delta) / (float)rangeHandle;
            else
                normValue = (float)(where.y - delta) / (float)rangeHandle;
            
            if (style & kRight || style & kBottom)
                normValue = 1.f - normValue;
            
            if (buttons & zoomer)
                normValue = oldVal + ((normValue - oldVal) / zoomFactor);
            
            setValueNormalized (normValue);
            
            if (isDirty ())
            {
                valueChanged ();
                invalid ();
            }
        }
        return kMouseEventHandled;
    }
    return kMouseEventNotHandled;
}
   

    
} // namespace
