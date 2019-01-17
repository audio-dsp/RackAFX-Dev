//	EffectsEditor.cpp - The VSTGUI editor handling the effects parameters.
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

#include "EffectsEditor.h"
#include "VstPlugin.h"

#include <sstream>

using namespace std;

//------------------------------------------------------------------------------
EffectsEditor::EffectsEditor(AudioEffect *effect):
AEffGUIEditor(effect),
delayMix(0),
delayShortTimes(0),
delayLTime(0),
delayLTimeEdit(0),
delayRTime(0),
delayFeedback(0),
delayLevel(0)
{
	backgroundImage = new CBitmap(BackgroundImage);

	//Init the size of the editor.
	rect.left   = 0;
	rect.top    = 0;
	rect.right  = static_cast<short>(backgroundImage->getWidth());
	rect.bottom = static_cast<short>(backgroundImage->getHeight());
}

//------------------------------------------------------------------------------
EffectsEditor::~EffectsEditor()
{
	//Decrement backgroundImage's reference count (because we initialised it
	//to 1 in it's constructor).  This will result in it being deleted.
	backgroundImage->forget();
}

//------------------------------------------------------------------------------
bool EffectsEditor::open(void *systemPointer)
{
	CPoint offset;
	CColor greyColour;

	AEffEditor::open(systemPointer);

	//Initialise the frame.
	CRect size(0,
			   0,
			   backgroundImage->getWidth(),
			   backgroundImage->getHeight());
	frame = new CFrame(size, systemPointer, this);
	frame->setBackground(backgroundImage);

	//Create bitmaps.
	knobImage = new CBitmap(KnobImage);
	sliderImage = new CBitmap(SliderImage);
	buttonImage = new CBitmap(ButtonImage);
	bigKnobImage = new CBitmap(BigKnobImage);
	reverseImage = new CBitmap(ReverseImage);
	mwTickImage = new CBitmap(MWTickImage);
	syncImage = new CBitmap(SyncImage);

	//----Delay----

	//Create Mix slider.
	size.x = 17;
	size.y = 63;
	size.right = (size.x+13);
	size.bottom = (size.y+89);
	offset.x = size.x;
	offset.y = size.y;
	delayMix = new CVerticalSlider(size,
								   this,
								   VstPlugin::DelayMix,
								   size.y,
								   (size.bottom - sliderImage->getHeight()),
								   sliderImage,
								   backgroundImage,
								   offset);
	frame->addView(delayMix);
	delayMix->setValue(effect->getParameter(VstPlugin::DelayMix));

	//Create Short Times button.
	size.x = 17;
	size.y = 159;
	size.right = (size.x + buttonImage->getWidth());
	size.bottom = (size.y + buttonImage->getHeight()/2);
	delayShortTimes = new COnOffButton(size,
									   this,
									   VstPlugin::DelayShortTimes,
									   buttonImage);
	frame->addView(delayShortTimes);
	delayShortTimes->setValue(effect->getParameter(VstPlugin::DelayShortTimes));

	//Create DelayL ModWheel.
	size.x = 37;
	size.y = 41;
	size.right = (size.x+42);
	size.bottom = (size.y+42);
	delayLWheel = new ModWheel(size,
							   this,
							   VstPlugin::DelayDelayLMW1,
							   VstPlugin::DelayDelayLMW2,
							   VstPlugin::DelayDelayLMW3,
							   VstPlugin::DLMx,
							   VstPlugin::DLMy,
							   mwTickImage);
	frame->addView(delayLWheel);
	delayLWheel->setValue1(effect->getParameter(VstPlugin::DelayDelayLMW1));
	delayLWheel->setValue2(effect->getParameter(VstPlugin::DelayDelayLMW2));
	delayLWheel->setValue3(effect->getParameter(VstPlugin::DelayDelayLMW3));
	delayLWheel->setValueX(effect->getParameter(VstPlugin::DLMx));
	delayLWheel->setValueY(effect->getParameter(VstPlugin::DLMy));

	//Create DelayL knob.
	size.x = 44;
	size.y = 87;
	size.right = (size.x+28);
	size.bottom = (size.y+28);
	offset.x = size.x;
	offset.y = size.y;
	delayLTime = new CKnob(size,
						   this,
						   VstPlugin::DelayDelayL,
						   backgroundImage,
						   knobImage,
						   offset);
	frame->addView(delayLTime);
	delayLTime->setValue(effect->getParameter(VstPlugin::DelayDelayL));

	//Create DelayL text editor.
	size.x = 38;
	size.y = 126;
	size.right = size.x + 38;
	size.bottom = size.y + 10;
	greyColour(128, 128, 128, 255);
	delayLTimeEdit = new CTextEdit(size, this, VstPlugin::DelayDelayL);
	delayLTimeEdit->setBackColor(kWhiteCColor);
	delayLTimeEdit->setFrameColor(kWhiteCColor);
	delayLTimeEdit->setFontColor(greyColour);
	delayLTimeEdit->setFont(kNormalFontVerySmall);
	frame->addView(delayLTimeEdit);
	setParameter(VstPlugin::DelayDelayL, effect->getParameter(VstPlugin::DelayDelayL));

	//Create DelayR ModWheel.
	size.x = 98;
	size.y = 41;
	size.right = (size.x+42);
	size.bottom = (size.y+42);
	delayRWheel = new ModWheel(size,
							   this,
							   VstPlugin::DelayDelayRMW1,
							   VstPlugin::DelayDelayRMW2,
							   VstPlugin::DelayDelayRMW3,
							   VstPlugin::DRMx,
							   VstPlugin::DRMy,
							   mwTickImage);
	frame->addView(delayRWheel);
	delayRWheel->setValue1(effect->getParameter(VstPlugin::DelayDelayRMW1));
	delayRWheel->setValue2(effect->getParameter(VstPlugin::DelayDelayRMW2));
	delayRWheel->setValue3(effect->getParameter(VstPlugin::DelayDelayRMW3));
	delayRWheel->setValueX(effect->getParameter(VstPlugin::DRMx));
	delayRWheel->setValueY(effect->getParameter(VstPlugin::DRMy));

	//Create DelayR knob.
	size.x = 105;
	size.y = 87;
	size.right = (size.x+28);
	size.bottom = (size.y+28);
	offset.x = size.x;
	offset.y = offset.y;
	delayRTime = new CKnob(size,
						   this,
						   VstPlugin::DelayDelayR,
						   backgroundImage,
						   knobImage,
						   offset);
	frame->addView(delayRTime);
	delayRTime->setValue(effect->getParameter(VstPlugin::DelayDelayR));

	//Create DelayR text editor.
	size.x = 99;
	size.y = 126;
	size.right = size.x + 38;
	size.bottom = size.y + 10;
	greyColour(128, 128, 128, 255);
	delayRTimeEdit = new CTextEdit(size, this, VstPlugin::DelayDelayR);
	delayRTimeEdit->setBackColor(kWhiteCColor);
	delayRTimeEdit->setFrameColor(kWhiteCColor);
	delayRTimeEdit->setFontColor(greyColour);
	delayRTimeEdit->setFont(kNormalFontVerySmall);
	frame->addView(delayRTimeEdit);
	setParameter(VstPlugin::DelayDelayR, effect->getParameter(VstPlugin::DelayDelayR));

	//Create Feedback knob.
	size.x = 165;
	size.y = 87;
	size.right = (size.x+28);
	size.bottom = (size.y+28);
	offset.x = size.x;
	offset.y = offset.y;
	delayFeedback = new CKnob(size,
							  this,
							  VstPlugin::DelayFeedback,
							  backgroundImage,
							  knobImage,
							  offset);
	frame->addView(delayFeedback);
	delayFeedback->setValue(effect->getParameter(VstPlugin::DelayFeedback));

	//Create Feedback text editor.
	size.x = 159;
	size.y = 126;
	size.right = size.x + 38;
	size.bottom = size.y + 10;
	greyColour(128, 128, 128, 255);
	delayFeedbackEdit = new CTextEdit(size, this, VstPlugin::DelayFeedback);
	delayFeedbackEdit->setBackColor(kWhiteCColor);
	delayFeedbackEdit->setFrameColor(kWhiteCColor);
	delayFeedbackEdit->setFontColor(greyColour);
	delayFeedbackEdit->setFont(kNormalFontVerySmall);
	frame->addView(delayFeedbackEdit);
	setParameter(VstPlugin::DelayFeedback, effect->getParameter(VstPlugin::DelayFeedback));

	//Create Level slider.
	size.x = 208;
	size.y = 63;
	size.right = (size.x+13);
	size.bottom = (size.y+89);
	offset.x = size.x;
	offset.y = size.y;
	delayLevel = new CVerticalSlider(size,
									 this,
									 VstPlugin::DelayLevel,
									 size.y,
									 (size.bottom - sliderImage->getHeight()),
									 sliderImage,
									 backgroundImage,
									 offset);
	frame->addView(delayLevel);
	delayLevel->setValue(effect->getParameter(VstPlugin::DelayLevel));

	//----Granulator----

	//Create Density knob.
	size.x = 255;
	size.y = 87;
	size.right = (size.x+28);
	size.bottom = (size.y+28);
	offset.x = size.x;
	offset.y = size.y;
	granDensity = new CKnob(size,
							this,
							VstPlugin::GranDensity,
							backgroundImage,
							knobImage,
							offset);
	frame->addView(granDensity);
	granDensity->setValue(effect->getParameter(VstPlugin::GranDensity));

	//Create Density text editor.
	size.x = 249;
	size.y = 125;
	size.right = size.x + 38;
	size.bottom = size.y + 10;
	greyColour(128, 128, 128, 255);
	granDensityEdit = new CTextEdit(size, this, VstPlugin::GranDensity);
	granDensityEdit->setBackColor(kWhiteCColor);
	granDensityEdit->setFrameColor(kWhiteCColor);
	granDensityEdit->setFontColor(greyColour);
	granDensityEdit->setFont(kNormalFontVerySmall);
	frame->addView(granDensityEdit);
	setParameter(VstPlugin::GranDensity, effect->getParameter(VstPlugin::GranDensity));

	//Create Density ModWheel.
	size.x = 248;
	size.y = 41;
	size.right = (size.x+42);
	size.bottom = (size.y+42);
	granDensityWheel = new ModWheel(size,
								    this,
								    VstPlugin::GranDensityMW1,
								    VstPlugin::GranDensityMW2,
								    VstPlugin::GranDensityMW3,
									VstPlugin::GDex,
									VstPlugin::GDey,
								    mwTickImage);
	frame->addView(granDensityWheel);
	granDensityWheel->setValue1(effect->getParameter(VstPlugin::GranDensityMW1));
	granDensityWheel->setValue2(effect->getParameter(VstPlugin::GranDensityMW2));
	granDensityWheel->setValue3(effect->getParameter(VstPlugin::GranDensityMW3));
	granDensityWheel->setValueX(effect->getParameter(VstPlugin::GDex));
	granDensityWheel->setValueY(effect->getParameter(VstPlugin::GDey));

	//Create Duration knob.
	size.x = 316;
	size.y = 87;
	size.right = (size.x+28);
	size.bottom = (size.y+28);
	offset.x = size.x;
	offset.y = size.y;
	granDuration = new CKnob(size,
							 this,
							 VstPlugin::GranDuration,
							 backgroundImage,
							 knobImage,
							 offset);
	frame->addView(granDuration);
	granDuration->setValue(effect->getParameter(VstPlugin::GranDuration));

	//Create Duration text editor.
	size.x = 310;
	size.y = 125;
	size.right = size.x + 38;
	size.bottom = size.y + 10;
	greyColour(128, 128, 128, 255);
	granDurationEdit = new CTextEdit(size, this, VstPlugin::GranDuration);
	granDurationEdit->setBackColor(kWhiteCColor);
	granDurationEdit->setFrameColor(kWhiteCColor);
	granDurationEdit->setFontColor(greyColour);
	granDurationEdit->setFont(kNormalFontVerySmall);
	frame->addView(granDurationEdit);
	setParameter(VstPlugin::GranDuration, effect->getParameter(VstPlugin::GranDuration));

	//Create Duration ModWheel.
	size.x = 309;
	size.y = 41;
	size.right = (size.x+42);
	size.bottom = (size.y+42);
	granDurationWheel = new ModWheel(size,
								     this,
								     VstPlugin::GranDurationMW1,
								     VstPlugin::GranDurationMW2,
								     VstPlugin::GranDurationMW3,
									 VstPlugin::GDrx,
									 VstPlugin::GDry,
								     mwTickImage);
	frame->addView(granDurationWheel);
	granDurationWheel->setValue1(effect->getParameter(VstPlugin::GranDurationMW1));
	granDurationWheel->setValue2(effect->getParameter(VstPlugin::GranDurationMW2));
	granDurationWheel->setValue3(effect->getParameter(VstPlugin::GranDurationMW3));
	granDurationWheel->setValueX(effect->getParameter(VstPlugin::GDrx));
	granDurationWheel->setValueY(effect->getParameter(VstPlugin::GDry));

	//Create Pitch knob.
	size.x = 376;
	size.y = 87;
	size.right = (size.x+28);
	size.bottom = (size.y+28);
	offset.x = size.x;
	offset.y = size.y;
	granPitch = new CKnob(size,
						  this,
						  VstPlugin::GranPitch,
						  backgroundImage,
						  knobImage,
						  offset);
	frame->addView(granPitch);
	granPitch->setValue(effect->getParameter(VstPlugin::GranPitch));

	//Create Pitch text editor.
	size.x = 370;
	size.y = 125;
	size.right = size.x + 38;
	size.bottom = size.y + 10;
	greyColour(128, 128, 128, 255);
	granPitchEdit = new CTextEdit(size, this, VstPlugin::GranPitch);
	granPitchEdit->setBackColor(kWhiteCColor);
	granPitchEdit->setFrameColor(kWhiteCColor);
	granPitchEdit->setFontColor(greyColour);
	granPitchEdit->setFont(kNormalFontVerySmall);
	frame->addView(granPitchEdit);
	setParameter(VstPlugin::GranPitch, effect->getParameter(VstPlugin::GranPitch));

	//Create Duration ModWheel.
	size.x = 370;
	size.y = 41;
	size.right = (size.x+42);
	size.bottom = (size.y+42);
	granPitchWheel = new ModWheel(size,
								  this,
								  VstPlugin::GranPitchMW1,
								  VstPlugin::GranPitchMW2,
								  VstPlugin::GranPitchMW3,
								  VstPlugin::GPix,
								  VstPlugin::GPiy,
								  mwTickImage);
	frame->addView(granPitchWheel);
	granPitchWheel->setValue1(effect->getParameter(VstPlugin::GranPitchMW1));
	granPitchWheel->setValue2(effect->getParameter(VstPlugin::GranPitchMW2));
	granPitchWheel->setValue3(effect->getParameter(VstPlugin::GranPitchMW3));
	granPitchWheel->setValueX(effect->getParameter(VstPlugin::GPix));
	granPitchWheel->setValueY(effect->getParameter(VstPlugin::GPiy));

	//Create Level slider.
	size.x = 419;
	size.y = 63;
	size.right = (size.x+13);
	size.bottom = (size.y+87);
	offset.x = size.x;
	offset.y = size.y;
	granLevel = new CVerticalSlider(size,
									this,
									VstPlugin::GranLevel,
									size.y,
									(size.bottom - sliderImage->getHeight()),
									sliderImage,
									backgroundImage,
									offset);
	frame->addView(granLevel);
	granLevel->setValue(effect->getParameter(VstPlugin::GranLevel));

	//----Transpose----

	//Create Value knob.
	size.x = 30;
	size.y = 258;
	size.right = (size.x+28);
	size.bottom = (size.y+28);
	offset.x = size.x;
	offset.y = size.y;
	transVal = new CKnob(size,
						 this,
						 VstPlugin::TransVal,
						 backgroundImage,
						 knobImage,
						 offset);
	frame->addView(transVal);
	transVal->setValue(effect->getParameter(VstPlugin::TransVal));

	//Create Value text editor.
	size.x = 24;
	size.y = 296;
	size.right = size.x + 38;
	size.bottom = size.y + 10;
	greyColour(128, 128, 128, 255);
	transEdit = new CTextEdit(size, this, VstPlugin::TransVal);
	transEdit->setBackColor(kWhiteCColor);
	transEdit->setFrameColor(kWhiteCColor);
	transEdit->setFontColor(greyColour);
	transEdit->setFont(kNormalFontVerySmall);
	frame->addView(transEdit);
	setParameter(VstPlugin::TransVal, effect->getParameter(VstPlugin::TransVal));

	//Create Value ModWheel.
	size.x = 23;
	size.y = 212;
	size.right = (size.x+42);
	size.bottom = (size.y+42);
	transValWheel = new ModWheel(size,
								 this,
								 VstPlugin::TransValMW1,
								 VstPlugin::TransValMW2,
								 VstPlugin::TransValMW3,
								 VstPlugin::TVax,
								 VstPlugin::TVay,
								 mwTickImage);
	frame->addView(transValWheel);
	transValWheel->setValue1(effect->getParameter(VstPlugin::TransValMW1));
	transValWheel->setValue2(effect->getParameter(VstPlugin::TransValMW2));
	transValWheel->setValue3(effect->getParameter(VstPlugin::TransValMW3));
	transValWheel->setValueX(effect->getParameter(VstPlugin::TVax));
	transValWheel->setValueY(effect->getParameter(VstPlugin::TVay));

	//Create Level slider.
	size.x = 74;
	size.y = 236;
	size.right = (size.x+13);
	size.bottom = (size.y+87);
	offset.x = size.x;
	offset.y = size.y;
	transLevel = new CVerticalSlider(size,
									 this,
									 VstPlugin::TransLevel,
									 size.y,
									 (size.bottom - sliderImage->getHeight()),
									 sliderImage,
									 backgroundImage,
									 offset);
	frame->addView(transLevel);
	transLevel->setValue(effect->getParameter(VstPlugin::TransLevel));

	//----Exaggerate----

	//Create Value knob.
	size.x = 120;
	size.y = 258;
	size.right = (size.x+28);
	size.bottom = (size.y+28);
	offset.x = size.x;
	offset.y = size.y;
	exagVal = new CKnob(size,
						this,
						VstPlugin::ExagVal,
						backgroundImage,
						knobImage,
						offset);
	frame->addView(exagVal);
	exagVal->setValue(effect->getParameter(VstPlugin::ExagVal));

	//Create Value text editor.
	size.x = 114;
	size.y = 296;
	size.right = size.x + 38;
	size.bottom = size.y + 10;
	greyColour(128, 128, 128, 255);
	exagEdit = new CTextEdit(size, this, VstPlugin::ExagVal);
	exagEdit->setBackColor(kWhiteCColor);
	exagEdit->setFrameColor(kWhiteCColor);
	exagEdit->setFontColor(greyColour);
	exagEdit->setFont(kNormalFontVerySmall);
	frame->addView(exagEdit);
	setParameter(VstPlugin::ExagVal, effect->getParameter(VstPlugin::ExagVal));

	//Create Value ModWheel.
	size.x = 113;
	size.y = 212;
	size.right = (size.x+42);
	size.bottom = (size.y+42);
	exagValWheel = new ModWheel(size,
								this,
								VstPlugin::ExagValMW1,
								VstPlugin::ExagValMW2,
								VstPlugin::ExagValMW3,
								VstPlugin::EVax,
								VstPlugin::EVay,
								mwTickImage);
	frame->addView(exagValWheel);
	exagValWheel->setValue1(effect->getParameter(VstPlugin::ExagValMW1));
	exagValWheel->setValue2(effect->getParameter(VstPlugin::ExagValMW2));
	exagValWheel->setValue3(effect->getParameter(VstPlugin::ExagValMW3));
	exagValWheel->setValueX(effect->getParameter(VstPlugin::EVax));
	exagValWheel->setValueY(effect->getParameter(VstPlugin::EVay));

	//Create Level slider.
	size.x = 164;
	size.y = 236;
	size.right = (size.x+13);
	size.bottom = (size.y+87);
	offset.x = size.x;
	offset.y = size.y;
	exagLevel = new CVerticalSlider(size,
									this,
									VstPlugin::ExagLevel,
									size.y,
									(size.bottom - sliderImage->getHeight()),
									sliderImage,
									backgroundImage,
									offset);
	frame->addView(exagLevel);
	exagLevel->setValue(effect->getParameter(VstPlugin::ExagLevel));

	//----Accumulate----

	//Create Gliss knob.
	size.x = 211;
	size.y = 258;
	size.right = (size.x+28);
	size.bottom = (size.y+28);
	offset.x = size.x;
	offset.y = size.y;
	accumGliss = new CKnob(size,
						   this,
						   VstPlugin::AccumGliss,
						   backgroundImage,
						   knobImage,
						   offset);
	frame->addView(accumGliss);
	accumGliss->setValue(effect->getParameter(VstPlugin::AccumGliss));

	//Create Gliss text editor.
	size.x = 205;
	size.y = 296;
	size.right = size.x + 38;
	size.bottom = size.y + 10;
	greyColour(128, 128, 128, 255);
	accumGlissEdit = new CTextEdit(size, this, VstPlugin::AccumGliss);
	accumGlissEdit->setBackColor(kWhiteCColor);
	accumGlissEdit->setFrameColor(kWhiteCColor);
	accumGlissEdit->setFontColor(greyColour);
	accumGlissEdit->setFont(kNormalFontVerySmall);
	frame->addView(accumGlissEdit);
	setParameter(VstPlugin::AccumGliss, effect->getParameter(VstPlugin::AccumGliss));

	//Create Gliss ModWheel.
	size.x = 204;
	size.y = 212;
	size.right = (size.x+42);
	size.bottom = (size.y+42);
	accumGlissWheel = new ModWheel(size,
								   this,
								   VstPlugin::AccumGlissMW1,
								   VstPlugin::AccumGlissMW2,
								   VstPlugin::AccumGlissMW3,
								   VstPlugin::AGlx,
								   VstPlugin::AGly,
								   mwTickImage);
	frame->addView(accumGlissWheel);
	accumGlissWheel->setValue1(effect->getParameter(VstPlugin::AccumGlissMW1));
	accumGlissWheel->setValue2(effect->getParameter(VstPlugin::AccumGlissMW2));
	accumGlissWheel->setValue3(effect->getParameter(VstPlugin::AccumGlissMW3));
	accumGlissWheel->setValueX(effect->getParameter(VstPlugin::AGlx));
	accumGlissWheel->setValueY(effect->getParameter(VstPlugin::AGly));

	//Create Decay knob.
	size.x = 272;
	size.y = 258;
	size.right = (size.x+28);
	size.bottom = (size.y+28);
	offset.x = size.x;
	offset.y = size.y;
	accumDecay = new CKnob(size,
						   this,
						   VstPlugin::AccumDecay,
						   backgroundImage,
						   knobImage,
						   offset);
	frame->addView(accumDecay);
	accumDecay->setValue(effect->getParameter(VstPlugin::AccumDecay));

	//Create Decay text editor.
	size.x = 266;
	size.y = 296;
	size.right = size.x + 38;
	size.bottom = size.y + 10;
	greyColour(128, 128, 128, 255);
	accumDecayEdit = new CTextEdit(size, this, VstPlugin::AccumDecay);
	accumDecayEdit->setBackColor(kWhiteCColor);
	accumDecayEdit->setFrameColor(kWhiteCColor);
	accumDecayEdit->setFontColor(greyColour);
	accumDecayEdit->setFont(kNormalFontVerySmall);
	frame->addView(accumDecayEdit);
	setParameter(VstPlugin::AccumDecay, effect->getParameter(VstPlugin::AccumDecay));

	//Create Decay ModWheel.
	size.x = 265;
	size.y = 212;
	size.right = (size.x+42);
	size.bottom = (size.y+42);
	accumDecayWheel = new ModWheel(size,
								   this,
								   VstPlugin::AccumDecayMW1,
								   VstPlugin::AccumDecayMW2,
								   VstPlugin::AccumDecayMW3,
								   VstPlugin::ADex,
								   VstPlugin::ADey,
								   mwTickImage);
	frame->addView(accumDecayWheel);
	accumDecayWheel->setValue1(effect->getParameter(VstPlugin::AccumDecayMW1));
	accumDecayWheel->setValue2(effect->getParameter(VstPlugin::AccumDecayMW2));
	accumDecayWheel->setValue3(effect->getParameter(VstPlugin::AccumDecayMW3));
	accumDecayWheel->setValueX(effect->getParameter(VstPlugin::ADex));
	accumDecayWheel->setValueY(effect->getParameter(VstPlugin::ADey));

	//Create Level slider.
	size.x = 318;
	size.y = 236;
	size.right = (size.x+13);
	size.bottom = (size.y+87);
	offset.x = size.x;
	offset.y = size.y;
	accumLevel = new CVerticalSlider(size,
									 this,
									 VstPlugin::AccumLevel,
									 size.y,
									 (size.bottom - sliderImage->getHeight()),
									 sliderImage,
									 backgroundImage,
									 offset);
	frame->addView(accumLevel);
	accumLevel->setValue(effect->getParameter(VstPlugin::AccumLevel));

	//----Reverse----

	//Create Direction button.
	size.x = 354;
	size.y = 273;
	size.right = (size.x + reverseImage->getWidth());
	size.bottom = (size.y + reverseImage->getHeight()/2);
	reverseDirection = new COnOffButton(size,
									    this,
									    VstPlugin::ReverseDirection,
									    reverseImage);
	frame->addView(reverseDirection);
	reverseDirection->setValue(effect->getParameter(VstPlugin::ReverseDirection));

	//Create Direction ModWheel.
	size.x = 363;
	size.y = 222;
	size.right = (size.x+42);
	size.bottom = (size.y+42);
	reverseDirectionWheel = new ModWheel(size,
										 this,
										 VstPlugin::ReverseDirectionMW1,
										 VstPlugin::ReverseDirectionMW2,
										 VstPlugin::ReverseDirectionMW3,
										 VstPlugin::RDix,
										 VstPlugin::RDiy,
										 mwTickImage);
	frame->addView(reverseDirectionWheel);
	reverseDirectionWheel->setValue1(effect->getParameter(VstPlugin::ReverseDirectionMW1));
	reverseDirectionWheel->setValue2(effect->getParameter(VstPlugin::ReverseDirectionMW2));
	reverseDirectionWheel->setValue3(effect->getParameter(VstPlugin::ReverseDirectionMW3));
	reverseDirectionWheel->setValueX(effect->getParameter(VstPlugin::RDix));
	reverseDirectionWheel->setValueY(effect->getParameter(VstPlugin::RDiy));

	//Create Level slider.
	size.x = 420;
	size.y = 236;
	size.right = (size.x+13);
	size.bottom = (size.y+87);
	offset.x = size.x;
	offset.y = size.y;
	reverseLevel = new CVerticalSlider(size,
									   this,
									   VstPlugin::ReverseLevel,
									   size.y,
									   (size.bottom - sliderImage->getHeight()),
									   sliderImage,
									   backgroundImage,
									   offset);
	frame->addView(reverseLevel);
	reverseLevel->setValue(effect->getParameter(VstPlugin::ReverseLevel));

	//----Output----

	//Create Mix knob.
	size.x = 462;
	size.y = 247 - 30;
	size.right = (size.x+39);
	size.bottom = (size.y+39);
	offset.x = size.x;
	offset.y = size.y;
	mix = new CKnob(size,
					this,
					VstPlugin::Mix,
					backgroundImage,
					bigKnobImage,
					offset);
	frame->addView(mix);
	mix->setValue(effect->getParameter(VstPlugin::Mix));

	//Create Mix text editor.
	size.x = 463;
	size.y = 296 - 30;
	size.right = size.x + 38;
	size.bottom = size.y + 10;
	greyColour(128, 128, 128, 255);
	mixEdit = new CTextEdit(size, this, VstPlugin::Mix);
	mixEdit->setBackColor(kWhiteCColor);
	mixEdit->setFrameColor(kWhiteCColor);
	mixEdit->setFontColor(greyColour);
	mixEdit->setFont(kNormalFontVerySmall);
	frame->addView(mixEdit);
	setParameter(VstPlugin::Mix, effect->getParameter(VstPlugin::Mix));

	//Create Level knob.
	size.x = 463;
	size.y = 155 - 30;
	size.right = (size.x+39);
	size.bottom = (size.y+39);
	offset.x = size.x;
	offset.y = size.y;
	level = new CKnob(size,
					  this,
					  VstPlugin::Level,
					  backgroundImage,
					  bigKnobImage,
					  offset);
	frame->addView(level);
	level->setValue(effect->getParameter(VstPlugin::Level));

	//Create Level text editor.
	size.x = 463;
	size.y = 205 - 30;
	size.right = size.x + 38;
	size.bottom = size.y + 10;
	greyColour(128, 128, 128, 255);
	levelEdit = new CTextEdit(size, this, VstPlugin::Level);
	levelEdit->setBackColor(kWhiteCColor);
	levelEdit->setFrameColor(kWhiteCColor);
	levelEdit->setFontColor(greyColour);
	levelEdit->setFont(kNormalFontVerySmall);
	frame->addView(levelEdit);
	setParameter(VstPlugin::Level, effect->getParameter(VstPlugin::Level));

	//Create Level ModWheel.
	size.x = 461;
	size.y = 107 - 30;
	size.right = (size.x+42);
	size.bottom = (size.y+42);
	levelWheel = new ModWheel(size,
							  this,
							  VstPlugin::LevelMW1,
							  VstPlugin::LevelMW2,
							  VstPlugin::LevelMW3,
							  VstPlugin::OLex,
							  VstPlugin::OLey,
							  mwTickImage);
	frame->addView(levelWheel);
	levelWheel->setValue1(effect->getParameter(VstPlugin::LevelMW1));
	levelWheel->setValue2(effect->getParameter(VstPlugin::LevelMW2));
	levelWheel->setValue3(effect->getParameter(VstPlugin::LevelMW3));
	levelWheel->setValueX(effect->getParameter(VstPlugin::OLex));
	levelWheel->setValueY(effect->getParameter(VstPlugin::OLey));


	//Create Host Sync button.
	size.x = 450;
	size.y = 311;
	size.right = (size.x + syncImage->getWidth());
	size.bottom = (size.y + syncImage->getHeight()/2);
	sync = new COnOffButton(size, this, VstPlugin::HostSync, syncImage);
	frame->addView(sync);
	sync->setValue(effect->getParameter(VstPlugin::HostSync));

#ifdef PRESETSAVER
	size.x = 146;
	size.y = 5;
	size.right = (size.x + buttonImage->getWidth());
	size.bottom = (size.y + buttonImage->getHeight()/2);
	presetButton = new CKickButton(size,
								   this,
								   VstPlugin::PresetSaver,
								   (buttonImage->getHeight()/2),
								   buttonImage,
								   CPoint(0, 0));
	frame->addView(presetButton);
#endif

	//Decrement reference count for all bitmaps except the background.
	knobImage->forget();
	sliderImage->forget();
	buttonImage->forget();
	bigKnobImage->forget();
	reverseImage->forget();
	mwTickImage->forget();
	syncImage->forget();

	setKnobMode(kLinearMode);

	//Indicate success.
	return true;
}

//------------------------------------------------------------------------------
void EffectsEditor::close()
{
	//Delete everything else.
	delete frame;
	frame = 0;
}

//------------------------------------------------------------------------------
void EffectsEditor::setParameter(VstInt32 index, float value)
{
	char tempch[16];
	stringstream tempConverter;
	VstPlugin *pluginInstance = (VstPlugin *)effect;

	if(!frame)
		return;

	if(value < 0.0f)
		value = 0.0f;

	switch(index)
	{
		case VstPlugin::DelayMix:
			delayMix->setValue(value);
			break;
		case VstPlugin::DelayShortTimes:
			delayShortTimes->setValue(value);
			setParameter(VstPlugin::DelayDelayL,
						 effect->getParameter(VstPlugin::DelayDelayL));
			setParameter(VstPlugin::DelayDelayR,
						 effect->getParameter(VstPlugin::DelayDelayR));
			break;
		case VstPlugin::DelayDelayL:
			delayLTime->setValue(value);
			pluginInstance->getParameterDisplay(VstPlugin::DelayDelayL, tempch);
			tempConverter << tempch;
			if((!pluginInstance->isSynced()) ||
			   (effect->getParameter(VstPlugin::DelayShortTimes) > 0.5f))
			{
				tempConverter << " s";
			}
			delayLTimeEdit->setText(const_cast<char *>(tempConverter.str().c_str()));
			break;
		case VstPlugin::DelayDelayLMW1:
			delayLWheel->setValue1(value);
			break;
		case VstPlugin::DelayDelayLMW2:
			delayLWheel->setValue2(value);
			break;
		case VstPlugin::DelayDelayLMW3:
			delayLWheel->setValue3(value);
			break;
		case VstPlugin::DLMx:
			delayLWheel->setValueX(value);
			break;
		case VstPlugin::DLMy:
			delayLWheel->setValueY(value);
			break;
		case VstPlugin::DelayDelayR:
			delayRTime->setValue(value);
			pluginInstance->getParameterDisplay(VstPlugin::DelayDelayR, tempch);
			tempConverter << tempch;
			if((!pluginInstance->isSynced()) ||
			   (effect->getParameter(VstPlugin::DelayShortTimes) > 0.5f))
			{
				tempConverter << " s";
			}
			delayRTimeEdit->setText(const_cast<char *>(tempConverter.str().c_str()));
			break;
		case VstPlugin::DelayDelayRMW1:
			delayRWheel->setValue1(value);
			break;
		case VstPlugin::DelayDelayRMW2:
			delayRWheel->setValue2(value);
			break;
		case VstPlugin::DelayDelayRMW3:
			delayRWheel->setValue3(value);
			break;
		case VstPlugin::DRMx:
			delayRWheel->setValueX(value);
			break;
		case VstPlugin::DRMy:
			delayRWheel->setValueY(value);
			break;
		case VstPlugin::DelayFeedback:
			delayFeedback->setValue(value);
			pluginInstance->getParameterDisplay(VstPlugin::DelayFeedback, tempch);
			delayFeedbackEdit->setText(tempch);
			break;
		case VstPlugin::DelayLevel:
			delayLevel->setValue(value);
			break;
		case VstPlugin::GranDensityMW1:
			granDensityWheel->setValue1(value);
			break;
		case VstPlugin::GranDensityMW2:
			granDensityWheel->setValue2(value);
			break;
		case VstPlugin::GranDensityMW3:
			granDensityWheel->setValue3(value);
			break;
		case VstPlugin::GDex:
			granDensityWheel->setValueX(value);
			break;
		case VstPlugin::GDey:
			granDensityWheel->setValueY(value);
			break;
		case VstPlugin::GranDensity:
			granDensity->setValue(value);
			pluginInstance->getParameterDisplay(VstPlugin::GranDensity, tempch);
			granDensityEdit->setText(tempch);
			break;
		case VstPlugin::GranDurationMW1:
			granDurationWheel->setValue1(value);
			break;
		case VstPlugin::GranDurationMW2:
			granDurationWheel->setValue2(value);
			break;
		case VstPlugin::GranDurationMW3:
			granDurationWheel->setValue3(value);
			break;
		case VstPlugin::GDrx:
			granDurationWheel->setValueX(value);
			break;
		case VstPlugin::GDry:
			granDurationWheel->setValueY(value);
			break;
		case VstPlugin::GranDuration:
			granDuration->setValue(value);
			pluginInstance->getParameterDisplay(VstPlugin::GranDuration, tempch);
			tempConverter << tempch;
			if(!pluginInstance->isSynced())
				tempConverter << " s";
			granDurationEdit->setText(const_cast<char *>(tempConverter.str().c_str()));
			break;
		case VstPlugin::GranPitchMW1:
			granPitchWheel->setValue1(value);
			break;
		case VstPlugin::GranPitchMW2:
			granPitchWheel->setValue2(value);
			break;
		case VstPlugin::GranPitchMW3:
			granPitchWheel->setValue3(value);
			break;
		case VstPlugin::GPix:
			granPitchWheel->setValueX(value);
			break;
		case VstPlugin::GPiy:
			granPitchWheel->setValueY(value);
			break;
		case VstPlugin::GranPitch:
			granPitch->setValue(value);
			pluginInstance->getParameterDisplay(VstPlugin::GranPitch, tempch);
			if(value > 0.5f)
				tempConverter << "+";
			tempConverter << tempch;
			granPitchEdit->setText(const_cast<char *>(tempConverter.str().c_str()));
			break;
		case VstPlugin::GranLevel:
			granLevel->setValue(value);
			break;
		case VstPlugin::TransValMW1:
			transValWheel->setValue1(value);
			break;
		case VstPlugin::TransValMW2:
			transValWheel->setValue2(value);
			break;
		case VstPlugin::TransValMW3:
			transValWheel->setValue3(value);
			break;
		case VstPlugin::TVax:
			transValWheel->setValueX(value);
			break;
		case VstPlugin::TVay:
			transValWheel->setValueY(value);
			break;
		case VstPlugin::TransVal:
			transVal->setValue(value);
			pluginInstance->getParameterDisplay(VstPlugin::TransVal, tempch);
			if(value > 0.5f)
				tempConverter << "+";
			tempConverter << tempch;
			transEdit->setText(const_cast<char *>(tempConverter.str().c_str()));
			break;
		case VstPlugin::TransLevel:
			transLevel->setValue(value);
			break;
		case VstPlugin::ExagValMW1:
			exagValWheel->setValue1(value);
			break;
		case VstPlugin::ExagValMW2:
			exagValWheel->setValue2(value);
			break;
		case VstPlugin::ExagValMW3:
			exagValWheel->setValue3(value);
			break;
		case VstPlugin::EVax:
			exagValWheel->setValueX(value);
			break;
		case VstPlugin::EVay:
			exagValWheel->setValueY(value);
			break;
		case VstPlugin::ExagVal:
			exagVal->setValue(value);
			pluginInstance->getParameterDisplay(VstPlugin::ExagVal, tempch);
			tempConverter << tempch;
			exagEdit->setText(const_cast<char *>(tempConverter.str().c_str()));
			break;
		case VstPlugin::ExagLevel:
			exagLevel->setValue(value);
			break;
		case VstPlugin::AccumGlissMW1:
			accumGlissWheel->setValue1(value);
			break;
		case VstPlugin::AccumGlissMW2:
			accumGlissWheel->setValue2(value);
			break;
		case VstPlugin::AccumGlissMW3:
			accumGlissWheel->setValue3(value);
			break;
		case VstPlugin::AGlx:
			accumGlissWheel->setValueX(value);
			break;
		case VstPlugin::AGly:
			accumGlissWheel->setValueY(value);
			break;
		case VstPlugin::AccumGliss:
			accumGliss->setValue(value);
			pluginInstance->getParameterDisplay(VstPlugin::AccumGliss, tempch);
			if(value > 0.5f)
				tempConverter << "+";
			tempConverter << tempch;
			accumGlissEdit->setText(const_cast<char *>(tempConverter.str().c_str()));
			break;
		case VstPlugin::AccumDecayMW1:
			accumDecayWheel->setValue1(value);
			break;
		case VstPlugin::AccumDecayMW2:
			accumDecayWheel->setValue2(value);
			break;
		case VstPlugin::AccumDecayMW3:
			accumDecayWheel->setValue3(value);
			break;
		case VstPlugin::ADex:
			accumDecayWheel->setValueX(value);
			break;
		case VstPlugin::ADey:
			accumDecayWheel->setValueY(value);
			break;
		case VstPlugin::AccumDecay:
			accumDecay->setValue(value);
			pluginInstance->getParameterDisplay(VstPlugin::AccumDecay, tempch);
			tempConverter << tempch;
			accumDecayEdit->setText(const_cast<char *>(tempConverter.str().c_str()));
			break;
		case VstPlugin::AccumLevel:
			accumLevel->setValue(value);
			break;
		case VstPlugin::ReverseDirectionMW1:
			reverseDirectionWheel->setValue1(value);
			break;
		case VstPlugin::ReverseDirectionMW2:
			reverseDirectionWheel->setValue2(value);
			break;
		case VstPlugin::ReverseDirectionMW3:
			reverseDirectionWheel->setValue3(value);
			break;
		case VstPlugin::RDix:
			reverseDirectionWheel->setValueX(value);
			break;
		case VstPlugin::RDiy:
			reverseDirectionWheel->setValueY(value);
			break;
		case VstPlugin::ReverseDirection:
			reverseDirection->setValue(value);
			break;
		case VstPlugin::ReverseLevel:
			reverseLevel->setValue(value);
			break;
		case VstPlugin::Mix:
			mix->setValue(value);
			pluginInstance->getParameterDisplay(VstPlugin::Mix, tempch);
			tempConverter << tempch;
			mixEdit->setText(const_cast<char *>(tempConverter.str().c_str()));
			break;
		case VstPlugin::LevelMW1:
			levelWheel->setValue1(value);
			break;
		case VstPlugin::LevelMW2:
			levelWheel->setValue2(value);
			break;
		case VstPlugin::LevelMW3:
			levelWheel->setValue3(value);
			break;
		case VstPlugin::OLex:
			levelWheel->setValueX(value);
			break;
		case VstPlugin::OLey:
			levelWheel->setValueY(value);
			break;
		case VstPlugin::Level:
			level->setValue(value);
			pluginInstance->getParameterDisplay(VstPlugin::Level, tempch);
			tempConverter << "x " << tempch;
			levelEdit->setText(const_cast<char *>(tempConverter.str().c_str()));
			break;
		case VstPlugin::HostSync:
			sync->setValue(value);
			setParameter(VstPlugin::DelayDelayL,
						 effect->getParameter(VstPlugin::DelayDelayL));
			setParameter(VstPlugin::DelayDelayR,
						 effect->getParameter(VstPlugin::DelayDelayR));
			setParameter(VstPlugin::GranDuration,
						 effect->getParameter(VstPlugin::GranDuration));
			break;
	}
}

//------------------------------------------------------------------------------
void EffectsEditor::valueChanged(CDrawContext *context, CControl *control)
{
	VstPlugin *pluginInstance = (VstPlugin *)effect;

	if(control == delayLWheel)
	{
		effect->setParameterAutomated(delayLWheel->getTag1(),
									  delayLWheel->getValue1());
		effect->setParameterAutomated(delayLWheel->getTag2(),
									  delayLWheel->getValue2());
		effect->setParameterAutomated(delayLWheel->getTag3(),
									  delayLWheel->getValue3());
		effect->setParameterAutomated(delayLWheel->getTagX(),
									  delayLWheel->getValueX());
		effect->setParameterAutomated(delayLWheel->getTagY(),
									  delayLWheel->getValueY());
	}
	else if(control == delayRWheel)
	{
		effect->setParameterAutomated(delayRWheel->getTag1(),
									  delayRWheel->getValue1());
		effect->setParameterAutomated(delayRWheel->getTag2(),
									  delayRWheel->getValue2());
		effect->setParameterAutomated(delayRWheel->getTag3(),
									  delayRWheel->getValue3());
		effect->setParameterAutomated(delayRWheel->getTagX(),
									  delayRWheel->getValueX());
		effect->setParameterAutomated(delayRWheel->getTagY(),
									  delayRWheel->getValueY());
	}
	else if(control == granDensityWheel)
	{
		effect->setParameterAutomated(granDensityWheel->getTag1(),
									  granDensityWheel->getValue1());
		effect->setParameterAutomated(granDensityWheel->getTag2(),
									  granDensityWheel->getValue2());
		effect->setParameterAutomated(granDensityWheel->getTag3(),
									  granDensityWheel->getValue3());
		effect->setParameterAutomated(granDensityWheel->getTagX(),
									  granDensityWheel->getValueX());
		effect->setParameterAutomated(granDensityWheel->getTagY(),
									  granDensityWheel->getValueY());
	}
	else if(control == granDurationWheel)
	{
		effect->setParameterAutomated(granDurationWheel->getTag1(),
									  granDurationWheel->getValue1());
		effect->setParameterAutomated(granDurationWheel->getTag2(),
									  granDurationWheel->getValue2());
		effect->setParameterAutomated(granDurationWheel->getTag3(),
									  granDurationWheel->getValue3());
		effect->setParameterAutomated(granDurationWheel->getTagX(),
									  granDurationWheel->getValueX());
		effect->setParameterAutomated(granDurationWheel->getTagY(),
									  granDurationWheel->getValueY());
	}
	else if(control == granPitchWheel)
	{
		effect->setParameterAutomated(granPitchWheel->getTag1(),
									  granPitchWheel->getValue1());
		effect->setParameterAutomated(granPitchWheel->getTag2(),
									  granPitchWheel->getValue2());
		effect->setParameterAutomated(granPitchWheel->getTag3(),
									  granPitchWheel->getValue3());
		effect->setParameterAutomated(granPitchWheel->getTagX(),
									  granPitchWheel->getValueX());
		effect->setParameterAutomated(granPitchWheel->getTagY(),
									  granPitchWheel->getValueY());
	}
	else if(control == transValWheel)
	{
		effect->setParameterAutomated(transValWheel->getTag1(),
									  transValWheel->getValue1());
		effect->setParameterAutomated(transValWheel->getTag2(),
									  transValWheel->getValue2());
		effect->setParameterAutomated(transValWheel->getTag3(),
									  transValWheel->getValue3());
		effect->setParameterAutomated(transValWheel->getTagX(),
									  transValWheel->getValueX());
		effect->setParameterAutomated(transValWheel->getTagY(),
									  transValWheel->getValueY());
	}
	else if(control == exagValWheel)
	{
		effect->setParameterAutomated(exagValWheel->getTag1(),
									  exagValWheel->getValue1());
		effect->setParameterAutomated(exagValWheel->getTag2(),
									  exagValWheel->getValue2());
		effect->setParameterAutomated(exagValWheel->getTag3(),
									  exagValWheel->getValue3());
		effect->setParameterAutomated(exagValWheel->getTagX(),
									  exagValWheel->getValueX());
		effect->setParameterAutomated(exagValWheel->getTagY(),
									  exagValWheel->getValueY());
	}
	else if(control == accumGlissWheel)
	{
		effect->setParameterAutomated(accumGlissWheel->getTag1(),
									  accumGlissWheel->getValue1());
		effect->setParameterAutomated(accumGlissWheel->getTag2(),
									  accumGlissWheel->getValue2());
		effect->setParameterAutomated(accumGlissWheel->getTag3(),
									  accumGlissWheel->getValue3());
		effect->setParameterAutomated(accumGlissWheel->getTagX(),
									  accumGlissWheel->getValueX());
		effect->setParameterAutomated(accumGlissWheel->getTagY(),
									  accumGlissWheel->getValueY());
	}
	else if(control == accumDecayWheel)
	{
		effect->setParameterAutomated(accumDecayWheel->getTag1(),
									  accumDecayWheel->getValue1());
		effect->setParameterAutomated(accumDecayWheel->getTag2(),
									  accumDecayWheel->getValue2());
		effect->setParameterAutomated(accumDecayWheel->getTag3(),
									  accumDecayWheel->getValue3());
		effect->setParameterAutomated(accumDecayWheel->getTagX(),
									  accumDecayWheel->getValueX());
		effect->setParameterAutomated(accumDecayWheel->getTagY(),
									  accumDecayWheel->getValueY());
	}
	else if(control == reverseDirectionWheel)
	{
		effect->setParameterAutomated(reverseDirectionWheel->getTag1(),
									  reverseDirectionWheel->getValue1());
		effect->setParameterAutomated(reverseDirectionWheel->getTag2(),
									  reverseDirectionWheel->getValue2());
		effect->setParameterAutomated(reverseDirectionWheel->getTag3(),
									  reverseDirectionWheel->getValue3());
		effect->setParameterAutomated(reverseDirectionWheel->getTagX(),
									  reverseDirectionWheel->getValueX());
		effect->setParameterAutomated(reverseDirectionWheel->getTagY(),
									  reverseDirectionWheel->getValueY());
	}
	else if(control == levelWheel)
	{
		effect->setParameterAutomated(levelWheel->getTag1(),
									  levelWheel->getValue1());
		effect->setParameterAutomated(levelWheel->getTag2(),
									  levelWheel->getValue2());
		effect->setParameterAutomated(levelWheel->getTag3(),
									  levelWheel->getValue3());
		effect->setParameterAutomated(levelWheel->getTagX(),
									  levelWheel->getValueX());
		effect->setParameterAutomated(levelWheel->getTagY(),
									  levelWheel->getValueY());
	}
	else if(control == delayLTimeEdit)
	{
		float tempf;
		char tempch[16];
		stringstream tempConverter;

		delayLTimeEdit->getText(tempch);
		tempConverter << tempch;
		tempConverter >> tempf;
		if(effect->getParameter(VstPlugin::DelayShortTimes) < 0.5f)
		{
			if(!pluginInstance->isSynced())
			{
				tempf *= 0.5f;
				if(tempf > 1.0f)
					tempf = 1.0f;
				else if(tempf < 0.0f)
					tempf = 0.0f;
			}
			else
				tempf = getCorrectBeat(tempf);
		}
		else
		{
			tempf *= 100.0f;
			if(tempf > 1.0f)
				tempf = 1.0f;
			else if(tempf < 0.0f)
				tempf = 0.0f;
		}
		effect->setParameterAutomated(control->getTag(), tempf);
	}
	else if(control == delayRTimeEdit)
	{
		float tempf;
		char tempch[16];
		stringstream tempConverter;

		delayRTimeEdit->getText(tempch);
		tempConverter << tempch;
		tempConverter >> tempf;
		if(effect->getParameter(VstPlugin::DelayShortTimes) < 0.5f)
		{
			if(!pluginInstance->isSynced())
			{
				tempf *= 0.5f;
				if(tempf > 1.0f)
					tempf = 1.0f;
				else if(tempf < 0.0f)
					tempf = 0.0f;
			}
			else
				tempf = getCorrectBeat(tempf);
		}
		else
		{
			tempf *= 100.0f;
			if(tempf > 1.0f)
				tempf = 1.0f;
			else if(tempf < 0.0f)
				tempf = 0.0f;
		}
		effect->setParameterAutomated(control->getTag(), tempf);
	}
	else if(control == delayFeedbackEdit)
	{
		float tempf;
		char tempch[16];
		stringstream tempConverter;

		delayFeedbackEdit->getText(tempch);
		tempConverter << tempch;
		tempConverter >> tempf;
		if(tempf > 1.0f)
			tempf = 1.0f;
		else if(tempf < 0.0f)
			tempf = 0.0f;
		effect->setParameterAutomated(control->getTag(), tempf);
	}
	else if(control == granDensityEdit)
	{
		float tempf;
		char tempch[16];
		stringstream tempConverter;

		granDensityEdit->getText(tempch);
		tempConverter << tempch;
		tempConverter >> tempf;
		if(tempf > 1.0f)
			tempf = 1.0f;
		else if(tempf < 0.0f)
			tempf = 0.0f;
		effect->setParameterAutomated(control->getTag(), tempf);
	}
	else if(control == granDurationEdit)
	{
		float tempf;
		char tempch[16];
		stringstream tempConverter;

		granDurationEdit->getText(tempch);
		tempConverter << tempch;
		tempConverter >> tempf;
		if(!pluginInstance->isSynced())
		{
			tempf *= 0.5f;
			if(tempf > 1.0f)
				tempf = 1.0f;
			else if(tempf < 0.0f)
				tempf = 0.0f;
		}
		else
			tempf = getCorrectBeat(tempf);
		effect->setParameterAutomated(control->getTag(), tempf);
	}
	else if(control == granPitchEdit)
	{
		float tempf;
		char tempch[16];
		stringstream tempConverter;

		granPitchEdit->getText(tempch);
		tempConverter << tempch;
		tempConverter >> tempf;
		/*if(tempf < 1.0f)
		{
			tempf -= 0.25f;
			tempf *= (1.0f/1.5f);
		}
		else if(tempf == 1.0f)
			tempf = 0.5f;
		else if(tempf > 1.0f)
		{
			tempf -= 1.0f;
			tempf *= (1.0f/6.0f);
			tempf += 0.5f;
		}*/
		tempf += 24.0f;
		tempf /= 48.0f;
		if(tempf > 1.0f)
			tempf = 1.0f;
		else if(tempf < 0.0f)
			tempf = 0.0f;
		effect->setParameterAutomated(control->getTag(), tempf);
	}
	else if(control == transEdit)
	{
		float tempf;
		char tempch[16];
		stringstream tempConverter;

		transEdit->getText(tempch);
		tempConverter << tempch;
		tempConverter >> tempf;
		tempf += 12.0f;
		tempf *= (1.0f/24.0f);
		if(tempf > 1.0f)
			tempf = 1.0f;
		else if(tempf < 0.0f)
			tempf = 0.0f;
		effect->setParameterAutomated(control->getTag(), tempf);
	}
	else if(control == exagEdit)
	{
		float tempf;
		char tempch[16];
		stringstream tempConverter;

		exagEdit->getText(tempch);
		tempConverter << tempch;
		tempConverter >> tempf;
		tempf *= (1.0f/4.0f);
		if(tempf > 1.0f)
			tempf = 1.0f;
		else if(tempf < 0.0f)
			tempf = 0.0f;
		effect->setParameterAutomated(control->getTag(), tempf);
	}
	else if(control == accumGlissEdit)
	{
		float tempf;
		char tempch[16];
		stringstream tempConverter;

		accumGlissEdit->getText(tempch);
		tempConverter << tempch;
		tempConverter >> tempf;
		tempf += 2.0f;
		tempf *= 0.25f;
		if(tempf > 1.0f)
			tempf = 1.0f;
		else if(tempf < 0.0f)
			tempf = 0.0f;
		effect->setParameterAutomated(control->getTag(), tempf);
	}
	else if(control == accumDecayEdit)
	{
		float tempf;
		char tempch[16];
		stringstream tempConverter;

		accumDecayEdit->getText(tempch);
		tempConverter << tempch;
		tempConverter >> tempf;
		if(tempf > 1.0f)
			tempf = 1.0f;
		else if(tempf < 0.0f)
			tempf = 0.0f;
		effect->setParameterAutomated(control->getTag(), tempf);
	}
	else if(control == mixEdit)
	{
		float tempf;
		char tempch[16];
		stringstream tempConverter;

		mixEdit->getText(tempch);
		tempConverter << tempch;
		tempConverter >> tempf;
		if(tempf > 1.0f)
			tempf = 1.0f;
		else if(tempf < 0.0f)
			tempf = 0.0f;
		effect->setParameterAutomated(control->getTag(), tempf);
	}
	else if(control == levelEdit)
	{
		float tempf;
		char tempch[16];
		stringstream tempConverter;

		levelEdit->getText(tempch);
		tempConverter << tempch;
		tempConverter >> tempf;
		tempf *= 0.5f;
		if(tempf > 1.0f)
			tempf = 1.0f;
		else if(tempf < 0.0f)
			tempf = 0.0f;
		effect->setParameterAutomated(control->getTag(), tempf);
	}
	else
		effect->setParameterAutomated(control->getTag(), control->getValue());
}

//------------------------------------------------------------------------------
float EffectsEditor::getCorrectBeat(float val)
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
