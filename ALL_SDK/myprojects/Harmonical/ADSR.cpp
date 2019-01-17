//	ADSR.cpp - ADSR classes, so we can use polymorphism to avoid big
//			   switch-case statements.
//	--------------------------------------------------------------------------
//	Copyright (c) 2005 Niall Moody
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

#include "ADSR.h"
#include "Voice.h"

#include <cmath>

//Static variables used by the Tanh class.
Tanh *Tanh::instance = 0;
int Tanh::refCount = 0;

//----------------------------------------------------------------------------
Tanh::Tanh()
{
	int i;

	for(i=0;i<arrSize;i++)
		array[i] = ((float)tanh((((float)i*TWOPI)/(float)arrSize)-PI)+1.0f)*0.5f;
}

Tanh *Tanh::getInstance()
{
	refCount++;
	if(!instance)
	{
		refCount = 1; //just in case something's gone weird...
		instance = new Tanh();
	}

	return instance;
}

void Tanh::removeInstance()
{
	refCount--;
	if(refCount < 1)
		delete instance;
}

//----------------------------------------------------------------------------
ADSRSection::ADSRSection(float samplerate, Voice *voice, bool isMain):
levelVal(0.0f),
index(0.0f),
increment(1.0f)
{
	this->samplerate = samplerate;
	this->voice = voice;
	this->isMain = isMain;

	envShape = Tanh::getInstance();
}

ADSRSection::~ADSRSection()
{
	Tanh::removeInstance();
}

void ADSRSection::setSamplerate(float val)
{
	float temptime;

	temptime = (static_cast<float>(Tanh::arrSize)/increment)/samplerate;
	samplerate = val;

	increment = (static_cast<float>(Tanh::arrSize)/samplerate)*temptime;
}

void ADSRSection::setTime(float val)
{
	//2.0f here means the range of times is 0->2 seconds.
	increment = static_cast<float>(Tanh::arrSize)/(samplerate*(1.999f*(val+0.001f)));
}

float ADSRSection::linInterp(float x, float val1, float val2)
{
	float tempf;

	//get the fractional value of x
	tempf = x - static_cast<float>(static_cast<int>(x));

	return (tempf*val2)+((1.0f-tempf)*val1);
}

//----------------------------------------------------------------------------
ADSRAttack::ADSRAttack(float samplerate, Voice *voice, bool isMain):
ADSRSection(samplerate, voice, isMain)
{
	
}

float ADSRAttack::getSample()
{
	int tempint, tempint2;

	//calculate correct envelope value from our Tanh table
	tempint = static_cast<int>(index);
	tempint2 = tempint+1;
	if(tempint2 > (Tanh::arrSize-1))
		tempint2 = Tanh::arrSize-1;
	levelVal = linInterp(index, (*envShape)[tempint], (*envShape)[tempint2]);

	//Increment index => if we've reached the end of the table, tell voice to
	//move to the next ADSR section.
	index += increment;
	if(index > (Tanh::arrSize-1))
		voice->incEnv_D(this);

	return levelVal;
}

//----------------------------------------------------------------------------
ADSRDecay::ADSRDecay(float samplerate, Voice *voice, bool isMain):
ADSRSection(samplerate, voice, isMain)
{
	
}

float ADSRDecay::getSample()
{
	int tempint, tempint2;

	//calculate correct envelope value from our Tanh table
	tempint = (Tanh::arrSize-1)-static_cast<int>(index);
	tempint2 = tempint + 1;
	if(tempint2 > (Tanh::arrSize-1))
		tempint2 = (Tanh::arrSize-1);
	levelVal = (1.0f-endVal)*(linInterp(index, (*envShape)[tempint], (*envShape)[tempint2]));
	levelVal += endVal;

	//Increment index => if we've reached the end of the table, tell voice to
	//move to the next ADSR section.
	index += increment;
	if(index > (Tanh::arrSize-1))
		voice->incEnv_S(this);

	return levelVal;
}

//----------------------------------------------------------------------------
ADSRSustain::ADSRSustain(float samplerate, Voice *voice, bool isMain):
ADSRSection(samplerate, voice, isMain)
{
	
}

//this one's easy...
float ADSRSustain::getSample()
{
	return levelVal;
}

//----------------------------------------------------------------------------
ADSRRelease::ADSRRelease(float samplerate, Voice *voice, bool isMain):
ADSRSection(samplerate, voice, isMain),
isFinished(false)
{
	
}

float ADSRRelease::getSample()
{
	float retval;
	int tempint, tempint2;

	if(isFinished)
		return 0.0f;

	//calculate correct envelope value from our Tanh table
	tempint = (Tanh::arrSize-1)-static_cast<int>(index);
	tempint2 = tempint + 1;
	if(tempint2 > (Tanh::arrSize-1))
		tempint2 = (Tanh::arrSize-1);
	retval = levelVal*(linInterp(index, (*envShape)[tempint], (*envShape)[tempint2]));

	//Increment index => if we've reached the end of the table, tell voice to
	//move to the next ADSR section.
	index += increment;
	if(index > (Tanh::arrSize-1))
	{
		if(isMain)
			voice->setInactive();
		else
			isFinished = true;
	}

	return retval;
}
