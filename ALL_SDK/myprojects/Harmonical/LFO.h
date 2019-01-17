//	LFO.h - LFO classes, so we can use polymorphism to avoid big switch-case
//			statements.
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

#ifndef LFO_H_
#define LFO_H_

//I got these from Windows' calculator.
#ifndef PI_DEFS
#define PI_DEFS
#define PI 3.1415926535897932384626433832795
#define TWOPI 6.283185307179586476925286766559
#define INV_TWOPI (1.0/TWOPI)
#endif

///	Base class all LFOs will inherit from.
/*!
	So, in your code, you have an LFOBase pointer, which you use to access the
	different types of LFO, instead of a switch-case statement, like:
	\code
	float out;
	LFOBase *lfoPointer;
	LFOSine lfoSine(44100.0f);
	LFOSaw lfoSaw(44100.0f);

	lfoPointer = &lfoSine;

	//and to get the current sample for the lfo:
	out = lfoPointer->getSample();

	//and if we change where lfoPointer's pointing...
	lfoPointer = &lfoSaw;
	//...it's still the same method to get the current sample for the lfo:
	out = lfoPointer->getSample();
	\endcode
	The point being that the whole switch-case statement can be replaced by
	a single call to LFOBase::getSample, and you just set which LFO type
	you're using when the user changes it via setParameter (or whatever).
 */
class LFOBase
{
  public:
	LFOBase(float samplerate);
	virtual ~LFOBase() {};

	///	Note: if you're only calculating modulation stuff every N samples, alter the samplerate accordingly (why didn't I think of this sooner?)
	virtual void setSamplerate(float val);
	///	Sets the rate/frequency of the oscillator.
	virtual void setRate(float val);
	///	Sets the phase of the oscillator (useful for re-triggering the osc).
	virtual void setPhase(float val);

	///	Returns the rate (frequency) the LFO is currently running at.
	virtual float getRate();
	///	Returns the current sample.
	virtual float getSample() = 0;
  protected:
	float samplerate;
	///	index goes 0 -> 2pi
	float index;
	///	increment set according to the rate
	float increment;
};

///	Simple sinewave LFO class.
class LFOSine : public LFOBase
{
  public:
	LFOSine(float samplerate);
	~LFOSine() {};

	float getSample();
};

///	Simple sawtooth LFO class.
class LFOSaw : public LFOBase
{
  public:
	LFOSaw(float samplerate);
	~LFOSaw() {};

	float getSample();
};

///	Simple ramp LFO class.
class LFORamp : public LFOBase
{
  public:
	LFORamp(float samplerate);
	~LFORamp() {};

	float getSample();
};

///	Simple square wave LFO class.
class LFOSquare : public LFOBase
{
  public:
	LFOSquare(float samplerate);
	~LFOSquare() {};

	float getSample();
};

///	Simple sample & hold wave LFO class.
class LFOSH : public LFOBase
{
  public:
	LFOSH(float samplerate);
	~LFOSH() {};

	float getSample();
  private:
	///	Only update the random value every PI, so we need to keep track of the last value.
	float lastval;
};

#endif
