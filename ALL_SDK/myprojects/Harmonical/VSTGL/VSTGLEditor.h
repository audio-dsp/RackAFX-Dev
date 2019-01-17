//	VSTGLEditor.h - Editor window for a VST plugin using OpenGL to handle all
//					the drawing.  This is a very basic implementation - it's
//					up to you to implement widgets etc.
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

#ifndef VSTGLEDITOR_H_
#define VSTGLEDITOR_H_

#include "AEffEditor.hpp"

#ifdef WIN32
	#include <windows.h>
	#include <GL/gl.h>
	#include <GL/glu.h>
#elif MACX
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
	#include <AGL/agl.h>
#endif

///	Editor window for VST plugins, using OpenGL to handle all the drawing.
/*!
	This is a very basic implementation, just setting up an opengl drawing
	context and providing it with constant idle() calls to refresh it. If
	you want widgets, you'll have to make your own.

	It should be pretty self-explanatory how to use it - most of the methods
	are the usual AEffEditor ones.  All the on...() methods are called
	whenever the associated event occurs.  You do your drawing stuff inside
	draw(), and if you want to update the gui when your plugins parameters
	change, you'd call setParameter from the plugin, and put whatever code
	you want inside the editor's setParameter method.
 */
class VSTGLEditor : public AEffEditor
{
  public:
	///	Constructor.
	VSTGLEditor(AudioEffect *effect);
	///	Destructor.
	virtual ~VSTGLEditor();

	///	Returns the size of the editor, so the host can provide the correct-sized window.
	virtual long getRect(ERect **rect) {*rect = &_rect; return true;};
	///	Called by the host once the window's been created, so we can set stuff up in it.
	virtual long open(void *ptr);
	///	Called by the host just before the window is destroyed/closed.
	virtual void close();
	///	Each time this is called, the window/drawing context is refreshed.
	virtual void idle();

	///	Called when there's a MouseDown event.
	/*!
		The buttons are numbered 1=left, 2=right, 3=middle.
	 */
	virtual void onMouseDown(int button, int x, int y) {};
	///	Called when there's a MouseMove event.
	virtual void onMouseMove(int x, int y) {};
	///	Called when there's a MouseUp event.
	/*!
		The buttons are numbered 1=left, 2=right, 3=middle.
	 */
	virtual void onMouseUp(int button, int x, int y) {};
	///	Called when there's a MouseWheel event.
	virtual void onMouseWheel(int wheel, int x, int y) {};
	///	Called when there's a KeyDown event (GL because the VST headers already use onKeyDown).
	/*!
		\todo test whether the key mapping's correct.
	 */
	virtual void onGLKeyDown(VstKeyCode& key) {};
	///	Called when there's a KeyUp event (GL because the VST headers already use onKeyUp).
	/*!
		\todo test whether the key mapping's correct.
	 */
	virtual void onGLKeyUp(VstKeyCode& key) {};

	///	This is the method where everything gets drawn.
	/*!
		It's into here that you'll want to put your drawing code (in a subclass).
		This is called repeatedly from the idle() method.
	 */
	virtual void draw() = 0;

	///	You'd call this from your plugin if you wanted to update the gui when your parameters change.
	virtual void setParameter(long index, float value) {};
  protected:
	///	The rect our opengl context is contained within.
	ERect _rect;
  private:
	///	Used to create the actual window when open() is called.
	/*!
		Only necessary on Windows (and only really for Tracktion, which does things
		kind of strange).
	 */
	void createWindow();

#ifdef WIN32
	///	Windows rendering context.
	HGLRC glRenderingContext;
	///	holds a handle to the window we created in createWindow().
	HWND tempHWnd;
	///	Device context for our window.
	HDC dc;
#elif MACX
	///	OS X rendering context.
	AGLContext context;
	///	The pixel format we used.
	/*!
		\todo Should make this cross-platform, and allow subclasses to set it.
	 */
	AGLPixelFormat pixels;
#endif
};

#endif
