//	CriticalSection.h - Simple cross-platform critical section class.
//	--------------------------------------------------------------------------
//	Copyright (c) 2005-2007 Niall Moody
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

#ifndef CRITICALSECTION_H_
#define CRITICALSECTION_H_

#ifdef WIN32
#include <windows.h>
#elif defined(OSX)
#include <Multiprocessing.h>
#endif

///	Simple cross-platform critical section class.
class CriticalSection
{
  public:
	///	Constructor.
	CriticalSection();
	///	Destructor.
	~CriticalSection();

	///	Enter the critical section.
	void enter();
	///	Leave the critical section.
	void leave();
  private:
#ifdef WIN32
	CRITICAL_SECTION critSec;
#elif defined(OSX)
	MPCriticalRegionID critSec;
#endif
};

#endif
