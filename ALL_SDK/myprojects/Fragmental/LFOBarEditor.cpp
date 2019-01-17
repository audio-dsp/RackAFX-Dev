//	LFOBarEditor.cpp - LFO waveshape editor made up of vertical bars.
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

#include "LFOBarEditor.h"

//-----------------------------------------------------------------------------
LFOBarEditor::LFOBarEditor(const CRect& size,
						   CControlListener *listener,
						   long tag,
						   CBitmap *background,
						   const CPoint& offset):
CControl(size, listener, tag, background),
tagStart(tag),
activeBars(32)
{
	int i;

	barColour(255, 255, 255, 255);
	frameColour(127, 127, 127, 255);

	for(i=0;i<32;++i)
		//values[i] = (float)i/32.0f;
		values[i] = 0.5f;
}

//-----------------------------------------------------------------------------
LFOBarEditor::~LFOBarEditor()
{
	
}

//-----------------------------------------------------------------------------
void LFOBarEditor::draw(CDrawContext *context)
{
	int i;
	int j = 0;
	CRect tempRect;
	const int width = size.getWidth();
	const int height = size.getHeight();
	const int barWidth = width/activeBars;

	context->setFillColor(barColour);
	context->setFrameColor(frameColour);

	tempRect.x = size.x;
	for(i=0;i<activeBars;++i)
	{
		float tempf = 1.0f-values[i];

		if(tempf > 0.5f)
		{
			tempRect.y = size.y + (height/2);
			tempRect.bottom = tempRect.y + (int)((tempf-0.5f)*2.0f * (float)(height/2));
		}
		else
		{
			tempRect.y = size.y + (int)(tempf*2.0f * (float)(height/2));
			tempRect.bottom = size.y + (height/2);
		}

		if(i == (activeBars-1))
			tempRect.right = size.right;
		else
			tempRect.right = tempRect.x + barWidth + 1;

		if(tempf != 0.5f)
		{
			context->fillRect(tempRect);
			context->drawRect(tempRect);
		}
		else
		{
			context->moveTo(CPoint(tempRect.x, (size.y + (height/2))));
			context->lineTo(CPoint(tempRect.right, (size.y + (height/2))));
		}

		tempRect.x += barWidth;
	}
}

//-----------------------------------------------------------------------------
void LFOBarEditor::mouse(CDrawContext *context, CPoint &mousePos, long buttons)
{
	int i;
	int tempX;
	float tempf;
	CRect bars[32];
	const int width = size.getWidth();
	const int height = size.getHeight();
	const int barWidth = width/activeBars;

	tempX = size.x;
	for(i=0;i<activeBars;++i)
	{
		bars[i].x = tempX;
		bars[i].y = size.y;

		if(i != (activeBars-1))
			bars[i].right = tempX + barWidth;
		else
			bars[i].right = size.right;

		bars[i].bottom = size.bottom;

		tempX += barWidth;
	}

	beginEdit();
	while(1)
	{
		//Check if it's time to quit the loop.
		buttons = context->getMouseButtons();
		if(!(buttons & kLButton))
			break;

		//Get the current mouse position.
		getMouseLocation(context, mousePos);

		//Determine which bar we're in.
		tempX = (int)(((float)(mousePos.x-size.x)/(float)size.getWidth()) * (float)activeBars);
		if(tempX >= activeBars)
			tempX = activeBars-1;
		else if(tempX < 0)
			tempX = 0;

		//Set the appropriate value.
		tempf = (float)(mousePos.y-size.y);
		tempf /= (float)(size.getHeight());
		tempf = 1.0f-tempf;
		if(tempf > 1.0f)
			tempf = 1.0f;
		else if(tempf < 0.0f)
			tempf = 0.0f;

		setTag(tagStart+tempX);
		setValue(tempX, tempf);
		setDirty();

		if(listener)
			listener->valueChanged(context, this);

		doIdleStuff();
	}
	endEdit();
}

//-----------------------------------------------------------------------------
void LFOBarEditor::setValue(long index, float val)
{
	values[index] = val;
}

//-----------------------------------------------------------------------------
void LFOBarEditor::setValue(float *indices, CDrawContext *context)
{
	int i;

	for(i=0;i<activeBars;++i)
	{
		values[i] = indices[i];
		setTag(tagStart+i);
		if(listener)
			listener->valueChanged(context, this);
	}

	setDirty();
}

//-----------------------------------------------------------------------------
float LFOBarEditor::getValue() const
{
	return values[getTag()-tagStart];
}

//-----------------------------------------------------------------------------
void LFOBarEditor::setNumBars(int val)
{
	activeBars = val;
}
