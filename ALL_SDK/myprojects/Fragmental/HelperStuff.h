//	HelperStuff.h - Some helper things.
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

#ifndef HELPERSTUFF_H_
#define HELPERSTUFF_H_

#ifdef WIN32
#define strictinline __forceinline
#elif defined (__GNUC__)
#define strictinline inline __attribute__((always_inline))
#else
#define strictinline inline
#endif

#define Pi 3.1415926535897932384626433832795f

//Taken from aeffect.h to keep the headers tidy.
#ifdef WIN32
typedef short VstInt16;				///< 16 bit integer type
typedef int VstInt32;				///< 32 bit integer type
typedef __int64 VstInt64;			///< 64 bit integer type
#else
#include <stdint.h>
typedef int16_t VstInt16;			///< 16 bit integer type
typedef int32_t VstInt32;			///< 32 bit integer type
typedef int64_t VstInt64;			///< 64 bit integer type
#endif

//------------------------------------------------------------------------------
///	Simple rounding method.
strictinline int round(float val)
{
	int retval;
	retval = static_cast<int>(val+0.5f);
	return retval;
}

//------------------------------------------------------------------------------
///	Method to convert 0->1 range to beat-based values for host sync.
strictinline float getBeatValue(float val)
{
	const float vals[] = {1.0f/32.0f,
						  1.0f/16.0f,
						  1.0f/8.0f,
						  1.0f/6.0f,
						  1.0f/4.0f,
						  1.0f/3.0f,
						  1.0f/2.0f,
						  1.0f,
						  2.0f,
						  3.0f,
						  4.0f,
						  6.0f,
						  8.0f,
						  16.0f,
						  32.0f,
						  64.0f};

	return vals[round(val*15.0f)];
}

#endif
