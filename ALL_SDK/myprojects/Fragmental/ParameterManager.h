//	ParameterManager.h - Singleton class which manages parameters for the
//						 plugin and all its various helper classes.
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

#ifndef PARAMETERMANAGER_H_
#define PARAMETERMANAGER_H_

#include "HelperStuff.h"
#include <string>
#include <vector>

//-----------------------------------------------------------------------------
///	Callback class, called when ParameterManager receives a setParameter()
///	call.
class ParameterCallback
{
  public:
	///	Implement this in your subclass to be informed when a parameter
	///	changes value.
	/*!
		\param index The index of the parameter whose value has changed.
		\param val The new value of the indexed parameter.
	 */
	virtual void parameterChanged(VstInt32 index, float val) = 0;
	///	Returns the current value of the indexed parameter.
	virtual float getValue(VstInt32 index) = 0;
	///	The textual display of the indexed parameter's current value.
	/*!
		\param index The index of the parameter whose value we want to
		determine.
		\return A string containing the parameters textual representation.
	 */
	virtual std::string getTextValue(VstInt32 index) = 0;
};

//----------------------------------------------------------------------------
///	Simple struct containing all the information for one parameter in
///	ParameterManager.
struct Parameter
{
	ParameterCallback *callback;
	std::string name;
	std::string units;
};

//----------------------------------------------------------------------------
///	Singleton class which manages parameters for the plugin and its helper
///	classes.
/*!
	Actually, it's not strictly a singleton, as we have to work in shared
	memory, which would screw up singletons.  In an ideal case it would be a
	singleton, but as it is, we just have to retrieve it from the VstPlugin
	instance.

	Because the ModulationSource classes, and the EffectBase classes, both
	need to access parameter values which will be set from the plugin class,
	decided a singleton class might help encapsulate things a bit better (e.g.
	plugin->setParameter(index, val) just calls
	ParameterManager::setParameter(index, val), rather than having to know
	which parameter is used by which effect/modulation class.
	ParameterManager will then call a callback method to handle the parameter
	setting - this way, only the effect or modulation class which actually
	uses that particular parameter knows anything about it.

	I'm still not sure this is the best approach for this kind of thing (you
	can't really hard-code parameter index values with enums, for example),
	but its the best solution I've come up with so far.
 */
class ParameterManager
{
  public:
	///	Constructor.
	ParameterManager();
	///	Destructor.
	~ParameterManager();

	///	Registers a parameter with ParameterManager.
	/*!
		\param callback Pointer to a subclass of ParameterCallback, providing
		the method to be called when the parameter changes, etc.
		\param name The name of the parameter.
		\param units The units the parameters value is measured in.

		\return The index assigned to the parameter.
	 */
	VstInt32 registerParameter(ParameterCallback *callback,
							   const std::string& name,
							   const std::string& units);

	///	Call this to set the value of the indexed parameter.
	/*!
		\param index The index of the parameter whose value has changed.
		\param val The new value for the parameter.
	 */
	void setParameter(VstInt32 index, float val);

	///	Returns the number of parameters.
	VstInt32 getNumParameters() const {return parameters.size();};

	///	Returns a textual display of the current value of the indexed
	///	parameter.
	std::string getParameterValue(VstInt32 index) const;
	///	Returns the indexed parameters name.
	const std::string& getParameterName(VstInt32 index) const;
	///	Returns the indexed parameters units.
	const std::string& getParameterUnits(VstInt32 index) const;

	///	Returns the value of the indexed parameter.
	/*!
		It's const because we want the user to have to use setParameter() to
		\e set the parameters value.
	 */
	float operator[](VstInt32 index) const;
  private:
	std::vector<Parameter> parameters;
};

#endif
