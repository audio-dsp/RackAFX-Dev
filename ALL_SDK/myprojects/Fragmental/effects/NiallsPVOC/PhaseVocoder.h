//	PhaseVocoder.h - Re-written version of PVOC's phasevocoder class.
//	--------------------------------------------------------------------------
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

#ifndef PHASEVOCODER_H_
#define PHASEVOCODER_H_

#include "pvpp.h"
#include "../../HelperStuff.h"
#include "../../DllStuff.h"

///	Re-written version of PVOC's phasevocoder class.
class DllStuff PhaseVocoder
{
  public:
	///	Constructor.
	PhaseVocoder();
	///	Destructor.
	~PhaseVocoder();

	///	Initialises the phase vocoder.
	void initialise(int samplerate,
					int fftLength,
					int overlap,
					pvocmode mode);
	///	Initialises the phase vocoder.
	void initialise2(int samplerate,
					 int fftLength,
					 int overlap,
					 pvocmode mode);
	///	Generates a frame from a selection of audio.
	/*!
		A frame is a buffer containing interleaved samples representing the
		amplitude and frequency (or phase) of the generated bins.  This method
		generates such a frame from a block of input audio.
	 */
	void generateFrame(float *input,
					   float *output,
					   int numSamples,
					   pvoc_frametype frameType);
	///	Processes a given frame.
	/*!
		Takes frame, and turns it back into audio.
	 */
	void processFrame(float *frame, float *output, pvoc_frametype frameType);
	///	Processes a given frame (2nd attempt).
	/*!
		Takes frame, and turns it back into audio.
	 */
	void processFrame2(float *frame, float *output, pvoc_frametype frameType);
  private:
	///	Hamming window.
	void hammingWindow(float *window, int length, int even);
	///	Von Hann window.
	void vonHannWindow(float *window, int length, int even);

	///	Input buffer.
	float *input;
	///	Output buffer.
	float *output;
	///	Analysis buffer.
	float *analysis;
	///	Synthesis buffer.
	float *synthesis;
	///	Pointer to analysis[1] (for FFT calls)(?).
	float *analysisB;
	///	Pointer to synthesis[1] (for FFT calls)(?).
	float *synthesisB;
	///	Next empty word in input.
	float *nextIn;
	///	Next empty word in output.
	float *nextOut;
	///	Center of analysis window.
	float *analysisWindow;
	///	Center of synthesis window.
	float *synthesisWindow;
	///	Max amplitude buffer.
	float *maxAmp;
	///	Average amplitude buffer.
	float *avgAmp;
	///	Average frequency buffer.
	float *avgFreq;
	///	Spectral envelope.
	float *spectralEnvelope;
	///	Amplitude channels.
	float *amplitude;
	///	Frequency channels.
	float *frequencyChannels;
	///	Old phase channels.
	float *oldPhase;
	///	Input phase buffer.
	float *inputPhase;
	///	Output phase buffer.
	float *outputPhase;

	///	Number of phase vocoder channels (bands).
	int numBands;
	///	Length of analysisWindow impulse response.
	int analysisImpulseLength;
	///	Length of synthesisWindow impulse response.
	int synthesisImpulseLength;
	///	Decimation factor (default will be analysisImpulseLength/8).
	int decimation;
	///	Interpolation factor (default will be interpolation = decimation).
	int interpolation;
	///	Filter overlap factor (determines analysisImpulseLength, numBands).
	int filterOverlap;
	///	Half-length of analysis window.
	int analysisLength;
	///	Half-length of synthesis window.
	int synthesisLength;

	///	Frequency(?).
	float frequency;

	///	Sample size for output file.
	int outputSize;
	///	Number of samples written to output.
	int outputCount;
	///	Length of input buffer.
	int inputLength;
	///	Length of output buffer.
	int outputLength;
	///	Current input sample (analysis).
	int analysisInput;
	///	Current output sample (synthesis).
	int synthesisOutput;
	///	Last output sample (synthesis).
	int lastSynthesisOutput;
	///	First input sample (analysis).
	int analysisInputFirst;
	///	Last input sample.
	int lastInput;
	///	Sample type of file analysed.
	int origSize;

	///	Real part of analysis data.
	float analysisReal;
	///	Imaginary part of analysis data.
	float analysisImaginary;
	///	Magnitude of analysis data.
	float analysisMagnitude;
	///	Phase of analysis data.
	float analysisPhase;
	///	Angle difference(?).
	float angleDifference;
	///	(inputSamplerate/decimation)/(2*Pi)
	float iSOverTwoPi;
	///	(2*Pi)/(inputSamplerate/interpolation)
	float twoPiOverIS;
	///	Scale factor for re-normalising windows.
	float renormalise;
	///	Scale factor for calculating statistics.
	float statistics;
	///	Decimated samplerate.
	float decSamplerate;
	///	Pre-interpolated samplerate.
	float interpSamplerate;
	///	1/samplerate.
	float invSamplerate;
	///	analysisInput/samplerate.
	float time;
	///	Spectral envelope warp factor.
	float warp;
	///	Input samplerate.
	float inputSamplerate;
	///	Pitch scale factor.
	float pitchScale;
	///	1/pitchScale.
	float invPitchScale;
	///	Time scale factor (> 1 to expand).
	float timeScale;

	///	Number of new inputs to read.
	int numNewInputs;
	///	Number of new outputs to write.
	int numNewOutputs;
	///	numBands/2
	int halfNumBands;
	///	numBands/pitchScale.
	int pitchBands;
	///	pitchBands/2
	int halfPitchBands;
	///	interpolation/pitchScale.
	int pitchInterp;
	///	numNewOutputs/pitchScale.
	int pitchOutputs;
	///	Flag for even analysisImpulseLength.
	int evenAnalysisImpulse;
	///	Flag for even synthesisImpulseLength.
	int evenSynthesisImpulse;
	///	End of input flag.
	int endOfInput;
	///	Flag for magnitude output.
	int magnitudeOutput;
	///	Flag for spectral envelope output.
	int spectralEnvelopeOutput;
	///	Flag for time-varying time-scaling.
	int timeVarTimeScale;
	///	Counter for stepping through time-varying function.
	int tvNext;
	///	Length of time-varying function.
	int tvLength;

	///	Samplerate.
	float samplerate;

	///	Dunno.
	float timeCheckF;
	///	Input samplerate(?).
	int inputSamplerate2;
	///	Number of channels.
	int numChannels;

	///	Von Hann window(?).
	int vonHann;
	///	Mode(?)
	pvocmode mode;
	///	To spread normPhase over many frames(?).
	int binIndex;
	///	Dunno.
	float *analysisBase;
	///	Dunno.
	float *synthesisBase;
};

#endif
