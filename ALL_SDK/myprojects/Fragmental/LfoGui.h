//	LfoGui.h - GUI for the LFO mod source.
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

#ifndef LFOGUI_H_
#define LFOGUI_H_

#include "ModSourceGui.h"
#include "VstPlugin.h"

class CButtonMenu;
class LFOBarEditor;

///	GUI for the LFO mod source.
class LfoGui : public ModSourceGui
{
  public:
	///	Constructor.
	LfoGui(ModContainer *container,
		   int tagStart,
		   CControlListener *listener,
		   VstPlugin *pluginInstance);
	///	Destructor.
	~LfoGui();

	///	Updates a parameter.
	void setParameter(int index, float value, CDrawContext *context = 0);

	///	Returns the number of parameters used by this mod source.
	int getNumParameters() const {return 38;};

	///	Shows the gui.
	void show();
	///	Hides the gui.
	void hide();
  private:
	///	Simple round function.
	inline int round(float val)
	{
		int tempint = (int)val;

		if((val-tempint) < 0.5f)
			return tempint;
		else
			return tempint+1;
	}

	///	Enums for the images.
	enum
	{
		BackgroundImage = 228,
		ComboImage,
		KnobImage = 128,
		ButtonImage = 130
	};

	///	The owning ModContainer.
	ModContainer *modContainer;
	///	Whether the gui's currently displayed or not.
	bool isDisplayed;
	///	The start tag for this mod source.
	int start;

	///	The background CView.
	CView *background;

	///	The type menu.
	COptionMenu *typeMenu;
	///	Kick button for the type menu.
	CKickButton *typeButton;
	///	The lfo shape control.
	LFOBarEditor *shape;
	///	The num steps menu.
	COptionMenu *numSteps;
	///	The preset menu.
	CButtonMenu *presetMenu;
	///	The frequency knob.
	CKnob *frequency;
	///	The frequency text editor.
	CTextEdit *frequencyEdit;
	///	The smooth knob.
	CKnob *smooth;
	///	The smooth text editor.
	CTextEdit *smoothEdit;
	///	The freq range button.
	COnOffButton *freqRange;

	///	The background image.
	CBitmap *backgroundImage;
	///	The combobox button image.
	CBitmap *comboImage;
	///	The knob tick image.
	CBitmap *knobImage;
	///	The tickbox image.
	CBitmap *buttonImage;

	///	VstPlugin instance.
	VstPlugin *effect;
};

#endif
