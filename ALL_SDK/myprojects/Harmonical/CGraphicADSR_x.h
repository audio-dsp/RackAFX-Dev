//	CGraphicADSR.h - A graphical ADSR (x because it replaces an older one I
//					 made, w/out anti-aliasing).
//	--------------------------------------------------------------------------
//	Copyright (c) 2005 Niall Moody
//	
//	Permission is hereby granted, free of charge, to any person obtaining a
//	copy of this software and associated documentation files (the "Software"),
//	to deal in the Software without restriction, including without limitation
//	the rights to use, copy, modify, merge, publish, distribute, sublicense,
//	and/or sell copies of the Software, and to permit persons to whom the
//	Software is furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//	THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//	DEALINGS IN THE SOFTWARE.
//	--------------------------------------------------------------------------

#ifndef CGRAPHICADSR_H_
#define CGRAPHICADSR_H_

#ifdef WIN32
#include "vstgui_3_0_beta2/vstcontrols.h"
#else
#include "vstcontrols.h"
#endif

///	Anti-aliased graphical ADSR widget.
class CGraphicADSR_x : public CControl
{
  public:
	///	Constructor.
	/*!
		\param tag_A The tag for the Attack control.
		\param tag_D The tag for the Decay control.
		\param tag_S The tag for the Sustain part of the control.
		\param tag_R The tag for the Release control.
	 */
	CGraphicADSR_x(CRect &size,
				   CControlListener *listener,
				   int tag_A,
				   int tag_D,
				   int tag_S,
				   int tag_R,
				   CColor BackColour,
				   CColor ForeColour,
				   CBitmap *pHandle = 0,
				   CBitmap *pBackground = 0);
	///	Destructor.
	~CGraphicADSR_x();

	///	Draws the widget.
	void draw(CDrawContext *context);
	///	Draws the widget (old, don't use).
	void draw_old(CDrawContext *context);
	///	Handles mouse movement in the widget.
	void mouse(CDrawContext *pContext, CPoint &where, long button = -1);

	///	Sets the size of the draggable handles.
	void SetHandleSize(int width);

	///	Sets the Attack value.
	void setValue_A(float value) {value_A = value; setPoints(0); setDirty();};
	///	Sets the Decay value.
	void setValue_D(float value) {value_D = value; setPoints(1); setDirty();};
	///	Sets the Sustain value.
	void setValue_S(float value) {value_S = value; setPoints(2); setDirty();};
	///	Sets the Release value.
	void setValue_R(float value) {value_R = value; setPoints(3); setDirty();};

	///	Returns the Attack value.
	float getValue_A() {return value_A;};
	///	Returns the Decay value.
	float getValue_D() {return value_D;};
	///	Returns the Sustain value.
	float getValue_S() {return value_S;};
	///	Returns the Release value.
	float getValue_R() {return value_R;};

	///	Sets the offset into the background image we start drawing from.
	void SetBackgroundOffset(CPoint val) {pBackOffs = val;};

  private:
	///	?
	void setBounds();
	///	Not sure what this is used for...
	void setPoints(int seg);

	///	Draws an anti-aliased line.
	void DrawAntiAliasedLine(CDrawContext *context, CPoint p1, CPoint p2, CColor Colour);
	///	Blends the current colour at point with col, according to col's alpha channel.
	void BlendPixel(CDrawContext *context, CPoint point, CColor col);

	int tag_A,
		tag_D,
		tag_S,
		tag_R;
	float value_A,
		  value_D,
		  value_S,
		  value_R;
	CPoint point_start,
		   point_A,
		   point_D,
		   point_R,
		   point_end;
	int xbounds_A,
		xbounds_D,
		xbounds_R;
	int ybounds_D,
		//ybounds_S,
		ybounds_R;

	COffscreenContext *oc;
	CBitmap *pHandle;
	CColor backg, foreg;
	CRect HandleSize;

	CPoint pBackOffs; //offset for the background CBitmap
};

#endif
