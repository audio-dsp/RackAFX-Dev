//	TempoSyncReverserMain.cpp - Main stuff.
//	--------------------------------------------------------------------------
//	Copyright (c) 2004-2006 Niall Moody
//	
//	Permission is hereby granted, free of charge, to any person obtaining a
//	copy of this software and associated documentation files (the "Software"),
//	to deal in the Software without restriction, including without limitation
//	the rights to use, copy, modify, merge, publish, distribute, sublicense,
//	and/or sell copies of the Software, and to permit persons to whom the
//	Software is furnished to do so, subject to the following conditions:

//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.

//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//	THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//	DEALINGS IN THE SOFTWARE.
//	--------------------------------------------------------------------------

#include "TempoSyncReverser.h"

static AudioEffect *effect = 0;
bool oome = false;

#if MAC
#pragma export on
#endif

// prototype of the export function main
#if __GNUC__ && (WIN32 || BEOS)
#define main main_plugin
extern "C" __declspec(dllexport) AEffect *main_plugin(audioMasterCallback audioMaster);

#elif MACX
#define main main_macho
extern "C" AEffect *main_macho(audioMasterCallback audioMaster);

#else
AEffect *main(audioMasterCallback audioMaster);
#endif

AEffect *main(audioMasterCallback audioMaster)
{
	// get vst version
	if(!audioMaster(0, audioMasterVersion, 0, 0, 0, 0))
		return 0;  // old version

	effect = new TempoSyncReverser(audioMaster);
	if(!effect)
		return 0;
	if(oome)
	{
		delete effect;
		return 0;
	}
	return effect->getAeffect();
}

#if MAC
#pragma export off
#endif

#if WIN32
#include <windows.h>
void* hInstance;
BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpvReserved)
{
	hInstance = hInst;
	return 1;
}
#endif
