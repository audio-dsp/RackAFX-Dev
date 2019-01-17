//	Colour.cpp - Simple struct representing a colour.
//	--------------------------------------------------------------------------
//	Copyright (c) 2006 Niall Moody
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

#include "Colour.h"

//----------------------------------------------------------------------------
Colour operator+(unsigned char val, const Colour& col)
{
	Colour retval;

	retval.red = val + col.red;
	retval.green = val + col.green;
	retval.blue = val + col.blue;

	return retval;
}

//----------------------------------------------------------------------------
Colour operator+(float val, const Colour& col)
{
	Colour retval;

	retval.red = static_cast<unsigned char>(val * 255.0f) + col.red;
	retval.green = static_cast<unsigned char>(val * 255.0f) + col.green;
	retval.blue = static_cast<unsigned char>(val * 255.0f) + col.blue;

	return retval;
}

//----------------------------------------------------------------------------
Colour operator-(unsigned char val, const Colour& col)
{
	Colour retval;

	retval.red = val - col.red;
	retval.green = val - col.green;
	retval.blue = val - col.blue;

	return retval;
}

//----------------------------------------------------------------------------
Colour operator-(float val, const Colour& col)
{
	Colour retval;

	retval.red = static_cast<unsigned char>(val * 255.0f) - col.red;
	retval.green = static_cast<unsigned char>(val * 255.0f) - col.green;
	retval.blue = static_cast<unsigned char>(val * 255.0f) - col.blue;

	return retval;
}

//----------------------------------------------------------------------------
Colour operator*(unsigned char val, const Colour& col)
{
	Colour retval;

	retval.red = val * col.red;
	retval.green = val * col.green;
	retval.blue = val * col.blue;

	return retval;
}

//----------------------------------------------------------------------------
Colour operator*(float val, const Colour& col)
{
	Colour retval;

	retval.red = static_cast<unsigned char>(val * static_cast<float>(col.red));
	retval.green = static_cast<unsigned char>(val * static_cast<float>(col.green));
	retval.blue = static_cast<unsigned char>(val * static_cast<float>(col.blue));

	return retval;
}

//----------------------------------------------------------------------------
Colour operator/(unsigned char val, const Colour& col)
{
	Colour retval;

	retval.red = static_cast<unsigned char>(static_cast<float>(val)/
											static_cast<float>(col.red));
	retval.green = static_cast<unsigned char>(static_cast<float>(val)/
											  static_cast<float>(col.green));
	retval.blue = static_cast<unsigned char>(static_cast<float>(val)/
											 static_cast<float>(col.blue));

	return retval;
}

//----------------------------------------------------------------------------
Colour operator/(float val, const Colour& col)
{
	Colour retval;

	retval.red = static_cast<unsigned char>(val/
											static_cast<float>(col.red));
	retval.green = static_cast<unsigned char>(val/
											  static_cast<float>(col.green));
	retval.blue = static_cast<unsigned char>(val/
											 static_cast<float>(col.blue));

	return retval;
}
