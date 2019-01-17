//	TwoTickKnob.h - CKnob subclass with an extra tick.
//	---------------------------------------------------------------------------
//	Copyright (c) 2006-2007 Niall Moody
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
//	---------------------------------------------------------------------------

#ifndef TWOTICKKNOB_H_
#define TWOTICKKNOB_H_

#include "vstcontrols.h"

///	CKnob subclass with an extra tick.
class TwoTickKnob : public CKnob
{
  public:
	///	Constructor.
	TwoTickKnob(const CRect &size,
				CControlListener *listener,
				long tag, 
				CBitmap *background,
				CBitmap *handle,
				CBitmap *extra,
				const CPoint &offset);
	///	Destructor.
	~TwoTickKnob();

	///	So we can draw our extra tick.
	void draw(CDrawContext *pContext);

	///	So we can set the position of the extra tick.
	/*!
		\param val Range = 0->1.
	 */
	void setExtraTick(float val);
	///	Returns the current position of the extra tick.
	float getExtraTick() const {return extraPosition;};
  private:
	///	Helper method (copied from CKnob::drawHandle()) to draw the extra tick.
	void drawExtraTick(CDrawContext *pContext);
	///	Helper method (copied from CKnob::valueToPoint()).
	void extraToPoint(CPoint &point) const;

	///	The bitmap for the extra tick.
	CBitmap *extraBitmap;
	///	The position of the extra tick.
	float extraPosition;
};

#endif
