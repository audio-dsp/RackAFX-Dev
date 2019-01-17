//	CycleShifterEditor.cpp - Plugin's gui.
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
#include "CycleShifterEditor.h"
#endif

#ifndef CYCLESHIFTER_H_
#include "CycleShifter.h"
#endif

#include <stdio.h>

//-----------------------------------------------------------------------------
// resource id's
enum {
	// bitmaps
	kBack = 128,
	kSlider,
};

//-----------------------------------------------------------------------------
// CycleShifterEditor class implementation
//-----------------------------------------------------------------------------
CycleShifterEditor::CycleShifterEditor(AudioEffect *effect)
 : AEffGUIEditor(effect) 
{
	NewCycleVolume = 0;

	hSlider = 0;

	// load the background bitmap
	// we don't need to load all bitmaps, this could be done when open is called
	hBackground  = new CBitmap(kBack);

	// init the size of the plugin
	rect.left   = 0;
	rect.top    = 0;
	rect.right  = (short)hBackground->getWidth();
	rect.bottom = (short)hBackground->getHeight();
}

//-----------------------------------------------------------------------------
CycleShifterEditor::~CycleShifterEditor()
{
	// free background bitmap
	if(hBackground)
		hBackground->forget();
	hBackground = 0;
}

//-----------------------------------------------------------------------------
long CycleShifterEditor::open(void *ptr)
{
	// !!! always call this !!!
	AEffGUIEditor::open(ptr);
	
	// load some bitmaps
	if(!hSlider)
		hSlider = new CBitmap(kSlider);

	//--init background frame-----------------------------------------------
	CRect size(0, 0, hBackground->getWidth(), hBackground->getHeight());
	frame = new CFrame(size, ptr, this);
	frame->setBackground(hBackground);

	size(11, 49, 256, 63);
	CPoint point(11, 49);
	NewCycleVolume = new CHorizontalSlider(size, this, kNewCycleVolume, 11, (256-hSlider->getWidth()), hSlider, hBackground, point, (kLeft|kHorizontal));
	NewCycleVolume->setValue(effect->getParameter(kNewCycleVolume));
	NewCycleVolume->setTransparency(false);
	frame->addView(NewCycleVolume);

	size(11, 80, 256, 94);
	point(11, 80);
	InputVolume = new CHorizontalSlider(size, this, kInputVolume, 11, (256-hSlider->getWidth()), hSlider, hBackground, point, (kLeft|kHorizontal));
	InputVolume->setValue(effect->getParameter(kInputVolume));
	InputVolume->setTransparency(false);
	frame->addView(InputVolume);

	return true;
}

//-----------------------------------------------------------------------------
void CycleShifterEditor::close()
{
	delete frame;
	frame = 0;

	if(hSlider)
	{
		hSlider->forget();
		hSlider = 0;
	}
}

//-----------------------------------------------------------------------------
void CycleShifterEditor::setParameter (long index, float value)
{
	if (!frame)
		return;

	// called from Template
	switch (index)
	{
		case kNewCycleVolume:
			NewCycleVolume->setValue(effect->getParameter(index));
			break;
		case kInputVolume:
			InputVolume->setValue(effect->getParameter(index));
			break;
	}
	postUpdate();
}

//-----------------------------------------------------------------------------
void CycleShifterEditor::valueChanged(CDrawContext* context, CControl* control)
{
	long tag = control->getTag();
	switch(tag)
	{
		case kNewCycleVolume:
		case kInputVolume:
			effect->setParameterAutomated(tag, control->getValue());
			control->update(context);
			break;
	}
}