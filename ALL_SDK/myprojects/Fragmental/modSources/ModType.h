//	ModType.h - Abstract base class used by the different types of mod source.
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

#ifndef MODTYPE_H_
#define MODTYPE_H_

#include "../HelperStuff.h"

class VstPlugin;

///	Abstract base class used by the different types of mod source.
class ModType
{
  public:
	///	Constructor.
	ModType(VstPlugin *plugin) {};
	///	Destructor.
	virtual ~ModType() {};

	///	Returns the current block for this ModType.
	/*!
		\param block Pointer to the block to write to.
		\param blockSize The number of samples to calculate.
		\param barStart The number of samples to the next bar start.
		\param ppq The number of ppq in a bar.
	 */
	virtual void getBlock(float *block,
						  VstInt32 blockSize,
						  VstInt32 barStart,
						  float ppq) = 0;

	///	So we're informed when the samplerate changes.
	virtual void setSamplerate(float rate) {};
};

#endif
