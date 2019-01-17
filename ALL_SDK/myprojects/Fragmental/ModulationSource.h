//	ModulationSource.h - Abtract base class for all modulation sources.
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

#ifndef MODULATIONSOURCE_H_
#define MODULATIONSOURCE_H_

#include "HelperStuff.h"

class VstPlugin;
class ModulationManager;

///	Abstract base class for all modulation sources.
class ModulationSource
{
  public:
	///	Constructor.
	/*!
		\param plugin Used to retrieve the ModulationManager and the
		ParameterManager from the VstPlugin instance.
	 */
	ModulationSource(VstPlugin *plugin);
	///	Destructor.
	virtual ~ModulationSource();

	///	Generates a block of output data.
	/*!
		\param blockSize The number of samples to generate.
		\param barStart The number of samples to the next bar start.
		\param ppq The Number of ppq in a bar.

		Note that the output of the ModulationSource may be obtained from
		ModulationManager - all this method does is fill up tempBlock with
		a new block of data.
	 */
	virtual void generateBlock(VstInt32 blockSize,
							   VstInt32 barStart,
							   float ppq) = 0;

	///	Called from the plugin whenever the process() block size changes.
	void setBlockSize(VstInt32 newSize);
	///	Returns the object's current blockSize.
	/*!
		i.e. the allocated size of tempBlock.
	 */
	long getBlockSize() const {return tempBlockSize;};

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
	///	Pointer to a temporary, pre-allocated block of memory.
	/*!
		Contains the output audio data for generateBlock() - i.e. in
		generateBlock(), you should write your output data to tempBlock, and
		then return a pointer to tempBlock.
	 */
	float *tempBlock;
  private:
	///	Our ModulationManager.
	ModulationManager *modManager;
	///	Our index with the ModulationManager.
	int index;

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
