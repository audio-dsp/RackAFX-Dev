//	Accum.h - PVOC Accum effect.
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

#ifndef ACCUM_H_
#define ACCUM_H_

#include "../EffectBase.h"
#include "../ParameterManager.h"
#include "../ModulationManager.h"
#include "NiallsPVOC/BaseClasses.h"
#include "transform.h"

///	PVOC Accum effect.
class Accum : public EffectBase,
						 ParameterCallback
{
  public:
	///	Constructor.
	Accum(VstPlugin *plugin);
	///	Destructor.
	~Accum();

	///	Returns a pointer to a block of audio for the current process() block.
	float **getBlock(float **input, VstInt32 blockSize);

	///	So we get informed when the parameter is changed.
	void parameterChanged(VstInt32 index, float val);
	///	Returns the current value of the indexed parameter.
	float getValue(VstInt32 index);
	///	The textual display of the indexed parameter's current value.
	std::string getTextValue(VstInt32 index);
  private:
	///	Overridden so we can re-configure exag accordingly.
	void samplerateChanged();

	///	Enum containing IDs for this effect's parameters.
	enum
	{
		GlissMW1,
		GlissMW2,
		GlissMW3,
		Gliss,
		DecayMW1,
		DecayMW2,
		DecayMW3,
		Decay,
		Level,

		NumParams
	};

	///	Our accutransformer class (left channel).
	//accutransformer accumLeft;
	AccuTransformerBase *accumLeft2;
	///	Our accutransformer class (right channel).
	//accutransformer accumRight;
	AccuTransformerBase *accumRight2;

	///	Pointer to this plugin instance's ParameterManager.
	ParameterManager *paramManager;
	///	The ParameterManager IDs for all our parameters.
	VstInt32 paramIds[NumParams];
	///	The parameter values.
	float parameters[NumParams];

	///	Pointer to this plugin instance's ModulationManager.
	ModulationManager *modManager;
};

#endif
