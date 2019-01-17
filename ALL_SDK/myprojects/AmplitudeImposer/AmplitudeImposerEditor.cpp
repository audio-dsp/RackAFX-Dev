//	AmplitudeImposerEditor.cpp - Plugin's gui.
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

#include "AmplitudeImposerEditor.h"
#include "AmplitudeImposer.h"

#include <stdio.h>

//-----------------------------------------------------------------------------
// resource id's
enum {
	// bitmaps
	kBack = 128,
	kSlider,
};

//-----------------------------------------------------------------------------
// AmplitudeImposerEditor class implementation
//-----------------------------------------------------------------------------
AmplitudeImposerEditor::AmplitudeImposerEditor(AudioEffect *effect)
 : AEffGUIEditor(effect) 
{
	depth = 0;
	threshold = 0;

	backBitmap = 0;
	sliderBitmap = 0;

	// load the background bitmap
	// we don't need to load all bitmaps, this could be done when open is called
	backBitmap  = new CBitmap(kBack);

	// init the size of the plugin
	rect.left   = 0;
	rect.top    = 0;
	rect.right  = (short)backBitmap->getWidth();
	rect.bottom = (short)backBitmap->getHeight();
}

//-----------------------------------------------------------------------------
AmplitudeImposerEditor::~AmplitudeImposerEditor()
{
	// free background bitmap
	if(backBitmap)
		backBitmap->forget();
	backBitmap = 0;
}

//-----------------------------------------------------------------------------
long AmplitudeImposerEditor::open(void *ptr)
{
	CPoint point;
	CRect size;

	// !!! always call this !!!
	AEffGUIEditor::open(ptr);
	
	// load some bitmaps
	if(!sliderBitmap)
		sliderBitmap = new CBitmap(kSlider);

	//--init background frame-------------------------------------------------
	size(0, 0, backBitmap->getWidth(), backBitmap->getHeight());
	frame = new CFrame(size, ptr, this);
	frame->setBackground(backBitmap);

	//--init sliders----------------------------------------------------------
	size(114, 71, 278, 87);
	point(114, 71);
	depth = new RightClickHSlider(size, this, kDepth, 114, (278-sliderBitmap->getWidth()), sliderBitmap, backBitmap, point, kLeft);
	depth->setValue(effect->getParameter(kDepth));
	depth->setDrawTransparentHandle(false);
	frame->addView(depth);

	size(114, 110, 278, 126);
	point(114, 110);
	threshold = new RightClickHSlider(size, this, kThreshold, 114, (278-sliderBitmap->getWidth()), sliderBitmap, backBitmap, point, kLeft);
	threshold->setValue(effect->getParameter(kThreshold));
	threshold->setDrawTransparentHandle(false);
	frame->addView(threshold);

	return true;
}

//-----------------------------------------------------------------------------
void AmplitudeImposerEditor::close()
{
	delete frame;
	frame = 0;

	if(sliderBitmap)
	{
		sliderBitmap->forget();
		sliderBitmap = 0;
	}
}

//-----------------------------------------------------------------------------
void AmplitudeImposerEditor::setParameter(long index, float value)
{
	if(!frame)
		return;

	// called from Template
	switch(index)
	{
		case kDepth:
			depth->setValue(effect->getParameter(index));
			break;
		case kThreshold:
			threshold->setValue(effect->getParameter(index));
			break;
	}
	postUpdate();
}

//-----------------------------------------------------------------------------
void AmplitudeImposerEditor::valueChanged(CDrawContext* context, CControl* control)
{
	long tag = control->getTag();

	switch(tag)
	{
		case kDepth:
		case kThreshold:
			effect->setParameterAutomated(tag, control->getValue());
			//control->update(context);
			break;
	}
}

