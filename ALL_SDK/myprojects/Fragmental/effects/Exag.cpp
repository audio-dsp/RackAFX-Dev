//	Exag.cpp - PVOC Exagerate effect.
//	--------------------------------------------------------------------------
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
//	--------------------------------------------------------------------------

#include "Exag.h"
#include "../VstPlugin.h"
#include "../LibraryLoader.h"
#include <sstream>

using namespace std;

typedef SpectralExaggeratorBase *(*ConstructExag)();

//----------------------------------------------------------------------------
Exag::Exag(VstPlugin *plugin):
EffectBase(plugin),
exaggLeft(0),
exaggRight(0)
{
	LibraryLoader *libLoader = plugin->getLibraryLoader();

	if(libLoader->libraryIsLoaded())
	{
		ConstructExag func = (ConstructExag)libLoader->getFunction("constructExag");

		if(func)
		{
			exaggLeft = func();
			exaggRight = func();
		}
	}

	paramManager = plugin->getParameterManager();
	paramIds[ExagValMW1] = paramManager->registerParameter(this, "ExagMW1", " ");
	paramIds[ExagValMW2] = paramManager->registerParameter(this, "ExagMW2", " ");
	paramIds[ExagValMW3] = paramManager->registerParameter(this, "ExagMW3", " ");
	paramIds[ExagVal] = paramManager->registerParameter(this, "Exag", " ");
	paramIds[Level] = paramManager->registerParameter(this, "Level", " ");

	//exagLeft.init(44100, 1024, 256, PVPP_STREAMING);
	//exagRight.init(44100, 1024, 256, PVPP_STREAMING);
	if(exaggLeft)
		exaggLeft->initialise(44100);
	if(exaggRight)
		exaggRight->initialise(44100);

	modManager = plugin->getModulationManager();
}

//----------------------------------------------------------------------------
Exag::~Exag()
{
	if(exaggLeft)
		delete exaggLeft;
	if(exaggRight)
		delete exaggRight;
}

//----------------------------------------------------------------------------
float **Exag::getBlock(float **input, VstInt32 blockSize)
{
	int i;
	float *modBuffers[3];
	float modValues[3];

	//Get modBuffers.
	for(i=0;i<3;++i)
		modBuffers[i] = (*modManager)[i];
	modValues[0] = parameters[ExagValMW1];
	modValues[1] = parameters[ExagValMW2];
	modValues[2] = parameters[ExagValMW3];

	if(exaggLeft)
	{
		exaggLeft->getAudioBlock(input[0],
								 tempBlock[0],
								 blockSize,
								 parameters[ExagVal],
								 modValues,
								 modBuffers,
								 (parameters[Level] * 1.5f));
	}
	if(exaggRight)
	{
		exaggRight->getAudioBlock(input[1],
								  tempBlock[1],
								  blockSize,
								  parameters[ExagVal],
								  modValues,
								  modBuffers,
								  (parameters[Level] * 1.5f));
	}

	if(!exaggLeft || !exaggRight)
	{
		for(i=0;i<blockSize;++i)
		{
			tempBlock[0][i] = input[0][i];
			tempBlock[1][i] = input[1][i];
		}
	}

	return tempBlock;
}

//----------------------------------------------------------------------------
void Exag::parameterChanged(VstInt32 index, float val)
{
	if(index == paramIds[ExagValMW1])
		parameters[ExagValMW1] = val;
	else if(index == paramIds[ExagValMW2])
		parameters[ExagValMW2] = val;
	else if(index == paramIds[ExagValMW3])
		parameters[ExagValMW3] = val;
	else if(index == paramIds[ExagVal])
		parameters[ExagVal] = val;
	else if(index == paramIds[Level])
		parameters[Level] = val;
}

//----------------------------------------------------------------------------
float Exag::getValue(VstInt32 index)
{
	float retval = 0.0f;

	if(index == paramIds[ExagValMW1])
		retval = parameters[ExagValMW1];
	else if(index == paramIds[ExagValMW2])
		retval = parameters[ExagValMW2];
	else if(index == paramIds[ExagValMW3])
		retval = parameters[ExagValMW3];
	else if(index == paramIds[ExagVal])
		retval = parameters[ExagVal];
	else if(index == paramIds[Level])
		retval = parameters[Level];

	return retval;
}

//----------------------------------------------------------------------------
string Exag::getTextValue(VstInt32 index)
{
	ostringstream tempstr;

	if(index == paramIds[ExagValMW1])
		tempstr << parameters[ExagValMW1];
	else if(index == paramIds[ExagValMW2])
		tempstr << parameters[ExagValMW2];
	else if(index == paramIds[ExagValMW3])
		tempstr << parameters[ExagValMW3];
	else if(index == paramIds[ExagVal])
		tempstr << (parameters[ExagVal]*4.0f);
	else if(index == paramIds[Level])
		tempstr << (parameters[Level]*2.0f);

	return tempstr.str();
}

//----------------------------------------------------------------------------
void Exag::samplerateChanged()
{
	/*exagLeft.init(static_cast<int>(getSamplerate()),
				  1024,
				  256,
				  PVPP_STREAMING);
	exagRight.init(static_cast<int>(getSamplerate()),
				   1024,
				   256,
				   PVPP_STREAMING);*/
	if(exaggLeft)
		exaggLeft->initialise(static_cast<int>(getSamplerate()));
	if(exaggRight)
		exaggRight->initialise(static_cast<int>(getSamplerate()));
}
