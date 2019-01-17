//	CSimpleKeyboard.h - Keyboard base class: just draws the keys, basically.
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

#ifndef CSIMPLEKEYBOARD_H_
#define CSIMPLEKEYBOARD_H_

#include "vstcontrols.h"

///	Base class for drawing keyboards.
/*!
	\note This is \e very old code - it's not pretty, and I wouldn't recommend
	you use it in your own plugins...
 */
class CSimpleKeyboard : public CControl
{
  public:
	CSimpleKeyboard(CRect &size,
					CControlListener *listener,
				    CColor bgcol,
					CColor fgcol,
					int numoctaves = 1);
	virtual ~CSimpleKeyboard() {};

	virtual void draw(CDrawContext *pContext);

	virtual void DrawKeyboard(CDrawContext *oc, bool *keys);	//takes an array variable (octaves) containing which keys to highlight
											//****note: the order of keys entries is: all the white keys first, then all the black keys****

	virtual void SetColours(CColor bgcol, CColor fgcol, CColor frcol) {BackColour = bgcol; ForeColour = fgcol; FrameColour = frcol;};
  protected:
	int NumOctaves;
	CColor BackColour;
	CColor ForeColour;
	CColor FrameColour;
};

#endif
