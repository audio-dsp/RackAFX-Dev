//	SixPointLineEditor.cpp - Simple line editor widget.
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

#include "SixPointLineEditor.h"

//-----------------------------------------------------------------------------
SixPointLineEditor::SixPointLineEditor(const CRect& size,
									   CControlListener *listener,
									   long tag,
									   CBitmap *background,
									   const CPoint& offset):
CControl(size, listener, tagStart, background),
tagStart(tag),
backgroundOffset(offset),
preset(0)
{
	int i, j;

	colour(127, 127, 127, 255);

	values[p0y] = 0.0f;
	values[p5y] = 1.0f;
	for(i=1,j=1;i<5;++i)
	{
		//x
		values[j] = (float)i/5.0f;
		++j;
		//y
		values[j] = (float)i/5.0f;
		++j;
	}
}

//-----------------------------------------------------------------------------
SixPointLineEditor::~SixPointLineEditor()
{
	
}

//-----------------------------------------------------------------------------
void SixPointLineEditor::draw(CDrawContext *context)
{
	CRect tempRect;
	CPoint p0(size.x,
			  size.y + (int)((1.0f-values[p0y]) * (float)size.getHeight()));
	CPoint p1(size.x + (int)(values[p1x] * (float)size.getWidth()),
			  size.y + (int)((1.0f-values[p1y]) * (float)size.getHeight()));
	CPoint p2(size.x + (int)(values[p2x] * (float)size.getWidth()),
			  size.y + (int)((1.0f-values[p2y]) * (float)size.getHeight()));
	CPoint p3(size.x + (int)(values[p3x] * (float)size.getWidth()),
			  size.y + (int)((1.0f-values[p3y]) * (float)size.getHeight()));
	CPoint p4(size.x + (int)(values[p4x] * (float)size.getWidth()),
			  size.y + (int)((1.0f-values[p4y]) * (float)size.getHeight()));
	CPoint p5(size.right,
			  size.y + (int)((1.0f-values[p5y]) * (float)size.getHeight()));

	//First draw background.
	if(pBackground)
	{
		if(bTransparencyEnabled)
			pBackground->drawTransparent(context, size, backgroundOffset);
		else
			pBackground->draw(context, size, backgroundOffset);
	}

	//Now draw lines.
	context->setFrameColor(colour);
	context->moveTo(p0);
	context->lineTo(p1);
	context->lineTo(p2);
	context->lineTo(p3);
	context->lineTo(p4);
	context->lineTo(p5);

	//Now draw point handles.
	tempRect.x = p0.x - 2;
	tempRect.y = p0.y - 2;
	tempRect.right = tempRect.x + 5;
	tempRect.bottom = tempRect.y + 5;
	context->drawRect(tempRect);

	tempRect.x = p1.x - 2;
	tempRect.y = p1.y - 2;
	tempRect.right = tempRect.x + 5;
	tempRect.bottom = tempRect.y + 5;
	context->drawRect(tempRect);

	tempRect.x = p2.x - 2;
	tempRect.y = p2.y - 2;
	tempRect.right = tempRect.x + 5;
	tempRect.bottom = tempRect.y + 5;
	context->drawRect(tempRect);

	tempRect.x = p3.x - 2;
	tempRect.y = p3.y - 2;
	tempRect.right = tempRect.x + 5;
	tempRect.bottom = tempRect.y + 5;
	context->drawRect(tempRect);

	tempRect.x = p4.x - 2;
	tempRect.y = p4.y - 2;
	tempRect.right = tempRect.x + 5;
	tempRect.bottom = tempRect.y + 5;
	context->drawRect(tempRect);

	tempRect.x = p5.x - 2;
	tempRect.y = p5.y - 2;
	tempRect.right = tempRect.x + 5;
	tempRect.bottom = tempRect.y + 5;
	context->drawRect(tempRect);

	setDirty(false);
}

//-----------------------------------------------------------------------------
void SixPointLineEditor::mouse(CDrawContext *context,
							   CPoint &mousePos,
							   long buttons)
{
	int i;
	float tempf;
	int selectedPoint;
	CRect tempRect;
	CPoint p0(size.x,
			  size.y + (int)((1.0f-values[p0y]) * (float)size.getHeight()));
	CPoint p1(size.x + (int)(values[p1x] * (float)size.getWidth()),
			  size.y + (int)((1.0f-values[p1y]) * (float)size.getHeight()));
	CPoint p2(size.x + (int)(values[p2x] * (float)size.getWidth()),
			  size.y + (int)((1.0f-values[p2y]) * (float)size.getHeight()));
	CPoint p3(size.x + (int)(values[p3x] * (float)size.getWidth()),
			  size.y + (int)((1.0f-values[p3y]) * (float)size.getHeight()));
	CPoint p4(size.x + (int)(values[p4x] * (float)size.getWidth()),
			  size.y + (int)((1.0f-values[p4y]) * (float)size.getHeight()));
	CPoint p5(size.right,
			  size.y + (int)((1.0f-values[p5y]) * (float)size.getHeight()));

	if(buttons == -1)
		buttons = context->getMouseButtons();
	if((!(buttons & kLButton)) && (!(buttons & kRButton)))
		return;
	else if(buttons & kRButton)
	{
		++preset;
		if(preset > 3)
			preset = 0;

		//Set a preset shape.
		switch(preset)
		{
			//Linear.
			case 0:
				setValue(p0y, 0.0f);
				setValue(p1x, 0.2f);
				setValue(p1y, 0.2f);
				setValue(p2x, 0.4f);
				setValue(p2y, 0.4f);
				setValue(p3x, 0.6f);
				setValue(p3y, 0.6f);
				setValue(p4x, 0.8f);
				setValue(p4y, 0.8f);
				setValue(p5y, 1.0f);
				break;
			//Reverse linear.
			case 1:
				setValue(p0y, 1.0f);
				setValue(p1x, 0.2f);
				setValue(p1y, 0.8f);
				setValue(p2x, 0.4f);
				setValue(p2y, 0.6f);
				setValue(p3x, 0.6f);
				setValue(p3y, 0.4f);
				setValue(p4x, 0.8f);
				setValue(p4y, 0.2f);
				setValue(p5y, 0.0f);
				break;
			//Switch.
			case 2:
				setValue(p0y, 0.0f);
				setValue(p1x, 0.2f);
				setValue(p1y, 0.0f);
				setValue(p2x, 0.5f);
				setValue(p2y, 0.0f);
				setValue(p3x, 0.5f);
				setValue(p3y, 1.0f);
				setValue(p4x, 0.8f);
				setValue(p4y, 1.0f);
				setValue(p5y, 1.0f);
				break;
			//Reverse switch.
			case 3:
				setValue(p0y, 1.0f);
				setValue(p1x, 0.2f);
				setValue(p1y, 1.0f);
				setValue(p2x, 0.5f);
				setValue(p2y, 1.0f);
				setValue(p3x, 0.5f);
				setValue(p3y, 0.0f);
				setValue(p4x, 0.8f);
				setValue(p4y, 0.0f);
				setValue(p5y, 0.0f);
				break;
		}
		setDirty();
		if(listener)
		{
			for(i=0;i<=p5y;++i)
			{
				setTag(tagStart+i);
				listener->valueChanged(context, this);
			}
		}

		return;
	}

	//Check which point the user's clicked on, if any.
	tempRect.x = mousePos.x - 2;
	tempRect.y = mousePos.y - 2;
	tempRect.right = tempRect.x + 5;
	tempRect.bottom = tempRect.y + 5;

	if(tempRect.pointInside(p0))
	{
		setTag(tagStart);
		selectedPoint = 0;
	}
	else if(tempRect.pointInside(p1))
	{
		setTag(tagStart+p1x);
		selectedPoint = 1;
	}
	else if(tempRect.pointInside(p2))
	{
		setTag(tagStart+p2x);
		selectedPoint = 2;
	}
	else if(tempRect.pointInside(p3))
	{
		setTag(tagStart+p3x);
		selectedPoint = 3;
	}
	else if(tempRect.pointInside(p4))
	{
		setTag(tagStart+p4x);
		selectedPoint = 4;
	}
	else if(tempRect.pointInside(p5))
	{
		setTag(tagStart+p5y);
		selectedPoint = 5;
	}
	else
		return;

	preset = -1;

	beginEdit();
	while(1)
	{
		//Check if it's time to quit the loop.
		buttons = context->getMouseButtons();
		if(!(buttons & kLButton))
			break;

		//Get the current mouse position.
		getMouseLocation(context, mousePos);

		switch(selectedPoint)
		{
			case 0:
				tempf = (float)(mousePos.y - size.y)/(float)size.getHeight();
				tempf = 1.0f-tempf;

				if(tempf < 0.0f)
					tempf = 0.0f;
				else if(tempf > 1.0f)
					tempf = 1.0f;

				setValue(p0y, tempf);
				setDirty();

				if(listener)
					listener->valueChanged(context, this);
				break;
			case 1:
				tempf = (float)(mousePos.x - size.x)/(float)size.getWidth();
				tempf = tempf;

				if(tempf < 0.0f)
					tempf = 0.0f;
				else if(tempf > values[p2x])
					tempf = values[p2x];

				setValue(p1x, tempf);
				tempf = (float)(mousePos.y - size.y)/(float)size.getHeight();
				tempf = 1.0f-tempf;

				if(tempf < 0.0f)
					tempf = 0.0f;
				else if(tempf > 1.0f)
					tempf = 1.0f;

				setValue(p1y, tempf);
				setDirty();

				if(listener)
				{
					setTag(tagStart+p1x);
					listener->valueChanged(context, this);
					setTag(tagStart+p1y);
					listener->valueChanged(context, this);
				}
				break;
			case 2:
				tempf = (float)(mousePos.x - size.x)/(float)size.getWidth();
				tempf = tempf;

				if(tempf < values[p1x])
					tempf = values[p1x];
				else if(tempf > values[p3x])
					tempf = values[p3x];

				setValue(p2x, tempf);
				tempf = (float)(mousePos.y - size.y)/(float)size.getHeight();
				tempf = 1.0f-tempf;

				if(tempf < 0.0f)
					tempf = 0.0f;
				else if(tempf > 1.0f)
					tempf = 1.0f;

				setValue(p2y, tempf);
				setDirty();

				if(listener)
				{
					setTag(tagStart+p2x);
					listener->valueChanged(context, this);
					setTag(tagStart+p2y);
					listener->valueChanged(context, this);
				}
				break;
			case 3:
				tempf = (float)(mousePos.x - size.x)/(float)size.getWidth();
				tempf = tempf;

				if(tempf < values[p2x])
					tempf = values[p2x];
				else if(tempf > values[p4x])
					tempf = values[p4x];

				setValue(p3x, tempf);
				tempf = (float)(mousePos.y - size.y)/(float)size.getHeight();
				tempf = 1.0f-tempf;

				if(tempf < 0.0f)
					tempf = 0.0f;
				else if(tempf > 1.0f)
					tempf = 1.0f;

				setValue(p3y, tempf);
				setDirty();

				if(listener)
				{
					setTag(tagStart+p3x);
					listener->valueChanged(context, this);
					setTag(tagStart+p3y);
					listener->valueChanged(context, this);
				}
				break;
			case 4:
				tempf = (float)(mousePos.x - size.x)/(float)size.getWidth();
				tempf = tempf;

				if(tempf < values[p3x])
					tempf = values[p3x];
				else if(tempf > 1.0f)
					tempf = 1.0f;

				setValue(p4x, tempf);
				tempf = (float)(mousePos.y - size.y)/(float)size.getHeight();
				tempf = 1.0f-tempf;

				if(tempf < 0.0f)
					tempf = 0.0f;
				else if(tempf > 1.0f)
					tempf = 1.0f;

				setValue(p4y, tempf);
				setDirty();

				if(listener)
				{
					setTag(tagStart+p4x);
					listener->valueChanged(context, this);
					setTag(tagStart+p4y);
					listener->valueChanged(context, this);
				}
				break;
			case 5:
				tempf = (float)(mousePos.y - size.y)/(float)size.getHeight();
				tempf = 1.0f-tempf;

				if(tempf < 0.0f)
					tempf = 0.0f;
				else if(tempf > 1.0f)
					tempf = 1.0f;

				setValue(p5y, tempf);
				setDirty();
				break;
		}

		/*if(listener)
			listener->valueChanged(context, this);*/

		doIdleStuff();
	}
	endEdit();
}

//-----------------------------------------------------------------------------
void SixPointLineEditor::setValue(long index, float val)
{
	values[index] = val;
}

//-----------------------------------------------------------------------------
float SixPointLineEditor::getValue() const
{
	return values[getTag()-tagStart];
}
