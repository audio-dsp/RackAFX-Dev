//	MainEditor.h - The main editor window.
//	--------------------------------------------------------------------------
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
//	--------------------------------------------------------------------------

#ifndef MAINEDITOR_H_
#define MAINEDITOR_H_

#include "MultiGUIEditor.h"
#include "EffectsEditor.h"
#include "ModEditor.h"
#include "RoutingsEditor.h"

class MainEditor : public MultiGUIEditor,
				   public Timer
{
  public:
	///	Constructor.
	MainEditor(AudioEffect *effect);
	///	Desctructor.
	~MainEditor();

	///	Updates the various GUIs.
	void setParameter(long index, float value);

	///	Used to update the graphics for the modPanel and the routingsPanel.
	void timerCallback();
  private:
	EffectsEditor *effectsPanel;
	ModEditor *modPanel;
	RoutingsEditor *routingsPanel;
};

#endif
