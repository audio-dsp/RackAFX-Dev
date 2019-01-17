//	ModContainer.h - Container for a single mod source.
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

#ifndef MODCONTAINER_H_
#define MODCONTAINER_H_

#include "vstcontrols.h"

class ModSourceGui;
class VstPlugin;

///	Container for a single mod source.
/*!
	Has a CViewContainer into which controls are placed depending on which
	mod source type is currently active for it.
 */
class ModContainer
{
  public:
	///	Constructor.
	/*!
		\param tagStart The index at which the parameters for this container
		start (so we can pass it to the ModSourceGui instances).
		\param container Pointer to the CViewContainer into which the
		ModSourceGui's controls will be placed.
		\param listener Pointer to the CControlListener which will handle any
		parameter changes in the individual mod sources.
		\param Pointer to the plugin instance.
	 */
	ModContainer(int tagStart,
				 CViewContainer *container,
				 CControlListener *listener,
				 VstPlugin *plugin);
	///	Destructor.
	~ModContainer();

	///	Changes which ModSourceGui is currently displayed.
	void setGui(int index);
	///	Called to display this container.
	void show();
	///	Called to hide this container.
	void hide();

	///	Returns this container's CViewContainer.
	CViewContainer *getViewContainer() {return viewContainer;};

	///	Called to update a control's value.
	void setParameter(int index, float value, CDrawContext *context = 0);

	///	Used to update the knob's position.
	void timerCallback();
  private:
	///	This container's CViewContainer.
	CViewContainer *viewContainer;

	///	Our three ModSourceGuis.
	ModSourceGui *modSources[3];
	///	Index of the currently-displayed ModSourceGui.
	int modIndex;
};

#endif
