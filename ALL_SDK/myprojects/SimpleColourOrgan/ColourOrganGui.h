//	ColourOrganGui.h - The GUI of the plugin (it's basically all gui anyway).
//	--------------------------------------------------------------------------
//	Copyright (c) 2006 Niall Moody
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

#ifndef COLOURORGANGUI_H_
#define COLOURORGANGUI_H_

#include "VSTGLEditor.h"
#include "VSTGLTimer.h"
#include "Colour.h"
#include "CriticalSection.h"
#include "ColourNote.h"
#include <vector>
#include <string>

///	The GUI of the plugin (it's basically all gui anyway).
class ColourOrganGui : public VSTGLEditor,
					   public Timer
{
  public:
	///	Constructor.
	ColourOrganGui(AudioEffect *effect);
	///	Destructor.
	~ColourOrganGui();

	///	Called when the Gui's window is opened.
	void guiOpen();
	///	Called when the Gui's window is closed.
	void guiClose();

	///	Use mouse buttons to cycle through colour palettes.
	void onMouseDown(int button, int x, int y);

	///	Draws the colours.
	void draw();

	///	To update the parameters.
	void setParameter(int index, float value);

	///	Called repeatedly, to update the graphics.
	void timerCallback();

	///	Called from VstPlugin when it receives a Note On MIDI message.
	void noteOn(int note, float vel);
	///	Called from VstPlugin when it receives a Note Off MIDI message.
	void noteOff(int note);
  private:
	///	Helper method: Loads the correct palette according to paletteIndex.
	/*!
		Palette will be loaded from the SimpleColourOrgan.xml file in the
		user's home directory.
	 */
	void loadPalette();
	///	Helper method: Determines whether or not the palette xml file exists.
	bool doesPaletteFileExist() const;
	///	Helper method: Creates the palette xml file if it doesn't exist.
	/*!
		File is created in the user's home directory - this will be different
		according to the os.
	 */
	void createPaletteFile() const;
	///	Helper method: Returns the number of palettes in the xml file.
	int getNumPalettes() const;

	///	Helper method: Returns a string containing the user's home directory.
	/*!
		Appends the appropriate directory separator onto the end.

		\todo Fix for OSX.
	 */
	std::string getHomeDir() const;

	///	Index to the current colour palette.
	int paletteIndex;
	///	The attack time in seconds.
	float attackTime;
	///	The release time in seconds.
	float releaseTime;
	///	The current MIDI velocity.
	float velocity;

	///	The current colour (full on, disregarding velocity and envelope).
	Colour currentColour;
	///	The 'silent' colour.
	/*!
		The 'silent' colour is the colour that's displayed when no note is
		playing.
	 */
	Colour silentColour;
	///	Our current colour palette.
	Colour colourPalette[12];
	///	The value of the envelope.
	float env;
	///	The number of notes currently playing
	/*!
		(used to calculate the colour weightings)
	 */
	int numNotes;
	///	Array of all the notes.
	ColourNote notes[128];

	///	Array of currently playing notes.
	/*!
		Each entry contains a value between 0 and 11 if it's playing, -1 if
		it's not.
	 */
	//std::vector<int> notes;

	///	Critical section used to make sure we don't run into multithreading issues.
	/*!
		This is used to avoid threading issues when setting/reading
		currentColour.
	 */
	//CriticalSection colourSection;
};

#endif
