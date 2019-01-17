//	PhaseVocoder.cpp - Re-written version of PVOC's phasevocoder class.
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

#include "PhaseVocoder.h"
#include <cmath>

#define Pi 3.1415926535897932384626433832795f //float
#define Pid 3.1415926535897932384626433832795 //double
#define TwoPi 6.283185307179586476925286766559f //float
#define TwoPid 6.283185307179586476925286766559 //double
#define HalfPi 1.5707963267948966192313216916398f //float
#define HalfPid 1.5707963267948966192313216916398f //double

extern "C"
{
//Functions in mxfft.c.
void fft_(float *, float *,int,int,int,int);
void fftmx(float *,float *,int,int,int,int,int,int *,float *,float *,float *,float *,int *,int[]);
void reals_(float *,float *,int,int);
}

//This code from: http://www.gamedev.net/community/forums/topic.asp?topic_id=139956
//(second post)
float InvSqrt (float x)
{
    float xhalf = 0.5f*x;
    int i = *(int*)&x;
    i = 0x5f3759df - (i >> 1);
    x = *(float*)&i;
    x = x*(1.5f - xhalf*x*x);
    return x;
}

//----------------------------------------------------------------------------
PhaseVocoder::PhaseVocoder():
input(0),
output(0),
analysis(0),
synthesis(0),
analysisB(0),
synthesisB(0),
nextIn(0),
nextOut(0),
analysisWindow(0),
synthesisWindow(0),
maxAmp(0),
avgAmp(0),
avgFreq(0),
spectralEnvelope(0),
amplitude(0),
frequencyChannels(0),
oldPhase(0),
inputPhase(0),
outputPhase(0),
numBands(0),
analysisImpulseLength(0),
synthesisImpulseLength(0),
decimation(0),
interpolation(0),
filterOverlap(-1),
analysisLength(0),
synthesisLength(0),
frequency(0.0f),
outputSize(0),
outputCount(0),
inputLength(0),
outputLength(0),
analysisInput(0),
synthesisOutput(0),
lastSynthesisOutput(0),
analysisInputFirst(0),
lastInput(100000000),
origSize(0),
analysisReal(0.0f),
analysisImaginary(0.0f),
analysisMagnitude(0.0f),
analysisPhase(0.0f),
angleDifference(0.0f),
iSOverTwoPi(0.0f),
twoPiOverIS(0.0f),
renormalise(0.0f),
statistics(0.0f),
decSamplerate(0.0f),
interpSamplerate(0.0f),
invSamplerate(0.0f),
warp(0.0f),
inputSamplerate(0.0f),
pitchScale(1.0f),
invPitchScale(0.0f),
timeScale(1.0f),
numNewInputs(0),
numNewOutputs(0),
halfNumBands(0),
pitchBands(0),
halfPitchBands(0),
pitchInterp(0),
pitchOutputs(0),
evenAnalysisImpulse(0),
evenSynthesisImpulse(0),
endOfInput(0),
magnitudeOutput(0),
spectralEnvelopeOutput(0),
timeVarTimeScale(0),
tvNext(0),
tvLength(0),
samplerate(0.0f),
timeCheckF(0),
inputSamplerate2(0),
numChannels(0),
vonHann(0),
mode(PVPP_NOT_SET),
binIndex(0),
analysisBase(0),
synthesisBase(0)
{
	
}

//----------------------------------------------------------------------------
PhaseVocoder::~PhaseVocoder()
{
	if(synthesisBase)
		delete [] synthesisBase;
	if(analysisBase)
		delete [] analysisBase;
	if(input)
		delete [] input;
	if(output)
		delete [] output;
	if(analysis)
		delete [] analysis;
	if(inputPhase)
		delete [] inputPhase;
	if(maxAmp)
		delete [] maxAmp;
	if(avgAmp)
		delete [] avgAmp;
	if(avgFreq)
		delete [] avgFreq;
	if(spectralEnvelope)
		delete [] spectralEnvelope;
	if(outputPhase)
		delete [] outputPhase;
}

//----------------------------------------------------------------------------
void PhaseVocoder::initialise(int samplerate,
							  int fftLength,
							  int overlap,
							  pvocmode mode)
{
	int i;
	double tempd;

	numBands = fftLength;
	decimation = overlap;
	if(numBands <= 0)
		return;
	if(decimation < 0)
		return;

	if((mode != PVPP_OFFLINE)&&(mode != PVPP_STREAMING))
		return;
	this->mode = mode;

	inputSamplerate2 = samplerate;
	inputSamplerate = static_cast<float>(samplerate);
	this->samplerate = inputSamplerate;

	numBands += (numBands%2);
	halfNumBands = numBands/2;
	if(halfNumBands > 16384)
		return;

	frequency = inputSamplerate/static_cast<float>(numBands);
	analysisImpulseLength = numBands * 2;
	evenAnalysisImpulse = 1 - (analysisImpulseLength%2);

	if(synthesisImpulseLength == 0)
		synthesisImpulseLength = analysisImpulseLength;
	evenSynthesisImpulse = 1 - (synthesisImpulseLength%2);
	inputLength = 4 * analysisImpulseLength;
	outputLength = 4 * synthesisImpulseLength;

	if(filterOverlap == -1)
	{
		filterOverlap = static_cast<int>(3.322 *
										 log10(static_cast<double>(4.0 *
											   numBands)/analysisImpulseLength));
	}

	if(decimation == 0)
	{
		if(timeScale > 1)
			decimation = static_cast<int>(static_cast<float>(analysisImpulseLength/
															 (8.0f*timeScale)));
		else
			decimation = 1;
	}
	if(decimation == 0)
		decimation = 1;

	if(interpolation == 0)
	{
		interpolation = static_cast<int>(timeScale *
										 static_cast<float>(decimation));
	}

	timeScale = static_cast<float>(interpolation)/
				static_cast<float>(decimation);

	if(pitchScale != 1.0f)
		pitchScale = timeScale;

	pitchBands = static_cast<int>(static_cast<float>(numBands)/pitchScale);
	pitchBands += (pitchBands%2);

	halfPitchBands = pitchBands/2;

	pitchScale = static_cast<float>(numBands)/static_cast<float>(pitchBands);
	invPitchScale = 1.0f/pitchScale;

	if(warp == -1.0f)
		warp = pitchScale;
	if((spectralEnvelopeOutput == 1) && (warp = 0.0f))
		warp = 1.0f;

	pitchInterp = static_cast<int>(static_cast<float>(interpolation)/
								   pitchScale);

	lastInput -= analysisInputFirst;

	if(analysisBase)
		delete [] analysisBase;
	analysisBase = new float[analysisImpulseLength + evenAnalysisImpulse];
	analysisLength = analysisImpulseLength/2;
	analysisWindow = analysisBase + analysisLength;

	if(vonHann)
		vonHannWindow(analysisWindow, analysisLength, evenAnalysisImpulse);
	else
		hammingWindow(analysisWindow, analysisLength, evenAnalysisImpulse);

	for(i=1;i<=analysisLength;++i)
	{
		/* Niall edit.
		*(analysisWindow - i) = *(analysisWindow + i - evenAnalysisImpulse);*/
		analysisWindow[-i] = analysisWindow[i-evenAnalysisImpulse];
	}

	if(analysisImpulseLength > numBands)
	{
		if(evenAnalysisImpulse)
		{
			tempd = (double)numBands;
			tempd *= sin(HalfPid/(double)(numBands))/HalfPid;
			*analysisWindow *= (float)tempd;
		}
		for(i=1;i<=analysisLength;++i)
		{
			tempd = (double)numBands;
			tempd *= sin((double)(Pid*(i+0.5*evenAnalysisImpulse)/numBands))/
					 (Pid*(i+0.5*evenAnalysisImpulse));
			/*! Niall edit.
			*(analysisWindow + i) *= (float)tempd;*/
			analysisWindow[i] *= static_cast<float>(tempd);
		}
		for(i=1;i<=analysisLength;++i)
		{
			/* Niall edit.
			*(analysisWindow - i) = *(analysisWindow + i -
									  evenAnalysisImpulse);*/
			analysisWindow[-i] = analysisWindow[i-evenAnalysisImpulse];
		}
	}

	renormalise = 0.0f;
	for(i=-analysisLength;i<=analysisLength;++i)
	{
		/* Niall edit.
		renormalise += *(analysisWindow + i);*/
		renormalise += analysisWindow[i];
	}
	//Factor of 2 comes in later in trig identity.
	renormalise = 2.0f/renormalise;

	for(i=-analysisLength;i<=analysisLength;++i)
	{
		/* Niall edit.
		*(analysisWindow+i) *= renormalise;*/
		analysisWindow[i] *= renormalise;
	}

	/*	Set up synthesis window:  For the minimal mean-square-error
		formulation (valid for N >= M), the synthesis window is identical to
		the analysis window (except for a scale factor), and both are even in
		length.  If N < M, then an interpolating synthesis window is used.
	 */
	if(synthesisBase)
		delete [] synthesisBase;
	synthesisBase = new float[synthesisImpulseLength + evenSynthesisImpulse];
	synthesisLength = synthesisImpulseLength/2;
	synthesisWindow = synthesisBase + synthesisLength;

	if(analysisImpulseLength <= numBands)
	{
		if(vonHann)
		{
			vonHannWindow(synthesisWindow,
						  synthesisLength,
						  evenSynthesisImpulse);
		}
		else
		{
			hammingWindow(synthesisWindow,
						  synthesisLength,
						  evenSynthesisImpulse);
		}

		for(i=1;i<=synthesisLength;++i)
		{
			/* Niall edit.
			*(synthesisWindow - i) = *(synthesisWindow + i -
									   evenSynthesisImpulse);*/
			synthesisWindow[-i] = synthesisWindow[i-evenSynthesisImpulse];
		}
		for(i=-synthesisLength;i<=synthesisLength;++i)
		{
			/* Niall edit.
			*(synthesisWindow + i) *= renormalise;*/
			synthesisWindow[i] *= renormalise;
		}

		renormalise = 0.0f;
		for(i=-synthesisLength;i<=synthesisLength;i+=interpolation)
		{
			/* Niall edit.
			renormalise += *(synthesisWindow + i) * *(synthesisWindow + i);*/
			renormalise += synthesisWindow[i] * synthesisWindow[i];
		}
		renormalise = 1.0f/renormalise;

		for(i=-synthesisLength;i<=synthesisLength;++i)
		{
			/* Niall edit.
			*(synthesisWindow + i) *= renormalise;*/
			synthesisWindow[i] *= renormalise;
		}
	}
	else
	{
		if(vonHann)
		{
			vonHannWindow(synthesisWindow,
						  synthesisLength,
						  evenSynthesisImpulse);
		}
		else
		{
			hammingWindow(synthesisWindow,
						  synthesisLength,
						  evenSynthesisImpulse);
		}
		for(i=1;i<=synthesisLength;++i)
		{
			/* Niall edit.
			*(synthesisWindow - i) = *(synthesisWindow + i - evenSynthesisImpulse);*/
			synthesisWindow[-i] = synthesisWindow[i-evenSynthesisImpulse];
		}

		if(evenSynthesisImpulse)
		{
			/* Niall edit.
			*synthesisWindow *= (float)((double)pitchInterp *
										sin(HalfPid/(double)pitchInterp)/
										HalfPid);*/
			synthesisWindow[0] *= (float)((double)pitchInterp *
										  sin(HalfPid/(double)pitchInterp)/
										  HalfPid);
		}
		for(i=1;i<=synthesisLength;++i)
		{
			/* Niall edit.
			*(synthesisWindow+i) *= (float)((double)pitchInterp *
											sin(Pid*
												((double)i+0.5*
												 evenSynthesisImpulse)/
												 pitchInterp)/
											(Pid*((double)i+0.5*
											 evenSynthesisImpulse)));*/
			synthesisWindow[i] *= (float)((double)pitchInterp *
										  sin(Pid*
											   ((double)i+0.5*
										        evenSynthesisImpulse)/
												pitchInterp)/
										  (Pid*((double)i+0.5*
										   evenSynthesisImpulse)));
		}
		for(i=1;i<=synthesisLength;++i)
		{
			/* Niall edit.
			*(synthesisWindow - i) = *(synthesisWindow + i -
									   evenSynthesisImpulse);*/
			synthesisWindow[-i] = synthesisWindow[i-evenSynthesisImpulse];
		}
		renormalise = 1.0f/renormalise;

		for(i=-synthesisLength;i<=synthesisLength;++i)
		{
			/* Niall edit.
			*(synthesisWindow + i) *= renormalise;*/
			synthesisWindow[i] *= renormalise;
		}
	}

	if(input)
		delete [] input;
	input = new float[inputLength];
	nextIn = input;

	if(output)
		delete [] output;
	output = new float[outputLength];
	nextOut = output;

	if(analysis)
		delete [] analysis;
	analysis = new float[numBands+2];
	analysisB = analysis+1;

	if(inputPhase)
		delete [] inputPhase;
	inputPhase = new float[halfNumBands+1];
	if(maxAmp)
		delete [] maxAmp;
	maxAmp = new float[halfNumBands+1];
	if(avgAmp)
		delete [] avgAmp;
	avgAmp = new float[halfNumBands+1];
	if(avgFreq)
		delete [] avgFreq;
	avgFreq = new float[halfNumBands+1];
	if(spectralEnvelope)
		delete [] spectralEnvelope;
	spectralEnvelope = new float[halfNumBands+1];

	if(synthesis)
		delete [] synthesis;
	synthesis = new float[pitchBands+2];
	synthesisB = synthesis + 1;
	if(outputPhase)
		delete [] outputPhase;
	outputPhase = new float[halfPitchBands+1];

	outputCount = 0;
	decSamplerate = inputSamplerate/static_cast<float>(decimation);
	interpSamplerate = inputSamplerate/static_cast<float>(interpolation);
	invSamplerate = 1.0f/inputSamplerate;
	iSOverTwoPi = decSamplerate/TwoPi;
	twoPiOverIS = TwoPi/interpSamplerate;
	analysisInput = -(analysisLength/decimation) * decimation;
	synthesisOutput = static_cast<int>(timeScale/pitchScale * analysisInput);
	numNewInputs = analysisLength + analysisInput + 1;
	numNewOutputs = 0;
	pitchOutputs = 0;
	endOfInput = 1;

	for(i=0;i<inputLength;++i)
	{
		input[i] = 0.0f;
		output[i] = 0.0f;
	}

	for(i=0;i<(pitchBands+2);++i)
		synthesis[i] = 0.0f;
	for(i=0;i<(numBands+2);++i)
		analysis[i] = 0.0f;
	for(i=0;i<(halfPitchBands+1);++i)
		outputPhase[i] = 0.0f;
	for(i=0;i<(halfNumBands+1);++i)
	{
		inputPhase[i] = 0.0f;
		maxAmp[i] = 0.0f;
		avgAmp[i] = 0.0f;
		avgFreq[i] = 0.0f;
		spectralEnvelope[i] = 0.0f;
	}
}

//----------------------------------------------------------------------------
void PhaseVocoder::initialise2(int samplerate,
							   int fftLength,
							   int overlap,
							   pvocmode mode)
{
	int i;

	numBands = fftLength;
	decimation = overlap;
	if(numBands <=0)
		return;
	if(decimation < 0)
		return;
	/*for now */
	if(!(mode == PVPP_OFFLINE || mode == PVPP_STREAMING) )
		return;
	this->mode	= mode;

	inputSamplerate2 = samplerate;
	inputSamplerate = this->samplerate = (float) samplerate;
	numBands = numBands  + numBands%2;	/* Make numBands even */
	halfNumBands = numBands/2;
	if(halfNumBands > 16384)
		return;

//	F		= (int)((float) R / N);
	frequency = (float)inputSamplerate/(float)numBands;		/* RWD */
//	F2		= F / 2;
	analysisImpulseLength = numBands*2;		  /* RWD make double-window the default	 */
//	M		=	N;
	evenAnalysisImpulse = 1 - analysisImpulseLength%2;

	synthesisImpulseLength = (synthesisImpulseLength != 0 ? synthesisImpulseLength : analysisImpulseLength);
	evenSynthesisImpulse = 1 - synthesisImpulseLength%2;
	inputLength = 4 * analysisImpulseLength;
	outputLength = 4 * synthesisImpulseLength;

	if(filterOverlap == -1)
		filterOverlap = (int)(3.322 * log10((double)(4. * numBands) / analysisImpulseLength));/* cosmetic */

	decimation = (int)((decimation != 0 ? decimation : analysisImpulseLength/(8.0*(timeScale > 1.0 ? timeScale : 1.0))));

	if (decimation == 0){
		//fprintf(stderr,"pvoc: warning - T greater than M/8 \n");
		decimation = 1;
	}

	interpolation = (int)(interpolation != 0 ? interpolation : (float) timeScale*decimation );

	timeScale = ((float) interpolation / decimation);

	if (pitchScale != 1.)
		pitchScale = timeScale;

	pitchBands = (int)((float) numBands / pitchScale);	/* synthesis transform will be NO points */
	pitchBands = pitchBands + pitchBands%2;				/* make NO even */

	halfPitchBands = pitchBands / 2;

	pitchScale = ((float) numBands / pitchBands);		/* ideally, numBands / pitchBands = interpolation / decimation = pitch change */
	invPitchScale = (float)(1.0/ pitchScale);

	if (warp == -1.)
		warp = pitchScale;
	if ((spectralEnvelopeOutput == 1) && (warp == 0.))
		warp = 1.0f;


	//if ((P != 1.) && (P != T))
	//	 fprintf(stderr,"pvoc: warning P=%f not equal to T=%f\n",P,T);

	pitchInterp = (int)((float) interpolation / pitchScale);

	lastInput -= analysisInputFirst;
	/*RWD need this to get sum setup for synth window! */
	/* set up analysis window: The window is assumed to be symmetric
		with M total points.  After the initial memory allocation,
		analWindow always points to the midpoint of the window
		(or one half sample to the right, if M is even); analWinLen
		is half the true window length (rounded down). Any low pass
		window will work; a Hamming window is generally fine,
		but a Kaiser is also available.  If the window duration is
		longer than the transform (M > N), then the window is
		multiplied by a sin(x)/x function to meet the condition:
		analWindow[Ni] = 0 for i != 0.  In either case, the
		window is renormalized so that the phase vocoder amplitude
		estimates are properly scaled.  The maximum allowable
		window duration is ibuflen/2. */


	analysisBase = new float[analysisImpulseLength+evenAnalysisImpulse];
	analysisWindow = analysisBase + (analysisLength = analysisImpulseLength/2);	
	
	if(vonHann)
		vonHannWindow(analysisWindow, analysisLength, evenAnalysisImpulse);
	else
		hammingWindow(analysisWindow, analysisLength, evenAnalysisImpulse);

	for (i = 1; i <= analysisLength; i++)
		*(analysisWindow - i) = *(analysisWindow + i - evenAnalysisImpulse);

	if (analysisImpulseLength > numBands) {
		if (evenAnalysisImpulse)
			*analysisWindow *=(float)( (double)numBands * sin(HalfPid/numBands) /HalfPid);
		for (i = 1; i <= analysisLength; i++) 
			*(analysisWindow + i) *=(float)
			((double)numBands * sin((double) (Pid*(i+.5*evenAnalysisImpulse)/numBands)) / (Pid*(i+.5*evenAnalysisImpulse)));	/* D.T. 2000*/
		for (i = 1; i <= analysisLength; i++)
			*(analysisWindow - i) = *(analysisWindow + i - evenAnalysisImpulse);
	}

	renormalise = 0.0f;
	for (i = -analysisLength; i <= analysisLength; i++)
		renormalise += *(analysisWindow + i);

	renormalise = (float)(2.0 / renormalise);		/*factor of 2 comes in later in trig identity*/

	for (i = -analysisLength; i <= analysisLength; i++)
		*(analysisWindow + i) *= renormalise;

	/* set up synthesis window:  For the minimal mean-square-error
		formulation (valid for N >= M), the synthesis window
		is identical to the analysis window (except for a
		scale factor), and both are even in length.  If N < M,
		then an interpolating synthesis window is used. */

	synthesisBase = new float[synthesisImpulseLength+evenSynthesisImpulse];
	synthesisWindow = synthesisBase + (synthesisLength = synthesisImpulseLength/2);

	if (synthesisImpulseLength <= numBands){
		if(vonHann)
			vonHannWindow(synthesisWindow, synthesisLength, evenSynthesisImpulse);
		else
			hammingWindow(synthesisWindow, synthesisLength, evenSynthesisImpulse);
		for (i = 1; i <= synthesisLength; i++)
			*(synthesisWindow - i) = *(synthesisWindow + i - evenSynthesisImpulse);

		for (i = -synthesisLength; i <= synthesisLength; i++)
			*(synthesisWindow + i) *= renormalise;

		renormalise = 0.0f;
		for (i = -synthesisLength; i <= synthesisLength; i+=interpolation)
			renormalise += *(synthesisWindow + i) * *(synthesisWindow + i);

		renormalise = (float)(1.0/ renormalise);

		for (i = -synthesisLength; i <= synthesisLength; i++)
			*(synthesisWindow + i) *= renormalise;
	}
	else {
		if(vonHann)
			vonHannWindow(synthesisWindow, synthesisLength, evenSynthesisImpulse);
		else
			hammingWindow(synthesisWindow, synthesisLength, evenSynthesisImpulse);
		for (i = 1; i <= synthesisLength; i++)
			*(synthesisWindow - i) = *(synthesisWindow + i - evenSynthesisImpulse);

		if (evenSynthesisImpulse)
			*synthesisWindow *= (float)((double)pitchInterp * sin((double) (HalfPid/pitchInterp)) / HalfPid);
		for (i = 1; i <= synthesisLength; i++) 
				*(synthesisWindow + i) *=(float)
				((double)pitchInterp * sin((double) (Pid*(i+.5*evenSynthesisImpulse)/pitchInterp)) /(double) (Pid*(i+.5*evenSynthesisImpulse)));
		for (i = 1; i <= synthesisLength; i++)
			*(synthesisWindow - i) = *(synthesisWindow + i - evenSynthesisImpulse);

		renormalise = (float)(1.0/renormalise);
		for (i = -synthesisLength; i <= synthesisLength; i++)
			*(synthesisWindow + i) *= renormalise;
	}


	
	try{

		/* set up input buffer:  nextIn always points to the next empty
		word in the input buffer (i.e., the sample following
		sample number (n + analWinLen)).  If the buffer is full,
		then nextIn jumps back to the beginning, and the old
		values are written over. */

		if(input)
			delete [] input;
		input = new float[inputLength];

		nextIn = input;

		/* set up output buffer:  nextOut always points to the next word
		to be shifted out.  The shift is simulated by writing the
		value to the standard output and then setting that word
		of the buffer to zero.  When nextOut reaches the end of
		the buffer, it jumps back to the beginning.  */
		if(output)
			delete [] output;
		output = new float [outputLength];

		nextOut = output;


		/* set up analysis buffer for (N/2 + 1) channels: The input is real,
		so the other channels are redundant. oldInPhase is used
		in the conversion to remember the previous phase when
		calculating phase difference between successive samples. */

		if(analysis)
			delete [] analysis;
		analysis = new float[numBands+2];
		analysisB = analysis + 1;

		if(inputPhase)
			delete [] inputPhase;
		inputPhase = new float[halfNumBands+1];
		if(maxAmp)
			delete [] maxAmp;
		maxAmp = new float[halfNumBands+1];
		if(avgAmp)
			delete [] avgAmp;
		avgAmp = new float[halfNumBands+1];
		if(avgFreq)
			delete [] avgFreq;
		avgFreq = new float[halfNumBands+1];
		if(spectralEnvelope)
			delete [] spectralEnvelope;
		spectralEnvelope = new float[halfNumBands+1];
		/* set up synthesis buffer for (N/2 + 1) channels: (This is included
		only for clarity.)  oldOutPhase is used in the re-
		conversion to accumulate angle differences (actually angle
		difference per second). */

		if(synthesis)
			delete [] synthesis;
		synthesis = new float[pitchBands+2];
		synthesisB = synthesis + 1;
		outputPhase = new float[halfPitchBands+1];
	}
	catch(...){
		if(synthesisBase){
			delete [] synthesisBase;
			synthesisBase = 0;
		}
		if(analysisBase){
			delete [] analysisBase;
			analysisBase = 0;

		}
		if(input) {
			delete [] input;
			input = 0;
		}

		if(output) {
			delete [] output;
			output = 0;
		}
		if(analysis) {
			delete [] analysis;
			analysis = 0;
		}
		if(inputPhase) {
			delete [] inputPhase;
			inputPhase = 0;
		}
		
		if(maxAmp){
			delete [] maxAmp;
			maxAmp = 0;
		}
		if(avgAmp) {
			delete [] avgAmp;
			avgAmp = 0;
		}
		if(avgFreq) {
			delete [] avgFreq;
			avgFreq = 0;
		}
		if(spectralEnvelope){
			delete [] spectralEnvelope;
			spectralEnvelope= 0;
		}
		if(synthesis){
			delete [] synthesis;
			synthesis = 0;
		}
		if(outputPhase){
			delete [] outputPhase;
			outputPhase = 0;
		}
		return;
	}

	outputCount = 0;
	decSamplerate = ((float) inputSamplerate / decimation);
	interpSamplerate = ((float) inputSamplerate / interpolation);
	invSamplerate =((float) 1. / inputSamplerate);
	iSOverTwoPi = (float)(decSamplerate / TwoPi);
	twoPiOverIS = (float)(TwoPi / interpSamplerate);
	analysisInput = -(analysisLength / decimation) * decimation;	/* input time (in samples) */
	synthesisOutput = (long)((float) timeScale/pitchScale * analysisInput);	/* output time (in samples) */
	numNewInputs = analysisLength + analysisInput + 1;	/* number of new inputs to read */
	numNewOutputs = 0;				/* number of new outputs to write */
	pitchOutputs = 0;
	endOfInput = 1;

	for(i=0;i < inputLength;i++) {
		input[i] = 0.0f;
		output[i] = 0.0f;
	}

	for(i=0;i < pitchBands+2;i++)
		synthesis[i] = 0.0f;
	for(i=0;i < numBands+2;i++)
		analysis[i] = 0.0f;
	for(i=0;i < halfPitchBands+1;i++)
		outputPhase[i] = 0.0f;
	for(i=0;i < halfNumBands+1;i++)
		inputPhase[i] = maxAmp[i] = avgAmp[i] = avgFreq[i] = spectralEnvelope[i] = 0.0f;
}

//----------------------------------------------------------------------------
void PhaseVocoder::generateFrame(float *input,
							     float *output,
							     int numSamples,
							     pvoc_frametype frameType)
{
	int i, j, k;
	int samples;
	int tempInt;
	float *framePointer;
	float *outputPointer;

	samples = numSamples;
	if(samples < numNewInputs)
		numNewInputs = samples;

	framePointer = input;

	tempInt = pvmin(samples, (this->input + inputLength - nextIn));
	samples -= tempInt;
	while(tempInt-- > 0)
		*nextIn++ = *framePointer++;

	if(samples > 0)
	{
		nextIn -= inputLength;
		while(samples-- > 0)
			*nextIn++ = *framePointer++;
	}
	if(nextIn > (this->input+inputLength))
		nextIn -= inputLength;

	if(analysisInput > 0)
	{
		//Zero fill at EOF.
		for(i=numNewInputs;i<decimation;++i)
		{
			*(nextIn)++ = 0.0f;
			if(nextIn >= (this->input+inputLength))
				nextIn -= inputLength;
		}
	}

	/*	Analysis: The analysis subroutine computes the complex output at time
		n of (numBands/2 + 1) of the phase vocoder channels.  It operates on
		input samples (n - analysisLength) thru (n + analysisLength) and
		expects to find these in input[(n +- analysisLength) % inputLength].
		It expects analWindow to point to the center of a symmetric window of
		length (2 * analysisLength + 1).  It is the responsibility of the main
		program to ensure that these values are correct!  The results are
		returned in analysis as succesive pairs of real and imaginary values
		for the lowest (numBands/2 + 1) channels.   The subroutines fft and
		reals together implement one efficient FFT call for a real input
		sequence.
	 */

	//Initialise.
	for(i=0;i<(numBands+2);++i)
	{
		/* Niall edit.
		*(analysis+i) = 0.0f;*/
		analysis[i] = 0.0f;
	}

	//Input pointer.
	j = (analysisInput - analysisLength - 1 + inputLength) % inputLength;

	//Time shift.
	k = analysisInput - analysisLength - 1;
	while(k < 0)
		k += numBands;
	k %= numBands;

	/*j = -analysisLength;
	k = (analysisInput - analysisLength - 1 + inputLength) % inputLength;
	for(i=0; i<numBands; ++i, ++j, ++k)
	{
		int tempK = k+analysisLength;

		if(k >= inputLength)
			k -= inputLength;
		if(tempK >= inputLength)
			tempK -= inputLength;
		analysis[i] = (analysisWindow[j] * this->input[k]) +
					   (analysisWindow[j+analysisLength] * this->input[tempK]);
	}*/
	for(i=-analysisLength;i<=analysisLength;++i)
	{
		if (++j >= inputLength)
			j -= inputLength;
		if (++k >= numBands)
			k -= numBands;

		//**This is the line that seems to be slowing things down the most.**
		/* Niall edit.
		*(analysis + k) += *(analysisWindow + i) * *(this->input + j);*/
		analysis[k] += analysisWindow[i] * this->input[j];
	}

	fft_(analysis, analysisB, 1, halfNumBands, 1, -2);
	reals_(analysis, analysisB, halfNumBands, -2);

	/*	Conversion: The real and imaginary values in analysis are converted to
		magnitude and angle-difference-per-second (assuming an intermediate
		sampling rate of decSamplerate) and are returned in analysis.
	 */
	if(frameType == PVOC_AMP_PHASE)
	{
		//PVOCEX uses plain (wrapped) phase format, ref Soundhack.
		for(i=0,
				amplitude=analysis,
				frequencyChannels=analysis+1,
				oldPhase=inputPhase;
			i<=halfNumBands;
			++i, amplitude+=2, frequencyChannels+=2, ++oldPhase)
		{
			analysisReal = *amplitude;
			analysisImaginary = *frequencyChannels;

			/* Niall edit.
			*amplitude = (float)sqrt((double)((analysisReal * analysisReal) +
									 (analysisImaginary * analysisImaginary)));*/
			*amplitude = sqrtf((analysisReal * analysisReal) +
							   (analysisImaginary * analysisImaginary));

			//Phase unwrapping.
			if(*amplitude < 1.0e-10f)
				analysisPhase = 0.0f;
			else
				//analysisPhase = atanTable(analysisImaginary, analysisReal);
				analysisPhase = atan2f(analysisImaginary, analysisReal);

			*frequencyChannels = analysisPhase;
		}
	}
	if(frameType == PVOC_AMP_FREQ)
	{
		float tempf = 0.0f;
		for(i=0,
				amplitude=analysis,
				frequencyChannels=analysis+1,
				oldPhase=inputPhase;
			i<=halfNumBands;
			++i, amplitude+=2, frequencyChannels+=2, ++oldPhase, tempf+=1.0f)
		{
			analysisReal = *amplitude;
			analysisImaginary = *frequencyChannels;

			/* Niall edit.
			*amplitude = (float)sqrt((double)((analysisReal * analysisReal) +
									 (analysisImaginary * analysisImaginary)));*/
			*amplitude = sqrtf((analysisReal * analysisReal) +
							   (analysisImaginary * analysisImaginary));
			/**amplitude = 1.0f/InvSqrt((analysisReal * analysisReal) +
							   (analysisImaginary * analysisImaginary));*/

			if(*amplitude < 1.0e-10f)
				angleDifference = 0.0f;
			else
			{
				analysisPhase = atan2f(analysisImaginary, analysisReal);
				//analysisPhase = atanTable(analysisImaginary, analysisReal);
				angleDifference = analysisPhase - *oldPhase;
				*oldPhase = analysisPhase;
			}

			if(angleDifference > Pi)
				angleDifference -= TwoPi;
			if(angleDifference < -Pi)
				angleDifference += TwoPi;

			//Add in filter center freq.
			/*Niall edit.
			*frequencyChannels = angleDifference * iSOverTwoPi +
								 (static_cast<float>(i) * frequency);*/
			*frequencyChannels = angleDifference * iSOverTwoPi +
								 (tempf * frequency);
		}
	}

	//Else must be PVOC_COMPLEX.
	framePointer = analysis;
	outputPointer = output;
	for(i=0;i<(numBands+2);++i)
	{
		/* Niall edit.
		*outputPointer++ = *framePointer++;*/
		outputPointer[i] = framePointer[i];
	}

	//Increment time.
	analysisInput += decimation;
	synthesisOutput += pitchInterp;

	//Deal with offline and streaming differently.
	if(mode== PVPP_OFFLINE)
	{
		numNewInputs = pvmin(decimation,
						   pvmax(0,
							   decimation+lastInput-analysisInput-analysisLength));
	}
	else
		numNewInputs = decimation;

	if(synthesisOutput > (synthesisLength + interpolation))
		numNewOutputs = interpolation;
	else
	{
		if(synthesisOutput > synthesisLength)
			numNewOutputs = synthesisOutput - synthesisLength;
		else
		{
			numNewOutputs = 0;
			for(i=(synthesisOutput+synthesisLength);i<outputLength;++i)
			{
				if(i > 0)
				{
					/* Niall edit.
					*(this->output+i) = 0.0f;*/
					this->output[i] = 0.0f;
				}
			}
		}
	}
	pitchOutputs = static_cast<int>(((float)numNewOutputs/pitchScale));
}

//----------------------------------------------------------------------------
void PhaseVocoder::processFrame(float *frame,
								float *output,
								pvoc_frametype frameType)
{
	int i, j, k;
	int tempInt;
	float *outputBuffer;
	float angleDifference, tempPhase;

	/*	Reconversion: The magnitude and angle-difference-per-second in
		synthesis (assuming an intermediate sampling rate of interpSamplerate)
		are converted to real and imaginary values and are returned in
		synthesis.  This automatically incorporates the proper phase scaling
		for time modifications.
	 */
	if(pitchBands <= numBands)
	{
		for(i=0;i<(pitchBands+2);++i)
		{
			/* Niall edit.
			*(synthesis+i) = *(analysis+i);*/
			synthesis[i] = frame[i];
		}
	}
	else
	{
		for(i=0;i<=(numBands+1);++i)
		{
			/* Niall edit.
			*(synthesis+i) = *(analysis+i);*/
			synthesis[i] = frame[i];
		}
		for(i=(numBands+2);i<(pitchBands+2);++i)
		{
			/* Niall edit.
			*(synthesis+i) = 0.0f;*/
			synthesis[i] = 0.0f;
		}
	}

	if(frameType == PVOC_AMP_PHASE)
	{
		for(i=0, amplitude=synthesis, frequencyChannels=(synthesis+1);
			i<=halfPitchBands;
			++i, amplitude+=2, frequencyChannels+=2)
		{
			analysisMagnitude = *amplitude;
			analysisPhase = *frequencyChannels;

			*amplitude = analysisMagnitude * cosf(analysisPhase);
			*frequencyChannels = analysisMagnitude * sinf(analysisPhase);
		}
	}
	else if(frameType == PVOC_AMP_FREQ)
	{
		float tempf = 0.0f;
		for(i=0, amplitude=synthesis, frequencyChannels=(synthesis+1);
			i<=halfPitchBands;
			++i, amplitude+=2, frequencyChannels+=2, tempf+=1.0f)
		{
			analysisMagnitude = *amplitude;

			/*	RWD: need this if running real-time.
				Keep phase wrapped within +- TWOPI.
				This is spread across several frame cycles, as the problem
				does not develop immediately.
			 */
			angleDifference = twoPiOverIS * (*frequencyChannels - tempf *
											 frequency);
			/* Niall edit.
			tempPhase = *(outputPhase+i) + angleDifference;*/
			tempPhase = outputPhase[i] + angleDifference;
			if(i == binIndex)
				tempPhase = fmodf(tempPhase, TwoPi);
			/* Niall edit.
			*(outputPhase+i) = tempPhase;*/
			outputPhase[i] = tempPhase;
			analysisPhase = tempPhase;

			*amplitude = analysisMagnitude * cosf(analysisPhase);
			*frequencyChannels = analysisMagnitude * sinf(analysisPhase);
		}
	}

	//RWD i.e we fixup one bin per frame.
	if(++binIndex == (halfPitchBands+1))
		binIndex = 0;

	//Else it must be PVOC_COMPLEX.
	if(pitchScale != 1.0f)
	{
		for(i=0;i<(pitchBands+2);++i)
		{
			/* Niall edit.
			*(synthesis+i) *= invPitchScale;*/
			synthesis[i] *= invPitchScale;
		}
	}

	/*	Synthesis: The synthesis subroutine uses the Weighted Overlap-Add
		technique to reconstruct the time-domain signal.  The (numBands/2 + 1)
		phase vocoder channel outputs at time n are inverse Fourier
		transformed, windowed, and added into the output array.  The
		subroutine thinks of output as a shift register in which locations are
		referenced modulo outputLength.  Therefore, the main program must take
		care to zero each location which it "shifts" out (to standard output).
		The subroutines reals and fft together perform an efficient inverse
		FFT.
	 */
	reals_(synthesis, synthesisB, halfPitchBands, 2);
	fft_(synthesis, synthesisB, 1, halfPitchBands, 1, 2);

	j = synthesisOutput - synthesisLength - 1;
	while(j < 0)
		j += outputLength;
	j %= outputLength;

	k = synthesisOutput - synthesisLength - 1;
	while(k < 0)
		k += pitchBands;
	k %= pitchBands;

	//Overlap-add.
	for(i=-synthesisLength;i<=synthesisLength;++i)
	{
		float *tempO;
		float *tempS;
		float *tempW;
		if(++j >= outputLength)
			j -= outputLength;
		/* Niall edit.
		if++j;
		j %= outputLength;*/

		if(++k >= pitchBands)
			k -= pitchBands;
		/* Niall edit.
		++k;
		k %= pitchBands;*/

		/* Niall edit.
		*(this->output + j) += *(synthesis + k) * *(synthesisWindow + i);*/
		//this->output[j] += synthesis[k] * synthesisWindow[i];
		tempO = this->output+j;
		tempS = synthesis+k;
		tempW = synthesisWindow+i;
		*tempO += *tempS * *tempW;
	}

	outputBuffer = output;
	//Shift out next pitchOutputs values.
	for(i=0;i<pitchOutputs;)
	{
		tempInt = pvmin((pitchOutputs-i), (this->output+outputLength-nextOut));
		//Copy data to external buffer.
		for(j=0;j<tempInt;++j)
		{
			/* Niall edit.
			*outputBuffer++ = nextOut[j];*/
			outputBuffer[j] = nextOut[j];
			//*outputBuffer++ = nextOut[j];
		}

		i += tempInt;
		outputCount += tempInt;
		for(j=0;j<tempInt;++j)
			*nextOut++ = 0.0f;
		if(nextOut >= (this->output + outputLength))
			nextOut -= outputLength;
	}

	//Flag means do this operation only once.
	if(endOfInput)
	{
		//End of file detected.
		if((analysisInput > 0) && (numNewInputs < decimation))
		{
			endOfInput = 0;
			lastInput = analysisInput +
						analysisLength -
						(decimation - numNewInputs);
		}
	}

	//Increment time.
	analysisInput += decimation;
	synthesisOutput += pitchInterp;

	//numNewInputs == decimation except when the end of the sample stream
	//intervenes.
	//RWD handle offline and streaming separately - can't count an infinite
	//number of real-time samples!
	if(mode == PVPP_OFFLINE)
	{
		numNewInputs = pvmin(decimation, pvmax(0, (decimation +
											   lastInput -
											   analysisInput -
											   analysisLength)));
	}
	else
		numNewInputs = decimation;

	if(synthesisOutput > (synthesisLength + interpolation))
		numNewOutputs = interpolation;
	else
	{
		if(synthesisOutput > synthesisLength)
			numNewOutputs = synthesisOutput - synthesisLength;
		else
		{
			numNewOutputs = 0;
			for(i=(synthesisOutput+synthesisLength);i<outputLength;++i)
			{
				if(i > 0)
				{
					/* Niall edit.
					*(this->output+i) = 0.0f;*/
					this->output[i] = 0.0f;
				}
			}
		}
	}
	pitchOutputs = (int)(static_cast<float>(numNewOutputs)/pitchScale);
}

//----------------------------------------------------------------------------
void PhaseVocoder::hammingWindow(float *window, int length, int even)
{
	float tempf;
	int i;

	tempf = Pi/(float)length;

	if(even)
	{
		for(i=0;i<length;++i)
		{
			/* Niall edit.
			*(window+i) = 0.54f + (0.46f * cosf(tempf * ((float)i+0.5f)));*/
			window[i] = 0.54f + (0.46f * cosf(tempf * ((float)i+0.5f)));
		}

		/* Niall edit.
		*(window+length) = 0.0f;*/
		window[length] = 0.0f;
	}
	else
	{	
		/* Niall edit.
		*(window) = 1.0f;*/
		window[0] = 1.0f;
		for(i=1;i<=length;++i)
		{
			/* Niall edit.
			*(window+i) = 0.54f + (0.46f * cosf(tempf * (float)i));*/
			window[i] = 0.54f + (0.46f * cosf(tempf * (float)i));
		}
	}
}

//----------------------------------------------------------------------------
void PhaseVocoder::vonHannWindow(float *window, int length, int even)
{
	float tempf;
	int i;

	tempf = Pi/length;

	if(even)
	{
		for(i=0;i<length;++i)
			*(window+i) = 0.5f + (0.5f * cosf(tempf * ((float)i+0.5f)));

		*(window+length) = 0.0f;
	}
	else
	{
		*(window) = 1.0f;
		for(i=1;i<=length;++i)
			*(window+i) = 0.5f + (0.5f * cosf(tempf * (float)i));
	}
}

//----------------------------------------------------------------------------
void PhaseVocoder::processFrame2(float *frame,
								 float *output,
								 pvoc_frametype frameType)
{
	/*RWD vars */
	int i, j, k, n;
	long written;
	float *obufptr;


		/* reconversion: The magnitude and angle-difference-per-second in syn
		(assuming an intermediate sampling rate of rOut) are
		converted to real and imaginary values and are returned in syn.
		This automatically incorporates the proper phase scaling for
		time modifications. */

	if (pitchBands <= numBands){
		for (i = 0; i < pitchBands+2; i++)
			*(synthesis+i) = *(frame+i);
	}
	else {
		for (i = 0; i <= numBands+1; i++) 
			*(synthesis+i) = *(frame+i);
		for (i = numBands+2; i < pitchBands+2; i++) 
			*(synthesis+i) = 0.0f;
	}
	
	if(frameType==PVOC_AMP_PHASE){
		for(i=0, amplitude=synthesis, frequencyChannels=synthesis+1; i<= halfPitchBands; i++, amplitude+=2,  frequencyChannels+=2){
			analysisMagnitude = *amplitude;			
			analysisPhase = *frequencyChannels;
			/* Niall
			*amplitude = (float)((double)analysisMagnitude * cos((double)analysisPhase));
			*frequencyChannels = (float)((double)analysisMagnitude * sin((double)analysisPhase));*/
			*amplitude = analysisMagnitude * cosf(analysisPhase);
			*frequencyChannels = analysisMagnitude * sinf(analysisPhase);
		}
	}
	else if(frameType == PVOC_AMP_FREQ){


		for(i=0, amplitude=synthesis, frequencyChannels=synthesis+1; i<= halfPitchBands; i++, amplitude+=2,  frequencyChannels+=2)
		{
			analysisMagnitude = *amplitude;

			/* RWD: need this if running ral-time */
			/* keep phase wrapped within +- TWOPI */
			/* this is spread across several frame cycles, as the problem does not
				develop immediately */
			float angledif, the_phase;
			angledif = twoPiOverIS * (*frequencyChannels  - ((float) i * /*F*/frequency));	
			the_phase = *(outputPhase + i) +angledif;
			if(i== binIndex)
				the_phase =  (float)fmod(the_phase,TwoPid);			
			*(outputPhase + i) = the_phase;
			//analysisPhase = *(outputPhase + i);
			analysisPhase = the_phase;

			/* Niall
			*amplitude = (float)((double)analysisMagnitude * cos((double)analysisPhase));
			*frequencyChannels = (float)((double)analysisMagnitude * sin((double)analysisPhase));*/
			*amplitude = analysisMagnitude * cosf(analysisPhase);
			*frequencyChannels = analysisMagnitude * sinf(analysisPhase);
		}
	}

	/* RWD i.e we fixup one bin per frame */
	if(++binIndex == halfPitchBands+1)
		binIndex = 0;

	/* else it must be PVOC_COMPLEX */
	if (pitchScale != 1.)
		for (i = 0; i < pitchBands+2; i++)
			*(synthesis+i) *= invPitchScale;

	/* synthesis: The synthesis subroutine uses the Weighted Overlap-Add
			technique to reconstruct the time-domain signal.  The (N/2 + 1)
			phase vocoder channel outputs at time n are inverse Fourier
			transformed, windowed, and added into the output array.  The
			subroutine thinks of output as a shift register in which 
			locations are referenced modulo obuflen.  Therefore, the main
			program must take care to zero each location which it "shifts"
			out (to standard output). The subroutines reals and fft
			together perform an efficient inverse FFT.  */
#ifdef USE_FFTW
	rfftwnd_one_complex_to_real(inverse_plan,(fftw_complex * )syn,NULL);

#else
	reals_(synthesis,synthesisB,halfPitchBands,2);
	fft_(synthesis,synthesisB,1,halfPitchBands,1,2);
#endif

	j = synthesisOutput - synthesisLength - 1;
	while (j < 0)
		j += outputLength;
	j = j % outputLength;

	k = synthesisOutput - synthesisLength - 1;
	while (k < 0)
		k += pitchBands;
	k = k % pitchBands;

	for (i = -synthesisLength; i <= synthesisLength; i++) {	/*overlap-add*/
		if (++j >= outputLength)
			j -= outputLength;
		if (++k >= pitchBands)
			k -= pitchBands;
		*(this->output + j) += *(synthesis + k) * *(synthesisWindow + i);
	}

	obufptr = output;	/*RWD */
	written = 0;
	for (i = 0; i < pitchOutputs;){	/* shift out next IOi values */
		int j;
		int todo = pvmin(pitchOutputs-i, this->output+outputLength-nextOut);
		/*outfloats(nextOut, todo, ofd);*/
		/*copy data to external buffer */
		for(n=0;n < todo;n++)
			*obufptr++ = nextOut[n];
		written += todo;

		i += todo;
		outputCount += todo;
		for(j = 0; j < todo; j++)
			*nextOut++ = 0.0f;
		if (nextOut >= (this->output + outputLength))
			nextOut -= outputLength;
	}
				
	if (endOfInput)	/* flag means do this operation only once */
		if ((analysisInput > 0) && (numNewInputs < decimation)){	/* EOF detected */
			endOfInput = 0;
			lastInput = analysisInput + analysisLength - (decimation - numNewInputs);
		}


	/*	D = some_function(nI);		for variable time-scaling */
	/*	rIn = ((float) R / D);		for variable time-scaling */
	/*	RoverTwoPi =  rIn / TwoPi;	for variable time-scaling */

	analysisInput += decimation;				/* increment time */
	synthesisOutput += pitchInterp;

	/* Dd = D except when the end of the sample stream intervenes */
	/* RWD handle offline and streaming separately - 
	    can't count an infinite number of real-time samples! */
	if(mode == PVPP_OFFLINE)
		numNewInputs = pvmin(decimation, pvmax(0, decimation+lastInput-analysisInput-analysisLength));	  /*  CARL */

	else
		numNewInputs = decimation;						/* RWD */

	if (synthesisOutput > (synthesisLength + interpolation))
		numNewOutputs = interpolation;
	else
		if (synthesisOutput > synthesisLength)
			numNewOutputs = synthesisOutput - synthesisLength;
		else {
			numNewOutputs = 0;
			for (i=synthesisOutput+synthesisLength; i<outputLength; i++)
				if (i > 0)
					*(this->output+i) = 0.0f;
		}
	pitchOutputs = (int)((float) numNewOutputs / pitchScale);
}
