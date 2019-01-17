/*================================================================================================*/
/*
 *	Copyright 2009-2015 by Avid Technology, Inc.
 *	All rights reserved.
 *	
 *	CONFIDENTIAL: This document contains confidential information.  Do not 
 *	read or examine this document unless you are an Avid Technology employee
 *	or have signed a non-disclosure agreement with Avid Technology which protects
 *	the	confidentiality of this document.  DO NOT DISCLOSE ANY INFORMATION 
 *	CONTAINED IN THIS DOCUMENT TO ANY THIRD-PARTY WITHOUT THE PRIOR WRITTEN	CONSENT 
 *	OF Avid Technology, INC.
 *
 */

/**  
 *	\file   AAX_CString.cpp
 *	
 *	\author Dave Tremblay
 *
 */ 
/*================================================================================================*/
#include "AAX_CString.h"
#include <sstream>
#include <cstring>

AAX_CString::AAX_CString()  :
	mString("")
{

}


AAX_CString::AAX_CString ( const char * inString ) :
mString(inString ? inString : "")
{
}

AAX_CString::AAX_CString(const std::string& other) :
mString(other)
{
}

AAX_CString::AAX_CString(const AAX_CString& other) :
	mString(other.mString)
{

}

AAX_CString::AAX_CString(const AAX_IString& other) :
	mString(other.Get())
{
}

AAX_CString&	AAX_CString::operator=(const AAX_CString& other)
{	
	mString = other.mString;
	return *this;
}

AAX_CString&	AAX_CString::operator=(const std::string& other)
{	
	mString = other;
	return *this;
}

std::ostream& operator<< (std::ostream& os, const AAX_CString& str)
{
	os << str.mString; 
	return os;
}

std::istream& operator>> (std::istream& is, AAX_CString& str)
{
	is >> str.mString;
	return is;
}


/*  Virtual Overrides ******************************************************************************************/
uint32_t	AAX_CString::Length() const
{
	return uint32_t(mString.length());
}

uint32_t	AAX_CString::MaxLength() const
{
	return kMaxStringLength;
}

const char *	AAX_CString::Get ()	const
{
	return mString.c_str();
}

void			AAX_CString::Set ( const char * inString )
{
	mString = inString;
}


void		AAX_CString::Clear()
{
	mString.clear();
}

bool		AAX_CString::Empty() const
{
	return mString.empty();
}


AAX_CString&	AAX_CString::Erase(uint32_t pos, uint32_t n)
{
	// bounds check
	uint32_t strlen = this->Length();
	if ( strlen - pos < n )
		n = n - (strlen - pos);
	mString.erase(pos, n);
	return *this;
}

AAX_CString&	AAX_CString::Append(const AAX_CString& str)
{
	mString.append(str.CString());
	return *this;
}

AAX_CString&	AAX_CString::Append(const char* str)
{
	mString.append(str);
	return *this;
}

AAX_CString&	AAX_CString::AppendNumber(double number, int32_t precision)
{
	std::ostringstream	outStringStream;
	outStringStream.setf(std::ios::fixed, std::ios::floatfield);       
	outStringStream.precision(precision);
	outStringStream << number;
	mString += outStringStream.str();
	return *this;
}

AAX_CString&	AAX_CString::AppendNumber(int32_t number)
{
	std::ostringstream	outStringStream;
	outStringStream << number;
	mString += outStringStream.str();
	return *this;
}

AAX_CString&	AAX_CString::AppendHex(int32_t number, int32_t width)
{
	std::ostringstream	outStringStream;
	outStringStream.setf(std::ios::hex, std::ios::basefield);
	outStringStream.setf(std::ios::right, std::ios::adjustfield);
	outStringStream.fill('0');
	outStringStream << "0x";
	const std::streamsize resetWidth = outStringStream.width(width);
	outStringStream << number;
	outStringStream.width(resetWidth);
	
	mString += outStringStream.str();
	return *this;
}

AAX_CString&	AAX_CString::Insert(uint32_t pos, const AAX_CString&	str)
{
	mString.insert(pos, str.CString());
	return *this;
}

AAX_CString&	AAX_CString::Insert(uint32_t pos, const char* str)
{
	mString.insert(pos, str);
	return *this;
}

AAX_CString&	AAX_CString::InsertNumber(uint32_t pos, double number, int32_t precision)
{
	std::ostringstream	outStringStream;
	outStringStream.setf(std::ios::fixed, std::ios::floatfield);       
	outStringStream.precision(precision);
	outStringStream << number;
	mString.insert(pos, outStringStream.str());
	return *this;
}

AAX_CString&	AAX_CString::InsertNumber(uint32_t pos, int32_t number)
{
	std::ostringstream	outStringStream;
	outStringStream << number;
	mString.insert(pos, outStringStream.str());
	return *this;
}

AAX_CString&	AAX_CString::InsertHex(uint32_t pos, int32_t number, int32_t width)
{
	std::ostringstream	outStringStream;
	outStringStream.setf(std::ios::hex, std::ios::basefield);
	outStringStream.setf(std::ios::right, std::ios::adjustfield);
	outStringStream.fill('0');
	outStringStream << "0x";
	const std::streamsize resetWidth = outStringStream.width(width);
	outStringStream << number;
	outStringStream.width(resetWidth);
	
	mString.insert(pos, outStringStream.str());
	return *this;
}

AAX_CString&	AAX_CString::Replace(uint32_t pos, uint32_t n, const AAX_CString& str)
{
	mString.replace(pos, n, str.CString());
	return *this;
}

AAX_CString&	AAX_CString::Replace(uint32_t pos, uint32_t n, const char* str)
{
	mString.replace(pos, n, str);
	return *this;
}

uint32_t		AAX_CString::FindFirst(const AAX_CString& ) const
{
	return kInvalidIndex;
}

uint32_t		AAX_CString::FindFirst(const char* ) const
{
	return kInvalidIndex;
}

uint32_t		AAX_CString::FindFirst(char ) const
{
	return kInvalidIndex;
}

uint32_t		AAX_CString::FindLast(const AAX_CString& ) const
{
	return kInvalidIndex;
}

uint32_t		AAX_CString::FindLast(const char* ) const
{
	return kInvalidIndex;
}

uint32_t		AAX_CString::FindLast(char ) const
{
	return kInvalidIndex;
}

/** Direct access to a std::string. */
std::string&		AAX_CString::StdString()
{
    return mString;
}
	
/** Direct access to a const std::string. */
const std::string&	AAX_CString::StdString() const
{
    return mString;
}

const char*		AAX_CString::CString()	const
{
	return mString.c_str();
}

bool			AAX_CString::ToDouble(double* outValue) const
{
	std::istringstream	inStringStream(mString, std::istream::in);
	inStringStream >> *outValue;
	if (inStringStream.fail())
	{
		*outValue = 0;
		return false;
	}
	return true;
}

bool			AAX_CString::ToInteger(int32_t* outValue) const
{
	std::istringstream	inStringStream(mString, std::istream::in);
	inStringStream >> *outValue;
	if (inStringStream.fail())
	{
		*outValue = 0;
		return false;
	}
	return true;
}

void		AAX_CString::SubString(uint32_t pos, uint32_t n, AAX_IString* outputStr) const
{
	outputStr->Set(mString.substr(pos, n).c_str());
}


/* Virtual overriden operators *************************************************************************/

AAX_IString&	AAX_CString::operator=(const AAX_IString& other)
{	
	mString = other.Get();
	return *this;
}

AAX_IString&	AAX_CString::operator=(const char*	str)
{
	mString = str;
	return *this;
}

bool		AAX_CString::operator==(const AAX_CString& other) const
{
	return !strcmp(CString(), other.CString());
}

bool		AAX_CString::operator==(const std::string& other) const
{
	return (mString == other);
}

bool		AAX_CString::operator==(const char* otherStr) const
{
	return !strcmp(CString(), otherStr);
}

bool		AAX_CString::operator!=(const AAX_CString& other) const
{
	return strcmp(CString(), other.CString()) != 0;
}

bool		AAX_CString::operator!=(const std::string& other) const
{
	return (mString != other);
}

bool		AAX_CString::operator!=(const char* otherStr) const
{
	return strcmp(CString(), otherStr) != 0;
}

bool		AAX_CString::operator<(const AAX_CString& other) const
{
	return (strcmp(CString(), other.CString()) < 0);
}

bool		AAX_CString::operator>(const AAX_CString& other) const
{
	return (strcmp(CString(), other.CString()) > 0);
}

const char&	AAX_CString::operator[](uint32_t index) const
{
	return mString[index];
}

char&		AAX_CString::operator[](uint32_t index)
{
	return mString[index];
}

AAX_CString&	AAX_CString::operator+=(const AAX_CString& str)
{
	mString += str.CString();
	return *this;
}

AAX_CString&	AAX_CString::operator+=(const std::string& str)
{
	mString += str.c_str();
	return *this;
}

AAX_CString&	AAX_CString::operator+=(const char* str)
{
	mString += str;
	return *this;
}
	








