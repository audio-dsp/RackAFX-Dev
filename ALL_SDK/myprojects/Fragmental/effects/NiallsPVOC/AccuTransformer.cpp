//	AccuTransformer.cpp - Re-written version of PVOC's AccuTransformer.
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

#include "AccuTransformer.h"
#include <cmath>

#define DenormalFix 1e-18f

extern "C" {
#include "plugins.h"
}

strictinline float fast_log2 (float val)
{
   //assert (val > 0);

   int * const  exp_ptr = reinterpret_cast <int *> (&val);
   int          x = *exp_ptr;
   const int    log_2 = ((x >> 23) & 255) - 128;
   x &= ~(255 << 23);
   x += 127 << 23;
   *exp_ptr = x;

   return (val + log_2);
}

strictinline double fast_exp2 (const double val)
{
   int    e;
   double ret;

   if (val >= 0)
   {
      e = int (val);
      ret = val - (e - 1);
      ((*(1 + (int *) &ret)) &= ~(2047 << 20)) += (e + 1023) << 20;
   }
   else
   {
      e = int (val + 1023);
      ret = val - (e - 1024);
      ((*(1 + (int *) &ret)) &= ~(2047 << 20)) += e << 20;
   }
   return (ret);
}

strictinline float niallPow2(float a, float b)
{
	return (float)fast_exp2(b*fast_log2(a));
}

//----------------------------------------------------------------------------
AccuTransformer::AccuTransformer():
bufferPos(0),
samplerate(44100)
{
	int i;

	for(i=0;i<(FFTLength+2);++i)
		frame[i] = 0.0f;
	for(i=0;i<Overlap;++i)
	{
		inputBuffer[i] = 0.0f;
		outputBuffer[i] = 0.0f;
	}
}

//----------------------------------------------------------------------------
AccuTransformer::~AccuTransformer()
{
	
}

//----------------------------------------------------------------------------
void AccuTransformer::initialise(int newSamplerate)
{
	int i;

	samplerate = newSamplerate;
	bufferPos = 0;

	nyquist = (float)(samplerate/2) * 0.9f;
	arate = (float)samplerate/(float)Overlap;
	frametime = 1.0f/arate;

	inputPVoc.initialise2(samplerate, FFTLength, Overlap, PVPP_STREAMING);
	outputPVoc.initialise2(samplerate, FFTLength, Overlap, PVPP_STREAMING);
	//outputPVoc.init(samplerate, FFTLength, Overlap, PVPP_STREAMING);

	for(i=0;i<(FFTLength+2);++i)
	{
		frameStore[i] = 0.0f;
		frame[i] = 0.0f;
	}
	for(i=0;i<Overlap;++i)
	{
		inputBuffer[i] = 0.0f;
		outputBuffer[i] = 0.0f;
	}
}

//----------------------------------------------------------------------------
void AccuTransformer::getAudioBlock(float *input,
									float *output,
									int numSamples,
									float decayVal,
									float *dModValues,
									float **dModBuffers,
									float glisVal,
									float *gModValues,
									float **gModBuffers,
									float level)
{
	int i, j;
	float thisGlis, thisDecay;
	float modAmts[2]; //The amount by which the parameters are modulated.
	float decay, gliss; //The up-to-date parameter values.

	//Update modAmts.
	modAmts[0] = dModValues[0] +
			     dModValues[1] +
			     dModValues[2];
	modAmts[1] = gModValues[0] +
			     gModValues[1] +
			     gModValues[2];

	//Update decay.
	if(modAmts[0] > 0.0f)
	{
		decay = decayVal * (1.0f-modAmts[0]);
		decay += dModValues[0] * dModBuffers[0][numSamples-1];
		decay += dModValues[1] * dModBuffers[1][numSamples-1];
		decay += dModValues[2] * dModBuffers[2][numSamples-1];
	}
	else
		decay = decayVal;

	//Update gliss.
	if(modAmts[1] > 0.0f)
	{
		gliss = glisVal * (1.0f-modAmts[1]);
		gliss += gModValues[0] * gModBuffers[0][numSamples-1];
		gliss += gModValues[1] * gModBuffers[1][numSamples-1];
		gliss += gModValues[2] * gModBuffers[2][numSamples-1];
	}
	else
		gliss = glisVal;

	//Convert decayVal and glissVal to the correct range.
	decayVal = decay;
	glisVal = (gliss*4.0f)-2.0f;

	for(i=0;i<numSamples;++i)
	{
		output[i] = outputBuffer[bufferPos] * level * 2.0f;
		if(bufferPos == Overlap-1)
		{
			inputPVoc.generateFrame(inputBuffer,
									frame,
									Overlap,
									PVOC_AMP_FREQ);

			//----------------------------------------------------------------
			/*glisVal = niallPow2(2.0f, glisVal*frametime);
			if(decayVal == 0.0f)
				decayVal = 0.000001f;
			else
				decayVal = (float)fast_exp2(fast_log2(decayVal) * frametime);*/
			thisGlis = (float)(pow(2.0, glisVal * frametime));
			if(decayVal==0.0f){
				/*  RWD need some non-zero value 'cos of log */
				thisDecay = 0.000001f;
			}
			else
				thisDecay = expf(logf(decayVal) * frametime);

			//pv_specaccu(1,1,thisDecay,thisGlis,NumberOfBins,frameStore,frame,nyquist);
			for(j=0;j<(NumberOfBins*2);j+=2)
			{
				frameStore[j] *= thisDecay;
				frameStore[j] += DenormalFix;
				frameStore[j] -= DenormalFix;
				frameStore[j+1] *= thisGlis;
				frameStore[j+1] += DenormalFix;
				frameStore[j+1] -= DenormalFix;

				if(frameStore[j+1] >= nyquist)
					frameStore[j] = 0.0f;

				//pv_accumulate(j,frame,frameStore);
				if(frame[j] > frameStore[j])
				{
					frameStore[j] = frame[j]; 
					frameStore[j+1] = frame[j+1];
				}
				else
				{
					frame[j] = frameStore[j];
					frame[j+1] = frameStore[j+1];								
				}
			}
			//----------------------------------------------------------------

			outputPVoc.processFrame(frame, outputBuffer, PVOC_AMP_FREQ);
			//outputPVoc.process_frame(frame, outputBuffer, PVOC_AMP_FREQ);

			bufferPos = 0;
		}
		else
			++bufferPos;
		inputBuffer[bufferPos] = input[i];
	}
}
