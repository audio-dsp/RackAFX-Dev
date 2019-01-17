//	MFSSGui.cpp - The GUI class for My First Step Sequencer.
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

#include "MFSSGui.h"
#include "MyFirstStepSequencer.h"

//----------------------------------------------------------------------------
MFSSGui::MFSSGui(AudioEffect *effect):
AEffGUIEditor(effect)
{
	//Set the scale tags.
	for(int i=0;i<12;++i)
		scaleTags[i] = MyFirstStepSequencer::ScaleStart + i;

	//Load the background bitmap.  We don't need to load all bitmaps, this
	//could be done when open is called.
	backgroundImage = new CBitmap(BackgroundImage);

	//Init the size of the plugin.
	rect.left   = 0;
	rect.top    = 0;
	rect.right  = (short)backgroundImage->getWidth();
	rect.bottom = (short)backgroundImage->getHeight();
}

//----------------------------------------------------------------------------
MFSSGui::~MFSSGui()
{
	//Decrement backgroundImage's reference count (because we initialised it
	//to 1 in it's constructor).  This will result in it being deleted.
	backgroundImage->forget();
}

//----------------------------------------------------------------------------
long MFSSGui::open(void *ptr)
{
	int i;
	CColor tempCol1 = {255, 0, 0, 0};
	CColor tempCol2 = {255, 192, 0, 0};

	//Set up VSTGUI stuff.
	AEffGUIEditor::open(ptr);

	//Initialise the frame.
	CRect size(0,
			   0,
			   backgroundImage->getWidth(),
			   backgroundImage->getHeight());
	frame = new CFrame(size, ptr, this);
	frame->setBackground(backgroundImage);

	//Create bitmaps.
	advancedHiddenImage = new CBitmap(AdvancedHiddenImage);
	advancedShownImage = new CBitmap(AdvancedShownImage);
	advancedSwitchImage = new CBitmap(AdvancedSwitchImage);
	octaveButtonImage = new CBitmap(OctaveButtonImage);
	bypassButtonImage = new CBitmap(BypassButtonImage);
	aboutButtonImage = new CBitmap(AboutButtonImage);
	aboutImage = new CBitmap(AboutImage);
	notesImage = new CBitmap(NotesImage);
	notesImage2 = new CBitmap(NotesImage2);
	volumesImage = new CBitmap(VolumesImage);
	shuffleImage = new CBitmap(ShuffleImage);
	notesHeadsImage = new CBitmap(NotesHeadsImage);
	numNotesImage = new CBitmap(NumNotesImage);
	randomImage = new CBitmap(RandomImage);

	//Create advanced section container.
	size.x = 55;
	size.y = 232;
	//size.right = 672;
	size.right = 55 + 715;
	size.bottom = 262;
	advancedSection = new CViewContainer(size, frame);
	frame->addView(advancedSection);

	//Create advanced section 'hidden' view.
	size.x = 0;
	size.y = 0;
	size.right = advancedHiddenImage->getWidth();
	size.bottom = advancedHiddenImage->getHeight();
	advancedHidden = new CView(size);
	advancedHidden->setBackground(advancedHiddenImage);
	//advancedHidden->setBackground(backgroundImage);

	//Create advanced section 'shown' view.
	advancedShown = new CView(size);
	advancedShown->setBackground(advancedShownImage);
	//advancedHidden->setBackground(backgroundImage);

	//Create shuffle button.
	size.x = 60;
	size.y = 4;
	size.right = (60+46);
	size.bottom = (4+17);
	shuffleButton = new COnOffButton(size,
									 this,
									 MyFirstStepSequencer::Shuffle,
									 shuffleImage);
	shuffleButton->setValue(effect->getParameter(MyFirstStepSequencer::Shuffle));

	//Create Notes/Heads button.
	size.x = 540;
	size.y = 1;
	size.right = (540+46);
	size.bottom = (1+25);
	notesButton = new COnOffButton(size,
								   this,
								   NotesHeads,
								   notesHeadsImage);

	//Create Num Notes button.
	size.x = 248;
	size.y = 3;
	size.right = (248+61);
	size.bottom = (3+25);
	numNotesButton = new COnOffButton(size,
									  this,
									  MyFirstStepSequencer::EightStepsMode,
									  numNotesImage);

	//Create the Scale keyboard.
	size.x = 367;
	size.y = 2;
	size.right = (367+119);
	size.bottom = 27;
	scaleKeyboard = new CSimpleKeyboardSwitch(size,
											  this,
											  scaleTags,
											  tempCol2,
											  //tempCol1);
											  kRedCColor);
	for(i=0;i<12;++i)
	{
		scaleKeyboard->SetValue(effect->getParameter(MyFirstStepSequencer::ScaleStart + i),
								i);
	}

	//Set the current view for the advanced section.
	advancedSection->addView(advancedHidden);

	//Create the advanced switch button.
	size.x = 20;
	size.y = 235;
	size.right = (20+24);
	size.bottom = (235+24);
	advancedSwitch = new COnOffButton(size,
									  this,
									  AdvancedSwitch,
									  advancedSwitchImage);
	frame->addView(advancedSwitch);

	//Create the octave 'button'.
	size.x = 704;
	size.y = 159;
	size.right = 816;
	size.bottom = 194;
	octaveButton = new StateButton(size,
								   octaveButtonImage,
								   10,
								   this,
								   MyFirstStepSequencer::Octave);
	octaveButton->setValue(effect->getParameter(MyFirstStepSequencer::Octave));
	frame->addView(octaveButton);

	//Create the bypass button.
	size.x = 793;
	size.y = 232;
	size.right = 816;
	size.bottom = 261;
	bypassButton = new COnOffButton(size,
									this,
									MyFirstStepSequencer::Bypass,
									bypassButtonImage);
	bypassButton->setValue(effect->getParameter(MyFirstStepSequencer::Bypass));
	frame->addView(bypassButton);

	//Create the about box.
	size.x = 775;
	size.y = 237;
	size.right = 791;
	size.bottom = 255;
	aboutBox = new NiallsAbout(size,
							   aboutButtonImage,
							   aboutImage);
	frame->addView(aboutBox);

	//Create the random button.
	size.x = 651;
	size.y = 1;
	size.right = (651+46);
	size.bottom = (1+25);
	CPoint temp(0, 0);
	randomButton = new CKickButton(size,
								   this,
								   MyFirstStepSequencer::Random,
								   25,
								   randomImage,
								   temp);

	//Create the note buttons.
	//Curse my not making them an equal distance apart!!!!
	size.x = 29;
	size.y = 146;
	size.right = 50;
	size.bottom = 167;
	for(i=0;i<16;++i)
	{
		noteButtons[i] = new StateButton(size,
										 notesImage,
										 13,
										 this,
										 i);
		noteButtons[i]->setHasDisabledState(true);
		noteButtons[i]->setValue(effect->getParameter(i));
		frame->addView(noteButtons[i]);

		if((i%4) == 0)
			size.offset(38, 0);
		else if((i%4) == 1)
			size.offset(40, 0);
		else if((i%4) == 2)
			size.offset(38, 0);
		else if((i%4) == 3)
			size.offset(44, 0);
	}

	//Create the volume buttons.
	//Curse my not making them an equal distance apart!!!!
	size.x = 28;
	size.y = 169;
	size.right = 50;
	size.bottom = 190;
	for(i=0;i<16;++i)
	{
		volumeButtons[i] = new StateButton(size,
										   volumesImage,
										   5,
										   this,
										   (i+16));
		volumeButtons[i]->setHasDisabledState(true);
		volumeButtons[i]->setValue(effect->getParameter(i+16));
		frame->addView(volumeButtons[i]);

		if((i%4) == 0)
			size.offset(38, 0);
		else if((i%4) == 1)
			size.offset(40, 0);
		else if((i%4) == 2)
			size.offset(38, 0);
		else if((i%4) == 3)
			size.offset(44, 0);
	}

	//Decrement reference count for all bitmaps except the background.
	advancedHiddenImage->forget();
	advancedShownImage->forget();
	advancedSwitchImage->forget();
	octaveButtonImage->forget();
	bypassButtonImage->forget();
	aboutButtonImage->forget();
	aboutImage->forget();
	volumesImage->forget();
	shuffleImage->forget();
	notesHeadsImage->forget();
	numNotesImage->forget();
	randomImage->forget();

	//Indicate success.
	return true;
}

//----------------------------------------------------------------------------
void MFSSGui::close()
{
	//Delete the advancedSection's views.
	if(advancedSwitch->getValue() > 0.5f)
	{
		advancedSection->removeView(advancedShown, true);
		advancedSection->removeView(shuffleButton, true);
		advancedSection->removeView(notesButton, true);
		advancedSection->removeView(numNotesButton, true);
		advancedSection->removeView(scaleKeyboard, true);
		advancedSection->removeView(randomButton, true);
		advancedHidden->forget();
	}
	else
	{
		advancedSection->removeView(advancedHidden, true);
		advancedShown->forget();
		shuffleButton->forget();
		notesButton->forget();
		numNotesButton->forget();
		scaleKeyboard->forget();
		randomButton->forget();
	}
	notesImage->forget();
	notesImage2->forget();

	//Delete everything else.
	delete frame;
	frame = 0;
}

//----------------------------------------------------------------------------
void MFSSGui::setParameter(long index, float value)
{
	int i;
	int tempint = 0;

	//Make sure the controls actually exist before we start calling
	//setValue()...
	if(!frame)
		return;

	switch(index)
	{
		case MyFirstStepSequencer::Octave:
			octaveButton->setValue(value);
			break;
		case MyFirstStepSequencer::Bypass:
			bypassButton->setValue(value);
			break;
		case MyFirstStepSequencer::Shuffle:
			shuffleButton->setValue(value);
			break;
		case MyFirstStepSequencer::EightStepsMode:
			numNotesButton->setValue(value);
			if(numNotesButton->getValue() < 0.5f)
			{
				for(i=1;i<16;i+=2)
				{
					noteButtons[i]->setTag(i);
					noteButtons[i]->setDisabled(false);
					volumeButtons[i]->setDisabled(false);
				}
				//Reset the tags.
				for(i=0;i<16;++i)
				{
					noteButtons[i]->setTag(i);
					noteButtons[i]->setValue(effect->getParameter(i));
					volumeButtons[i]->setTag(i+16);
					volumeButtons[i]->setValue(effect->getParameter(i+16));
				}
			}
			else
			{
				for(i=1;i<16;i+=2)
				{
					noteButtons[i]->setTag(102);
					noteButtons[i]->setDisabled(true);
					volumeButtons[i]->setDisabled(true);
				}
				//Switch the tags.
				for(i=0;i<16;i+=2)
				{
					noteButtons[i]->setTag(tempint);
					noteButtons[i]->setValue(effect->getParameter(tempint));
					volumeButtons[i]->setTag(tempint+16);
					volumeButtons[i]->setValue(effect->getParameter(tempint+16));

					++tempint;
				}
			}
			break;
	}
	if(index < MyFirstStepSequencer::Step1Volume)
	{
		for(i=0;i<16;++i)
		{
			if(index == noteButtons[i]->getTag())
			{
				noteButtons[i]->setValue(value);
				break;
			}
		}
	}
	else if(index < MyFirstStepSequencer::Octave)
	{
		for(i=0;i<16;++i)
		{
			if(index == volumeButtons[i]->getTag())
			{
				volumeButtons[i]->setValue(value);
				break;
			}
		}
	}
	else if((index > MyFirstStepSequencer::Bypass) &&
			(index < MyFirstStepSequencer::Random))
	{
		scaleKeyboard->SetValue(value,
								(index - MyFirstStepSequencer::ScaleStart));
	}
}

//----------------------------------------------------------------------------
void MFSSGui::valueChanged(CDrawContext* context, CControl* control)
{
	//int ignoredCount = 0;
	//long ignoredStates = 0;
	int i;
	//int tempint = 0;
	long tempTag;
	int keyCount = 0;

	//Handle the Scale keyboard first.
	if(control == scaleKeyboard)
	{
		tempTag = control->getTag();

		//First check that this isn't the only key currently on.
		for(i=0;i<12;++i)
		{
			if(scaleKeyboard->GetValue(i) > 0.5f)
				++keyCount;
		}
		if(keyCount < 1)
		{
			scaleKeyboard->SetValue(1.0f, (tempTag-MyFirstStepSequencer::ScaleStart));
			return;
		}

		effect->setParameterAutomated(tempTag,
									  scaleKeyboard->GetValue(tempTag-MyFirstStepSequencer::ScaleStart));
		//setUpScaleForcing();
		return;
	}

	if(control->getTag() < MyFirstStepSequencer::Step1Volume)
	{
		MyFirstStepSequencer *plugin = static_cast<MyFirstStepSequencer *>(effect);
		float tempf = plugin->getParameter(control->getTag());

		if((tempf == 1.0f)&&(control->getValue() == 0.0f))
			tempf = tempf; //Do nothing - Is this a really stupid way to do this?
		else if(tempf > control->getValue())
			plugin->rightClickNoteUpdate();
	}

	switch(control->getTag())
	{
		case AdvancedSwitch:
			if(control->getValue() > 0.5f)
			{
				advancedSection->removeView(advancedHidden, false);
				advancedSection->addView(advancedShown);
				advancedSection->addView(shuffleButton);
				advancedSection->addView(notesButton);
				advancedSection->addView(numNotesButton);
				advancedSection->addView(scaleKeyboard);
				advancedSection->addView(randomButton);
			}
			else
			{
				advancedSection->removeView(shuffleButton, false);
				advancedSection->removeView(notesButton, false);
				advancedSection->removeView(numNotesButton, false);
				advancedSection->removeView(scaleKeyboard, false);
				advancedSection->removeView(randomButton, false);
				advancedSection->removeView(advancedShown, false);
				advancedSection->addView(advancedHidden);
			}
			break;
		case NotesHeads:
			if(control->getValue() > 0.5f)
			{
				for(i=0;i<16;++i)
					noteButtons[i]->setImage(notesImage2);
			}
			else
			{
				for(i=0;i<16;++i)
					noteButtons[i]->setImage(notesImage);
			}
			break;
		default:
			effect->setParameterAutomated(control->getTag(),
										  control->getValue());
			break;
	}
}

//----------------------------------------------------------------------------
void MFSSGui::redrawEverything()
{
	scaleKeyboard->setDirty(false);
}

//----------------------------------------------------------------------------
void MFSSGui::setUpScaleForcing()
{
	int i;
	int ignoredCount = 0;
	long ignoredStates = 0;

	//Make sure we always leave one state free, hence the
	//'ignoredCount<11' bit.
	for(i=0;(i<12) && (ignoredCount<11);++i)
	{
		if(scaleKeyboard->GetValue(i) < 0.5f)
		{
			//Have to take into account the fact that the keyboard control
			//numbers it's keys: 0->6 = white keys, 7-11 = black keys.
			switch(i)
			{
				case 0:
					ignoredStates += 1;
					break;
				case 1:
					ignoredStates += (1<<2);
					break;
				case 2:
					ignoredStates += (1<<4);
					break;
				case 3:
					ignoredStates += (1<<5);
					break;
				case 4:
					ignoredStates += (1<<7);
					break;
				case 5:
					ignoredStates += (1<<9);
					break;
				case 6:
					ignoredStates += (1<<11);
					break;
				case 7:
					ignoredStates += (1<<1);
					break;
				case 8:
					ignoredStates += (1<<3);
					break;
				case 9:
					ignoredStates += (1<<6);
					break;
				case 10:
					ignoredStates += (1<<8);
					break;
				case 11:
					ignoredStates += (1<<10);
					break;
			}
			++ignoredCount;
		}
	}
	for(i=0;i<16;++i)
		noteButtons[i]->setStatesIgnored(ignoredStates);
}
