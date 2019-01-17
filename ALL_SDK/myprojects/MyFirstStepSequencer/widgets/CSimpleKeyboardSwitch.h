//	CSimpleKeyboardSwitch.h - Each key acts as a switch, has a tag & value for
//							  every key.
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

#ifndef CSIMPLEKEYBOARDSWITCH_H_
#define CSIMPLEKEYBOARDSWITCH_H_

#include "CSimpleKeyboard.h"

///	Class where each key in the keyboard acts as a COnOffButton.
/*!
	\note This is \e very old code - it's not pretty, and I wouldn't recommend
	you use it in your own plugins...
 */
class CSimpleKeyboardSwitch : public CSimpleKeyboard
{
  public:
	CSimpleKeyboardSwitch(CRect &size,
						  CControlListener *listener,
						  long *tag,
						  CColor bgcol,
						  CColor fgcol,
						  int numoctaves = 1);
	~CSimpleKeyboardSwitch();

	void draw(CDrawContext *pContext);
	void mouse(CDrawContext *pContext, CPoint& position, long buttons = -1);

	void SetValue(float val, int index)
	{
		if(val < 0.5f)
			Values[index] = false;
		else
			Values[index] = true;

		setDirty();
	}

	float GetValue(int index)
	{
		if(!Values[index])
			return 0.0f;
		else
			return 1.0f;
	}
  private:
	long *Tags;
	bool *Values;
};

#endif
