//	EffectBase.h - Abstract base class for the individual effect classes.
//	--------------------------------------------------------------------------
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
//	--------------------------------------------------------------------------

#ifndef EFFECTBASE_H_
#define EFFECTBASE_H_

#include "HelperStuff.h"

#define DenormalFix 1e-18f

class VstPlugin;

///	Abstract base class for the individual effect classes.
class EffectBase
{
  public:
	///	Constructor.
	/*!
		\param plugin Should be used by subclasses to retrieve the
		ModulationManager and the ParameterManager from the VstPlugin
		instance.
	 */
	EffectBase(VstPlugin *plugin);
	///	Destructor.
	virtual ~EffectBase();

	///	Returns a pointer to a block of audio for the current process() block.
	/*!
		\param input Pointer to a block of input samples, blockSize in size.
		\param blockSize The size of the block of input samples.

		Really just returns a pointer to tempBlock - tempBlock will have it's
		size allocated in setBlockSize(), so you will never have to worry
		about having to handle this yourself.

		To handle modulation from any of the modulation sources, you have to
		go through the ModulationManager singleton.  This object contains
		a float array for each modulation source (publicly accessable), the
		same size as the current audio block, which contains the output of
		that modulation source for this block.
	 */
	virtual float **getBlock(float **input, VstInt32 blockSize) = 0;

	///	Called from the plugin whenever the process() block size changes.
	void setBlockSize(VstInt32 newSize);
	///	Returns the object's current blockSize.
	/*!
		i.e. the allocated size of tempBlock.
	 */
	strictinline VstInt32 getBlockSize() const {return tempBlockSize;};

	///	Called from the plugin to set the current samplerate.
	void setSamplerate(float newRate);
	///	Returns the current samplerate.
	strictinline float getSamplerate() const {return samplerate;};
	///	Called from the plugin to set the current tempo.
	void setTempo(float val);
	///	Returns the current tempo.
	strictinline float getTempo() const {return tempo;};

	///	Called from the plugin to indicate we're in host sync mode.
	void setSyncMode(bool val);
	///	Returns whether or not we're in host sync mode at the moment.
	strictinline bool getSyncMode() const {return syncMode;};
  protected:
	///	Called when the samplerate changes.
	virtual void samplerateChanged() {};

	///	Pointer to a temporary, pre-allocated block of memory.
	/*!
		Contains the output audio data for getBlock() - i.e. in getBlock(),
		you should write your output data to tempBlock, and then return a
		pointer to tempBlock.
	 */
	float *tempBlock[2];
  private:
	///	The current size of tempBlock.
	VstInt32 tempBlockSize;
	///	The current samplerate.
	float samplerate;
	///	The current tempo.
	float tempo;
	///	Whether or not we're in host sync mode.
	bool syncMode;
};

#endif
