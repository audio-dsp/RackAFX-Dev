//	NoteMaster.h - Coordinates the voices.
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

#ifndef NOTEMASTER_H_
#define NOTEMASTER_H_

#include "Voice.h"

#define NUM_VOICES 16

class HarmonicalEditor3D;

///	Class to handle all the voices in an instrument.
/*!
	Handles voice allocation etc.
 */
class NoteMaster
{
  public:
	NoteMaster(float samplerate);
	~NoteMaster();

	///	Gets the current sample.
	twofloats getSample();

	///	Handles a Note On message.
	void onNoteOn(unsigned char note, float velocity);
	///	Handles a Note Off message.
	void onNoteOff(unsigned char note);
	///	Handles a CC message (for MIDI controller support).
	void onCC(unsigned char cc, float value);
	///	Handles a pitch-bend message.
	void onPitchBend(float value);

	///	Sets the samplerate.
	void setSamplerate(float value);
	///	Sets a parameter.
	void setParameter(long index, float value);
	///	Sets the 3d editor which displays the objects.
	void setEditor(HarmonicalEditor3D *ed);
  private:
	///	The array of voices.
	Voice *voices;
	///	Array to keep track of which notes are currently active.
	bool voiceIsActive[NUM_VOICES];
	///	Array to keep track of which voices are assigned which notes.
	unsigned char notes[NUM_VOICES];
	///	The current samplerate.
	float samplerate;

	///	Counter to only update gui vertices every so often.
	int guiCount;
	///	3d editor to call to update the vertices.
	HarmonicalEditor3D *ed;
};

#endif
