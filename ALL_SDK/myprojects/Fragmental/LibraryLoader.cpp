//	LibraryLoader.cpp - Simple class for loading shared libraries.
//	--------------------------------------------------------------------------
//	Copyright (c) 2007 Niall Moody
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

#include "LibraryLoader.h"

using namespace std;

//----------------------------------------------------------------------------
LibraryLoader::LibraryLoader():
isLoaded(false)
{
	
}

//----------------------------------------------------------------------------
LibraryLoader::LibraryLoader(const string& path):
isLoaded(false)
{
	load(path);
}

//----------------------------------------------------------------------------
LibraryLoader::~LibraryLoader()
{
#ifdef WIN32
	FreeLibrary(library);
#endif
}

//----------------------------------------------------------------------------
void LibraryLoader::load(const std::string& path)
{
#ifdef WIN32
	library = LoadLibrary(path.c_str());

	if(!library)
	{
		MessageBox(NULL,
				   "Could not find NiallsPVOCLib library.  Trans, Exag and Accum effects will be disabled.",
				   "Fragmental Error.",
				   MB_OK);
	}
	else
		isLoaded = true;
#endif
}

//----------------------------------------------------------------------------
void *LibraryLoader::getFunction(const std::string& functionName)
{
#ifdef WIN32
	if(isLoaded)
		return GetProcAddress(library, functionName.c_str());
	else
		return 0;
#endif
}

//----------------------------------------------------------------------------
string LibraryLoader::getModulePath(void *data)
{
	string retval;

#ifdef WIN32
	char tempstr[4096];
	int tempint;

	GetModuleFileName((HMODULE)data, tempstr, 4096);
	retval = tempstr;

	tempint = retval.find_last_of('\\');
	retval = retval.substr(0, tempint+1);
#endif

	return retval;
}
