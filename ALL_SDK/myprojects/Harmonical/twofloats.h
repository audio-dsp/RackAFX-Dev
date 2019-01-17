//	twofloats.h - My twofloats data type, useful for passing stereo data
//				  around.
//	--------------------------------------------------------------------------
//	Copyright (c) 2005 Niall Moody
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

#ifndef TWOFLOATS_H_
#define TWOFLOATS_H_

typedef struct _twofloats
{
	union
	{
		float left;
		float x;
		float val1;
	};
	union
	{
		float right;
		float y;
		float val2;
	};

	_twofloats()
	{
		left = 0.0f;
		right = 0.0f;
	};

	_twofloats(float val1, float val2)
	{
		left = val1;
		right = val2;
	};

	_twofloats(float val)
	{
		left = right = val;
	};

	_twofloats operator+(_twofloats op2)
	{
		_twofloats temp;
		temp.left = op2.left + left;
		temp.right = op2.right + right;
		return temp;
	};

	_twofloats operator+(float op2)
	{
		_twofloats temp;
		temp.left = op2 + left;
		temp.right = op2 + right;
		return temp;
	};

	_twofloats operator+=(_twofloats op2)
	{
		left = op2.left + left;
		right = op2.right + right;
		return *this;
	};

	_twofloats operator+=(float op2)
	{
		left = op2 + left;
		right = op2 + right;
		return *this;
	};

	_twofloats operator-(_twofloats op2)
	{
		_twofloats temp;
		temp.left = left - op2.left;
		temp.right = right - op2.right;
		return temp;
	};

	_twofloats operator-(float op2)
	{
		_twofloats temp;
		temp.left = left - op2;
		temp.right = right - op2;
		return temp;
	};

	_twofloats operator-=(_twofloats op2)
	{
		left = left - op2.left;
		right = right - op2.right;
		return *this;
	};

	_twofloats operator-=(float op2)
	{
		left = left - op2;
		right = right - op2;
		return *this;
	};

	_twofloats operator*(_twofloats op2)
	{
		_twofloats temp;
		temp.left = op2.left * left;
		temp.right = op2.right * right;
		return temp;
	};

	_twofloats operator*(float op2)
	{
		_twofloats temp;
		temp.left = op2 * left;
		temp.right = op2 * right;
		return temp;
	};

	_twofloats operator*=(_twofloats op2)
	{
		left = op2.left * left;
		right = op2.right * right;
		return *this;
	};

	_twofloats operator*=(float op2)
	{
		left = op2 * left;
		right = op2 * right;
		return *this;
	};

	_twofloats operator=(_twofloats op2)
	{
		left = op2.left;
		right = op2.right;
		return *this;
	};

	_twofloats operator=(float op2)
	{
		left = op2;
		right = op2;
		return *this;
	};
} twofloats;

#endif