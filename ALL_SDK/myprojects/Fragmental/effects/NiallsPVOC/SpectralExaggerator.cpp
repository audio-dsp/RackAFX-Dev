//	SpectralExaggerator.cpp - Re-written version of PVOC's
//							  spectralexaggerator.
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

#include "SpectralExaggerator.h"
#include <cmath>

#define DenormalFix 1e-18f

extern "C" {
#include "plugins.h"
}

//From: http://music.calarts.edu/pipermail/music-dsp/2002-September/017708.html
float ipow (float x, long n)
{
   const int   org_n = n;
   if (n < 0)
   {
      n = -n;
   }

   float   z = 1;
   while (n != 0)
   {
      if ((n & 1) != 0)
      {
         z *= x;
      }
      n >>= 1;
      x *= x;
   }

   return ((org_n < 0) ? 1.0f / z : z);
}

/*
 * (c) Ian Stephenson
 *
 * ian@dctsystems.co.uk
 *
 * Fast pow() reference implementation
 */

float shift23=(1<<23);
float OOshift23=1.0/(1<<23);

strictinline float myLog2(float i)
	{
	float LogBodge=0.346607f;
	float x;
	float y;
	x=*(int *)&i;
	x*= OOshift23; //1/pow(2,23);
	x=x-127;

	y=x-floorf(x);
	y=(y-y*y)*LogBodge;
	return x+y;
	}
strictinline float myPow2(float i)
	{
	float PowBodge=0.33971f;
	float x;
	float y=i-floorf(i);
	y=(y-y*y)*PowBodge;

	x=i+127-y;
	x*= shift23; //pow(2,23);
	*(int*)&x=(int)x;
	return x;
	}

strictinline float myPow(float a, float b)
	{
	return myPow2(b*myLog2(a));
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

strictinline float niallPow1(float a, float b)
{
	float tempf;

	float LogBodge=0.346607f;
	float PowBodge=0.33971f;
	float x;
	float y;
	x=*(int *)&a;
	x*= OOshift23; //1/pow(2,23);
	x=x-127;

	y=x-floorf(x);
	y=(y-y*y)*LogBodge;
	tempf = x+y;

	tempf *= b;
	y=tempf-floorf(tempf);
	x=tempf+127-y;
	x*= shift23; //pow(2,23);
	*(int*)&x=(int)x;

	return x;
}

//----------------------------------------------------------------------------
SpectralExaggerator::SpectralExaggerator():
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
SpectralExaggerator::~SpectralExaggerator()
{
	
}

//----------------------------------------------------------------------------
void SpectralExaggerator::initialise(int newSamplerate)
{
	int i;

	samplerate = newSamplerate;
	bufferPos = 0;

	inputPVoc.initialise2(samplerate, FFTLength, Overlap, PVPP_STREAMING);
	outputPVoc.initialise2(samplerate, FFTLength, Overlap, PVPP_STREAMING);
	//outputPVoc.init(samplerate, FFTLength, Overlap, PVPP_STREAMING);

	for(i=0;i<(FFTLength+2);++i)
		frame[i] = 0.0f;
	for(i=0;i<Overlap;++i)
	{
		inputBuffer[i] = 0.0f;
		outputBuffer[i] = 0.0f;
	}
}

//----------------------------------------------------------------------------
void SpectralExaggerator::getAudioBlock(float *input,
										float *output,
										int numSamples,
										float exagVal,
										float *modValues,
										float **modBuffers,
										float level)
{
	int i;
	int vc;	
	float post_totalamp = 0.0f, pre_totalamp = 0.0f;
	float maxamp = 0.0f, normaliser;
	float modAmt; //The amount by which the exagVal parameter is modulated.
	float exaggerate; //The up-to-date parameter values.

	//Update modAmts.
	modAmt = modValues[0] +
			 modValues[1] +
			 modValues[2];

	//Update exaggerate.
	if(modAmt > 0.0f)
	{
		exaggerate = exagVal * (1.0f-modAmt);
		exaggerate += modValues[0] * modBuffers[0][numSamples-1];
		exaggerate += modValues[1] * modBuffers[1][numSamples-1];
		exaggerate += modValues[2] * modBuffers[2][numSamples-1];
	}
	else
		exaggerate = exagVal;

	//Convert transVal to the correct range.
	exagVal = exaggerate * 4.0f;

	for(i=0;i<numSamples;++i)
	{
		output[i] = outputBuffer[bufferPos] * level * 2.0f;
		if(bufferPos == Overlap-1)
		{
			maxamp = 0.0f;
			post_totalamp = 0.0f;
			pre_totalamp = 0.0f;

			inputPVoc.generateFrame(inputBuffer,
									frame,
									Overlap,
									PVOC_AMP_FREQ);

			//----------------------------------------------------------------
			//pv_specexag(NumberOfBins, frame, exagVal, NumberOfBins * 2);
			for(vc=0;vc<(NumberOfBins*2);vc+=2)
			{
				pre_totalamp += frame[vc];
				if(frame[vc] > maxamp)
					maxamp = frame[vc];
			}
			if(maxamp > 0.0f)
			{
				normaliser = 1.0f/maxamp;
				for(vc=0;vc<(NumberOfBins*2);vc+=2)
				{
					frame[vc] *= normaliser;
					frame[vc] += DenormalFix;
					frame[vc] -= DenormalFix;
					//frame[vc] = powf(frame[vc], exagVal);
					frame[vc] = niallPow2(frame[vc], exagVal);
					post_totalamp += frame[vc];
				}

				/*pv_normalise(frame,
							 pre_totalamp,
							 post_totalamp,
							 (NumberOfBins*2));*/
				if(post_totalamp >= VERY_TINY_VAL)
				{
					normaliser = (pre_totalamp/post_totalamp) * 0.5f;	

					for(vc=0;vc<(NumberOfBins*2);vc+=2)
					{
						frame[vc] *= normaliser;
						frame[vc] += DenormalFix;
						frame[vc] -= DenormalFix;
					}
				}
			}
			//----------------------------------------------------------------

			outputPVoc.processFrame(frame, outputBuffer, PVOC_AMP_FREQ);

			bufferPos = 0;
		}
		else
			++bufferPos;
		inputBuffer[bufferPos] = input[i];
	}
}
