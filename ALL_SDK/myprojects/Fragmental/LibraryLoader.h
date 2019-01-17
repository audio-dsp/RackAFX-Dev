//	LibraryLoader.h - Simple class for loading shared libraries.
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

#ifndef LIBRARYLOADER_H_
#define LIBRARYLOADER_H_

#ifdef WIN32
#include <windows.h>
#endif

#include <string>

///	Simple class for loading shared libraries.
class LibraryLoader
{
  public:
	///	Default constructor.
	LibraryLoader();
	///	Constructor.
	/*!
		\param path The path to the library.
	 */
	LibraryLoader(const std::string& path);
	///	Destructor.
	/*!
		Unloads the library.
	 */
	~LibraryLoader();

	///	Attempts to load the library in path.
	void load(const std::string& path);

	///	Returns a pointer to the named function.
	/*!
		\return 0 if the function doesn't exist, or the library couldn't
		be found in the constructor.
	 */
	void *getFunction(const std::string& functionName);

	///	Returns true if the library was loaded correctly.
	bool libraryIsLoaded() {return isLoaded;};

	///	Helper method to return the directory of the active dll/module.
	/*!
		\param data The calling dll's HINSTANCE.
	 */
	static std::string getModulePath(void *data);
  private:
	///	Whether or not the library is loaded.
	bool isLoaded;

#ifdef WIN32
	///	The loaded instance of the library.
	HINSTANCE library;
#endif
};

#endif
