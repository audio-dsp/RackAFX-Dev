//	EnvelopeFollower.h - Envelope follower mod source.
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

#ifndef ENVELOPEFOLLOWER_H_
#define ENVELOPEFOLLOWER_H_

#include "ModType.h"
#include "../HelperStuff.h"
#include "../ParameterManager.h"

class ModulationManager;

///	Envelope follower mod source.
class EnvelopeFollower : public ModType,
						 public ParameterCallback
{
  public:
	///	Constructor.
	EnvelopeFollower(VstPlugin *plugin);
	///	Destructor.
	~EnvelopeFollower();

	///	Returns the current block for this mod source.
	void getBlock(float *block,
				  VstInt32 blockSize,
				  VstInt32 barStart,
				  float ppq);

	///	To set the parameters.
	void parameterChanged(VstInt32 index, float val);
	///	So we're informed when the samplerate changes.
	void setSamplerate(float rate);

	///	Returns the current value of the indexed parameter.
	float getValue(VstInt32 index);
	///	Returns a textual version of the indexed parameter's value.
	std::string getTextValue(VstInt32 index);
  private:
	///	Simple type representing a point.
	struct TwoFloats
	{
		float x;
		float y;
	};

	///	Helper method to determine which point the amplitude's nearest to.
	strictinline int getNearestPoint(float amp)
	{
		int retval = 4;

		for(int i=1;i<5;++i)
		{
			if(amp <= points[i].x)
			{
				retval = i-1;
				break;
			}
		}

		return retval;
	}
	///	Helper method to interpolate between two points for a given i/p amplitude.
	strictinline float interp(const TwoFloats& p1,
							  const TwoFloats& p2,
							  float amp)
	{
		float retval = 0.0f;
		const float width = p2.x - p1.x;

		if(width > 0.0f)
			retval = (amp-p1.x)/width;

		retval = ((1.0f-retval)*p1.y) + (retval*p2.y);

		return retval;
	};

	///	IDs for our parameters.
	enum
	{
		Attack,
		Hold,
		Decay,
		Point0y,
		Point1x,
		Point1y,
		Point2x,
		Point2y,
		Point3x,
		Point3y,
		Point4x,
		Point4y,
		Point5y,

		NumParameters
	};

	///	The ModulationManager for this plugin.
	ModulationManager *modManager;
	///	The ParameterManager for this plugin.
	ParameterManager *paramManager;
	///	The ParameterManager IDs for all our parameters.
	VstInt32 paramIds[NumParameters];

	///	The attack time.
	float attack;
	///	The hold time.
	float hold;
	///	The decay time.
	float decay;
	///	The points from the waveshaper-y thing.
	TwoFloats points[6];

	///	The current samplerate.
	float samplerate;

	///	The current value of the envelope.
	float envelope;
	///	Counter for the hold parameter.
	int holdCount;
};

#endif
