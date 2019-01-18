
#ifndef __csliderwp__
#define __csliderwp__

#include "vstgui/lib/controls/cslider.h"

namespace VSTGUI {

/* ------------------------------------------------------
     CVerticalSliderWP, CHorizontalSliderWP
     Custom VSTGUI Objects by Will Pirkle
     Created with RackAFX(TM) Plugin Development Software
     www.willpirkle.com
 -------------------------------------------------------*/

class CVerticalSliderWP : public CVerticalSlider
{
public:
	CVerticalSliderWP (const CRect& size, IControlListener* listener, int32_t tag, int32_t iMinPos, int32_t iMaxPos, CBitmap* handle, CBitmap* background, const CPoint& offset = CPoint (0, 0), const int32_t style = kBottom);

	// overrides
	virtual void draw (CDrawContext*) override;
    CMouseEventResult onMouseUp(CPoint& where, const CButtonState& buttons) override;
    CMouseEventResult onMouseDown(CPoint& where, const CButtonState& buttons) override;
    virtual CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
    bool checkDefaultValue (CButtonState button);

	void setSwitchMax(float f){maxControlValue = f;}
	bool isSwitchSlider(){return switchSlider;}
	void setSwitchSlider(bool b){switchSlider = b;}
    
    void setAAXSlider(bool b){aaxSlider = b;}
    bool isAAXSlider(){return aaxSlider;}

	CLASS_METHODS(CVerticalSliderWP, CControl)

protected:
	~CVerticalSliderWP ();
	bool switchSlider;
    bool aaxSlider;
	float maxControlValue;
    
private:
    CCoord	delta;
    float	oldVal;
    float	startVal;
    CButtonState oldButton;
    CPoint mouseStartPoint;
};


class CHorizontalSliderWP : public CHorizontalSlider
{
public:
	CHorizontalSliderWP (const CRect& size, IControlListener* listener, int32_t tag, int32_t iMinPos, int32_t iMaxPos, CBitmap* handle, CBitmap* background, const CPoint& offset = CPoint (0, 0), const int32_t style = kLeft);

	// overrides
	virtual void draw (CDrawContext*) override;
    CMouseEventResult onMouseUp(CPoint& where, const CButtonState& buttons) override;
    CMouseEventResult onMouseDown(CPoint& where, const CButtonState& buttons) override;
    virtual CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
    bool checkDefaultValue (CButtonState button) override;

	void setSwitchMax(float f){maxControlValue = f;}
	void setSwitchSlider(bool b){switchSlider = b;}
	bool isSwitchSlider(){return switchSlider;}
    
    void setAAXSlider(bool b){aaxSlider = b;}
    bool isAAXSlider(){return aaxSlider;}

	CLASS_METHODS(CHorizontalSliderWP, CControl)

protected:
	~CHorizontalSliderWP ();
	bool switchSlider;
	float maxControlValue;
    bool aaxSlider;
    
private:
    CCoord	delta;
    float	oldVal;
    float	startVal;
    CButtonState oldButton;
    CPoint mouseStartPoint;
};

} // namespace

#endif
