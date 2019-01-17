//	SixPointLineEditor.h - Simple line editor widget.
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

#ifndef SIXPOINTLINEEDITOR_H_
#define SIXPOINTLINEEDITOR_H_

#include "vstcontrols.h"

///	Simple line editor widget.
class SixPointLineEditor : public CControl
{
  public:
	///	Constructor.
	/*!
		\param size The position and dimensions of the widget.
		\param listener The CControlListener for this widget.
		\param tag The first tag for this widget (remember it has 12 tags
		in total).
		\param background The background image for the widget.
		\param offset The offset within the background image to draw it from.
	 */
	SixPointLineEditor(const CRect& size,
					   CControlListener *listener,
					   long tag,
					   CBitmap *background,
					   const CPoint& offset);
	///	Destructor.
	~SixPointLineEditor();

	///	Draws the widget.
	void draw(CDrawContext *context);

	///	Handles the mouse input.
	void mouse(CDrawContext *context, CPoint &mousePos, long buttons = -1);

	///	Used to set the indexed value.
	void setValue(long index, float val);
	///	This will be different depending on which tag is active.
	float getValue() const;
  private:
	///	Enum giving names to the indices to values.
	enum
	{
		p0y,
		p1x,
		p1y,
		p2x,
		p2y,
		p3x,
		p3y,
		p4x,
		p4y,
		p5y
	};

	///	Our point values.
	float values[10];
	///	The start tag.
	long tagStart;

	///	The colour to draw the line and points as.
	CColor colour;
	///	Offset to draw the background from.
	CPoint backgroundOffset;

	///	Used for the preset triggering.
	int preset;
};

#endif
