//	Reverse.h - Reversinator effect.
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

#ifndef REVERSE_H_
#define REVERSE_H_

#include "../EffectBase.h"
#include "../ParameterManager.h"
#include "../ModulationManager.h"
#include "../CriticalSection.h"

///	Reversinator effect.
class Reverse : public EffectBase,
				public ParameterCallback
{
  public:
	///	Constructor.
	Reverse(VstPlugin *plugin);
	///	Destructor.
	~Reverse();

	///	Returns a pointer to a block of audio for the current process() block.
	float **getBlock(float **input, VstInt32 blockSize);

	///	So we get informed when one of our parameters changes.
	void parameterChanged(VstInt32 index, float val);
	///	Returns the current value of the indexed parameter.
	float getValue(VstInt32 index);
	///	The textual display of the indexed parameter's current value.
	std::string getTextValue(VstInt32 index);
  private:
	///	So we can resize the delay buffer accordingly.
	void samplerateChanged();

	///	constants for the paramIds variables.
	enum
	{
		DirectionMW1,
		DirectionMW2,
		DirectionMW3,
		Direction,
		Level,

		NumParams
	};

	///	Pointer to the ParameterManager for this plugin instance.
	ParameterManager *paramManager;
	///	The ParameterManager IDs for all our parameters.
	VstInt32 paramIds[NumParams];
	///	The parameter values.
	float parameters[NumParams];

	///	Pointer to the ModulationManager for this plugin instance.
	ModulationManager *modManager;

	///	Used to protect our readPointers from getting messed up when Direction changes.
	CriticalSection critSec;

	///	Size of the delay buffer.
	VstInt64 bufferSize;
	///	The delay buffer.
	float *buffer[2];
	///	The cosine buffer.
	float *cosBuffer;
	///	Write pointer.
	VstInt64 writePointer;
	///	1st read pointer.
	VstInt64 readPointer1;
	///	2nd read pointer.
	VstInt64 readPointer2;
	///	Read pointer offset.
	/*!
		Used so that when the Direction parameter changes to backward, we
		always start at the current writePointer position.  This way we can do
		neat 'instant reverse' effects whenthe parameter's modulated.
	 */
	VstInt64 readOffset;
};

#endif
