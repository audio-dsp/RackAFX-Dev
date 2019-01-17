//	ModEditor.cpp - The VSTGUI editor handling the modulation parameters.
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

#include "ModEditor.h"
#include "LfoGui.h"
#include "VstPlugin.h"

#include <sstream>

using namespace std;

//-----------------------------------------------------------------------------
ModEditor::ModEditor(AudioEffect *effect):
AEffGUIEditor(effect),
container(0),
redButton(0),
greenButton(0),
blueButton(0),
redSource(0),
greenSource(0),
blueSource(0),
displayedSource(0)
{
	backgroundImage = new CBitmap(BackgroundImage);

	//Init the size of the editor.
	rect.left   = 0;
	rect.top    = 0;
	rect.right  = static_cast<short>(backgroundImage->getWidth());
	rect.bottom = static_cast<short>(backgroundImage->getHeight());
}

//-----------------------------------------------------------------------------
ModEditor::~ModEditor()
{
	//Decrement backgroundImage's reference count (because we initialised it
	//to 1 in it's constructor).  This will result in it being deleted.
	backgroundImage->forget();
}

//-----------------------------------------------------------------------------
bool ModEditor::open(void *systemPointer)
{
	CPoint offset;
	int tempint;
	float tempf;

	AEffEditor::open(systemPointer);

	//Initialise the frame.
	CRect size(0,
			   0,
			   backgroundImage->getWidth(),
			   backgroundImage->getHeight());
	frame = new CFrame(size, systemPointer, this);
	frame->setBackground(backgroundImage);

	//Load images.
	redImage = new CBitmap(RedImage);
	greenImage = new CBitmap(GreenImage);
	blueImage = new CBitmap(BlueImage);

	//Blue source button.
	size.x = 1;
	size.y = 0;
	size.right = size.x + 25;
	size.bottom = size.y + 22;
	blueButton = new COnOffButton(size,
								  this,
								  VstPlugin::ModSelect,
								  blueImage);
	blueButton->setValue(1.0f);
	frame->addView(blueButton);

	//Green source button.
	size.x = 45;
	size.y = 0;
	size.right = size.x + 25;
	size.bottom = size.y + 22;
	greenButton = new COnOffButton(size,
								   this,
								   VstPlugin::ModSelect,
								   greenImage);
	frame->addView(greenButton);

	//Red source button.
	size.x = 23;
	size.y = 0;
	size.right = size.x + 25;
	size.bottom = size.y + 22;
	redButton = new COnOffButton(size,
								 this,
								 VstPlugin::ModSelect,
								 redImage);
	frame->addView(redButton);

	//Initialise CViewContainer for the mod sources to go in.
	size.x = 4;
	size.y = 22;
	size.right = size.x + 264;
	size.bottom = size.y + 124;
	container = new CViewContainer(size, frame);
	frame->addView(container);

	//Initialise red mod source.
	redSource = new ModContainer(VstPlugin::ModRed,
								 container,
								 this,
								 (VstPlugin *)effect);
	tempint = (int)(effect->getParameter(VstPlugin::ModRed + VstPlugin::Type)*2);
	redSource->setGui(tempint);

	//Initialise green mod source.
	greenSource = new ModContainer(VstPlugin::ModGreen,
								   container,
								   this,
								   (VstPlugin *)effect);
	tempint = (int)(effect->getParameter(VstPlugin::ModGreen + VstPlugin::Type)*2);
	greenSource->setGui(tempint);

	//Initialise blue mod source.
	blueSource = new ModContainer(VstPlugin::ModBlue,
								  container,
								  this,
								  (VstPlugin *)effect);
	tempint = (int)(effect->getParameter(VstPlugin::ModBlue + VstPlugin::Type)*2);
	blueSource->setGui(tempint);

	tempf = effect->getParameter(VstPlugin::ModSelect);
	if(tempf < 0.5f)
	{
		blueSource->show();
		displayedSource = 0;
	}
	else if(tempf < 1.0f)
	{
		redSource->show();
		displayedSource = 1;
	}
	else
	{
		greenSource->show();
		displayedSource = 2;
	}

	redImage->forget();
	greenImage->forget();
	blueImage->forget();

	return true;
}

//-----------------------------------------------------------------------------
void ModEditor::close()
{
	switch(displayedSource)
	{
		case 0:
			blueSource->hide();
			break;
		case 1:
			redSource->hide();
			break;
		case 2:
			greenSource->hide();
			break;
	}
	delete blueSource;
	delete redSource;
	delete greenSource;
	blueSource = 0;
	redSource = 0;
	greenSource = 0;

	//Delete everything else.
	delete frame;
	frame = 0;
}

//-----------------------------------------------------------------------------
void ModEditor::setParameter(VstInt32 index, float value)
{
	if(frame)
	{
		if(index == (VstPlugin::ModRed + VstPlugin::Type))
		{
			int tempint;

			if(value < 0.33f)
				tempint = 0;
			else if(value < 0.66f)
				tempint = 1;
			else
				tempint = 2;

			redSource->setGui(tempint);
		}
		else if(index == (VstPlugin::ModGreen + VstPlugin::Type))
		{
			int tempint;

			if(value < 0.33f)
				tempint = 0;
			else if(value < 0.66f)
				tempint = 1;
			else
				tempint = 2;

			greenSource->setGui(tempint);
		}
		else if(index == (VstPlugin::ModBlue + VstPlugin::Type))
		{
			int tempint;

			if(value < 0.33f)
				tempint = 0;
			else if(value < 0.66f)
				tempint = 1;
			else
				tempint = 2;

			blueSource->setGui(tempint);
		}
		else if(index == VstPlugin::ModSelect)
		{
			if(value < 0.5f)
			{
				redButton->setValue(0.0f);
				greenButton->setValue(0.0f);
				blueButton->setValue(1.0f);

				switch(displayedSource)
				{
					case 1:
						redSource->hide();
						blueSource->show();
						break;
					case 2:
						greenSource->hide();
						blueSource->show();
						break;
				}
				displayedSource = 0;
			}
			else if(value < 1.0f)
			{
				redButton->setValue(1.0f);
				greenButton->setValue(0.0f);
				blueButton->setValue(0.0f);

				switch(displayedSource)
				{
					case 0:
						blueSource->hide();
						redSource->show();
						break;
					case 2:
						greenSource->hide();
						redSource->show();
						break;
				}
				displayedSource = 1;
			}
			else
			{
				redButton->setValue(0.0f);
				greenButton->setValue(1.0f);
				blueButton->setValue(0.0f);

				switch(displayedSource)
				{
					case 0:
						blueSource->hide();
						greenSource->show();
						break;
					case 1:
						redSource->hide();
						greenSource->show();
						break;
				}
				displayedSource = 2;
			}
		}

		redSource->setParameter(index, value);
		greenSource->setParameter(index, value);
		blueSource->setParameter(index, value);
	}
}

//-----------------------------------------------------------------------------
void ModEditor::valueChanged(CDrawContext *context, CControl *control)
{
	VstPlugin *pluginInstance = (VstPlugin *)effect;

	if(control->getTag() == (VstPlugin::ModRed + VstPlugin::TypeKick))
	{
		redSource->setParameter(control->getTag(),
								control->getValue(),
								context);
		effect->setParameterAutomated(control->getTag(), control->getValue());
	}
	else if(control->getTag() == (VstPlugin::ModGreen + VstPlugin::TypeKick))
	{
		greenSource->setParameter(control->getTag(),
								  control->getValue(),
								  context);
		effect->setParameterAutomated(control->getTag(), control->getValue());
	}
	else if(control->getTag() == (VstPlugin::ModBlue + VstPlugin::TypeKick))
	{
		blueSource->setParameter(control->getTag(),
								 control->getValue(),
								 context);
		effect->setParameterAutomated(control->getTag(), control->getValue());
	}
	else if(control->getTag() == (VstPlugin::ModRed + VstPlugin::Type))
		effect->setParameterAutomated(control->getTag(), (control->getValue()*0.5f));
	else if(control->getTag() == (VstPlugin::ModGreen + VstPlugin::Type))
		effect->setParameterAutomated(control->getTag(), (control->getValue()*0.5f));
	else if(control->getTag() == (VstPlugin::ModBlue + VstPlugin::Type))
		effect->setParameterAutomated(control->getTag(), (control->getValue()*0.5f));
	else if(control == blueButton)
		effect->setParameterAutomated(VstPlugin::ModSelect, 0.0f);
	else if(control == redButton)
		effect->setParameterAutomated(VstPlugin::ModSelect, 0.5f);
	else if(control == greenButton)
		effect->setParameterAutomated(VstPlugin::ModSelect, 1.0f);
	else if((control->getTag() == (VstPlugin::ModBlue+VstPlugin::LFOFreq) &&
			(dynamic_cast<CTextEdit *>(control) != 0)))
	{
		float tempf;
		char tempch[16];
		stringstream tempConverter;
		CTextEdit *tempControl = dynamic_cast<CTextEdit *>(control);

		tempControl->getText(tempch);
		tempConverter << tempch;
		tempConverter >> tempf;

		if((!pluginInstance->isSynced()) ||
		   (pluginInstance->getParameter(VstPlugin::ModBlue+VstPlugin::LFOFreqRange) > 0.5f))
		{
			if(effect->getParameter(VstPlugin::ModBlue+VstPlugin::LFOFreqRange) < 0.5f)
			{
				tempf -= 0.0001f;
				tempf /= 19.9999f;
			}
			else
			{
				tempf -= 20.0f;
				tempf /= 20000.0f;
			}
			tempf = 1.0f-(fakeExp(1.0f-tempf));

			if(tempf > 1.0f)
				tempf = 1.0f;
			else if(tempf < 0.0f)
				tempf = 0.0f;
		}
		else
			tempf = getCorrectBeat(tempf);
		effect->setParameterAutomated(control->getTag(), tempf);
	}
	else if((control->getTag() == (VstPlugin::ModRed+VstPlugin::LFOFreq) &&
			(dynamic_cast<CTextEdit *>(control) != 0)))
	{
		float tempf;
		char tempch[16];
		stringstream tempConverter;
		CTextEdit *tempControl = dynamic_cast<CTextEdit *>(control);

		tempControl->getText(tempch);
		tempConverter << tempch;
		tempConverter >> tempf;

		if((!pluginInstance->isSynced()) ||
		   (pluginInstance->getParameter(VstPlugin::ModRed+VstPlugin::LFOFreqRange) > 0.5f))
		{
			if(effect->getParameter(VstPlugin::ModRed+VstPlugin::LFOFreqRange) < 0.5f)
			{
				tempf -= 0.0001f;
				tempf /= 19.9999f;
			}
			else
			{
				tempf -= 20.0f;
				tempf /= 20000.0f;
			}
			tempf = 1.0f-(fakeExp(1.0f-tempf));

			if(tempf > 1.0f)
				tempf = 1.0f;
			else if(tempf < 0.0f)
				tempf = 0.0f;
		}
		else
			tempf = getCorrectBeat(tempf);
		effect->setParameterAutomated(control->getTag(), tempf);
	}
	else if((control->getTag() == (VstPlugin::ModGreen+VstPlugin::LFOFreq) &&
			(dynamic_cast<CTextEdit *>(control) != 0)))
	{
		float tempf;
		char tempch[16];
		stringstream tempConverter;
		CTextEdit *tempControl = dynamic_cast<CTextEdit *>(control);

		tempControl->getText(tempch);
		tempConverter << tempch;
		tempConverter >> tempf;

		if((!pluginInstance->isSynced()) ||
		   (pluginInstance->getParameter(VstPlugin::ModGreen+VstPlugin::LFOFreqRange) > 0.5f))
		{
			if(effect->getParameter(VstPlugin::ModGreen+VstPlugin::LFOFreqRange) < 0.5f)
			{
				tempf -= 0.0001f;
				tempf /= 19.9999f;
			}
			else
			{
				tempf -= 20.0f;
				tempf /= 20000.0f;
			}
			tempf = 1.0f-(fakeExp(1.0f-tempf));

			if(tempf > 1.0f)
				tempf = 1.0f;
			else if(tempf < 0.0f)
				tempf = 0.0f;
		}
		else
			tempf = getCorrectBeat(tempf);
		effect->setParameterAutomated(control->getTag(), tempf);
	}
	else if((control->getTag() == (VstPlugin::ModBlue+VstPlugin::LFOSmooth) &&
			(dynamic_cast<CTextEdit *>(control) != 0)))
	{
		float tempf;
		char tempch[16];
		stringstream tempConverter;
		CTextEdit *tempControl = dynamic_cast<CTextEdit *>(control);

		tempControl->getText(tempch);
		tempConverter << tempch;
		tempConverter >> tempf;
		if(tempf > 1.0f)
			tempf = 1.0f;
		else if(tempf < 0.0f)
			tempf = 0.0f;
		effect->setParameterAutomated(control->getTag(), tempf);
	}
	else if((control->getTag() == (VstPlugin::ModRed+VstPlugin::LFOSmooth) &&
			(dynamic_cast<CTextEdit *>(control) != 0)))
	{
		float tempf;
		char tempch[16];
		stringstream tempConverter;
		CTextEdit *tempControl = dynamic_cast<CTextEdit *>(control);

		tempControl->getText(tempch);
		tempConverter << tempch;
		tempConverter >> tempf;
		if(tempf > 1.0f)
			tempf = 1.0f;
		else if(tempf < 0.0f)
			tempf = 0.0f;
		effect->setParameterAutomated(control->getTag(), tempf);
	}
	else if((control->getTag() == (VstPlugin::ModGreen+VstPlugin::LFOSmooth) &&
			(dynamic_cast<CTextEdit *>(control) != 0)))
	{
		float tempf;
		char tempch[16];
		stringstream tempConverter;
		CTextEdit *tempControl = dynamic_cast<CTextEdit *>(control);

		tempControl->getText(tempch);
		tempConverter << tempch;
		tempConverter >> tempf;
		if(tempf > 1.0f)
			tempf = 1.0f;
		else if(tempf < 0.0f)
			tempf = 0.0f;
		effect->setParameterAutomated(control->getTag(), tempf);
	}
	else if((control->getTag() == (VstPlugin::ModBlue+VstPlugin::EnvAttack) &&
			(dynamic_cast<CTextEdit *>(control) != 0)))
	{
		float tempf;
		char tempch[16];
		stringstream tempConverter;
		CTextEdit *tempControl = dynamic_cast<CTextEdit *>(control);

		tempControl->getText(tempch);
		tempConverter << tempch;
		tempConverter >> tempf;
		if(tempf > 1.0f)
			tempf = 1.0f;
		else if(tempf < 0.0f)
			tempf = 0.0f;
		effect->setParameterAutomated(control->getTag(), tempf);
	}
	else if((control->getTag() == (VstPlugin::ModRed+VstPlugin::EnvAttack) &&
			(dynamic_cast<CTextEdit *>(control) != 0)))
	{
		float tempf;
		char tempch[16];
		stringstream tempConverter;
		CTextEdit *tempControl = dynamic_cast<CTextEdit *>(control);

		tempControl->getText(tempch);
		tempConverter << tempch;
		tempConverter >> tempf;
		if(tempf > 1.0f)
			tempf = 1.0f;
		else if(tempf < 0.0f)
			tempf = 0.0f;
		effect->setParameterAutomated(control->getTag(), tempf);
	}
	else if((control->getTag() == (VstPlugin::ModGreen+VstPlugin::EnvAttack) &&
			(dynamic_cast<CTextEdit *>(control) != 0)))
	{
		float tempf;
		char tempch[16];
		stringstream tempConverter;
		CTextEdit *tempControl = dynamic_cast<CTextEdit *>(control);

		tempControl->getText(tempch);
		tempConverter << tempch;
		tempConverter >> tempf;
		if(tempf > 1.0f)
			tempf = 1.0f;
		else if(tempf < 0.0f)
			tempf = 0.0f;
		effect->setParameterAutomated(control->getTag(), tempf);
	}
	else if((control->getTag() == (VstPlugin::ModBlue+VstPlugin::EnvDecay) &&
			(dynamic_cast<CTextEdit *>(control) != 0)))
	{
		float tempf;
		char tempch[16];
		stringstream tempConverter;
		CTextEdit *tempControl = dynamic_cast<CTextEdit *>(control);

		tempControl->getText(tempch);
		tempConverter << tempch;
		tempConverter >> tempf;
		if(tempf > 1.0f)
			tempf = 1.0f;
		else if(tempf < 0.0f)
			tempf = 0.0f;
		effect->setParameterAutomated(control->getTag(), tempf);
	}
	else if((control->getTag() == (VstPlugin::ModRed+VstPlugin::EnvDecay) &&
			(dynamic_cast<CTextEdit *>(control) != 0)))
	{
		float tempf;
		char tempch[16];
		stringstream tempConverter;
		CTextEdit *tempControl = dynamic_cast<CTextEdit *>(control);

		tempControl->getText(tempch);
		tempConverter << tempch;
		tempConverter >> tempf;
		if(tempf > 1.0f)
			tempf = 1.0f;
		else if(tempf < 0.0f)
			tempf = 0.0f;
		effect->setParameterAutomated(control->getTag(), tempf);
	}
	else if((control->getTag() == (VstPlugin::ModGreen+VstPlugin::EnvDecay) &&
			(dynamic_cast<CTextEdit *>(control) != 0)))
	{
		float tempf;
		char tempch[16];
		stringstream tempConverter;
		CTextEdit *tempControl = dynamic_cast<CTextEdit *>(control);

		tempControl->getText(tempch);
		tempConverter << tempch;
		tempConverter >> tempf;
		if(tempf > 1.0f)
			tempf = 1.0f;
		else if(tempf < 0.0f)
			tempf = 0.0f;
		effect->setParameterAutomated(control->getTag(), tempf);
	}
	else if((control->getTag() == (VstPlugin::ModBlue+VstPlugin::EnvHold) &&
			(dynamic_cast<CTextEdit *>(control) != 0)))
	{
		float tempf;
		char tempch[16];
		stringstream tempConverter;
		CTextEdit *tempControl = dynamic_cast<CTextEdit *>(control);

		tempControl->getText(tempch);
		tempConverter << tempch;
		tempConverter >> tempf;
		if(tempf > 1.0f)
			tempf = 1.0f;
		else if(tempf < 0.0f)
			tempf = 0.0f;
		effect->setParameterAutomated(control->getTag(), tempf);
	}
	else if((control->getTag() == (VstPlugin::ModRed+VstPlugin::EnvHold) &&
			(dynamic_cast<CTextEdit *>(control) != 0)))
	{
		float tempf;
		char tempch[16];
		stringstream tempConverter;
		CTextEdit *tempControl = dynamic_cast<CTextEdit *>(control);

		tempControl->getText(tempch);
		tempConverter << tempch;
		tempConverter >> tempf;
		if(tempf > 1.0f)
			tempf = 1.0f;
		else if(tempf < 0.0f)
			tempf = 0.0f;
		effect->setParameterAutomated(control->getTag(), tempf);
	}
	else if((control->getTag() == (VstPlugin::ModGreen+VstPlugin::EnvHold) &&
			(dynamic_cast<CTextEdit *>(control) != 0)))
	{
		float tempf;
		char tempch[16];
		stringstream tempConverter;
		CTextEdit *tempControl = dynamic_cast<CTextEdit *>(control);

		tempControl->getText(tempch);
		tempConverter << tempch;
		tempConverter >> tempf;
		if(tempf > 1.0f)
			tempf = 1.0f;
		else if(tempf < 0.0f)
			tempf = 0.0f;
		effect->setParameterAutomated(control->getTag(), tempf);
	}
	else if((control->getTag() == (VstPlugin::ModBlue+VstPlugin::KnobMass) &&
			(dynamic_cast<CTextEdit *>(control) != 0)))
	{
		float tempf;
		char tempch[16];
		stringstream tempConverter;
		CTextEdit *tempControl = dynamic_cast<CTextEdit *>(control);

		tempControl->getText(tempch);
		tempConverter << tempch;
		tempConverter >> tempf;

		tempf -= 0.01f;
		tempf /= 9.99f;

		if(tempf > 1.0f)
			tempf = 1.0f;
		else if(tempf < 0.0f)
			tempf = 0.0f;
		effect->setParameterAutomated(control->getTag(), tempf);
	}
	else if((control->getTag() == (VstPlugin::ModRed+VstPlugin::KnobMass) &&
			(dynamic_cast<CTextEdit *>(control) != 0)))
	{
		float tempf;
		char tempch[16];
		stringstream tempConverter;
		CTextEdit *tempControl = dynamic_cast<CTextEdit *>(control);

		tempControl->getText(tempch);
		tempConverter << tempch;
		tempConverter >> tempf;

		tempf -= 0.01f;
		tempf /= 9.99f;

		if(tempf > 1.0f)
			tempf = 1.0f;
		else if(tempf < 0.0f)
			tempf = 0.0f;
		effect->setParameterAutomated(control->getTag(), tempf);
	}
	else if((control->getTag() == (VstPlugin::ModGreen+VstPlugin::KnobMass) &&
			(dynamic_cast<CTextEdit *>(control) != 0)))
	{
		float tempf;
		char tempch[16];
		stringstream tempConverter;
		CTextEdit *tempControl = dynamic_cast<CTextEdit *>(control);

		tempControl->getText(tempch);
		tempConverter << tempch;
		tempConverter >> tempf;

		tempf -= 0.01f;
		tempf /= 9.99f;

		if(tempf > 1.0f)
			tempf = 1.0f;
		else if(tempf < 0.0f)
			tempf = 0.0f;
		effect->setParameterAutomated(control->getTag(), tempf);
	}
	else if((control->getTag() == (VstPlugin::ModBlue+VstPlugin::KnobSpring) &&
			(dynamic_cast<CTextEdit *>(control) != 0)))
	{
		float tempf;
		char tempch[16];
		stringstream tempConverter;
		CTextEdit *tempControl = dynamic_cast<CTextEdit *>(control);

		tempControl->getText(tempch);
		tempConverter << tempch;
		tempConverter >> tempf;

		tempf /= 10.0f;

		if(tempf > 1.0f)
			tempf = 1.0f;
		else if(tempf < 0.0f)
			tempf = 0.0f;
		effect->setParameterAutomated(control->getTag(), tempf);
	}
	else if((control->getTag() == (VstPlugin::ModRed+VstPlugin::KnobSpring) &&
			(dynamic_cast<CTextEdit *>(control) != 0)))
	{
		float tempf;
		char tempch[16];
		stringstream tempConverter;
		CTextEdit *tempControl = dynamic_cast<CTextEdit *>(control);

		tempControl->getText(tempch);
		tempConverter << tempch;
		tempConverter >> tempf;

		tempf /= 10.0f;

		if(tempf > 1.0f)
			tempf = 1.0f;
		else if(tempf < 0.0f)
			tempf = 0.0f;
		effect->setParameterAutomated(control->getTag(), tempf);
	}
	else if((control->getTag() == (VstPlugin::ModGreen+VstPlugin::KnobSpring) &&
			(dynamic_cast<CTextEdit *>(control) != 0)))
	{
		float tempf;
		char tempch[16];
		stringstream tempConverter;
		CTextEdit *tempControl = dynamic_cast<CTextEdit *>(control);

		tempControl->getText(tempch);
		tempConverter << tempch;
		tempConverter >> tempf;

		tempf /= 10.0f;

		if(tempf > 1.0f)
			tempf = 1.0f;
		else if(tempf < 0.0f)
			tempf = 0.0f;
		effect->setParameterAutomated(control->getTag(), tempf);
	}
	else
		effect->setParameterAutomated(control->getTag(), control->getValue());
}

//-----------------------------------------------------------------------------
void ModEditor::timerCallback()
{
	if(frame)
	{
		switch(displayedSource)
		{
			case 0:
				blueSource->timerCallback();
				break;
			case 1:
				redSource->timerCallback();
				break;
			case 2:
				greenSource->timerCallback();
				break;
		}
	}
}

//------------------------------------------------------------------------------
float ModEditor::getCorrectBeat(float val)
{
	float retval;

	if(val < (3.0f/64.0f))
		retval = 0.0f/15.0f;	//1/32
	else if(val < (3.0f/32.0f))
		retval = 1.0f/15.0f;	//1/16
	else if(val < (3.0f/16.0f))
		retval = 2.0f/15.0f;	//1/8
	else if(val < (3.0f/12.0f))
		retval = 3.0f/12.0f;	//1/6
	else if(val < (2.5f/8.0f))
		retval = 4.0f/15.0f;	//1/4
	else if(val < (3.0f/6.0f))
		retval = 5.0f/15.0f;	//1/3
	else if(val < (3.0f/4.0f))
		retval = 6.0f/15.0f;	//1/2
	else if(val < 1.5f)
		retval = 7.0f/15.0f;	//1
	else if(val < 2.5f)
		retval = 8.0f/15.0f;	//2
	else if(val < 3.5f)
		retval = 9.0f/15.0f;	//3
	else if(val < 5.0f)
		retval = 10.0f/15.0f;	//4
	else if(val < 7.0f)
		retval = 11.0f/15.0f;	//6
	else if(val < 12.0f)
		retval = 12.0f/15.0f;	//8
	else if(val < 24.0f)
		retval = 13.0f/15.0f;	//16
	else if(val < 48.0f)
		retval = 14.0f/15.0f;	//32
	else
		retval = 1.0f;			//64

	return retval;
}
