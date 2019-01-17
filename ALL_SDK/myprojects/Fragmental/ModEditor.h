//	ModEditor.h - The VSTGUI editor handling the modulation parameters.
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

#ifndef MODEDITOR_H_
#define MODEDITOR_H_

#include "ModContainer.h"

#include "aeffguieditor.h"

///	The VSTGUI editor handling the modulation parameters.
class ModEditor : public AEffGUIEditor,
				  public CControlListener
{
  public:
	///	Constructor.
	ModEditor(AudioEffect *effect);
	///	Destructor.
	~ModEditor();

	///	Called when the editor should be opened.
	bool open(void *systemPointer);
	///	Called when the editor should be closed.
	void close();

	///	Called from the plugin to set a parameter.
	void setParameter(VstInt32 index, float value);
	///	Called from controls when their value changes.
	void valueChanged(CDrawContext *context, CControl *control);

	///	Used to update the knob's position.
	void timerCallback();
  private:
	///	Helper method to quantise a float number to the correct beat value.
	float getCorrectBeat(float val);
	///	Helper method for the freq displays.
	inline float fakeExp(float val)
	{
		return val/(val + (-9.0f*(val-1.0f)));
	};

	///	Enum keeping track of the image IDs.
	enum
	{
		BackgroundImage = 227,
		BlueImage = 232,
		RedImage,
		GreenImage
	};

	///	The CViewContainer for the mod sources.
	CViewContainer *container;
	///	The red source button.
	COnOffButton *redButton;
	///	The green source button.
	COnOffButton *greenButton;
	///	The blue source button.
	COnOffButton *blueButton;


	///	Background image.
	CBitmap *backgroundImage;
	///	Red source image.
	CBitmap *redImage;
	///	Green source image.
	CBitmap *greenImage;
	///	Blue source image.
	CBitmap *blueImage;

	///	The red mod source.
	ModContainer *redSource;
	///	The green mod source.
	ModContainer *greenSource;
	///	The blue mod source.
	ModContainer *blueSource;

	///	Which mod source is currently displayed.
	int displayedSource;
};

#endif
