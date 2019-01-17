//	Transpose.cpp - PVOC Transpose effect.
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

#include "Transpose.h"
#include "../VstPlugin.h"
#include "../LibraryLoader.h"
#include <sstream>

using namespace std;

typedef SpectralTransposerBase *(*ConstructTrans)();

//----------------------------------------------------------------------------
Transpose::Transpose(VstPlugin *plugin):
EffectBase(plugin),
transLeft2(0),
transRight2(0)
{
	LibraryLoader *libLoader = plugin->getLibraryLoader();

	if(libLoader->libraryIsLoaded())
	{
		ConstructTrans func = (ConstructTrans)libLoader->getFunction("constructTrans");

		if(func)
		{
			transLeft2 = func();
			transRight2 = func();
		}
	}

	paramManager = plugin->getParameterManager();
	paramIds[TransValMW1] = paramManager->registerParameter(this, "TransMW1", " ");
	paramIds[TransValMW2] = paramManager->registerParameter(this, "TransMW2", " ");
	paramIds[TransValMW3] = paramManager->registerParameter(this, "TransMW3", " ");
	paramIds[TransVal] = paramManager->registerParameter(this, "Trans", " ");
	paramIds[Level] = paramManager->registerParameter(this, "Level", " ");

	//transLeft.init(44100, 1024, 160, PVPP_STREAMING);
	if(transLeft2)
		transLeft2->initialise(44100);
	//transRight.init(44100, 1024, 160, PVPP_STREAMING);
	if(transRight2)
		transRight2->initialise(44100);

	modManager = plugin->getModulationManager();
}

//----------------------------------------------------------------------------
Transpose::~Transpose()
{
	if(transLeft2)
		delete transLeft2;
	if(transRight2)
		delete transRight2;
}

//----------------------------------------------------------------------------
float **Transpose::getBlock(float **input, VstInt32 blockSize)
{
	int i;
	float *modBuffers[3];
	float modValues[3];

	//Get modBuffers.
	for(i=0;i<3;++i)
		modBuffers[i] = (*modManager)[i];
	modValues[0] = parameters[TransValMW1];
	modValues[1] = parameters[TransValMW2];
	modValues[2] = parameters[TransValMW3];

	/*transLeft2.getAudioBlock(input[0],
							 tempBlock[0],
							 blockSize,
							 (parameters[TransVal]*24.0f)-12.0f,
							 (parameters[Level] * 2.0f));
	transRight2.getAudioBlock(input[1],
							  tempBlock[1],
							  blockSize,
							  (parameters[TransVal]*24.0f)-12.0f,
							  (parameters[Level] * 2.0f));*/
	if(transLeft2)
	{
		transLeft2->getAudioBlock(input[0],
								  tempBlock[0],
								  blockSize,
								  parameters[TransVal],
								  modValues,
								  modBuffers,
								  (parameters[Level] * 1.5f));
	}
	if(transRight2)
	{
		transRight2->getAudioBlock(input[1],
								   tempBlock[1],
								   blockSize,
								   parameters[TransVal],
								   modValues,
								   modBuffers,
								   (parameters[Level] * 1.5f));
	}

	if(!transLeft2 || !transRight2)
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
void Transpose::parameterChanged(VstInt32 index, float val)
{
	if(index == paramIds[TransValMW1])
		parameters[TransValMW1] = val;
	else if(index == paramIds[TransValMW2])
		parameters[TransValMW2] = val;
	else if(index == paramIds[TransValMW3])
		parameters[TransValMW3] = val;
	else if(index == paramIds[TransVal])
		parameters[TransVal] = val;
	else if(index == paramIds[Level])
		parameters[Level] = val;
}

//----------------------------------------------------------------------------
float Transpose::getValue(VstInt32 index)
{
	float retval = 0.0f;

	if(index == paramIds[TransValMW1])
		retval = parameters[TransValMW1];
	else if(index == paramIds[TransValMW2])
		retval = parameters[TransValMW2];
	else if(index == paramIds[TransValMW3])
		retval = parameters[TransValMW3];
	else if(index == paramIds[TransVal])
		retval = parameters[TransVal];
	else if(index == paramIds[Level])
		retval = parameters[Level];

	return retval;
}

//----------------------------------------------------------------------------
string Transpose::getTextValue(VstInt32 index)
{
	ostringstream tempstr;

	if(index == paramIds[TransValMW1])
		tempstr << parameters[TransValMW1];
	else if(index == paramIds[TransValMW2])
		tempstr << parameters[TransValMW2];
	else if(index == paramIds[TransValMW3])
		tempstr << parameters[TransValMW3];
	else if(index == paramIds[TransVal])
		tempstr << (parameters[TransVal]*24.0f)-12.0f;
	else if(index == paramIds[Level])
		tempstr << (parameters[Level] * 2.0f);

	return tempstr.str();
}

//----------------------------------------------------------------------------
void Transpose::samplerateChanged()
{
	/*transLeft.init(static_cast<int>(getSamplerate()),
				   1024,
				   160,
				   PVPP_STREAMING);
	transRight.init(static_cast<int>(getSamplerate()),
				    1024,
				    160,
				    PVPP_STREAMING);*/
	if(transLeft2)
		transLeft2->initialise(static_cast<int>(getSamplerate()));
	if(transRight2)
		transRight2->initialise(static_cast<int>(getSamplerate()));
}
