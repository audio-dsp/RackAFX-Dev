//	LfoGui.cpp - GUI for the LFO mod source.
//	----------------------------------------------------------------------------
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
//	----------------------------------------------------------------------------

#include "LfoGui.h"
#include "ModContainer.h"
#include "CButtonMenu.h"
#include "LFOBarEditor.h"

#include <cmath>
#include <sstream>

using namespace std;

//------------------------------------------------------------------------------
LfoGui::LfoGui(ModContainer *container,
			   int tagStart,
			   CControlListener *listener,
			   VstPlugin *pluginInstance):
ModSourceGui(container, tagStart, listener, pluginInstance),
modContainer(container),
isDisplayed(false),
start(tagStart),
background(0),
typeMenu(0),
typeButton(0),
shape(0),
numSteps(0),
presetMenu(0),
frequency(0),
smooth(0),
freqRange(0),
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
	buttonImage = new CBitmap(ButtonImage);

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
	typeMenu->setCurrent(0);

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
	size.x = 3;
	size.y = 24;
	size.right = size.x + 162;
	size.bottom = size.y + 97;
	offset.x = size.x;
	offset.y = size.y;
	shape = new LFOBarEditor(size,
							 listener,
							 (tagStart+VstPlugin::LFOStep0),
							 backgroundImage,
							 offset);
	for(i=(start+VstPlugin::LFOStep0);i<(start+VstPlugin::LFOStep31);++i)
		setParameter(i, pluginInstance->getParameter(i));

	//Num steps menu.
	size.x = 137;
	size.y = 5;
	size.right = size.x + 19;
	size.bottom = size.y + 11;
	numSteps = new COptionMenu(size,
							   listener,
							   (tagStart+VstPlugin::LFONumSteps));
	numSteps->addEntry("32");
	numSteps->addEntry("16");
	numSteps->addEntry("12");
	numSteps->addEntry("8");
	numSteps->addEntry("6");
	numSteps->addEntry("4");
	numSteps->addEntry("3");
	numSteps->addEntry("2");
	numSteps->setFontColor(tempcolour);
	numSteps->setBackColor(kWhiteCColor);
	numSteps->setFrameColor(kWhiteCColor);
	numSteps->setFont(kNormalFontVerySmall);
	//numSteps->setCurrent((int)pluginInstance->getParameter(tagStart+VstPlugin::LFONumSteps));
	//numSteps->setCurrent(round(pluginInstance->getParameter(tagStart+VstPlugin::LFONumSteps)));
	setParameter(tagStart+VstPlugin::LFONumSteps,
				 pluginInstance->getParameter(tagStart+VstPlugin::LFONumSteps));

	//Presets menu.
	size.x = 244;
	size.y = 2;
	size.right = size.x + 18;
	size.bottom = size.y + 17;
	presetMenu = new CButtonMenu(size,
								 listener,
								 (tagStart+VstPlugin::LFOPreset),
								 comboImage);
	presetMenu->addEntry("sine");
	presetMenu->addEntry("saw");
	presetMenu->addEntry("square");
	presetMenu->addEntry("ramp");
	presetMenu->addEntry("random");

	//Frequency knob.
	size.x = 176;
	size.y = 33;
	size.right = size.x + 26;
	size.bottom = size.y + 26;
	offset.x = size.x;
	offset.y = size.y;
	frequency = new CKnob(size,
						  listener,
						  (tagStart+VstPlugin::LFOFreq),
						  backgroundImage,
						  knobImage,
						  offset);
	frequency->setValue(pluginInstance->getParameter(tagStart+VstPlugin::LFOFreq));

	//Frequency text editor.
	size.x = 169;
	size.y = 72;
	size.right = size.x + 38;
	size.bottom = size.y + 10;
	greyColour(128, 128, 128, 255);
	frequencyEdit = new CTextEdit(size, listener, (tagStart+VstPlugin::LFOFreq));
	frequencyEdit->setBackColor(kWhiteCColor);
	frequencyEdit->setFrameColor(kWhiteCColor);
	frequencyEdit->setFontColor(greyColour);
	frequencyEdit->setFont(kNormalFontVerySmall);
	/*setParameter((tagStart+VstPlugin::LFOFreq),
				 effect->getParameter((tagStart+VstPlugin::LFOFreq)));*/

	//Smooth knob.
	size.x = 229;
	size.y = 33;
	size.right = size.x + 26;
	size.bottom = size.y + 26;
	offset.x = size.x;
	offset.y = size.y;
	smooth = new CKnob(size,
					   listener,
					   (tagStart+VstPlugin::LFOSmooth),
					   backgroundImage,
					   knobImage,
					   offset);
	smooth->setValue(pluginInstance->getParameter(tagStart+VstPlugin::LFOSmooth));

	//Smooth text editor.
	size.x = 222;
	size.y = 72;
	size.right = size.x + 38;
	size.bottom = size.y + 10;
	greyColour(128, 128, 128, 255);
	smoothEdit = new CTextEdit(size, listener, (tagStart+VstPlugin::LFOSmooth));
	smoothEdit->setBackColor(kWhiteCColor);
	smoothEdit->setFrameColor(kWhiteCColor);
	smoothEdit->setFontColor(greyColour);
	smoothEdit->setFont(kNormalFontVerySmall);
	setParameter((tagStart+VstPlugin::LFOSmooth),
				 effect->getParameter((tagStart+VstPlugin::LFOSmooth)));

	//Freq range button.
	size.x = 172;
	size.y = 105;
	size.right = size.x + 16;
	size.bottom = size.y + 16;
	freqRange = new COnOffButton(size,
								 listener,
								 (tagStart+VstPlugin::LFOFreqRange),
								 buttonImage);
	freqRange->setValue(pluginInstance->getParameter(tagStart+VstPlugin::LFOFreqRange));

	setParameter((tagStart+VstPlugin::LFOFreq),
				 effect->getParameter((tagStart+VstPlugin::LFOFreq)));

	//Decrement reference count for all bitmaps.
	backgroundImage->forget();
	comboImage->forget();
	knobImage->forget();
	buttonImage->forget();
}

//------------------------------------------------------------------------------
LfoGui::~LfoGui()
{
	if(isDisplayed)
		hide();

	background->forget();
	typeMenu->forget();
	typeButton->forget();
	shape->forget();
	numSteps->forget();
	presetMenu->forget();
	frequency->forget();
	frequencyEdit->forget();
	smooth->forget();
	smoothEdit->forget();
	freqRange->forget();
}

//------------------------------------------------------------------------------
void LfoGui::setParameter(int index, float value, CDrawContext *context)
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
	else if(index == (start+VstPlugin::LFONumSteps))
	{
		numSteps->setValue(value);

		switch((int)value)
		{
			case 0:
				shape->setNumBars(32);
				break;
			case 1:
				shape->setNumBars(16);
				break;
			case 2:
				shape->setNumBars(12);
				break;
			case 3:
				shape->setNumBars(8);
				break;
			case 4:
				shape->setNumBars(6);
				break;
			case 5:
				shape->setNumBars(4);
				break;
			case 6:
				shape->setNumBars(3);
				break;
			case 7:
				shape->setNumBars(2);
				break;
		}
	}
	else if(index == (start+VstPlugin::LFOPreset))
	{
		int i;
		float values[32];
		int numStepsTemp;

		switch((int)numSteps->getValue())
		{
			case 0:
				numStepsTemp = 32;
				break;
			case 1:
				numStepsTemp = 16;
				break;
			case 2:
				numStepsTemp = 12;
				break;
			case 3:
				numStepsTemp = 8;
				break;
			case 4:
				numStepsTemp = 6;
				break;
			case 5:
				numStepsTemp = 4;
				break;
			case 6:
				numStepsTemp = 3;
				break;
			case 7:
				numStepsTemp = 2;
				break;
		}

		presetMenu->setValue(value);

		if(value == 0.0f)
		{
			for(i=0;i<numStepsTemp;++i)
			{
				values[i] = sinf(((float)i/(float)numStepsTemp) * 6.283185307179586476925286766559f);
				values[i] *= 0.5f;
				values[i] += 0.5f;
			}
		}
		else if(value == 1.0f)
		{
			for(i=0;i<numStepsTemp;++i)
				values[i] = (1.0f-((float)i/(float)numStepsTemp));
		}
		else if(value == 2.0f)
		{
			for(i=0;i<(numStepsTemp/2);++i)
				values[i] = 1.0f;
			for(i=(numStepsTemp/2);i<numStepsTemp;++i)
				values[i] = 0.0f;
		}
		else if(value == 3.0f)
		{
			for(i=0;i<numStepsTemp;++i)
				values[i] = (float)i/(float)numStepsTemp;
		}
		else
		{
			for(i=0;i<numStepsTemp;++i)
				values[i] = (float)rand()/(float)RAND_MAX;
		}
		shape->setValue(values, context);
	}
	else if(index == (start+VstPlugin::LFOFreq))
	{
		frequency->setValue(value);
		pluginInstance->getParameterDisplay((start+VstPlugin::LFOFreq), tempch);
		tempConverter << tempch;
		if((!pluginInstance->isSynced()) || (freqRange->getValue() > 0.5f))
			tempConverter << "Hz";
		frequencyEdit->setText(const_cast<char *>(tempConverter.str().c_str()));
	}
	else if(index == (start+VstPlugin::LFOSmooth))
	{
		smooth->setValue(value);
		pluginInstance->getParameterDisplay((start+VstPlugin::LFOSmooth), tempch);
		tempConverter << tempch;
		smoothEdit->setText(const_cast<char *>(tempConverter.str().c_str()));
	}
	else if(index == (start+VstPlugin::LFOFreqRange))
	{
		freqRange->setValue(value);
		setParameter((start+VstPlugin::LFOFreq), frequency->getValue(), context);
	}
	else if((index > (start+VstPlugin::LFOFreqRange)) &&
		(index <= (start+VstPlugin::LFOStep31)))
	{
		int tempint = index - (start+VstPlugin::LFOStep0);

		shape->setValue(tempint, value);
	}
	else if(index == VstPlugin::HostSync)
	{
		setParameter(start+VstPlugin::LFOFreq,
					 effect->getParameter(start+VstPlugin::LFOFreq));
	}
}

//------------------------------------------------------------------------------
void LfoGui::show()
{
	if(!isDisplayed)
	{
		typeMenu->setCurrent(0);

		modContainer->getViewContainer()->addView(background);
		modContainer->getViewContainer()->addView(typeMenu);
		modContainer->getViewContainer()->addView(typeButton);
		modContainer->getViewContainer()->addView(shape);
		modContainer->getViewContainer()->addView(numSteps);
		modContainer->getViewContainer()->addView(presetMenu);
		modContainer->getViewContainer()->addView(frequency);
		modContainer->getViewContainer()->addView(frequencyEdit);
		modContainer->getViewContainer()->addView(smooth);
		modContainer->getViewContainer()->addView(smoothEdit);
		modContainer->getViewContainer()->addView(freqRange);

		isDisplayed = true;
	}
}

//------------------------------------------------------------------------------
void LfoGui::hide()
{
	if(isDisplayed)
	{
		modContainer->getViewContainer()->removeView(background, false);
		modContainer->getViewContainer()->removeView(typeMenu, false);
		modContainer->getViewContainer()->removeView(typeButton, false);
		modContainer->getViewContainer()->removeView(shape, false);
		modContainer->getViewContainer()->removeView(numSteps, false);
		modContainer->getViewContainer()->removeView(presetMenu, false);
		modContainer->getViewContainer()->removeView(frequency, false);
		modContainer->getViewContainer()->removeView(frequencyEdit, false);
		modContainer->getViewContainer()->removeView(smooth, false);
		modContainer->getViewContainer()->removeView(smoothEdit, false);
		modContainer->getViewContainer()->removeView(freqRange, false);

		isDisplayed = false;
	}
}
