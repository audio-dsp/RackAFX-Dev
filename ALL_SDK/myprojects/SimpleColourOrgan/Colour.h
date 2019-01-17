//	Colour.h - Simple struct representing a colour.
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

#ifndef COLOUR_H_
#define COLOUR_H_

#include <string>
#include <sstream>
#include <iostream>

///	Simple struct representing a colour.
struct Colour
{
	unsigned char red;
	unsigned char green;
	unsigned char blue;

	/// Default constructor.
	Colour() {red = blue = green = 0;};
	/// Greyscale constructor.
	Colour(unsigned char val) {red = blue = green = val;};
	/// Full constructor.
	Colour(unsigned char r, unsigned char g, unsigned char b)
	{
		red = r;
		blue = b;
		green = g;
	};
	///	Copy constructor.
	Colour(const Colour& other)
	{
		red = other.red;
		green = other.green;
		blue = other.blue;
	};

	///	Assignment operator.
	Colour& operator=(const Colour& other)
	{
		red = other.red;
		green = other.green;
		blue = other.blue;

		return *this;
	};
	///	Assignment operator (unsigned char).
	Colour& operator=(unsigned char val)
	{
		red = val;
		green = val;
		blue = val;

		return *this;
	};
	///	Assignment operator (float).
	/*!
		\param val Is assumed to be a value limited between 0 and 1.
	 */
	Colour& operator=(float val)
	{
		red = static_cast<unsigned char>(val * 255.0f);
		green = static_cast<unsigned char>(val * 255.0f);
		blue = static_cast<unsigned char>(val * 255.0f);

		return *this;
	};
	///	Assignment operator (string).
	/*!
		This version's to convert a 6-digit hexadecimal ascii number into
		the correct red, green and blue values.
	 */
	Colour& operator=(const std::string& val)
	{
		std::stringstream tempConverter(val);
		int tempint;

		tempConverter >> std::hex >> tempint;

		red = (tempint & 0xFF0000) >> 16;
		green = (tempint & 0x00FF00) >> 8;
		blue = tempint & 0x0000FF;

		return *this;
	};

	///	Addition operator.
	Colour operator+(const Colour& other)
	{
		Colour retval;

		retval.red = red + other.red;
		retval.green = green + other.green;
		retval.blue = blue + other.blue;

		return retval;
	};
	///	Addition operator (unsigned char).
	Colour operator+(unsigned char val)
	{
		Colour retval;

		retval.red = red + val;
		retval.green = green + val;
		retval.blue = blue + val;

		return retval;
	};
	///	Addition operator (float).
	/*!
		\param val Is assumed to be a value limited between 0 and 1.
	 */
	Colour operator+(float val)
	{
		Colour retval;

		retval.red = red + static_cast<unsigned char>(val * 255.0f);
		retval.green = green + static_cast<unsigned char>(val * 255.0f);
		retval.blue = blue + static_cast<unsigned char>(val * 255.0f);

		return retval;
	};

	///	Subtraction operator.
	Colour operator-(const Colour& other)
	{
		Colour retval;

		retval.red = red - other.red;
		retval.green = green - other.green;
		retval.blue = blue - other.blue;

		return retval;
	};
	///	Subtraction operator (unsigned char).
	Colour operator-(unsigned char val)
	{
		Colour retval;

		retval.red = red - val;
		retval.green = green - val;
		retval.blue = blue - val;

		return retval;
	};
	///	Subtraction operator (float).
	/*!
		\param val Is assumed to be a value limited between 0 and 1.
	 */
	Colour operator-(float val)
	{
		Colour retval;

		retval.red = red - static_cast<unsigned char>(val * 255.0f);
		retval.green = green - static_cast<unsigned char>(val * 255.0f);
		retval.blue = blue - static_cast<unsigned char>(val * 255.0f);

		return retval;
	};

	///	Multiplication operator.
	Colour operator*(const Colour& other)
	{
		Colour retval;

		retval.red = red * other.red;
		retval.green = green * other.green;
		retval.blue = blue * other.blue;

		return retval;
	};
	///	Multiplication operator (unsigned char).
	Colour operator*(unsigned char val)
	{
		Colour retval;

		retval.red = red * val;
		retval.green = green * val;
		retval.blue = blue * val;

		return retval;
	};
	///	Multiplication operator (float).
	/*!
		\param val Not limited (so you can multiply by 1 and get the correct
		result).
	 */
	Colour operator*(float val)
	{
		Colour retval;

		retval.red = static_cast<unsigned char>(static_cast<float>(red) * val);
		retval.green = static_cast<unsigned char>(static_cast<float>(green) * val);
		retval.blue = static_cast<unsigned char>(static_cast<float>(blue) * val);

		return retval;
	};

	///	Division operator.
	Colour operator/(const Colour& other)
	{
		Colour retval;

		retval.red = static_cast<unsigned char>(static_cast<float>(red)/
												static_cast<float>(other.red));
		retval.green = static_cast<unsigned char>(static_cast<float>(green)/
												  static_cast<float>(other.green));
		retval.blue = static_cast<unsigned char>(static_cast<float>(blue)/
												 static_cast<float>(other.blue));

		return retval;
	};
	///	Division operator (unsigned char).
	Colour operator/(unsigned char val)
	{
		Colour retval;

		retval.red = static_cast<unsigned char>(static_cast<float>(red)/
												static_cast<float>(val));
		retval.green = static_cast<unsigned char>(static_cast<float>(green)/
												  static_cast<float>(val));
		retval.blue = static_cast<unsigned char>(static_cast<float>(blue)/
												 static_cast<float>(val));

		return retval;
	};
	///	Division operator (float).
	/*!
		\param val Not limited (so you can divide by 1 and get the correct
		result).
	 */
	Colour operator/(float val)
	{
		Colour retval;

		retval.red = static_cast<unsigned char>(static_cast<float>(red)/
												val);
		retval.green = static_cast<unsigned char>(static_cast<float>(green)/
												  val);
		retval.blue = static_cast<unsigned char>(static_cast<float>(blue)/
												 val);

		return retval;
	};

	///	Addition and assignment operator.
	Colour& operator+=(const Colour& other)
	{
		red = red + other.red;
		green = green + other.green;
		blue = blue + other.blue;

		return *this;
	};
	///	Addition and assignment operator (unsigned char).
	Colour& operator+=(unsigned char val)
	{
		red = red + val;
		green = green + val;
		blue = blue + val;

		return *this;
	};
	///	Addition and assignment operator (float).
	/*!
		\param val Is assumed to be a value limited between 0 and 1.
	 */
	Colour& operator+=(float val)
	{
		red = red + static_cast<unsigned char>(val * 255.0f);
		green = green + static_cast<unsigned char>(val * 255.0f);
		blue = blue + static_cast<unsigned char>(val * 255.0f);

		return *this;
	};

	///	Subtraction and assignment operator.
	Colour& operator-=(const Colour& other)
	{
		red = red - other.red;
		green = green - other.green;
		blue = blue - other.blue;

		return *this;
	};
	///	Subtraction and assignment operator (unsigned char).
	Colour& operator-=(unsigned char val)
	{
		red = red - val;
		green = green - val;
		blue = blue - val;

		return *this;
	};
	///	Subtraction and assignment operator (float).
	/*!
		\param val Is assumed to be a value limited between 0 and 1.
	 */
	Colour& operator-=(float val)
	{
		red = red - static_cast<unsigned char>(val * 255.0f);
		green = green - static_cast<unsigned char>(val * 255.0f);
		blue = blue - static_cast<unsigned char>(val * 255.0f);

		return *this;
	};

	///	Multiplication and assignment operator.
	Colour& operator*=(const Colour& other)
	{
		red = red * other.red;
		green = green * other.green;
		blue = blue * other.blue;

		return *this;
	};
	///	Multiplication and assignment operator (unsigned char).
	Colour& operator*=(unsigned char val)
	{
		red = red * val;
		green = green * val;
		blue = blue * val;

		return *this;
	};
	///	Multiplication and assignment operator (float).
	/*!
		\param val Not limited (so you can multiply by 1 and get the correct
		result).
	 */
	Colour& operator*=(float val)
	{
		red = static_cast<unsigned char>(static_cast<float>(red) * val);
		green = static_cast<unsigned char>(static_cast<float>(green) * val);
		blue = static_cast<unsigned char>(static_cast<float>(blue) * val);

		return *this;
	};

	///	Division and assignment operator.
	Colour& operator/=(const Colour& other)
	{
		red = static_cast<unsigned char>(static_cast<float>(red)/
										 static_cast<float>(other.red));
		green = static_cast<unsigned char>(static_cast<float>(green)/
										   static_cast<float>(other.green));
		blue = static_cast<unsigned char>(static_cast<float>(blue)/
										  static_cast<float>(other.blue));

		return *this;
	};
	///	Division and assignment operator (unsigned char).
	Colour& operator/=(unsigned char val)
	{
		red = static_cast<unsigned char>(static_cast<float>(red)/
										 static_cast<float>(val));
		green = static_cast<unsigned char>(static_cast<float>(green)/
										   static_cast<float>(val));
		blue = static_cast<unsigned char>(static_cast<float>(blue)/
										  static_cast<float>(val));

		return *this;
	};
	///	Division and assignment operator (float).
	/*!
		\param val Not limited (so you can divide by 1 and get the correct
		result).
	 */
	Colour& operator/=(float val)
	{
		red = static_cast<unsigned char>(static_cast<float>(red)/
										 val);
		green = static_cast<unsigned char>(static_cast<float>(green)/
										   val);
		blue = static_cast<unsigned char>(static_cast<float>(blue)/
										  val);

		return *this;
	};

	///	Addition operator (unsigned char), (uc + Colour).
	friend Colour operator+(unsigned char val, const Colour& col);
	///	Addition operator (float), (f + Colour).
	/*!
		\param val Is assumed to be a value limited between 0 and 1.
	 */
	friend Colour operator+(float val, const Colour& col);
	///	Substraction operator (unsigned char), (uc - Colour).
	friend Colour operator-(unsigned char val, const Colour& col);
	///	Substraction operator (float), (f - Colour).
	/*!
		\param val Is assumed to be a value limited between 0 and 1.
	 */
	friend Colour operator-(float val, const Colour& col);
	///	Multiplication operator (unsigned char), (uc * Colour).
	friend Colour operator*(unsigned char val, const Colour& col);
	///	Multiplication operator (float), (f * Colour).
	/*!
		\param val Not limited (so you can multiply by 1 and get the correct
		result).
	 */
	friend Colour operator*(float val, const Colour& col);
	///	Division operator (unsigned char), (uc / Colour).
	friend Colour operator/(unsigned char val, const Colour& col);
	///	Division operator (float), (f / Colour).
	/*!
		\param val Not limited (so you can divide by 1 and get the correct
		result).
	 */
	friend Colour operator/(float val, const Colour& col);
};

///	Addition operator (unsigned char), (uc + Colour).
Colour operator+(unsigned char val, const Colour& col);
///	Addition operator (float), (f + Colour).
/*!
	\param val Is assumed to be a value limited between 0 and 1.
 */
Colour operator+(float val, const Colour& col);
///	Substraction operator (unsigned char), (uc - Colour).
Colour operator-(unsigned char val, const Colour& col);
///	Substraction operator (float), (f - Colour).
/*!
	\param val Is assumed to be a value limited between 0 and 1.
 */
Colour operator-(float val, const Colour& col);
///	Multiplication operator (unsigned char), (uc * Colour).
Colour operator*(unsigned char val, const Colour& col);
///	Multiplication operator (float), (f * Colour).
/*!
	\param val Not limited (so you can multiply by 1 and get the correct
	result).
 */
Colour operator*(float val, const Colour& col);
///	Division operator (unsigned char), (uc / Colour).
Colour operator/(unsigned char val, const Colour& col);
///	Division operator (float), (f / Colour).
/*!
	\param val Not limited (so you can divide by 1 and get the correct
	result).
 */
Colour operator/(float val, const Colour& col);

#endif
