//	Grain.h - Simple class representing a single grain.
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

#ifndef GRAIN_H_
#define GRAIN_H_

#ifdef WIN32
#define strictinline __forceinline
#elif defined (__GNUC__)
#define strictinline inline __attribute__((always_inline))
#else
#define strictinline inline
#endif

///	Simple class representing a single grain.
class Grain
{
  public:
	///	Constructor.
	/*!
		\param delayLine Pointer to the (stereo) buffer where the input audio
		is stored.
	 */
	Grain();
	///	Destructor.
	~Grain();

	///	Used to set the buffer and it's size in the plugin's constructor.
	/*!
		\param delayLine Pointer to the (stereo) buffer where the input audio
		is stored.
		\param size Size of delayLine.
	 */
	void setDelayLine(float **delayLine, int size);

	///	Call this when the Grain is activated to reset various internal parameters.
	/*!
		\param startPos Position in buffer to start reading from.
		\param size Size of the Grain in samples.
		\param inc The increment to use (to change the grain's pitch).
	 */
	void activate(int startPos, int size, float inc);
	///	Returns whether this block is active or not.
	strictinline bool getIsActive() const {return isActive;};
	///	Informs us how many Grains are active so we can scale our o/p accordingly.
	strictinline void setNumGrains(int num) {numGrains = static_cast<float>(num);};

	///	Returns the next block of audio for this Grain.
	/*!
		\param audioBlock Pointer to a stereo array of a block of audio
		samples to write to.
		\param numSamples Number of samples in audioBlock.
	 */
	void getBlock(float **audioBlock, int numSamples);
  private:
	///	Whether the grain is active or not.
	bool isActive;

	///	Pointer to the (stereo) buffer where the input audio is stored.
	float **buffer;
	///	Size of the buffer.
	int bufferSize;

	///	Current index in the buffer.
	float index;
	///	Index in the grain.
	float grainIndex;
	///	Increment controlling how fast the buffer is played back.
	float increment;
	///	Start point in the buffer.
	int startPosition;
	///	Size of the current Grain.
	int grainSize;
	///	The number of Grains currently active.
	/*!
		Used to scale the output accordingly.
	 */
	float numGrains;
};

#endif
