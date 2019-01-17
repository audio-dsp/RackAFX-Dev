//	MultiModSource.h - ModulationSource which can have multiple
//					   characteristics.
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

#ifndef MULTIMODSOURCE_H_
#define MULTIMODSOURCE_H_

#include "../ModulationSource.h"
#include "ModType.h"

class VstPlugin;
class ModulationManager;

///	ModulationSource which can have multiple characteristics.
class MultiModSource : public ModulationSource
{
  public:
	///	Constructor.
	/*!
		\param plugin Pointer to the owning plugin.
	 */
	MultiModSource(VstPlugin *plugin);
	///	Destructor.
	virtual ~MultiModSource();

	///	Changes which ModType we're using.
	void changeModType(unsigned int val);

	///	Generates a block of output data.
	/*!
		\param blockSize The number of samples to generate.
		\param barStart The number of samples to the next bar start.
		\param ppq The number of ppq in a bar.

		Note that the output of the MultiModSource may be obtained from
		ModulationManager - all this method does is fill up tempBlock with
		a new block of data.
	 */
	void generateBlock(VstInt32 blockSize, VstInt32 barStart, float ppq);

	///	Called from the plugin whenever the process() block size changes.
	void setBlockSize(VstInt32 newSize);

	///	Called from the plugin to set the current samplerate.
	void setSamplerate(float newRate);

	///	Returns the current position of the PhysModKnob.
	float getKnobPos() const;
  private:
	///	Our array of ModTypes.
	ModType *modTypes[3];
	///	The ModType we're currently using.
	int modIndex;
};

#endif
