//	SpectralTransposer.cpp - Re-written version of PVOC's
//							 spectraltransposer.
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

#include "SpectralTransposer.h"
#include <cmath>

extern "C" {
#include "plugins.h"
}

#if defined _WIN32 && defined _MSC_VER
strictinline int pvround(double fval)
{
	int result;
	_asm{
		fld		fval
		fistp	result
		mov		eax,result
	}
	return result;
}

#else
strictinline int pvround(double val)
{
	int k;
	k = (int)(fabs(val)+0.5);
	if(val < 0.0)
		k = -k;
	return k;
}
#endif

strictinline float fast_log2(float val)
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

strictinline double fast_exp2(const double val)
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
	return static_cast<float>(fast_exp2(b*fast_log2(a)));
}

strictinline float niallsConvertShiftpVals(float semitone)
{
	float retval = semitone * 0.08333333333f;
	retval = niallPow2(2.0f, retval);

	return retval;
}

//----------------------------------------------------------------------------
SpectralTransposer::SpectralTransposer():
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
SpectralTransposer::~SpectralTransposer()
{
	
}

//----------------------------------------------------------------------------
void SpectralTransposer::initialise(int newSamplerate)
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
void SpectralTransposer::getAudioBlock(float *input,
									   float *output,
									   int numSamples,
									   float transVal,
									   float *modValues,
									   float **modBuffers,
									   float level)
{
	int i, j, k;
	float actualTransVal;
	double invTransVal;
	float modAmt; //The amount by which the transVal parameter is modulated.
	float transpose; //The up-to-date parameter values.

	//Update modAmts.
	modAmt = modValues[0] +
			 modValues[1] +
			 modValues[2];

	//Update transpose.
	if(modAmt > 0.0f)
	{
		transpose = transVal * (1.0f-modAmt);
		transpose += modValues[0] * modBuffers[0][numSamples-1];
		transpose += modValues[1] * modBuffers[1][numSamples-1];
		transpose += modValues[2] * modBuffers[2][numSamples-1];
	}
	else
		transpose = transVal;

	//Convert transVal to the correct semitones-based value.
	transVal = (transpose*24.0f)-12.0f;

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
			if(fabs(transVal) < 0.0001f)
				actualTransVal = 0.0f;
			else
				actualTransVal = transVal;

			//get_amp_and_frq(frame,amp,freq,NumberOfBins);
			for(j=0,k=0; j<NumberOfBins; ++j,k+=2)
			{
				amp[j] = frame[k];
				freq[j] = frame[k+1];
			}

			//do_spectral_shiftp(amp,freq,convert_shiftp_vals(actualTransVal),NumberOfBins);
			actualTransVal = niallsConvertShiftpVals(actualTransVal);

			invTransVal = 1.0/actualTransVal;
			if(actualTransVal > 1.0f)
			{
				j = NumberOfBins-1;
				k = pvround((double)j * invTransVal);
				while(k >= 0)
				{
					if(j < 0)
						break;
					amp[j] = amp[k];
					freq[j] = actualTransVal * freq[k];
					--j;
					k = pvround((double)j * invTransVal);
				}
				for(k=j;k>=0;--k)
				{  //RWD was k > 
					amp[k]  = 0.0f;
					freq[k] = 0.0f;
				}				
			}
			else if(actualTransVal < 1.0f)
			{		//RWD : actualTransVal = 1 = no change
				j = 0;
				k = pvround((double)j * invTransVal);
				while(k <= (NumberOfBins-1))
				{
					amp[j]  = amp[k];
					freq[j] = actualTransVal * freq[k];
					++j;
					k = pvround((double)j * invTransVal);
				}
				for(k=j;k<NumberOfBins;++k)
				{
					amp[k]  = 0.0f;
					freq[k] = 0.0f;
				}				
			}

			//put_amp_and_frq(frame,amp,freq,NumberOfBins);
			for(j=0,k=0; j<NumberOfBins; ++j,k+=2)
			{
				frame[k] = amp[j];
				frame[k+1] = freq[j];
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
