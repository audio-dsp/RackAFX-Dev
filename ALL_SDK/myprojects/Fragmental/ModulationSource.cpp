//	ModulationSource.cpp - Abtract base class for all modulation sources.
//	----------------------------------------------------------------------------
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
//	----------------------------------------------------------------------------

#include "ModulationSource.h"
#include "ModulationManager.h"
#include "VstPlugin.h"

//------------------------------------------------------------------------------
ModulationSource::ModulationSource(VstPlugin *plugin):
tempBlock(0),
tempBlockSize(0),
samplerate(44100.0f),
tempo(120.0f),
syncMode(false)
{
	modManager = plugin->getModulationManager();
	index = modManager->registerModSource();
}

//------------------------------------------------------------------------------
ModulationSource::~ModulationSource()
{
	if(tempBlockSize > 0)
		delete [] tempBlock;
}

//------------------------------------------------------------------------------
void ModulationSource::setBlockSize(VstInt32 newSize)
{
	int i;

	if(tempBlockSize > 0)
		delete [] tempBlock;

	tempBlockSize = newSize;
	tempBlock = new float[tempBlockSize];
	modManager->updateAudioBlock(index, tempBlock);

	for(i=0;i<tempBlockSize;++i)
		tempBlock[i] = 0.0f;
}

//------------------------------------------------------------------------------
void ModulationSource::setSamplerate(float newRate)
{
	samplerate = newRate;
}

//------------------------------------------------------------------------------
void ModulationSource::setTempo(float val)
{
	tempo = val;
}

//------------------------------------------------------------------------------
void ModulationSource::setSyncMode(bool val)
{
	syncMode = val;
}
