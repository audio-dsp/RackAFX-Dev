//	ModSourceGui.h - Abstract base class for all mod source Guis.
//	---------------------------------------------------------------------------
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
//	---------------------------------------------------------------------------

#ifndef MODSOURCEGUI_H_
#define MODSOURCEGUI_H_

#include "vstcontrols.h"

class ModContainer;
class VstPlugin;

///	Abstract base class for all mod source Guis.
class ModSourceGui
{
  public:
	///	Constructor.
	/*!
		\param container The owning ModContainer.
		\param tagStart The index at which the source's parameters start.
		\param listener Pointer to the listener which will handle any parameter
		changes.
		\param plugin Pointer to the plugin instance.
	 */
	ModSourceGui(ModContainer *container,
				 int tagStart,
				 CControlListener *listener,
				 VstPlugin *plugin) {};
	///	Destructor.
	virtual ~ModSourceGui() {};

	///	Called to update a control's value.
	virtual void setParameter(int index,
							  float value,
							  CDrawContext *context = 0) = 0;

	///	Returns the number of parameters used by this mod source.
	virtual int getNumParameters() const = 0;

	///	Called to display the gui.
	/*!
		Make sure you set the parameters after/before you call this!!!!
	 */
	virtual void show() = 0;
	///	Called to hide the gui.
	virtual void hide() = 0;

	///	Used to update the knob's position.
	virtual void timerCallback() {};
};

#endif
