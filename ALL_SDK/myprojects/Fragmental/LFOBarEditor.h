//	LFOBarEditor.h - LFO waveshape editor made up of vertical bars.
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

#ifndef LFOBAREDITOR_H_
#define LFOBAREDITOR_H_

#include "vstcontrols.h"

///	LFO waveshape editor made up of vertical bars.
class LFOBarEditor : public CControl
{
  public:
	///	Constructor.
	/*!
		\param size The position and dimensions of the widget.
		\param listener The CControlListener for this widget.
		\param tag The first tag for this widget (remember it has 32 tags
		in total).
		\param background The background image for the widget.
		\param offset The offset within the background image to draw it from.
	 */
	LFOBarEditor(const CRect& size,
				 CControlListener *listener,
				 long tag,
				 CBitmap *background,
				 const CPoint& offset);
	///	Destructor.
	~LFOBarEditor();

	///	Draws the widget.
	void draw(CDrawContext *context);

	///	Handles the mouse input.
	void mouse(CDrawContext *context, CPoint &mousePos, long buttons = -1);

	///	Used to set the indexed value.
	void setValue(long index, float val);
	///	Overloaded version which calls valueChanged() too.
	void setValue(float *indices, CDrawContext *context);
	///	This will be different depending on which tag is active.
	float getValue() const;

	///	Sets the number of bars to use.
	void setNumBars(int val);
  private:
	///	The values of all the bars.
	float values[32];
	///	The starting tag.
	long tagStart;
	///	The number of currently active bars.
	int activeBars;

	///	The colour to draw the bars as.
	CColor barColour;
	///	The colour to draw the bars' frames as.
	CColor frameColour;
};

#endif
