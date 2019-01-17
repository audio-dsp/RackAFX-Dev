//	Step.h - Simple class representing a single step.
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

#ifndef STEP_H_
#define STEP_H_

//----------------------------------------------------------------------------
///	Represents a MIDI message - this will always contain 3 chars
typedef unsigned char *MidiMessage;

//----------------------------------------------------------------------------
///	Represents an event handler for the two Step events (start & stop).
class StepEventHandler
{
  public:
	///	Called when a Step starts.
	virtual void stepStartEvent(long id) = 0;
	///	Called when a Step stops.
	virtual void stepStopEvent(long id) = 0;
};

//----------------------------------------------------------------------------
///	Simple class representing a single step in the sequencer.
/*!
	Includes volume/velocity data about the step, as well as the length of
	the step.  Also includes methods to 'tick' through the step, and construct
	the appropriate MIDI messages when the step starts and stops.
 */
class Step
{
  public:
	///	Default Constructor, for construction from an array.
	Step();
	///	Constructor.
	/*!
		\param We'll be using a number of Step objects, so we'll tend to want
		some way of identifying them in the event handlers, hence the id.
	 */
	Step(long id);
	///	Destructor.
	~Step();

	///	Sets this Step's id.
	void setId(long val);
	///	Register a step event handler with the object.
	/*!
		When the step starts and stops, you may want to have it precipitate
		some kind of action.  The StepEventHandler object contains two methods
		which will be called (respectively) when these events occur.
	 */
	void registerEventHandler(StepEventHandler *handler);

	///	Called to signal to the step that it should start now.
	/*!
		\param offset You may have to activate a step halfway through it's
		length - this lets you set that correctly.

		So, the call sequence is:
		activate();
			Step will then call eventHandler->stepStartEvent(), if
			eventHandler exists.
		Call tick() repeatedly, until it returns false, or Step calls
		eventHandler->stepStopEvent().

		Repeat...
	 */
	void activate(long offset = 0);
	///	Used to make sure the stop event handler is called.
	void deactivate();

	///	Sets the samplerate we're using.
	void setSamplerate(float val);
	///	Sets the bpm we're supposed to be using.
	void setBpm(float val);
	///	Sets the note length for this step.
	/*!
		1 = 1 beat, 1/2 = 0.5 beats, etc.
	 */
	void setNoteLength(float val);
	///	Sets the note value for the step.
	void setNoteValue(unsigned char val);
	///	Sets the volume/velocity value to be used with this step.
	void setVolume(float val);

	///	Returns the note length for this step.
	float getNoteLength() {return noteLength;};
	///	Returns the note value for this step.
	unsigned char getNoteValue() {return noteValue;};
	///	Returns the volume/velocity for this step.
	float getVolume() {return volume;};
	///	Returns whether or not the Step is currently running.
	bool getIsRunning() {return isRunning;};

	///	Called for every sample, to increment the internal timer.
	/*!
		\return true when the step has reached it's end.
	 */
	bool tick();
	///	Called to update the note value, if necessary.
	/*!
		Must be called from the audio thread.
	 */
#ifdef WIN32
	__forceinline void updateNoteValue()
#else
	inline void updateNoteValue()
#endif
	{
		if(noteValue != newNoteValue)
		{
			eventHandler->stepStopEvent(id);
			if(isRunning)
			{
				noteValue = newNoteValue;
				eventHandler->stepStartEvent(id);
			}
			else
				noteValue = newNoteValue;
		}
	};
	///	Returns the appropriate MidiMessage.
	/*!
		\param onOff Whether it's a NoteOn(true) or a NoteOff(false) we want.
		Make sure you delete the returned MidiMessage when you're done...
	 */
	MidiMessage getMIDI(bool onOff);
  private:
	///	The samplerate we're running at.
	float samplerate;
	///	The bpm we're using.
	float bpm;
	///	The current note length of the step.
	float noteLength;
	///	The current note value of the step.
	unsigned char noteValue;
	///	The current volume/velocity used with this step.
	float volume;

	///	Where we currently are in the step (in samples).
	long index;
	///	The event handler to call when the event starts and stops.
	StepEventHandler *eventHandler;
	///	Used to cover ourselves if activate is called while the step's still running.
	bool isRunning;

	///	Because we'll typically be using a number of Step objects, we really want to be able to identify them in the event handlers.
	long id;

	///	Used to avoid threading issues when the user changes octave etc. via the gui.
	unsigned char newNoteValue;
};

#endif
