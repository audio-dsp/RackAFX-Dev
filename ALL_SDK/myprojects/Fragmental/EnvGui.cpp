//	EnvGui.cpp - GUI for the envelope detector mod source.
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
//	The above copyright notice and thid permission notice shall be included in
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

#include "EnvGui.h"
#include "ModContainer.h"
#include "VstPlugin.h"
#include "SixPointLineEditor.h"

#include <sstream>

using namespace std;

//-----------------------------------------------------------------------------
EnvGui::EnvGui(ModContainer *container,
			   int tagStart,
			   CControlListener *listener,
			   VstPlugin *pluginInstance):
ModSourceGui(container, tagStart, listener, pluginInstance),
modContainer(container),
isDisplayed(false),
start(tagStart),
typeMenu(0),
typeButton(0),
shape(0),
attack(0),
attackEdit(0),
decay(0),
decayEdit(0),
hold(0),
holdEdit(0),
effect(pluginInstance)
{
	int i;
	CPoint offset(0, 0);
	CRect size;
	CColor tempcolour = {128, 128, 128, 255};
	CColor greyColour;

	//Create bitmaps.
	backgroundImage = new CBitmap(BackgroundImage);
	comboImage = new CBitmap(ComboImage);
	knobImage = new CBitmap(KnobImage);

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
	typeMenu->setCurrent(1);

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

	//Shape editor.
	size.x = 4;
	size.y = 25;
	size.right = size.x + 149;
	size.bottom = size.y + 95;
	offset.x = size.x;
	offset.y = size.y;
	shape = new SixPointLineEditor(size,
								   listener,
								   (tagStart+VstPlugin::EnvPoint0y),
								   backgroundImage,
								   offset);
	for(i=0;i<10;++i)
	{
		shape->setValue(i,
						pluginInstance->getParameter(tagStart+VstPlugin::EnvPoint0y+i));
	}

	//Attack knob.
	size.x = 169;
	size.y = 67;
	size.right = size.x + 26;
	size.bottom = size.y + 26;
	offset.x = size.x;
	offset.y = size.y;
	attack = new CKnob(size,
					   listener,
					   (tagStart+VstPlugin::EnvAttack),
					   backgroundImage,
					   knobImage,
					   offset);
	attack->setValue(pluginInstance->getParameter(tagStart+VstPlugin::EnvAttack));

	//Attack text editor.
	size.x = 163;
	size.y = 100;
	size.right = size.x + 38;
	size.bottom = size.y + 10;
	greyColour(128, 128, 128, 255);
	attackEdit = new CTextEdit(size, listener, (tagStart+VstPlugin::EnvAttack));
	attackEdit->setBackColor(kWhiteCColor);
	attackEdit->setFrameColor(kWhiteCColor);
	attackEdit->setFontColor(greyColour);
	attackEdit->setFont(kNormalFontVerySmall);
	setParameter((tagStart+VstPlugin::EnvAttack),
				 effect->getParameter((tagStart+VstPlugin::EnvAttack)));

	//Decay knob.
	size.x = 222;
	size.y = 67;
	size.right = size.x + 26;
	size.bottom = size.y + 26;
	offset.x = size.x;
	offset.y = size.y;
	decay = new CKnob(size,
					  listener,
					  (tagStart+VstPlugin::EnvDecay),
					  backgroundImage,
					  knobImage,
					  offset);
	decay->setValue(pluginInstance->getParameter(tagStart+VstPlugin::EnvDecay));

	//Decay text editor.
	size.x = 216;
	size.y = 100;
	size.right = size.x + 38;
	size.bottom = size.y + 10;
	greyColour(128, 128, 128, 255);
	decayEdit = new CTextEdit(size, listener, (tagStart+VstPlugin::EnvDecay));
	decayEdit->setBackColor(kWhiteCColor);
	decayEdit->setFrameColor(kWhiteCColor);
	decayEdit->setFontColor(greyColour);
	decayEdit->setFont(kNormalFontVerySmall);
	setParameter((tagStart+VstPlugin::EnvDecay),
				 effect->getParameter((tagStart+VstPlugin::EnvDecay)));

	//Hold knob.
	size.x = 223;
	size.y = 4;
	size.right = size.x + 26;
	size.bottom = size.y + 26;
	offset.x = size.x;
	offset.y = size.y;
	hold = new CKnob(size,
					 listener,
					 (tagStart+VstPlugin::EnvHold),
					 backgroundImage,
					 knobImage,
					 offset);
	hold->setValue(pluginInstance->getParameter(tagStart+VstPlugin::EnvHold));

	//Hold text editor.
	size.x = 217;
	size.y = 37;
	size.right = size.x + 38;
	size.bottom = size.y + 10;
	greyColour(128, 128, 128, 255);
	holdEdit = new CTextEdit(size, listener, (tagStart+VstPlugin::EnvHold));
	holdEdit->setBackColor(kWhiteCColor);
	holdEdit->setFrameColor(kWhiteCColor);
	holdEdit->setFontColor(greyColour);
	holdEdit->setFont(kNormalFontVerySmall);
	setParameter((tagStart+VstPlugin::EnvHold),
				 effect->getParameter((tagStart+VstPlugin::EnvHold)));

	//Decrement reference count for all bitmaps.
	backgroundImage->forget();
	comboImage->forget();
	knobImage->forget();
}

//-----------------------------------------------------------------------------
EnvGui::~EnvGui()
{
	if(isDisplayed)
		hide();

	background->forget();
	typeMenu->forget();
	typeButton->forget();
	shape->forget();
	attack->forget();
	attackEdit->forget();
	decay->forget();
	decayEdit->forget();
	hold->forget();
	holdEdit->forget();
}

//-----------------------------------------------------------------------------
void EnvGui::setParameter(int index, float value, CDrawContext *context)
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
	else if(index == (start+VstPlugin::EnvAttack))
	{
		attack->setValue(value);
		pluginInstance->getParameterDisplay((start+VstPlugin::EnvAttack), tempch);
		tempConverter << tempch << " s";
		attackEdit->setText(const_cast<char *>(tempConverter.str().c_str()));
	}
	else if(index == (start+VstPlugin::EnvHold))
	{
		hold->setValue(value);
		pluginInstance->getParameterDisplay((start+VstPlugin::EnvHold), tempch);
		tempConverter << tempch << " s";
		holdEdit->setText(const_cast<char *>(tempConverter.str().c_str()));
	}
	else if(index == (start+VstPlugin::EnvDecay))
	{
		decay->setValue(value);
		pluginInstance->getParameterDisplay((start+VstPlugin::EnvDecay), tempch);
		tempConverter << tempch << " s";
		decayEdit->setText(const_cast<char *>(tempConverter.str().c_str()));
	}
	else if(index == (start+VstPlugin::EnvPoint0y))
		shape->setValue(0, value);
	else if(index == (start+VstPlugin::EnvPoint1x))
		shape->setValue(1, value);
	else if(index == (start+VstPlugin::EnvPoint1y))
		shape->setValue(2, value);
	else if(index == (start+VstPlugin::EnvPoint2x))
		shape->setValue(3, value);
	else if(index == (start+VstPlugin::EnvPoint2y))
		shape->setValue(4, value);
	else if(index == (start+VstPlugin::EnvPoint3x))
		shape->setValue(5, value);
	else if(index == (start+VstPlugin::EnvPoint3y))
		shape->setValue(6, value);
	else if(index == (start+VstPlugin::EnvPoint4x))
		shape->setValue(7, value);
	else if(index == (start+VstPlugin::EnvPoint4y))
		shape->setValue(8, value);
	else if(index == (start+VstPlugin::EnvPoint5y))
		shape->setValue(9, value);
}

//-----------------------------------------------------------------------------
void EnvGui::show()
{
	if(!isDisplayed)
	{
		typeMenu->setCurrent(1);

		modContainer->getViewContainer()->addView(background);
		modContainer->getViewContainer()->addView(typeMenu);
		modContainer->getViewContainer()->addView(typeButton);
		modContainer->getViewContainer()->addView(shape);
		modContainer->getViewContainer()->addView(attack);
		modContainer->getViewContainer()->addView(attackEdit);
		modContainer->getViewContainer()->addView(decay);
		modContainer->getViewContainer()->addView(decayEdit);
		modContainer->getViewContainer()->addView(hold);
		modContainer->getViewContainer()->addView(holdEdit);

		isDisplayed = true;
	}
}

//-----------------------------------------------------------------------------
void EnvGui::hide()
{
	if(isDisplayed)
	{
		modContainer->getViewContainer()->removeView(background, false);
		modContainer->getViewContainer()->removeView(typeMenu, false);
		modContainer->getViewContainer()->removeView(typeButton, false);
		modContainer->getViewContainer()->removeView(shape, false);
		modContainer->getViewContainer()->removeView(attack, false);
		modContainer->getViewContainer()->removeView(attackEdit, false);
		modContainer->getViewContainer()->removeView(decay, false);
		modContainer->getViewContainer()->removeView(decayEdit, false);
		modContainer->getViewContainer()->removeView(hold, false);
		modContainer->getViewContainer()->removeView(holdEdit, false);

		isDisplayed = false;
	}
}
