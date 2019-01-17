//	KnobGui.cpp - GUI for the knob mod source.
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

#include "KnobGui.h"
#include "ModContainer.h"
#include "VstPlugin.h"
#include "TwoTickKnob.h"

#include <sstream>

using namespace std;

//-----------------------------------------------------------------------------
KnobGui::KnobGui(ModContainer *container,
			     int tagStart,
			     CControlListener *listener,
				 VstPlugin *pluginInstance):
ModSourceGui(container, tagStart, listener, pluginInstance),
modContainer(container),
plugin(pluginInstance),
isDisplayed(false),
start(tagStart),
typeMenu(0),
typeButton(0),
knob(0),
mass(0),
massEdit(0),
stiffness(0),
stiffnessEdit(0),
effect(pluginInstance)
{
	CPoint offset(0, 0);
	CRect size;
	CColor tempcolour = {128, 128, 128, 255};
	CColor greyColour;

	//Create bitmaps.
	backgroundImage = new CBitmap(BackgroundImage);
	comboImage = new CBitmap(ComboImage);
	knobImage = new CBitmap(KnobImage);
	bigKnobImage = new CBitmap(BigKnobImage);
	knobExtraImage = new CBitmap(KnobExtraImage);

	//Create background view.
	size.x = 0;
	size.y = 0;
	size.right = backgroundImage->getWidth();
	size.bottom = backgroundImage->getHeight();
	background = new CView(size);
	background->setBackground(backgroundImage);

	//Type menu.
	size.x = 38;
	size.y = 5;
	size.right = size.x + 51;
	size.bottom = size.y + 11;
	typeMenu = new COptionMenu(size, listener, (tagStart+VstPlugin::Type));
	typeMenu->addEntry("LFO");
	typeMenu->addEntry("Env");
	typeMenu->addEntry("Knob");
	typeMenu->setFontColor(tempcolour);
	typeMenu->setBackColor(kWhiteCColor);
	typeMenu->setFrameColor(kWhiteCColor);
	typeMenu->setFont(kNormalFontVerySmall);
	typeMenu->setCurrent(2);

	//Type button.
	size.x = 89;
	size.y = 2;
	size.right = size.x + 18;
	size.bottom = size.y + 18;
	typeButton = new CKickButton(size,
								 listener,
								 (tagStart+VstPlugin::TypeKick),
								 comboImage,
								 offset);

	//The big knob.
	size.x = 46;
	size.y = 40;
	size.right = size.x + 66;
	size.bottom = size.y + 66;
	offset.x = size.x;
	offset.y = size.y;
	knob = new TwoTickKnob(size,
						   listener,
						   (tagStart+VstPlugin::KnobVal),
						   backgroundImage,
						   bigKnobImage,
						   knobExtraImage,
						   offset);
	knob->setValue(pluginInstance->getParameter(tagStart+VstPlugin::KnobVal));

	//Mass knob.
	size.x = 169;
	size.y = 39;
	size.right = size.x + 26;
	size.bottom = size.y + 26;
	offset.x = size.x;
	offset.y = size.y;
	mass = new CKnob(size,
					 listener,
					 (tagStart+VstPlugin::KnobMass),
					 backgroundImage,
					 knobImage,
					 offset);
	mass->setValue(pluginInstance->getParameter(tagStart+VstPlugin::KnobMass));

	//Mass text editor.
	size.x = 163;
	size.y = 77;
	size.right = size.x + 38;
	size.bottom = size.y + 10;
	greyColour(128, 128, 128, 255);
	massEdit = new CTextEdit(size, listener, (tagStart+VstPlugin::KnobMass));
	massEdit->setBackColor(kWhiteCColor);
	massEdit->setFrameColor(kWhiteCColor);
	massEdit->setFontColor(greyColour);
	massEdit->setFont(kNormalFontVerySmall);
	setParameter((tagStart+VstPlugin::KnobMass),
				 effect->getParameter((tagStart+VstPlugin::KnobMass)));

	//Spring stiffness knob.
	size.x = 222;
	size.y = 39;
	size.right = size.x + 26;
	size.bottom = size.y + 26;
	offset.x = size.x;
	offset.y = size.y;
	stiffness = new CKnob(size,
						  listener,
						  (tagStart+VstPlugin::KnobSpring),
						  backgroundImage,
						  knobImage,
						  offset);
	stiffness->setValue(pluginInstance->getParameter(tagStart+VstPlugin::KnobSpring));

	//Stiffness text editor.
	size.x = 216;
	size.y = 77;
	size.right = size.x + 38;
	size.bottom = size.y + 10;
	greyColour(128, 128, 128, 255);
	stiffnessEdit = new CTextEdit(size, listener, (tagStart+VstPlugin::KnobSpring));
	stiffnessEdit->setBackColor(kWhiteCColor);
	stiffnessEdit->setFrameColor(kWhiteCColor);
	stiffnessEdit->setFontColor(greyColour);
	stiffnessEdit->setFont(kNormalFontVerySmall);
	setParameter((tagStart+VstPlugin::KnobSpring),
				 effect->getParameter((tagStart+VstPlugin::KnobSpring)));

	//Decrement reference count for all bitmaps.
	backgroundImage->forget();
	comboImage->forget();
	knobImage->forget();
	bigKnobImage->forget();
	knobExtraImage->forget();
}

//-----------------------------------------------------------------------------
KnobGui::~KnobGui()
{
	if(isDisplayed)
		hide();

	background->forget();
	typeMenu->forget();
	typeButton->forget();
	knob->forget();
	mass->forget();
	massEdit->forget();
	stiffness->forget();
	stiffnessEdit->forget();
}

//-----------------------------------------------------------------------------
void KnobGui::setParameter(int index, float value, CDrawContext *context)
{
	char tempch[16];
	stringstream tempConverter;
	VstPlugin *pluginInstance = effect;

	if((index == (start+VstPlugin::TypeKick)) && context)
	{
		CPoint temp(60, 10);
		typeMenu->mouse(context, temp);
	}
	else if(index == (start+VstPlugin::Type))
		typeMenu->setValue(value * 2.0f);
	else if(index == (start+VstPlugin::KnobMass))
	{
		mass->setValue(value);
		pluginInstance->getParameterDisplay((start+VstPlugin::KnobMass), tempch);
		tempConverter << tempch;
		massEdit->setText(const_cast<char *>(tempConverter.str().c_str()));
	}
	else if(index == (start+VstPlugin::KnobSpring))
	{
		stiffness->setValue(value);
		pluginInstance->getParameterDisplay((start+VstPlugin::KnobSpring), tempch);
		tempConverter << tempch;
		stiffnessEdit->setText(const_cast<char *>(tempConverter.str().c_str()));
	}
	else if(index == (start+VstPlugin::KnobVal))
		knob->setValue(value);
}

//-----------------------------------------------------------------------------
void KnobGui::show()
{
	if(!isDisplayed)
	{
		typeMenu->setCurrent(2);

		modContainer->getViewContainer()->addView(background);
		modContainer->getViewContainer()->addView(typeMenu);
		modContainer->getViewContainer()->addView(typeButton);
		modContainer->getViewContainer()->addView(knob);
		modContainer->getViewContainer()->addView(mass);
		modContainer->getViewContainer()->addView(massEdit);
		modContainer->getViewContainer()->addView(stiffness);
		modContainer->getViewContainer()->addView(stiffnessEdit);

		isDisplayed = true;
	}
}

//-----------------------------------------------------------------------------
void KnobGui::hide()
{
	if(isDisplayed)
	{
		modContainer->getViewContainer()->removeView(background, false);
		modContainer->getViewContainer()->removeView(typeMenu, false);
		modContainer->getViewContainer()->removeView(typeButton, false);
		modContainer->getViewContainer()->removeView(knob, false);
		modContainer->getViewContainer()->removeView(mass, false);
		modContainer->getViewContainer()->removeView(massEdit, false);
		modContainer->getViewContainer()->removeView(stiffness, false);
		modContainer->getViewContainer()->removeView(stiffnessEdit, false);

		isDisplayed = false;
	}
}

//-----------------------------------------------------------------------------
void KnobGui::timerCallback()
{
	/*static float tempf = 0.0f;
	knob->setExtraTick(tempf);*/

	knob->setExtraTick(plugin->getKnobPosition(start));
	knob->setDirty();

	/*tempf += 0.01f;
	if(tempf > 1.0f)
		tempf = 0.0f;*/
}
