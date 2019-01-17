//	MainEditor.cpp - The main editor window.
//	---------------------------------------------------------------------------
//	Copyright (c) 2005-2007 Niall Moody
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

#include "MainEditor.h"
#include "AudioEffect.h"

//-----------------------------------------------------------------------------
MainEditor::MainEditor(AudioEffect *effect):
MultiGUIEditor(effect),
Timer(30)
{
	ERect *panelRect = 0;
	NiallsRect tempRect;

	//These are deleted in MultiGUIEditor's destructor.
	/*glPanel = new GlEditor(effect);

	tempRect.x = 0;
	tempRect.y = 0;
	glPanel->getRect(&panelRect);
	tempRect.width = (panelRect->right - panelRect->left);
	tempRect.height = (panelRect->bottom - panelRect->top);
	addEditor(glPanel, tempRect);*/

	effectsPanel = new EffectsEditor(effect);
	modPanel = new ModEditor(effect);
	routingsPanel = new RoutingsEditor(effect);

	tempRect.x = 0;
	tempRect.y = 0;
	effectsPanel->getRect(&panelRect);
	tempRect.width = (panelRect->right - panelRect->left);
	tempRect.height = (panelRect->bottom - panelRect->top);
	addEditor(effectsPanel, tempRect);

	tempRect.y = tempRect.height;
	modPanel->getRect(&panelRect);
	tempRect.width = (panelRect->right - panelRect->left);
	tempRect.height = (panelRect->bottom - panelRect->top);
	addEditor(modPanel, tempRect);

	tempRect.x = tempRect.width;
	routingsPanel->getRect(&panelRect);
	tempRect.width = (panelRect->right - panelRect->left);
	tempRect.height = (panelRect->bottom - panelRect->top);
	addEditor(routingsPanel, tempRect);

	//Is this necessary?
	effect->setEditor(this);

	//Start timer.
	start();
}

//-----------------------------------------------------------------------------
MainEditor::~MainEditor()
{
	//Stop timer.
	stop();
}

//-----------------------------------------------------------------------------
void MainEditor::setParameter(long index, float value)
{
	effectsPanel->setParameter(index, value);
	modPanel->setParameter(index, value);
	routingsPanel->setParameter(index, value);
}

//-----------------------------------------------------------------------------
void MainEditor::timerCallback()
{
	routingsPanel->timerCallback();
	modPanel->timerCallback();
}
