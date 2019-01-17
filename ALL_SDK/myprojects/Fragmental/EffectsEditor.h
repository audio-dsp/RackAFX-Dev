//	EffectsEditor.h - The VSTGUI editor handling the effects parameters.
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

#ifndef EFFECTSEDITOR_H_
#define EFFECTSEDITOR_H_

#include "aeffguieditor.h"
#include "ModWheel.h"

///	The VSTGUI editor handling the effects parameters.
class EffectsEditor : public AEffGUIEditor,
					  public CControlListener
{
  public:
	///	Constructor.
	EffectsEditor(AudioEffect *effect);
	///	Destructor.
	~EffectsEditor();

	///	Called when the editor should be opened.
	bool open(void *systemPointer);
	///	Called when the editor should be closed.
	void close();

	///	Called from the plugin to set a parameter.
	void setParameter(VstInt32 index, float value);
	///	Called from controls when their value changes.
	void valueChanged(CDrawContext *context, CControl *control);
  private:
	///	Helper method to quantise a float number to the correct beat value.
	float getCorrectBeat(float val);

	///	Enum keeping track of the image IDs.
	enum
	{
		BackgroundImage = 127,
		KnobImage,
		SliderImage,
		ButtonImage,
		BigKnobImage,
		ReverseImage,
		MWTickImage,
		SyncImage
	};

	///	Delay Mix slider.
	CVerticalSlider *delayMix;
	///	Delay Short Time button.
	COnOffButton *delayShortTimes;
	///	Delay Left Time knob.
	CKnob *delayLTime;
	///	Delay Left Time Text Editor.
	CTextEdit *delayLTimeEdit;
	///	Delay Right Time knob.
	CKnob *delayRTime;
	///	Delay Right Time Text Editor.
	CTextEdit *delayRTimeEdit;
	///	Delay Feedback knob.
	CKnob *delayFeedback;
	///	Delay Feedback Text Editor.
	CTextEdit *delayFeedbackEdit;
	///	Delay Level slider.
	CVerticalSlider *delayLevel;
	///	Delay Left Time ModWheel.
	ModWheel *delayLWheel;
	///	Delay Right Time ModWheel.
	ModWheel *delayRWheel;

	///	Granulator Density knob.
	CKnob *granDensity;
	///	Granulator Density Text Editor.
	CTextEdit *granDensityEdit;
	///	Granulator Duration knob.
	CKnob *granDuration;
	///	Granulator Duration Text Editor.
	CTextEdit *granDurationEdit;
	///	Granulator Pitch knob.
	CKnob *granPitch;
	///	Granulator Pitch Text Editor.
	CTextEdit *granPitchEdit;
	///	Granulator Level slider.
	CVerticalSlider *granLevel;
	///	Granulator Density ModWheel.
	ModWheel *granDensityWheel;
	///	Granulator Duration ModWheel.
	ModWheel *granDurationWheel;
	///	Granulator Pitch ModWheel.
	ModWheel *granPitchWheel;

	///	Transpose Value knob.
	CKnob *transVal;
	///	Transpose Value Text Editor.
	CTextEdit *transEdit;
	///	Transpose Level slider.
	CVerticalSlider *transLevel;
	///	Transpose Value ModWheel.
	ModWheel *transValWheel;

	///	Exaggerate Value knob.
	CKnob *exagVal;
	///	Exaggerate Value Text Editor.
	CTextEdit *exagEdit;
	///	Exaggerate Level slider.
	CVerticalSlider *exagLevel;
	///	Exaggerate Value ModWheel.
	ModWheel *exagValWheel;

	///	Accumulate Gliss knob.
	CKnob *accumGliss;
	///	Accumulate Gliss Text Editor.
	CTextEdit *accumGlissEdit;
	///	Accumulate Decay knob.
	CKnob *accumDecay;
	///	Accumulate Decay Text Editor.
	CTextEdit *accumDecayEdit;
	///	Accumulate Level slider.
	CVerticalSlider *accumLevel;
	///	Accumulate Gliss ModWheel.
	ModWheel *accumGlissWheel;
	///	Accumulate Decay ModWheel.
	ModWheel *accumDecayWheel;

	///	Reverse Direction button.
	COnOffButton *reverseDirection;
	///	Reverse Level slider.
	CVerticalSlider *reverseLevel;
	///	Reverse Direction ModWheel.
	ModWheel *reverseDirectionWheel;

	///	Mix slider.
	CKnob *mix;
	///	Mix Text Editor.
	CTextEdit *mixEdit;
	///	Level knob.
	CKnob *level;
	///	Level Text Editor.
	CTextEdit *levelEdit;
	///	Level ModWheel.
	ModWheel *levelWheel;

	///	Host Sync button.
	COnOffButton *sync;

#ifdef PRESETSAVER
	CKickButton *presetButton;
#endif

	///	Background image.
	CBitmap *backgroundImage;
	///	Knob tick image.
	CBitmap *knobImage;
	///	Slider tick image.
	CBitmap *sliderImage;
	///	Button image.
	CBitmap *buttonImage;
	///	Big knob image.
	CBitmap *bigKnobImage;
	///	Reverse button image.
	CBitmap *reverseImage;
	///	ModWheel tick image.
	CBitmap *mwTickImage;
	///	Host Sync image.
	CBitmap *syncImage;
};

#endif
