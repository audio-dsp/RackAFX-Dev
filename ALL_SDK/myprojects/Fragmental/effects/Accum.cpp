//	Accum.cpp - PVOC Accum effect.
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

#include "Accum.h"
#include "../VstPlugin.h"
#include "../LibraryLoader.h"
#include <sstream>

using namespace std;

typedef AccuTransformerBase *(*ConstructAccum)();

//----------------------------------------------------------------------------
Accum::Accum(VstPlugin *plugin):
EffectBase(plugin),
accumLeft2(0),
accumRight2(0)
{
	LibraryLoader *libLoader = plugin->getLibraryLoader();

	if(libLoader->libraryIsLoaded())
	{
		ConstructAccum func = (ConstructAccum)libLoader->getFunction("constructAccum");

		if(func)
		{
			accumLeft2 = func();
			accumRight2 = func();
		}
	}

	paramManager = plugin->getParameterManager();
	paramIds[GlissMW1] = paramManager->registerParameter(this, "GlissMW1", " ");
	paramIds[GlissMW2] = paramManager->registerParameter(this, "GlissMW2", " ");
	paramIds[GlissMW3] = paramManager->registerParameter(this, "GlissMW3", " ");
	paramIds[Gliss] = paramManager->registerParameter(this, "Gliss", " ");
	paramIds[DecayMW1] = paramManager->registerParameter(this, "DecayMW1", " ");
	paramIds[DecayMW2] = paramManager->registerParameter(this, "DecayMW2", " ");
	paramIds[DecayMW3] = paramManager->registerParameter(this, "DecayMW3", " ");
	paramIds[Decay] = paramManager->registerParameter(this, "Decay", " ");
	paramIds[Level] = paramManager->registerParameter(this, "Level", " ");

	//accumLeft.init(44100, 1024, 256, PVPP_STREAMING);
	//accumRight.init(44100, 1024, 256, PVPP_STREAMING);
	if(accumLeft2)
		accumLeft2->initialise(44100);
	if(accumRight2)
		accumRight2->initialise(44100);

	modManager = plugin->getModulationManager();
}

//----------------------------------------------------------------------------
Accum::~Accum()
{
	if(accumLeft2)
		delete accumLeft2;
	if(accumLeft2)
		delete accumRight2;
}

//----------------------------------------------------------------------------
float **Accum::getBlock(float **input, VstInt32 blockSize)
{
	int i;
	float *dModBuffers[3];
	float dModValues[3];
	float *gModBuffers[3];
	float gModValues[3];

	//Get modBuffers.
	for(i=0;i<3;++i)
	{
		dModBuffers[i] = (*modManager)[i];
		gModBuffers[i] = dModBuffers[i]; //Oops!  This is a bit pointless...
	}
	dModValues[0] = parameters[DecayMW1];
	dModValues[1] = parameters[DecayMW2];
	dModValues[2] = parameters[DecayMW3];
	gModValues[0] = parameters[GlissMW1];
	gModValues[1] = parameters[GlissMW2];
	gModValues[2] = parameters[GlissMW3];

	if(accumLeft2)
	{
		accumLeft2->getAudioBlock(input[0],
								  tempBlock[0],
								  blockSize,
								  parameters[Decay],
								  dModValues,
								  dModBuffers,
								  parameters[Gliss],
								  gModValues,
								  gModBuffers,
								  (parameters[Level] * 1.5f));
	}
	if(accumRight2)
	{
		accumRight2->getAudioBlock(input[1],
								   tempBlock[1],
								   blockSize,
								   parameters[Decay],
								   dModValues,
								   dModBuffers,
								   parameters[Gliss],
								   gModValues,
								   gModBuffers,
								   (parameters[Level] * 1.5f));
	}

	if(!accumLeft2 || !accumRight2)
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
void Accum::parameterChanged(VstInt32 index, float val)
{
	if(index == paramIds[GlissMW1])
		parameters[GlissMW1] = val;
	else if(index == paramIds[GlissMW2])
		parameters[GlissMW2] = val;
	else if(index == paramIds[GlissMW3])
		parameters[GlissMW3] = val;
	else if(index == paramIds[Gliss])
		parameters[Gliss] = val;
	else if(index == paramIds[DecayMW1])
		parameters[DecayMW1] = val;
	else if(index == paramIds[DecayMW2])
		parameters[DecayMW2] = val;
	else if(index == paramIds[DecayMW3])
		parameters[DecayMW3] = val;
	else if(index == paramIds[Decay])
		parameters[Decay] = val;
	else if(index == paramIds[Level])
		parameters[Level] = val;
}

//----------------------------------------------------------------------------
float Accum::getValue(VstInt32 index)
{
	float retval = 0.0f;

	if(index == paramIds[GlissMW1])
		retval = parameters[GlissMW1];
	if(index == paramIds[GlissMW2])
		retval = parameters[GlissMW2];
	if(index == paramIds[GlissMW3])
		retval = parameters[GlissMW3];
	else if(index == paramIds[Gliss])
		retval = parameters[Gliss];
	else if(index == paramIds[Decay])
		retval = parameters[Decay];
	else if(index == paramIds[Level])
		retval = parameters[Level];

	return retval;
}

//----------------------------------------------------------------------------
string Accum::getTextValue(VstInt32 index)
{
	ostringstream tempstr;

	if(index == paramIds[GlissMW1])
		tempstr << parameters[GlissMW1];
	else if(index == paramIds[GlissMW2])
		tempstr << parameters[GlissMW2];
	else if(index == paramIds[GlissMW3])
		tempstr << parameters[GlissMW3];
	else if(index == paramIds[Gliss])
		tempstr << (parameters[Gliss]*4.0f)-2.0f;
	else if(index == paramIds[Decay])
		tempstr << parameters[Decay];
	else if(index == paramIds[Level])
		tempstr << (parameters[Level] * 2.0f);

	return tempstr.str();
}

//----------------------------------------------------------------------------
void Accum::samplerateChanged()
{
	/*accumLeft.init(static_cast<int>(getSamplerate()),
				   1024,
				   256,
				   PVPP_STREAMING);
	accumRight.init(static_cast<int>(getSamplerate()),
				    1024,
				    256,
				    PVPP_STREAMING);*/
	if(accumLeft2)
		accumLeft2->initialise(static_cast<int>(getSamplerate()));
	if(accumRight2)
		accumRight2->initialise(static_cast<int>(getSamplerate()));
}
