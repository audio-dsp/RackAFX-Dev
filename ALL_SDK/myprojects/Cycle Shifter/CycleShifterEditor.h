//	CycleShifterEditor.h - Plugin's gui.
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

#ifndef CYCLESHIFTEREDITOR_H_
#define CYCLESHIFTEREDITOR_H_


// include VSTGUI
#ifndef __vstgui__
#include "vstgui.h"
#endif

//-----------------------------------------------------------------------------
class CycleShifterEditor : public AEffGUIEditor, public CControlListener
{
public:
	CycleShifterEditor(AudioEffect *effect);
	virtual ~CycleShifterEditor();

	virtual void setParameter(long index, float value);
	virtual void valueChanged(CDrawContext* context, CControl* control);

protected:
	virtual long open(void *ptr);
	virtual void close();

private:
	CHorizontalSlider *NewCycleVolume;
	CHorizontalSlider *InputVolume;

	// Bitmaps
	CBitmap *hBackground;
	CBitmap *hSlider;
};

#endif