//	MultiGUIEditor.h - Class to allow you to add multiple editors to the
//					   window passed to you by the host.
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

#ifndef MULTIGUIEDITOR_H_
#define MULTIGUIEDITOR_H_

#include "AEffEditor.hpp"

#include <vector>

#ifdef WIN32
#include <windows.h>
#endif
#ifdef MACX
#include <Carbon/Carbon.h>
#endif

///	A more sensible way to store a rect, imo.
typedef struct fourints
{
	int x;
	int y;
	int width;
	int height;
} NiallsRect;

///	An editor class which is used to add multiple types of editors to the window provided by the host.
/*!
	The main use of this is to allow for OpenGL editor panels alongside
	standard VSTGUI/2D ones.
	You probably want to subclass this class to use as your main editor class
	(i.e. the one you construct from your plugin's constructor).
 */
class MultiGUIEditor : public AEffEditor
{
  public:
	MultiGUIEditor(AudioEffect *effect);
	virtual ~MultiGUIEditor();

	///	_rect is calculated from the editor panels held in _editors.
	virtual long getRect(ERect **rect) {*rect = &_rect; return true;};
	virtual long open(void *ptr);
	virtual void close();
	virtual void idle();

	///	So we can call this from the plugin, and pass it on to the appropriate editor panels.
	/*!
		You have to implement this in your MultiGUIEditor subclass, because
		AEffEditor doesn't have a setParameter() method...
	 */
	virtual void setParameter(long index, float value) = 0;
	///	Intended to allow for resizeable interfaces (although I don't know if this is enough...).
	virtual void setSize(ERect rect) {_rect = rect;};

	///	Use this to add a new editor panel to the main plugin window.
	/*!
		_rect is updated for every editor panel added.
	 */
	void addEditor(AEffEditor *editor, NiallsRect rect);
  private:
	///	Dynamic array of all the current editor panels.
	std::vector<AEffEditor *> _editors;
	///	Dynamic array of the positions & sizes for each editor panel.
	std::vector<NiallsRect> _rects;
	///	Dynamic array to hold the systemWindow pointers for each editor
	/*!	
		(since AEffEditor doesn't actually provide a getSystemWindow()
		method...)
	 */
	std::vector<void *> _systemWindows;

	///	The rectangle holding the actual size of the plugin window (i.e. the size of each editor panel added up).
	ERect _rect;

#ifdef WIN32
	HWND tempHWnd;
#elif MACX
	//ToolboxObjectClassRef controlClass;
	//ControlDefSpec controlSpec;
	WindowGroupRef mainGroup;
	//WindowGroupRef parentGroup;
#endif
};

#endif
