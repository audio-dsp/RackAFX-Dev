//	RightClickControls.h - Sliders & knobs which will centre on a right click.
//	--------------------------------------------------------------------------
//	Copyright (c) 2004 Niall Moody
//	
//	Permission is hereby granted, free of charge, to any person obtaining a
//	copy of this software and associated documentation files (the "Software"),
//	to deal in the Software without restriction, including without limitation
//	the rights to use, copy, modify, merge, publish, distribute, sublicense,
//	and/or sell copies of the Software, and to permit persons to whom the
//	Software is furnished to do so, subject to the following conditions:

//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.

//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//	THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//	DEALINGS IN THE SOFTWARE.
//	--------------------------------------------------------------------------

#ifndef RIGHTCLICKCONTROLS_H_
#define RIGHTCLICKCONTROLS_H_

#ifndef VSTGUI3
#include "newVSTGUI/vstcontrols.h"
#else
#include "vstgui_3_0_beta2/vstcontrols.h"
#endif

//----------------------------------------------------------------------------
class RightClickAnimKnob : public CAnimKnob
{
  public:
	RightClickAnimKnob(const CRect &size,
					   CControlListener *listener,
					   long tag, 
					   long subPixmaps,
					   long heightOfOneImage,
					   CBitmap *background,
					   CPoint &offset);
	~RightClickAnimKnob();

	bool checkDefaultValue(CDrawContext *pContext, long button);

	void mouse(CDrawContext *pContext, CPoint &where, long button=-1);
};

//----------------------------------------------------------------------------
class RightClickSlider : public CSlider
{
  public:
	RightClickSlider(const CRect &size,
					 CControlListener *listener,
					 long tag, 
					 long    iMinPos,
					 long    iMaxPos,
					 CBitmap *handle,
					 CBitmap *background,
					 CPoint  &offset,
					 const long style = kLeft|kHorizontal);
	virtual ~RightClickSlider();

	virtual bool checkDefaultValue(CDrawContext *pContext, long button);

	virtual bool attached(CView *parent);
	virtual bool removed(CView *parent);
	virtual void draw(CDrawContext *context);
};

//----------------------------------------------------------------------------
class RightClickHSlider : public RightClickSlider
{
  public:
	RightClickHSlider(const CRect &size,
					  CControlListener *listener,
					  long tag, 
                      long    iMinPos,
                      long    iMaxPos,
                      CBitmap *handle,
                      CBitmap *background,
                      CPoint  &offset,
                      const long style = kRight);
};

//----------------------------------------------------------------------------
class RightClickVSlider : public RightClickSlider
{
  public:
	RightClickVSlider(const CRect &size,
					  CControlListener *listener,
					  long tag, 
                      long    iMinPos,
                      long    iMaxPos,
                      CBitmap *handle,
                      CBitmap *background,
                      CPoint  &offset,
                      const long style = kBottom);
};

#endif