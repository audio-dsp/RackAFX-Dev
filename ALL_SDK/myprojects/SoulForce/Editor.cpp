//	Editor.cpp - Declaration of the editor class.
//	---------------------------------------------------------------------------
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
//	---------------------------------------------------------------------------

#include "Editor.h"
#include "VstPlugin.h"

//-----------------------------------------------------------------------------
Editor::Editor(AudioEffect *effect):
AEffGUIEditor(effect),
shape(0),
feedback(0),
source(0),
light(0),
footswitch(0),
about(0),
backgroundImage(0),
shapeImage(0),
feedbackImage(0),
sourceImage(0),
lightImage(0),
footswitchImage(0),
aboutImage(0)
{
	backgroundImage = new CBitmap(BackgroundImage);

	//Init the size of the editor.
	rect.left   = 0;
	rect.top    = 0;
	rect.right  = static_cast<short>(backgroundImage->getWidth());
	rect.bottom = static_cast<short>(backgroundImage->getHeight());
}

//-----------------------------------------------------------------------------
Editor::~Editor()
{
	//Decrement backgroundImage's reference count (because we initialised it
	//to 1 in it's constructor).  This will result in it being deleted.
	backgroundImage->forget();
}

//-----------------------------------------------------------------------------
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
	shapeImage = new CBitmap(ShapeImage);
	feedbackImage = new CBitmap(FeedbackImage);
	sourceImage = new CBitmap(SourceImage);
	lightImage = new CBitmap(LightImage);
	footswitchImage = new CBitmap(FootswitchImage);
	aboutImage = new CBitmap(AboutImage);

	//Create shape knob.
	size.x = 17;
	size.y = 64;
	size.right = (size.x+80);
	size.bottom = (size.y+95);
	offset.x = size.x;
	offset.y = size.y;
	shape = new CAnimKnob(size,
						  this,
						  VstPlugin::Shape,
						  shapeImage,
						  offset);
	frame->addView(shape);
	shape->setValue(effect->getParameter(VstPlugin::Shape));

	//Create feedback knob.
	size.x = 176;
	size.y = 64;
	size.right = (size.x+80);
	size.bottom = (size.y+95);
	offset.x = size.x;
	offset.y = size.y;
	feedback = new CAnimKnob(size,
						     this,
						     VstPlugin::Feedback,
						     feedbackImage,
						     offset);
	frame->addView(feedback);
	feedback->setValue(effect->getParameter(VstPlugin::Feedback));

	//Create source button.
	size.x = 116;
	size.y = 191;
	size.right = (size.x+51);
	size.bottom = (size.y+41);
	source = new COnOffButton(size,
							  this,
							  VstPlugin::Source,
							  sourceImage);
	frame->addView(source);
	source->setValue(effect->getParameter(VstPlugin::Source));

	//Create footswitch light.
	size.x = 123;
	size.y = 240;
	size.right = (size.x+34);
	size.bottom = (size.y+35);
	offset.x = 0;
	offset.y = 0;
	light = new CMovieBitmap(size,
							 this,
							 VstPlugin::Footswitch,
							 lightImage,
							 offset);
	frame->addView(light);
	light->setValue(effect->getParameter(VstPlugin::Footswitch));

	//Create footswitch kick button.
	size.x = 125;
	size.y = 282;
	size.right = (size.x+29);
	size.bottom = (size.y+57);
	offset.x = 0;
	offset.y = 0;
	footswitch = new CKickButton(size,
								 this,
								 VstPlugin::numParameters,
								 footswitchImage,
								 offset);
	frame->addView(footswitch);

	//Create about page.
	size.x = 31;
	size.y = 19;
	size.right = (size.x+221);
	size.bottom = (size.y+30);
	offset.x = 0;
	offset.y = 0;
	CRect aboutRect(0, 0, aboutImage->getWidth(), aboutImage->getHeight());
	about = new CSplashScreen(size,
							  this,
							  (VstPlugin::numParameters+1),
							  aboutImage,
							  aboutRect,
							  offset);
	frame->addView(about);

	//Decrement reference count for all bitmaps except the background.
	shapeImage->forget();
	feedbackImage->forget();
	sourceImage->forget();
	lightImage->forget();
	footswitchImage->forget();
	aboutImage->forget();

	setKnobMode(kLinearMode);

	//Indicate success.
	return true;
}

//-----------------------------------------------------------------------------
void Editor::close()
{
	//Delete everything else.
	delete frame;
	frame = 0;
}

//-----------------------------------------------------------------------------
void Editor::setParameter(VstInt32 index, float value)
{
	if(!frame)
		return;

	switch(index)
	{
		case VstPlugin::Shape:
			shape->setValue(value);
			break;
		case VstPlugin::Feedback:
			feedback->setValue(value);
			break;
		case VstPlugin::Source:
			source->setValue(value);
			break;
		case VstPlugin::Footswitch:
			light->setValue(value);
			break;
	}
}

//-----------------------------------------------------------------------------
void Editor::valueChanged(CDrawContext *context, CControl *control)
{
	if(control->getTag() == VstPlugin::numParameters)
	{
		if((footswitch->getValue() > 0.5f) &&
		   (effect->getParameter(VstPlugin::Footswitch) < 0.5f))
			effect->setParameterAutomated(VstPlugin::Footswitch, 1.0f);
		else if(footswitch->getValue() > 0.5f)
			effect->setParameterAutomated(VstPlugin::Footswitch, 0.0f);
	}
	else if(control->getTag() < VstPlugin::numParameters)
		effect->setParameterAutomated(control->getTag(), control->getValue());
}
