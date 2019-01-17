//	ModContainer.cpp - Container for a single mod source.
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

#include "ModContainer.h"
#include "LfoGui.h"
#include "EnvGui.h"
#include "KnobGui.h"

#include "VstPlugin.h"

//-----------------------------------------------------------------------------
ModContainer::ModContainer(int tagStart,
						   CViewContainer *container,
						   CControlListener *listener,
						   VstPlugin *plugin):
viewContainer(container),
modIndex(0)
{
	modSources[0] = new LfoGui(this, tagStart, listener, plugin);
	modSources[1] = new EnvGui(this, tagStart, listener, plugin);
	modSources[2] = new KnobGui(this, tagStart, listener, plugin);
}

//-----------------------------------------------------------------------------
ModContainer::~ModContainer()
{
	int i;

	for(i=0;i<3;++i)
		delete modSources[i];
}

//-----------------------------------------------------------------------------
void ModContainer::setGui(int index)
{
	if(index > 2)
		index %= 3;

	if(index != modIndex)
	{
		modSources[modIndex]->hide();
		modIndex = index;
		modSources[modIndex]->show();
	}
}

//-----------------------------------------------------------------------------
void ModContainer::show()
{
	modSources[modIndex]->show();
}

//-----------------------------------------------------------------------------
void ModContainer::hide()
{
	modSources[modIndex]->hide();
}

//-----------------------------------------------------------------------------
void ModContainer::setParameter(int index, float value, CDrawContext *context)
{
	int i;

	for(i=0;i<3;++i)
		modSources[i]->setParameter(index, value, context);
}

//-----------------------------------------------------------------------------
void ModContainer::timerCallback()
{
	if(modIndex == 2)
		modSources[2]->timerCallback();
	modSources[0]->timerCallback();
	modSources[1]->timerCallback();
	modSources[2]->timerCallback();
}
