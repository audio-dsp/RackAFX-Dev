//	ColourNote.h - A single 'note' of colour.
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

#ifndef COLOURNOTE_H_
#define COLOURNOTE_H_

#include "Colour.h"

///	A single 'note' of colour.
/*!
	Because this is a polyphonic instrument, need to store the Colour and
	Attack/Release data for each separate note.
 */
class ColourNote
{
  public:
	///	Constructor.
	ColourNote();
	///	Destructor.
	~ColourNote();

	///	Set the note's attack time.
	void setAttack(float val);
	///	Set the note's release time.
	void setRelease(float val);
	///	Set the note's 'silent' colour.
	void setSilentColour(const Colour& val);
	///	Update the note's colour if it's playing.
	/*!
		\param newColours ColourOrganGui's array of colours, so we can work
		out which one applies to us.
	 */
	void setColour(Colour * const newColours);
	///	The the note's associated MIDI note.
	void setMidiNote(int val);

	///	Set the note's 'playing' colour, reset envelope
	void noteOn(const Colour& val, float velocity);
	///	Switch the envelope to release mode.
	void noteOff();
	///	Returns this note's associated MIDI note.
	int getMidiNote() const {return midiNote;};

	///	Returns whether or not the note is currently active.
	bool getIsActive() const {return isActive;};
	///	Updates the envelope, returns the current colour.
	Colour tick();
  private:
	///	The note's 'silent' colour.
	Colour silentColour;
	///	The note's 'playing' colour.
	Colour playingColour;
	///	The note's attack time.
	float attackTime;
	///	The note's release time.
	float releaseTime;
	///	The note's envelope.
	float env;
	///	Whether or not the envelope is currently in it's attack or release stage.
	bool attackRelease;
	///	Whether or not the note is currently active.
	bool isActive;
	///	The MIDI note associated with this instance.
	int midiNote;
	///	The velocity of this note.
	float velocity;
};

#endif
