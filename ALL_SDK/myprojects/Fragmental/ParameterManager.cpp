//	ParameterManager.cpp - Singleton class which manages parameters for the
//						   plugin and all its various helper classes.
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

#include "ParameterManager.h"

using namespace std;

//----------------------------------------------------------------------------
ParameterManager::ParameterManager()
{
	
}

//----------------------------------------------------------------------------
ParameterManager::~ParameterManager()
{
	
}

//----------------------------------------------------------------------------
VstInt32 ParameterManager::registerParameter(ParameterCallback *callback,
											 const string& name,
											 const string& units)
{
	Parameter tempParam;

	tempParam.callback = callback;
	tempParam.name = name;
	tempParam.units = units;

	parameters.push_back(tempParam);

	return (parameters.size() - 1);
}

//----------------------------------------------------------------------------
void ParameterManager::setParameter(VstInt32 index, float val)
{
	parameters[index].callback->parameterChanged(index, val);
}

//----------------------------------------------------------------------------
string ParameterManager::getParameterValue(VstInt32 index) const
{
	return parameters[index].callback->getTextValue(index);
}

//----------------------------------------------------------------------------
const string& ParameterManager::getParameterName(VstInt32 index) const
{
	return parameters[index].name;
}

//----------------------------------------------------------------------------
const string& ParameterManager::getParameterUnits(VstInt32 index) const
{
	return parameters[index].units;
}

//----------------------------------------------------------------------------
float ParameterManager::operator[](VstInt32 index) const
{
	return parameters[index].callback->getValue(index);
}
