//	ADSR.h - ADSR classes, so we can use polymorphism to avoid big switch-case
//			 statements.
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

#ifndef ADSR_H_
#define ADSR_H_

class Voice;

//----------------------------------------------------------------------------
///	Simple singleton class to create and store a tanh waveform.
/*!
	Used by the Attack, Decay and Release sections to provide a nice smooth
	transition.
	We don't need to have more than one of these in existence and taking up
	space at any one time, hence the fact it's a singleton.
 */
class Tanh
{
  public:
	///	This is how we access the single instance of the Tanh object.
	/*!
		If it has not been created yet, this method will create it and return
		a pointer to it, else it will just return a pointer to it.
	 */
	static Tanh *getInstance();

	///	Called to delete the single instance of the object.
	static void removeInstance();

	///	We want to treat the Tanh class as a simple float array, so we overload the [] operator to simplify things...
	/*!
		i.e. you'd just call Tanh::getInstance()[index] to get the index-ed
		value of the array. (you'd probably want to simplify things by
		storing the Tanh pointer returned by Tanh::getInstance() earlier, and
		just doing tanhPointer[index])
	 */
#ifdef WIN32
	__forceinline float operator[](int index) {return array[index];};
#else
	inline float operator[](int index) {return array[index];};
#endif

	enum
	{
		arrSize = 4096 //size of the array
	};
  private:
	///	The constructor is private because we don't want to ever create the object outside of the getInstance() method.
	Tanh();
	///	Private so we can't accidentally delete it, without calling removeInstance().
	~Tanh() {};

	///	The instance of the Tanh class.
	static Tanh *instance;
	///	Reference counting, to only delete instance when all the objects etc. using it have finished with it.
	static int refCount;

	///	The array itself.
	float array[arrSize];
};

//----------------------------------------------------------------------------
///	ADSR section base class, all ADSR section subclasses will inherit from this.
class ADSRSection
{
  public:
	ADSRSection(float samplerate, Voice *voice, bool isMain);
	virtual ~ADSRSection();

	///	Some section subclasses (Decay, Release) will have a destination/start value which depends on some other value - it's set with this method.
	/*!
		Also sets index to 0, because this is the only place we can do it.
	 */
	virtual void setLevelVal(float val) {levelVal = val; index = 0.0f;};
	///	Sets the samplerate (take the control rate into account).
	virtual void setSamplerate(float val);
	///	Sets the time taken to reach the end of the section.
	virtual void setTime(float val);

	///	Returns the current levelVal.
	virtual float getLevelVal() {return levelVal;};

	///	Returns the current sample.
	virtual float getSample() = 0;
  protected:
	///	Simple linear interpolation method.
	float linInterp(float x, float val1, float val2);

	///	The voice this section belongs to.
	/*!
		We need this to tell the voice when to update it's ADSR pointer to
		the next section.
	 */
	Voice *voice;
	///	Contains a tanh wave in a lookup table.
	Tanh *envShape;
	///	Samplerate (/controlrate)
	float samplerate;

	float levelVal;

	float index;
	float increment;

	///	Determines whether the object should signal it's voice when it's finished.
	bool isMain;
};

//----------------------------------------------------------------------------
///	Attack section class.
/*!
	i.e. 0->1
 */
class ADSRAttack : public ADSRSection
{
  public:
	ADSRAttack(float samplerate, Voice *voice, bool isMain);
	~ADSRAttack() {};

	float getSample();
};

//----------------------------------------------------------------------------
///	Decay section class.
/*!
	i.e. 1->levelVal
 */
class ADSRDecay : public ADSRSection
{
  public:
	ADSRDecay(float samplerate, Voice *voice, bool isMain);
	~ADSRDecay() {};

	float getSample();

	///	Because levelVal here is just the current value of the envelope.
	void setLevelVal(float val) {endVal = val; index = 0.0f;};
  private:
	///	The sustain section level.
	float endVal;
};

//----------------------------------------------------------------------------
///	Sustain section class.
/*!
	i.e. levelVal while note is held down
 */
class ADSRSustain : public ADSRSection
{
  public:
	ADSRSustain(float samplerate, Voice *voice, bool isMain);
	~ADSRSustain() {};

	float getSample();
};

//----------------------------------------------------------------------------
///	Release section class.
/*!
	i.e. levelVal->0
 */
class ADSRRelease : public ADSRSection
{
  public:
	ADSRRelease(float samplerate, Voice *voice, bool isMain);
	~ADSRRelease() {};

	void setLevelVal(float val) {levelVal = val; index = 0.0f; isFinished = false;};

	float getSample();
  private:
	bool isFinished;
};

#endif
