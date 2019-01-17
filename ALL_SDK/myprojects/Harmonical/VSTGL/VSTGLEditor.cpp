//	VSTGLEditor.cpp - Editor window for a VST plugin using OpenGL to handle
//					  all the drawing.  This is a very basic implementation -
//					  it's up to you to implement widgets etc.
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
//	ToDo:
//	-----
//	- Test onGLKey methods - are we getting the same values on Windows & OS X?
//	  (and do they correspond correctly to the vstKeyCode values?)
//	- GL attributes.
//	- Context sharing.
//	--------------------------------------------------------------------------

#include "VSTGLEditor.h"
#include "AudioEffect.hpp"

#include <cstdio>

#ifdef WIN32
//This is the instance of the application, set in the main source file.
extern void* hInstance;

//Message loop - we use this to intercept mouse messages, among other things.
LONG WINAPI GLWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
#elif MACX
#include <Carbon/Carbon.h>

static pascal OSStatus macEventHandler(EventHandlerCallRef handler,
									   EventRef event,
									   void *userData);
#endif

//----------------------------------------------------------------------------
VSTGLEditor::VSTGLEditor(AudioEffect *effect):
AEffEditor(effect)
{
	effect->setEditor(this);

	_rect.left = 0;
	_rect.top = 0;
	_rect.right = 0;
	_rect.bottom = 0;

#ifdef WIN32
	char tempstr[32];

	dc = 0;
	glRenderingContext = 0;

	//Register window class.
	WNDCLASSEX winClass;

	//most of this stuff is copied from VSTGUI
	winClass.cbSize = sizeof(WNDCLASSEX);
	//winClass.style = CS_GLOBALCLASS|CS_OWNDC;
	winClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
	winClass.lpfnWndProc = GLWndProc;
	winClass.cbClsExtra = 0;
	winClass.cbWndExtra = 0;
	winClass.hInstance = static_cast<HINSTANCE>(hInstance);
	winClass.hIcon = 0;
	winClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	//winClass.hbrBackground = NULL;
	winClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	winClass.lpszMenuName = 0;
	sprintf(tempstr, "VSTGLWindow%08x", static_cast<HINSTANCE>(hInstance));
	winClass.lpszClassName = tempstr;
	winClass.hIconSm = NULL;

	//Register the window class (this is unregistered in the
	//VSTGLEditor destructor).
	RegisterClassEx(&winClass);
#elif MACX
	context = NULL;
	pixels = NULL;
#endif
}

//----------------------------------------------------------------------------
VSTGLEditor::~VSTGLEditor()
{
#ifdef WIN32
	char tempstr[32];

	sprintf(tempstr, "VSTGLWindow%08x", static_cast<HINSTANCE>(hInstance));
	//unregisters the window class
	UnregisterClass(tempstr, static_cast<HINSTANCE>(hInstance));
#endif
}

//----------------------------------------------------------------------------
long VSTGLEditor::open(void *ptr)
{
	AEffEditor::open(ptr);
	createWindow();

#ifdef WIN32
	int tempint;
	PIXELFORMATDESCRIPTOR pixelformat;
	dc = GetDC(tempHWnd);

	//have to set the pixel format first...
	pixelformat.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pixelformat.nVersion = 1;
    pixelformat.dwFlags = PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
    pixelformat.iPixelType = PFD_TYPE_RGBA;
    pixelformat.cColorBits = 32;
    pixelformat.cRedBits = 0;
    pixelformat.cRedShift = 0;
    pixelformat.cGreenBits = 0;
    pixelformat.cGreenShift = 0;
    pixelformat.cBlueBits = 0;
    pixelformat.cBlueShift = 0;
    pixelformat.cAlphaBits = 0;
    pixelformat.cAlphaShift = 0;
    pixelformat.cAccumBits = 0;
    pixelformat.cAccumRedBits = 0;
    pixelformat.cAccumGreenBits = 0;
    pixelformat.cAccumBlueBits = 0;
    pixelformat.cAccumAlphaBits = 0;
    pixelformat.cDepthBits = 32;
    pixelformat.cStencilBits = 0;
    pixelformat.cAuxBuffers = 0;
    pixelformat.iLayerType = PFD_MAIN_PLANE;
    pixelformat.bReserved = 0;
    pixelformat.dwLayerMask = 0;
    pixelformat.dwVisibleMask = 0;
    pixelformat.dwDamageMask = 0;
	tempint = ChoosePixelFormat(dc, &pixelformat);
	SetPixelFormat(dc, tempint, &pixelformat);

	glRenderingContext = wglCreateContext(dc);
	wglMakeCurrent(dc, glRenderingContext);
#elif MACX
	GrafPtr port;
	Rect windowBounds;
	WindowRef window = static_cast<WindowRef>(ptr);

	//Set up the event handler to take care of mouse & keyboard events.
	EventHandlerUPP eventHandler;
    EventTypeSpec eventList[] = {{kEventClassMouse, kEventMouseDown},
								 {kEventClassMouse, kEventMouseUp},
								 {kEventClassMouse, kEventMouseMoved},
								 {kEventClassMouse, kEventMouseDragged},
								 {kEventClassMouse, kEventMouseWheelMoved},
								 {kEventClassKeyboard, kEventRawKeyDown},
								 {kEventClassKeyboard, kEventRawKeyUp}};

	eventHandler = NewEventHandlerUPP(macEventHandler);
	InstallWindowEventHandler(window,
							  eventHandler,
							  GetEventTypeCount(eventList),
							  eventList,
							  this,
							  NULL);

	//--Bidule Stuff------
	GetPort(&port);
	SetPort(GetWindowPort(window));

	//We do this so that the opengl stuff doesn't draw over the buttons in
	//Bidule.
	PortSize((_rect.right-_rect.left), (_rect.bottom-_rect.top));
	//--------

	//Set up the AGL control.
	GLint attributes[] = {AGL_RGBA,
						  AGL_ACCELERATED,
						  AGL_DOUBLEBUFFER,
						  AGL_DEPTH_SIZE, 32,
						  AGL_NONE};

	pixels = aglChoosePixelFormat(NULL, 0, attributes);
	if(pixels)
		context = aglCreateContext(pixels, NULL);
	else
		return false;
	if(context)
	{
		GLint swap = 1;

		if(!aglSetDrawable(context, GetWindowPort(window)))
			return false;
		if(!aglSetCurrentContext(context))
			return false;

		//This means that the buffers are swapped during the vertical retrace
		//of the monitor, which could be useful...
		aglSetInteger(context, AGL_SWAP_INTERVAL, &swap);
	}
	else
		return false;

	//--Bidule Stuff------
	//Reset the window port before we leave, just in case.
	GetWindowBounds(window, kWindowContentRgn, &windowBounds);
	PortSize((windowBounds.right - windowBounds.left),
			 (windowBounds.bottom - windowBounds.top));
	SetPort(port);
	//--------
#endif

	return true;
}

//----------------------------------------------------------------------------
void VSTGLEditor::close()
{
#ifdef WIN32
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(glRenderingContext);
	ReleaseDC(tempHWnd, dc);
	DestroyWindow(tempHWnd);
#elif MACX
	aglSetCurrentContext(NULL);
	aglSetDrawable(context, NULL);
	if(context)
	{
		aglDestroyContext(context);
		context = NULL;
	}
	if(pixels)
	{
		aglDestroyPixelFormat(pixels);
		pixels = NULL;
	}
#endif
}

//----------------------------------------------------------------------------
void VSTGLEditor::idle()
{
#ifdef WIN32
	wglMakeCurrent(dc, glRenderingContext);
#elif MACX
	GrafPtr port;
	Rect windowBounds;
	WindowRef window = static_cast<WindowRef>(systemWindow);

	GetPort(&port);
	SetPort(GetWindowPort(window));

	//We do this so that the opengl stuff doesn't draw over the buttons in
	//Bidule.
	PortSize((_rect.right-_rect.left), (_rect.bottom-_rect.top));

	//Also necessary for Bidule.
	aglSetCurrentContext(context);
#endif

	draw();

#ifdef WIN32
	SwapBuffers(dc);
#elif MACX
	aglSwapBuffers(context);

	//Reset the window port before we leave, just in case.
	GetWindowBounds(window, kWindowContentRgn, &windowBounds);
	PortSize((windowBounds.right - windowBounds.left),
			 (windowBounds.bottom - windowBounds.top));
	SetPort(port);
#endif
}

//----------------------------------------------------------------------------
//This is only necessary in Windows (and only really for Tracktion) - the
//window will have already been constructed for us on OS X.
void VSTGLEditor::createWindow()
{
#ifdef WIN32
	char tempstr[32];
	HWND parentHWnd = static_cast<HWND>(systemWindow);

	sprintf(tempstr, "VSTGLWindow%08x", static_cast<HINSTANCE>(hInstance));
	tempHWnd = CreateWindowEx(0,				   //extended window style
							  tempstr,	   //pointer to registered class name
							  "VSTGLEditor",	   //pointer to window name
							  WS_CHILD|
							  WS_VISIBLE/*|
							  WS_CLIPSIBLINGS|
							  CS_DBLCLKS*/,	   //window style
							  0,				   //horizontal position of window
							  0,				   //vertical position of window
							  (_rect.right-_rect.left),//window width
							  (_rect.bottom-_rect.top),//window height
							  parentHWnd,		   //handle to parent or owner window
							  NULL,				   //handle to menu, or child-window identifier
							  (HINSTANCE)hInstance,//handle to application instance
							  NULL);			   //pointer to window-creation data

	//This is so we can send messages to this object from the message loop.
	SetWindowLong(tempHWnd, GWL_USERDATA, (long)this);
#endif
}

//----------------------------------------------------------------------------
#ifdef WIN32

//Don't know why MSVC doesn't seem to recognise WM_MOUSEWHEEL for me...
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A
#endif

LONG WINAPI GLWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	VstKeyCode tempkey;
	VSTGLEditor *ed = reinterpret_cast<VSTGLEditor *>(GetWindowLong(hwnd, GWL_USERDATA));

	switch(message)
	{
		case WM_LBUTTONDOWN:
			if(ed)
				ed->onMouseDown(1, LOWORD(lParam), HIWORD(lParam));
			break;
		case WM_MBUTTONDOWN:
			if(ed)
				ed->onMouseDown(3, LOWORD(lParam), HIWORD(lParam));
			break;
		case WM_RBUTTONDOWN:
			if(ed)
				ed->onMouseDown(2, LOWORD(lParam), HIWORD(lParam));
			break;
		case WM_LBUTTONUP:
			if(ed)
				ed->onMouseUp(1, LOWORD(lParam), HIWORD(lParam));
			break;
		case WM_MBUTTONUP:
			if(ed)
				ed->onMouseUp(3, LOWORD(lParam), HIWORD(lParam));
			break;
		case WM_RBUTTONUP:
			if(ed)
			ed->onMouseUp(2, LOWORD(lParam), HIWORD(lParam));
			break;
		case WM_MOUSEMOVE:
			if(ed)
				ed->onMouseMove(LOWORD(lParam), HIWORD(lParam));
			break;
		case WM_MOUSEWHEEL:
			if(ed)
				ed->onMouseWheel(HIWORD(wParam), LOWORD(lParam), HIWORD(lParam));
			break;
		case WM_KEYDOWN:
			if(ed)
			{
				//This could be improved?
				tempkey.character = wParam;
				ed->onGLKeyDown(tempkey);
			}
			break;
		case WM_KEYUP:
			if(ed)
			{
				tempkey.character = wParam;
				ed->onGLKeyUp(tempkey);
			}
			break;
		case WM_PAINT:
			if(ed)
				ed->idle();
			break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}
#endif

//----------------------------------------------------------------------------
#ifdef MACX

static pascal OSStatus macEventHandler(EventHandlerCallRef handler,
									   EventRef event,
									   void *userData)
{
	int actualButton;
	EventMouseButton button;
	HIPoint location;
	EventMouseWheelAxis wheelAxis;
	SInt32 wheelDelta;
	UInt32 key;
	VstKeyCode tempKey;
	OSStatus result = eventNotHandledErr;
	VSTGLEditor *ed = static_cast<VSTGLEditor *>(userData);

	if(ed)
	{
		UInt32 eventClass = GetEventClass(event);
		UInt32 eventType = GetEventKind(event);

		if(eventClass == kEventClassMouse)
		{
			switch(eventType)
			{
				case kEventMouseDown:
			/*SInt16 tempty;
			StandardAlert(kAlertNoteAlert,
						  "thing",
						  NULL,
						  NULL,
						  &tempty);*/
					GetEventParameter(event,
									 kEventParamMouseButton,
									 typeMouseButton,
									 NULL,
									 sizeof(EventMouseButton),
									 NULL,
									 &button);
					GetEventParameter(event,
									 kEventParamWindowMouseLocation,
									 typeHIPoint,
									 NULL,
									 sizeof(HIPoint),
									 NULL,
									 &location);
					switch(button)
					{
						case kEventMouseButtonPrimary:
							actualButton = 1;
							break;
						case kEventMouseButtonSecondary:
							actualButton = 2;
							break;
						case kEventMouseButtonTertiary:
							actualButton = 3;
							break;
						default:
							actualButton = -1;
					}

					if(actualButton != -1)
						ed->onMouseDown(actualButton,
										static_cast<int>(location.x),
										static_cast<int>(location.y));

					//Apparently this is necessary for this event?
					CallNextEventHandler(handler, event);
					break;
				case kEventMouseUp:
					GetEventParameter(event,
									  kEventParamMouseButton,
									  typeMouseButton,
									  NULL,
									  sizeof(EventMouseButton),
									  NULL,
									  &button);
					GetEventParameter(event,
									  kEventParamWindowMouseLocation,
									  typeHIPoint,
									  NULL,
									  sizeof(HIPoint),
									  NULL,
									  &location);
					switch(button)
					{
						case kEventMouseButtonPrimary:
							actualButton = 1;
							break;
						case kEventMouseButtonSecondary:
							actualButton = 2;
							break;
						case kEventMouseButtonTertiary:
							actualButton = 3;
							break;
						default:
							actualButton = -1;
					}

					if(actualButton != -1)
						ed->onMouseUp(actualButton,
									  static_cast<int>(location.x),
									  static_cast<int>(location.y));
					break;
				case kEventMouseMoved:
					GetEventParameter(event,
									  kEventParamWindowMouseLocation,
									  typeHIPoint,
									  NULL,
									  sizeof(HIPoint),
									  NULL,
									  &location);

					ed->onMouseMove(static_cast<int>(location.x),
									static_cast<int>(location.y));
					break;
				//We handle dragging as well as moving events, because OS X
				//makes a specific distinction between them, whereas I
				//prefer to use the mouseUp, and mouseDown events to tell
				//whether I'm supposed to be dragging...
				case kEventMouseDragged:
					GetEventParameter(event,
									  kEventParamWindowMouseLocation,
									  typeHIPoint,
									  NULL,
									  sizeof(HIPoint),
									  NULL,
									  &location);

					ed->onMouseMove(static_cast<int>(location.x),
									static_cast<int>(location.y));
					break;
				case kEventMouseWheelMoved:
					GetEventParameter(event,
									  kEventParamMouseWheelAxis,
									  typeMouseWheelAxis,
									  NULL,
									  sizeof(EventMouseWheelAxis),
									  NULL,
									  &wheelAxis);
					GetEventParameter(event,
									  kEventParamMouseWheelDelta,
									  typeLongInteger,
									  NULL,
									  sizeof(SInt32),
									  NULL,
									  &wheelDelta);

					if(wheelAxis == kEventMouseWheelAxisY)
					{
						ed->onMouseWheel(static_cast<int>(wheelDelta),
										 static_cast<int>(location.x),
										 static_cast<int>(location.y));
					}
					break;
			}
		}
		else if(eventClass == kEventClassKeyboard)
		{
			switch(eventType)
			{
				case kEventRawKeyDown:
					GetEventParameter(event,
									  kEventParamKeyCode,
									  typeUInt32,
									  NULL,
									  sizeof(UInt32),
									  NULL,
									  &key);
					tempKey.character = key;
					ed->onGLKeyDown(tempKey);
					break;
				case kEventRawKeyUp:
					GetEventParameter(event,
									  kEventParamKeyCode,
									  typeUInt32,
									  NULL,
									  sizeof(UInt32),
									  NULL,
									  &key);
					tempKey.character = key;
					ed->onGLKeyUp(tempKey);
					break;
			}
		}
	}

	return result;
}

#endif
