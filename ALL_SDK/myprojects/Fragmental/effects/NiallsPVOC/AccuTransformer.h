//	AccuTransformer.h - Re-written version of PVOC's AccuTransformer.
//	---------------------------------------------------------------------------
//	Copyright (c) 2007 Niall Moody
//	
//	This library is free software; you can redistribute it and/or
//	modify it under the terms of the GNU Lesser General Public
//	License as published by the Free Software Foundation; either
//	version 2.1 of the License, or (at your option) any later version.
//
//	This library is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//	Lesser General Public License for more details.
//
//	You should have received a copy of the GNU Lesser General Public
//	License along with this library; if not, write to the Free Software
//	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//	--------------------------------------------------------------------------

#ifndef ACCUTRANSFORMER_H_
#define ACCUTRANSFORMER_H_

#include "PhaseVocoder.h"
#include "BaseClasses.h"

///	Re-written version of PVOC's AccuTransformer.
class AccuTransformer : public AccuTransformerBase
{
  public:
	///	Constructor.
	AccuTransformer();
	///	Destructor.
	~AccuTransformer();

	///	Initialises the effect.
	void initialise(int newSamplerate);
	///	Generates a block of audio.
	/*!
		\todo Make exagVal a block of parameter values.
	 */
	void getAudioBlock(float *input,
					   float *output,
					   int numSamples,
					   float decayVal,
					   float *dModValues,
					   float **dModBuffers,
					   float glisVal,
					   float *gModValues,
					   float **gModBuffers,
					   float level);
  private:
	///	Enum containing the constants used for the phase vocoder.
	enum
	{
		FFTLength = 1024,
		Overlap = 256,
		NumberOfBins = 513
	};

	///	Input phase vocoder.
	//phasevocoder inputPVoc;
	PhaseVocoder inputPVoc;
	///	Output phase vocoder.
	//phasevocoder outputPVoc;
	PhaseVocoder outputPVoc;
	///	The current processing frame.
	float frame[FFTLength+2];
	///	Input buffer.
	float inputBuffer[Overlap];
	///	Output buffer.
	float outputBuffer[Overlap];
	///	Current position in the input and output buffers.
	int bufferPos;
	///	Current samplerate.
	int samplerate;

	float frameStore[FFTLength+2];
	float nyquist,arate,frametime;
};

#endif
