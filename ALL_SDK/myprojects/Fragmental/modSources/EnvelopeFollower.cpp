//	EnvelopeFollower.cpp - Envelope follower mod source.
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

#include "EnvelopeFollower.h"
#include "../VstPlugin.h"
#include "../ParameterManager.h"
#include "../ModulationManager.h"

#include <sstream>
#include <cmath>

using namespace std;

//-----------------------------------------------------------------------------
EnvelopeFollower::EnvelopeFollower(VstPlugin *plugin):
ModType(plugin),
attack(0.0f),
hold(0.0f),
decay(0.0f),
samplerate(44100.0f),
envelope(0.0f),
holdCount(0)
{
	int i;

	paramManager = plugin->getParameterManager();
	modManager = plugin->getModulationManager();

	paramIds[Attack] = paramManager->registerParameter(this, "Env att", "s");
	paramIds[Hold] = paramManager->registerParameter(this, "Env hld", "s");
	paramIds[Decay] = paramManager->registerParameter(this, "Env dec", "s");
	paramIds[Point0y] = paramManager->registerParameter(this, "Env p0y", " ");
	paramIds[Point1x] = paramManager->registerParameter(this, "Env p1x", " ");
	paramIds[Point1y] = paramManager->registerParameter(this, "Env p1y", " ");
	paramIds[Point2x] = paramManager->registerParameter(this, "Env p2x", " ");
	paramIds[Point2y] = paramManager->registerParameter(this, "Env p2y", " ");
	paramIds[Point3x] = paramManager->registerParameter(this, "Env p3x", " ");
	paramIds[Point3y] = paramManager->registerParameter(this, "Env p3y", " ");
	paramIds[Point4x] = paramManager->registerParameter(this, "Env p4x", " ");
	paramIds[Point4y] = paramManager->registerParameter(this, "Env p4y", " ");
	paramIds[Point5y] = paramManager->registerParameter(this, "Env p5y", " ");

	for(i=0;i<6;++i)
	{
		points[i].x = (float)i/5.0f;
		points[i].y = (float)i/5.0f;
	}
}

//-----------------------------------------------------------------------------
EnvelopeFollower::~EnvelopeFollower()
{
	
}

//-----------------------------------------------------------------------------
void EnvelopeFollower::getBlock(float *block,
								VstInt32 blockSize,
								VstInt32 barStart,
								float ppq)
{
	int i;
	float tempf;
	int tempint;
	const float ** const inputAudio = modManager->getInputAudio();

	for(i=0;i<blockSize;++i)
	{
		//First calculate straight envelope according to attack, hold and decay
		//parameters.
		tempf = fabs(inputAudio[0][i]);

		/*tempint = getNearestPoint(tempf);
		if(tempint < 0)
			tempint = 0;
		else if(tempint > 4)
			tempint = 4;
		tempf = interp(points[tempint], points[tempint+1], tempf);*/

		//In the attack phase.
		if(tempf > envelope)
		{
			//Update envelope.
			if(attack == 0.0f)
				envelope = tempf;
			else
			{
				envelope += (1.0f/samplerate)/attack;
				if(envelope > tempf)
					envelope = tempf;
			}

			//Reset holdCount;
			holdCount = static_cast<int>(hold * samplerate);
		}
		//In the decay phase.
		else if((tempf < envelope) && (envelope > 0.0f))
		{
			if((hold > 0.0f) && (holdCount > 0))
				--holdCount;
			else if(decay == 0.0f)
				envelope = tempf;
			else
			{
				envelope -= (1.0f/samplerate)/decay;
				if(envelope < 0.0f)
					envelope = 0.0f;
			}
		}

		//Make sure envelope's not gone out of bounds.
		if(envelope > 1.0f)
			envelope = 1.0f;

		//Now apply waveshaping-type process.
		tempint = getNearestPoint(envelope);
		if(tempint < 0)
			tempint = 0;
		else if(tempint > 4)
			tempint = 4;
		block[i] = interp(points[tempint], points[tempint+1], envelope);
	}
}

//-----------------------------------------------------------------------------
void EnvelopeFollower::parameterChanged(VstInt32 index, float val)
{
	if(index == paramIds[Attack])
		attack = val;
	else if(index == paramIds[Hold])
		hold = val;
	else if(index == paramIds[Decay])
		decay = val;
	else if(index == paramIds[Point0y])
		points[0].y = val;
	else if(index == paramIds[Point1x])
		points[1].x = val;
	else if(index == paramIds[Point1y])
		points[1].y = val;
	else if(index == paramIds[Point2x])
		points[2].x = val;
	else if(index == paramIds[Point2y])
		points[2].y = val;
	else if(index == paramIds[Point3x])
		points[3].x = val;
	else if(index == paramIds[Point3y])
		points[3].y = val;
	else if(index == paramIds[Point4x])
		points[4].x = val;
	else if(index == paramIds[Point4y])
		points[4].y = val;
	else if(index == paramIds[Point5y])
		points[5].y = val;
}

//-----------------------------------------------------------------------------
void EnvelopeFollower::setSamplerate(float rate)
{
	samplerate = rate;
}

//-----------------------------------------------------------------------------
float EnvelopeFollower::getValue(VstInt32 index)
{
	float retval;

	if(index == paramIds[Attack])
		retval = attack;
	else if(index == paramIds[Hold])
		retval = hold;
	else if(index == paramIds[Decay])
		retval = decay;
	else if(index == paramIds[Point0y])
		retval = points[0].y;
	else if(index == paramIds[Point1x])
		retval = points[1].x;
	else if(index == paramIds[Point1y])
		retval = points[1].y;
	else if(index == paramIds[Point2x])
		retval = points[2].x;
	else if(index == paramIds[Point2y])
		retval = points[2].y;
	else if(index == paramIds[Point3x])
		retval = points[3].x;
	else if(index == paramIds[Point3y])
		retval = points[3].y;
	else if(index == paramIds[Point4x])
		retval = points[4].x;
	else if(index == paramIds[Point4y])
		retval = points[4].y;
	else if(index == paramIds[Point5y])
		retval = points[5].y;

	return retval;
}

//-----------------------------------------------------------------------------
string EnvelopeFollower::getTextValue(VstInt32 index)
{
	stringstream tempConverter;

	if(index == paramIds[Attack])
		tempConverter << attack;
	else if(index == paramIds[Hold])
		tempConverter << hold;
	else if(index == paramIds[Decay])
		tempConverter << decay;
	else if(index == paramIds[Point0y])
		tempConverter << points[0].y;
	else if(index == paramIds[Point1x])
		tempConverter << points[1].x;
	else if(index == paramIds[Point1y])
		tempConverter << points[1].y;
	else if(index == paramIds[Point2x])
		tempConverter << points[2].x;
	else if(index == paramIds[Point2y])
		tempConverter << points[2].y;
	else if(index == paramIds[Point3x])
		tempConverter << points[3].x;
	else if(index == paramIds[Point3y])
		tempConverter << points[3].y;
	else if(index == paramIds[Point4x])
		tempConverter << points[4].x;
	else if(index == paramIds[Point4y])
		tempConverter << points[4].y;
	else if(index == paramIds[Point5y])
		tempConverter << points[5].y;

	return tempConverter.str();
}
