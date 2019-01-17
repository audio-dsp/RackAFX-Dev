//	ModulationManager.h - Singleton class which holds the output of all the
//						  modulation sources.
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

#ifndef MODULATIONMANAGER_H_
#define MODULATIONMANAGER_H_

#include "HelperStuff.h"

#include <vector>

///	Singleton class which holds the output of all the modulation sources.
/*!
	Actually, it's not strictly a singleton, as we have to work in shared
	memory, which would screw up singletons.  In an ideal case it would be a
	singleton, but as it is, we just have to retrieve it from the VstPlugin
	instance.

	Note: There are quite a few things here that are expected to be
	initialised correctly - the size of the modulation sources' audio blocks
	is assumed to be the same as that of process(), it is assumed that all
	ModulationSource objects will register themselves with ModulationManager
	and keep it up to date with their output block pointers...
 */
class ModulationManager
{
  public:
	///	Constructor.
	ModulationManager();
	///	Destructor.
	~ModulationManager();

	///	Registers a modulation source with the ModulationManager.
	/*!
		This method will return the ModulationSource's index in it's internal
		modSources vector - ModulationSource should use this index whenever
		it calls updateAudioBlock().
	 */
	int registerModSource();
	///	Updates the pointer held for the indexed ModulationSource.
	/*!
		\param index The index of the ModulationSource (retrieved from
		registerModSource().
		\param newBlock The pointer to the block of audio for the indexed
		ModulationSource.
	 */
	void updateAudioBlock(int index, float *newBlock);

	///	Called by VstPlugin at the start of process()/processReplacing() to set our copy of the input audio.
	void setInputAudio(float **input);
	///	Returns a pointer to the input audio for this block.
	const float ** const getInputAudio() const {return (const float **)inputAudio;};

	///	Returns a pointer to the indexed modulation source's output block.
	strictinline float *operator[](int index) {return modSources[index];};
  private:
	///	Our vector of all the modulation sources' output blocks.
	std::vector<float *> modSources;

	///	The current pointer to the input audio for the block.
	float **inputAudio;
};

#endif
