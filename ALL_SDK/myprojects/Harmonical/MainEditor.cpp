//	MainEditor.cpp - The main editor window, a subclass of MultiGUIEditor.
//					 (you'll want to construct & use this in your plugin's
//					  constructor)
//	--------------------------------------------------------------------------
//	Copyright (c) 2005 Niall Moody
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

#include "MainEditor.h"
#include "AudioEffect.hpp"

MainEditor::MainEditor(AudioEffect *effect):
MultiGUIEditor(effect)
{
	ERect *stupidRect = 0;
	NiallsRect tempRect;

	//These are deleted in MultiGUIEditor's destructor.
	panel1 = new HarmonicalEditor3D(effect);
	panel2 = new HarmonicalEditor2D(effect);

	tempRect.x = 0;
	tempRect.y = 0;
	panel1->getRect(&stupidRect);
	tempRect.width = (stupidRect->right - stupidRect->left);
	tempRect.height = (stupidRect->bottom - stupidRect->top);
	addEditor(panel1, tempRect);

	tempRect.y = tempRect.height;
	panel2->getRect(&stupidRect);
	tempRect.width = (stupidRect->right - stupidRect->left);
	tempRect.height = (stupidRect->bottom - stupidRect->top);
	addEditor(panel2, tempRect);

	//Is this necessary?
	effect->setEditor(this);
}

void MainEditor::setParameter(long index, float value)
{
	panel1->setParameter(index, value);
	panel2->setParameter(index, value);
}
