//	MultiGUIEditor.cpp - Class to allow you to add multiple editors to the
//						 window passed to you by the host.
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

#include "MultiGUIEditor.h"

using namespace std;

#ifdef WIN32
//This is the instance of the application, set in the main source file.
//We use it to create the various child windows.
extern void* hInstance;

//Message loop - we don't do anything with this, but it has to be here...
LONG WINAPI WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
#endif

//----------------------------------------------------------------------------
MultiGUIEditor::MultiGUIEditor(AudioEffect *effect):
AEffEditor(effect)
{
	_rect.left = 0;
	_rect.top = 0;
	_rect.right = 0;
	_rect.bottom = 0;

#ifdef WIN32
	char tempstr[32];

	//Register window class.
	WNDCLASSEX winClass;

	//most of this stuff is copied from VSTGUI
	winClass.cbSize = sizeof(WNDCLASSEX);
	winClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
	winClass.lpfnWndProc = WndProc;
	winClass.cbClsExtra = 0;
	winClass.cbWndExtra = 0;
	winClass.hInstance = static_cast<HINSTANCE>(hInstance);
	winClass.hIcon = 0;
	winClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	winClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	winClass.lpszMenuName = 0;
	sprintf(tempstr, "MultiGUIWindow%08x", static_cast<HINSTANCE>(hInstance));
	winClass.lpszClassName = tempstr;
	winClass.hIconSm = NULL;

	//Register the window class (this is unregistered in the
	//MultiGUIEditor destructor).
	RegisterClassEx(&winClass);
#endif
}

//----------------------------------------------------------------------------
MultiGUIEditor::~MultiGUIEditor()
{
	int i;
	vector<AEffEditor *>::iterator it;
	vector<void *>::iterator it_syswin;

	i =0;
	it_syswin = _systemWindows.begin();

	for(it=_editors.begin();it!=_editors.end();it++)
	{
		delete *it; //deletes the actual editors

#ifdef WIN32
		char tempstr[32];
		//unregisters the window class
		sprintf(tempstr, "MGUIWinClass%d", i);
		UnregisterClass(tempstr, static_cast<HINSTANCE>(hInstance));
#endif

		i++;
		it_syswin++;
	}
	_editors.clear(); //removes the entries from _editors (is this necessary?)

#ifdef WIN32
	char tempstr[64];
	sprintf(tempstr, "MultiGUIWindow%08x", static_cast<HINSTANCE>(hInstance));
	//unregisters the window class
	UnregisterClass(tempstr, static_cast<HINSTANCE>(hInstance));
#endif
}

//----------------------------------------------------------------------------
long MultiGUIEditor::open(void *ptr)
{
	vector<AEffEditor *>::iterator it_ed;
	vector<NiallsRect>::iterator it_rect;

	systemWindow = ptr;

#ifdef WIN32
	int i;
	char tempstr[32];
	HWND tempHWnd2;
	HWND parentHWnd = static_cast<HWND>(ptr);

	//create the main window...  (This seems to be necessary for Tracktion...)
	sprintf(tempstr, "MultiGUIWindow%08x", static_cast<HINSTANCE>(hInstance));
	tempHWnd = CreateWindowEx(0,				   //extended window style
							  tempstr,	   //pointer to registered class name
							  "MultiGUIEditor",	   //pointer to window name
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
	//SetWindowLong(tempHWnd, GWL_USERDATA, (long)this);

	i = 0;
	it_ed = _editors.begin();

	//Loop through the editor panels, and create the actual windows.
	for(it_rect=_rects.begin();it_rect!=_rects.end();it_rect++)
	{
		sprintf(tempstr, "MGUIWinClass%d", i);
		tempHWnd2 = CreateWindowEx(0,				   //extended window style
								  tempstr,			   //pointer to registered class name
								  "MultiGUIEditor",	   //pointer to window name
								  WS_CHILD|WS_VISIBLE, //window style
								  it_rect->x,          //horizontal position of window
								  it_rect->y,          //vertical position of window
								  it_rect->width,      //window width
								  it_rect->height,     //window height
								  tempHWnd,		   //handle to parent or owner window
								  NULL,				   //handle to menu, or child-window identifier
								  (HINSTANCE)hInstance,//handle to application instance
								  NULL);			   //pointer to window-creation data

		//Now open the individual editor panels, with the newly-created hWnd.
		(*it_ed)->open(static_cast<void *>(tempHWnd2));

		it_ed++;
		i++;
	}
#elif MACX
	Rect temprect, temprect2;
	WindowGroupRef parentGroup;
	WindowRef tempwin;
	WindowRef window = static_cast<WindowRef>(systemWindow);

	temprect.left = _rect.left;
	temprect.right = _rect.right;
	temprect.top = _rect.top;
	temprect.bottom = _rect.bottom;

	parentGroup = GetWindowGroup(window);
	CreateWindowGroup(kWindowGroupAttrSelectAsLayer|
					  kWindowGroupAttrMoveTogether|
					  kWindowGroupAttrLayerTogether|
					  kWindowGroupAttrSharedActivation|
					  kWindowGroupAttrHideOnCollapse,
					  &mainGroup);
	//SetWindowGroupParent(parentGroup, mainGroup);
	SetWindowGroupParent(mainGroup, parentGroup);
	SetWindowGroup(window, mainGroup);

	//So we know where to move the child windows to.
	GetWindowBounds(window, kWindowContentRgn, &temprect2);

	it_ed = _editors.begin();
	//Loop through the editor panels, and create the actual windows.
	for(it_rect=_rects.begin();it_rect!=_rects.end();it_rect++)
	{
		temprect.left = temprect2.left + it_rect->x;
		temprect.top = temprect2.top + it_rect->y;
		temprect.right = temprect2.left + (it_rect->x + it_rect->width);
		temprect.bottom = temprect2.top + (it_rect->y + it_rect->height);
		CreateNewWindow(kOverlayWindowClass,
						kWindowStandardHandlerAttribute|
						kWindowCompositingAttribute|
						kWindowNoShadowAttribute/*|
						kWindowNoActivatesAttribute|	//Not sure if these lines are necessary...
						kWindowDoesNotCycleAttribute*/,
						&temprect,
						&tempwin);
		//Not sure if the following line is actually necessary...
		//SetWindowActivationScope(tempwin, kWindowActivationScopeAll);

		ShowWindow(tempwin);
		SetWindowGroup(tempwin, mainGroup);

		_systemWindows.push_back(static_cast<void *>(tempwin));
		(*it_ed)->open(static_cast<void *>(tempwin));
		it_ed++;
	}
#endif

	return true;
}

//----------------------------------------------------------------------------
void MultiGUIEditor::close()
{
	vector<AEffEditor *>::iterator it_ed;
	vector<void *>::iterator it_syswin;

	for(it_ed=_editors.begin();it_ed!=_editors.end();it_ed++)
	{
		//close the individual editor panels
		(*it_ed)->close();
	}

#ifdef WIN32
	for(it_syswin=_systemWindows.begin();it_syswin!=_systemWindows.end();it_syswin++)
	{
		//destroy the individual windows
		DestroyWindow(static_cast<HWND>(*it_syswin));
		*it_syswin = 0;
	}
#elif MACX
	for(it_syswin=_systemWindows.begin();it_syswin!=_systemWindows.end();it_syswin++)
	{
		DisposeWindow(static_cast<WindowRef>(*it_syswin));
	}
	_systemWindows.clear();
	ReleaseWindowGroup(mainGroup);
#endif
}

//----------------------------------------------------------------------------
void MultiGUIEditor::idle()
{
	vector<AEffEditor *>::iterator it_ed;

	for(it_ed=_editors.begin();it_ed!=_editors.end();it_ed++)
	{
		(*it_ed)->idle();
	}
}

/* (AEffEditor doesn't have a setParameter method, so you'll have to set this
	up yourself, in your MultiGUIEditor subclass...)
//----------------------------------------------------------------------------
void MultiGUIEditor::setParameter(long index, float value)
{
	vector<AEffEditor *>::iterator it_ed;

	for(it_ed=_editors.begin();it_ed!=_editors.end();it_ed++)
	{
		(*it_ed)->setParameter(index, value);
	}
}*/

//----------------------------------------------------------------------------
void MultiGUIEditor::addEditor(AEffEditor *editor, NiallsRect rect)
{
	vector<NiallsRect>::iterator it_rect;
	int width = 0;
	int height = 0;

	if(editor)
	{
		_editors.push_back(editor);
		_rects.push_back(rect);

#ifdef WIN32
		char tempstr[32];
		WNDCLASSEX winClass;

		//most of this stuff is copied from VSTGUI
		winClass.cbSize = sizeof(WNDCLASSEX);
		winClass.style = 0;
		winClass.lpfnWndProc = WndProc;
		winClass.cbClsExtra = 0;
		winClass.cbWndExtra = 0;
		winClass.hInstance = static_cast<HINSTANCE>(hInstance);
		winClass.hIcon = 0;
		winClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		winClass.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
		winClass.lpszMenuName = 0;
		sprintf(tempstr, "MGUIWinClass%d", (_editors.size()-1));
		winClass.lpszClassName = tempstr;
		winClass.hIconSm = NULL;

		//Register the window class (this is unregistered in the
		//MultiGUIEditor destructor).
		RegisterClassEx(&winClass);
#endif

		//First set the size of the window to be the accumulated size of the
		//individual editor panels.

		//Set the width & height of the main window.
		for(it_rect=_rects.begin();it_rect!=_rects.end();it_rect++)
		{
			if((it_rect->x+it_rect->width) > width)
				width = (it_rect->x+it_rect->width);
			if((it_rect->y+it_rect->height) > height)
				height = (it_rect->y+it_rect->height);
		}

		_rect.right = width;
		_rect.bottom = height;
	}
}

//----------------------------------------------------------------------------
#ifdef WIN32
LONG WINAPI WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//We shouldn't have to do anything here - it's up to the editor panels
	//to do stuff...
	return DefWindowProc (hwnd, message, wParam, lParam);
}
#endif
