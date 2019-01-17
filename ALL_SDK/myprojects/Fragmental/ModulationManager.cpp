//	ModulationManager.cpp - Singleton class which holds the output of all the
//							modulation sources.
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

#include "ModulationManager.h"

//----------------------------------------------------------------------------
ModulationManager::ModulationManager():
inputAudio(0)
{
	
}

//----------------------------------------------------------------------------
ModulationManager::~ModulationManager()
{
	
}

//----------------------------------------------------------------------------
int ModulationManager::registerModSource()
{
	modSources.push_back(0);

	return (modSources.size()-1);
}

//----------------------------------------------------------------------------
void ModulationManager::updateAudioBlock(int index, float *newBlock)
{
	modSources[index] = newBlock;
}

//----------------------------------------------------------------------------
void ModulationManager::setInputAudio(float **input)
{
	inputAudio = input;
}
