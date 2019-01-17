//	KnobGui.h - GUI for the knob mod source.
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

#ifndef KNOBGUI_H_
#define KNOBGUI_H_

#include "ModSourceGui.h"

class TwoTickKnob;

///	GUI for the knob mod source.
class KnobGui : public ModSourceGui
{
  public:
	///	Constructor.
	KnobGui(ModContainer *container,
			int tagStart,
			CControlListener *listener,
			VstPlugin *pluginInstance);
	///	Destructor.
	~KnobGui();

	///	Updates a parameter.
	void setParameter(int index, float value, CDrawContext *context = 0);

	///	Returns the number of parameters used by this mod source.
	int getNumParameters() const {return 38;};

	///	Shows the gui.
	void show();
	///	Hides the gui.
	void hide();

	///	Used to update the knob's position.
	void timerCallback();
  private:
	///	Enums for the images.
	enum
	{
		BackgroundImage = 231,
		ComboImage = 229,
		KnobImage = 128,
		BigKnobImage = 235,
		KnobExtraImage
	};

	///	The owning ModContainer.
	ModContainer *modContainer;
	///	Pointer to the plugin instance.
	VstPlugin *plugin;
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
	///	The big knob.
	TwoTickKnob *knob;
	///	The mass knob.
	CKnob *mass;
	///	The mass text editor.
	CTextEdit *massEdit;
	///	The spring stiffness knob.
	CKnob *stiffness;
	///	The spring stiffness text editor.
	CTextEdit *stiffnessEdit;

	///	The background image.
	CBitmap *backgroundImage;
	///	The combobox button image.
	CBitmap *comboImage;
	///	The knob tick image.
	CBitmap *knobImage;
	///	The big knob tick image.
	CBitmap *bigKnobImage;
	///	The knob extra tick image.
	CBitmap *knobExtraImage;

	///	The plugin instance.
	VstPlugin *effect;
};

#endif
