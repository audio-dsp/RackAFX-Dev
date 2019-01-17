//	BaseClasses.h - Abstract base classes for the 3 PVOC effects.
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

#ifndef BASECLASSES_H_
#define BASECLASSES_H_

class AccuTransformerBase
{
  public:
	///	Constructor.
	AccuTransformerBase() {};
	///	Destructor.
	virtual ~AccuTransformerBase() {};

	///	Initialises the effect.
	virtual void initialise(int newSamplerate) {};
	///	Generates a block of audio.
	/*!
		\todo Make exagVal a block of parameter values.
	 */
	virtual void getAudioBlock(float *input,
					   float *output,
					   int numSamples,
					   float decayVal,
					   float *dModValues,
					   float **dModBuffers,
					   float glisVal,
					   float *gModValues,
					   float **gModBuffers,
					   float level) {};
};

class SpectralExaggeratorBase
{
  public:
	///	Constructor.
	SpectralExaggeratorBase() {};
	///	Destructor.
	virtual ~SpectralExaggeratorBase() {};

	///	Initialises the effect.
	virtual void initialise(int newSamplerate) {};
	///	Generates a block of audio.
	/*!
		\todo Make exagVal a block of parameter values.
	 */
	virtual void getAudioBlock(float *input,
					   float *output,
					   int numSamples,
					   float exagVal,
					   float *modValues,
					   float **modBuffers,
					   float level) {};
};

class SpectralTransposerBase
{
  public:
	///	Constructor.
	SpectralTransposerBase() {};
	///	Destructor.
	virtual ~SpectralTransposerBase() {};

	///	Initialises the effect.
	virtual void initialise(int newSamplerate) {};
	///	Generates a block of audio.
	/*!
		\todo Make transVal a block of parameter values.
	 */
	virtual void getAudioBlock(float *input,
					   float *output,
					   int numSamples,
					   float transVal,
					   float *modValues,
					   float **modBuffers,
					   float level) {};
};

#endif
