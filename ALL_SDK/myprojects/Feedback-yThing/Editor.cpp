//	Editor.cpp - Declaration of the editor class.
//	--------------------------------------------------------------------------
//	Copyright (c) 2006 Niall Moody
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

#include "Editor.h"
#include "VstPlugin.h"

//----------------------------------------------------------------------------
Editor::Editor(AudioEffect *effect):
AEffGUIEditor(effect),
pitchKnob(0),
boostKnob(0),
bypassButton(0),
backgroundImage(0),
knobTickImage(0),
bypassImage(0)
{
	backgroundImage = new CBitmap(BackgroundImage);

	//Init the size of the editor.
	rect.left   = 0;
	rect.top    = 0;
	rect.right  = static_cast<short>(backgroundImage->getWidth());
	rect.bottom = static_cast<short>(backgroundImage->getHeight());
}

//----------------------------------------------------------------------------
Editor::~Editor()
{
	//Decrement backgroundImage's reference count (because we initialised it
	//to 1 in it's constructor).  This will result in it being deleted.
	backgroundImage->forget();
}

//----------------------------------------------------------------------------
bool Editor::open(void *systemPointer)
{
	CPoint offset;

	AEffEditor::open(systemPointer);

	//Initialise the frame.
	CRect size(0,
			   0,
			   backgroundImage->getWidth(),
			   backgroundImage->getHeight());
	frame = new CFrame(size, systemPointer, this);
	frame->setBackground(backgroundImage);

	//Create bitmaps.
	knobTickImage = new CBitmap(KnobTickImage);
	bypassImage = new CBitmap(BypassImage);

	//Create pitch knob.
	size.x = 31;
	size.y = 33;
	size.right = (size.x+28);
	size.bottom = (size.y+28);
	offset.x = 31;
	offset.y = 33;
	pitchKnob = new CKnob(size,
						  this,
						  VstPlugin::Pitch,
						  backgroundImage,
						  knobTickImage,
						  offset);
	frame->addView(pitchKnob);
	pitchKnob->setValue(effect->getParameter(VstPlugin::Pitch));

	//Create boost knob.
	size.x = 115;
	size.y = 33;
	size.right = (size.x+28);
	size.bottom = (size.y+28);
	offset.x = 31;
	offset.y = 33;
	boostKnob = new CKnob(size,
						  this,
						  VstPlugin::Boost,
						  backgroundImage,
						  knobTickImage,
						  offset);
	frame->addView(boostKnob);
	boostKnob->setValue(effect->getParameter(VstPlugin::Boost));

	//Create bypass button.
	size.x = 55;
	size.y = 89;
	size.right = (size.x+64);
	size.bottom = (size.y+26);
	bypassButton = new COnOffButton(size,
									this,
									VstPlugin::Bypass,
									bypassImage);
	frame->addView(bypassButton);
	bypassButton->setValue(effect->getParameter(VstPlugin::Bypass));

	//Decrement reference count for all bitmaps except the background.
	knobTickImage->forget();
	bypassImage->forget();

	setKnobMode(kLinearMode);

	//Indicate success.
	return true;
}

//----------------------------------------------------------------------------
void Editor::close()
{
	//Delete everything else.
	delete frame;
	frame = 0;
}

//----------------------------------------------------------------------------
void Editor::setParameter(VstInt32 index, float value)
{
	if(!frame)
		return;

	switch(index)
	{
		case VstPlugin::Pitch:
			pitchKnob->setValue(value);
			break;
		case VstPlugin::Boost:
			boostKnob->setValue(value);
			break;
		case VstPlugin::Bypass:
			bypassButton->setValue(value);
			break;
	}
}

//----------------------------------------------------------------------------
void Editor::valueChanged(CDrawContext *context, CControl *control)
{
	effect->setParameterAutomated(control->getTag(), control->getValue());
}
