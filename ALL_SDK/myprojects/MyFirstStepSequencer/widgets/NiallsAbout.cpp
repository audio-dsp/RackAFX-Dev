//	NiallsAbout.cpp - An about box that is only displayed as long as the mouse
//					  button is held down.
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

#include "vstgui.h"
#include "NiallsAbout.h"
#include "MFSSGui.h"

//----------------------------------------------------------------------------
NiallsAbout::NiallsAbout(const CRect& size, CBitmap *button, CBitmap *about):
CControl(size),
displayingAboutBox(false)
{
	if(button)
	{
		buttonImage = button;
		buttonImage->remember();
		if(about)
		{
			aboutImage = about;
			aboutImage->remember();
		}
	}
}

//----------------------------------------------------------------------------
NiallsAbout::~NiallsAbout()
{
	if(buttonImage)
		buttonImage->forget();
	if(aboutImage)
		aboutImage->forget();
}

//----------------------------------------------------------------------------
void NiallsAbout::draw(CDrawContext *context)
{
	CRect tempRect;

	if(buttonImage)
	{
		if(bTransparencyEnabled)
			buttonImage->drawTransparent(context, size);
		else
			buttonImage->draw(context, size);
	}

	if(aboutImage && displayingAboutBox)
	{
		tempRect.x = 0;
		tempRect.y = 0;
		tempRect.right = aboutImage->getWidth();
		tempRect.bottom = aboutImage->getHeight();

		//So we actually draw over the whole frame, not just the button.
		context->setClipRect(tempRect);

		if(bTransparencyEnabled)
			aboutImage->drawTransparent(context, tempRect);
		else
			aboutImage->draw(context, tempRect);
	}
}

//----------------------------------------------------------------------------
void NiallsAbout::mouse(CDrawContext *context, CPoint& position, long buttons)
{
	MFSSGui *editor;
	bool firstGo = true;

	if(buttons == -1)
		buttons = context->getMouseButtons();

	if((buttons & kLButton)||(buttons & kRButton))
	{
		displayingAboutBox = true;
		setDirty(true);

		beginEdit();
		//Oh dear god!  What have I done!!!!  I've gone over to the dark
		//side!!!!!!!!
		while(1)
		{
			buttons = context->getMouseButtons();
			if((!(buttons & kLButton)) && (!(buttons & kRButton)))
				break;

			doIdleStuff();

			//This is because if we have CViewContainers in the frame, they
			//tend to get drawn in front of the about box the first time
			//through, so this fixes it so that the user never sees that.
			if(firstGo)
			{
				editor = reinterpret_cast<MFSSGui *>(getFrame()->getEditor());
				editor->redrawEverything();
				getFrame()->redraw();
				redraw();
				firstGo = false;
				setDirty(true);
			}
			else
				setDirty(false);
		}
		endEdit();

		displayingAboutBox = false;
		setDirty(true);
		context->getFrame()->setDirty(true);
	}
}
