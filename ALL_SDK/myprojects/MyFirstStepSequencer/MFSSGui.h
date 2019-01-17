//	MFSSGui.h - The GUI class for My First Step Sequencer.
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

#ifndef MFSSGUI_H_
#define MFSSGUI_H_

#include "vstgui.h"
#include "StateButton.h"
#include "NiallsAbout.h"
#include "CSimpleKeyboardSwitch.h"

///	The GUI class (AEffGuiEditor) for the My First Step Sequencer plugin.
class MFSSGui : public AEffGUIEditor,
				public CControlListener
{
  public:
	///	Constructor.
	MFSSGui(AudioEffect *effect);
	///	Destructor.
	~MFSSGui();

	///	Called to display the gui.
	/*!
		\param ptr Pointer to the (platform-specific) window the host has
		provided for us.
	 */
	long open(void *ptr);
	///	Called when the gui is closed.
	/*!
		Use this to free memory etc.
	 */
	void close();

	///	Called from the plugin when a parameter has changed.
	/*!
		\param index The index of the parameter which has changed.
		\param value The new value of the parameter which has changed.
	 */
	void setParameter(long index, float value);
	///	Called when one of the gui's controls' value changes.
	/*!
		\param context The draw context which the control which has changed
		belongs to (not sure why this is necessary?).
		\param control Pointer to the control which has changed.

		We use this to inform the plugin of the updated parameter data.
	 */
	void valueChanged(CDrawContext* context, CControl* control);

	///	A kind of nasty little workaround for the about box.
	/*!
		For some reason, the scaleKeyboard widget always gets redrawn on top
		of the about box, so you have to manually tell it not to do this.
		Unfortunately it means the NiallsAbout widget isn't portable to other
		editors without a similar modification, but I don't see how else you
		could do it.
	 */
	void redrawEverything();
  private:
	///	Sets up the scale forcing.
	/*!
		i.e. what happens when the user sets the scale via the scaleKeyboard.
	 */
	void setUpScaleForcing();

	///	Control holding the advanced section.
	CViewContainer *advancedSection;
	///	The advanced section's 'hidden' state.
	CView *advancedHidden;
	///	The advanced section's 'shown' state.
	CView *advancedShown;

	///	Button to switch states on the advanced section.
	COnOffButton *advancedSwitch;
	///	The Octave 'button'.
	StateButton *octaveButton;
	///	Bypass button.
	COnOffButton *bypassButton;
	///	The about box.
	NiallsAbout *aboutBox;
	///	The shuffle button.
	COnOffButton *shuffleButton;
	///	The Notes/Heads button.
	COnOffButton *notesButton;
	///	The '8-notes' button.
	COnOffButton *numNotesButton;
	///	The 'Scale' keyboard.
	CSimpleKeyboardSwitch *scaleKeyboard;
	///	The random button.
	CKickButton *randomButton;

	///	Array of buttons for the Notes/Animal Heads.
	StateButton *noteButtons[16];
	///	Array of buttons for the Volumes.
	StateButton *volumeButtons[16];

	///	The gui's background image.
	CBitmap *backgroundImage;
	///	The advanced section's 'hidden' state background.
	CBitmap *advancedHiddenImage;
	///	The advanced section's 'shown' state background.
	CBitmap *advancedShownImage;
	///	The bitmap for the advancedSwitch button.
	CBitmap *advancedSwitchImage;
	///	The bitmap for the octave 'button'.
	CBitmap *octaveButtonImage;
	///	The bitmap for the bypass button.
	CBitmap *bypassButtonImage;
	///	The bitmap for the about box button.
	CBitmap *aboutButtonImage;
	///	The bitmap for the actual about box.
	CBitmap *aboutImage;
	///	The bitmap for the Note buttons.
	CBitmap *notesImage;
	///	The bitmap for the Animal Head buttons.
	CBitmap *notesImage2;
	///	The bitmap for the volume buttons.
	CBitmap *volumesImage;
	///	The bitmap for the shuffle button.
	CBitmap *shuffleImage;
	///	The bitmap for the Notes/Heads button.
	CBitmap *notesHeadsImage;
	///	The bitmap for the '8-notes' button.
	CBitmap *numNotesImage;
	///	The bitmap for the random button.
	CBitmap *randomImage;

	///	Array of tags for the Scale keyboard (starts at MyFirstStepSequencer::ScaleStart).
	long scaleTags[12];

	///	Resource IDs.
	enum
	{
		BackgroundImage = 127,
		AdvancedHiddenImage,
		AdvancedShownImage,
		AdvancedSwitchImage,
		OctaveButtonImage,
		BypassButtonImage,
		AboutButtonImage,
		AboutImage,
		NotesImage2,
		VolumesImage,
		ShuffleImage,
		NotesHeadsImage,
		NumNotesImage,
		NotesImage,
		RandomImage,

		///Gui only tags...
		AdvancedSwitch = 2000,
		NotesHeads
	};
};

#endif
