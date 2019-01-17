//	ModWheel.cpp - VSTGUI 'mod wheel' control.
//	--------------------------------------------------------------------------
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
//	--------------------------------------------------------------------------

#include "ModWheel.h"

#include <cmath>

//----------------------------------------------------------------------------
ModWheel::ModWheel(CRect& size,
				   CControlListener *listener,
				   int tag1,
				   int tag2,
				   int tag3,
				   int tagX,
				   int tagY,
				   CBitmap *tickImage):
CControl(size, listener, tag1),
tickRadius(5),
circleRadius(20),
value1(0.0f),
value2(0.0f),
value3(0.0f),
xVal(0.0f),
yVal(0.0f),
debugLines(false)
{
	this->tag1 = tag1;
	this->tag2 = tag2;
	this->tag3 = tag3;
	this->tagX = tagX;
	this->tagY = tagY;

	this->tickImage = tickImage;

	if(tickImage)
	{
		tickImage->remember();

		tickRadius = static_cast<float>(tickImage->getWidth())*0.5f;
		--tickRadius;

		tickPosition.x = static_cast<int>(tickRadius)+1;
		tickPosition.y = static_cast<int>(tickRadius)+1;
	}

	//Leave 5px gap around main circle.
	circleRadius = static_cast<float>(size.getWidth()-10)*0.5f;
}

//----------------------------------------------------------------------------
ModWheel::~ModWheel()
{
	if(tickImage)
		tickImage->forget();
}

//----------------------------------------------------------------------------
void ModWheel::draw(CDrawContext *context)
{
	int tickRadiusReal = static_cast<int>(tickRadius+1.0f);

	if(tickImage)
	{
		CRect tempRect(size.x+(tickPosition.x-tickRadiusReal),
					   size.y+(tickPosition.y-tickRadiusReal),
					   size.x+(tickPosition.x+tickRadiusReal),
					   size.y+(tickPosition.y+tickRadiusReal));

		tickImage->drawTransparent(context, tempRect);
	}
	else //Just in case...
	{
		context->setFillColor(kBlackCColor);
		context->fillRect(CRect(size.x+(tickPosition.x-tickRadiusReal),
					   size.y+(tickPosition.y-tickRadiusReal),
					   size.x+(tickPosition.x+tickRadiusReal),
					   size.y+(tickPosition.y+tickRadiusReal)));
	}

	if(debugLines)
	{
		context->setFillColor(kBlackCColor);
		context->fillRect(CRect(size.x,
								size.bottom,
								(size.x+2),
								size.bottom-(int)(value1*42.0f)));
		context->fillRect(CRect(size.x+2,
								size.bottom,
								(size.x+4),
								size.bottom-(int)(value2*42.0f)));
		context->fillRect(CRect(size.x+4,
								size.bottom,
								(size.x+6),
								size.bottom-(int)(value3*42.0f)));
	}
}

//----------------------------------------------------------------------------
void ModWheel::mouse(CDrawContext *context, CPoint &where, long button)
{
	int tickRadiusReal = static_cast<int>(tickRadius+1.0f);

	beginEdit();
	while(button & kLButton)
	{
		tickPosition.x = where.x - size.x;
		tickPosition.y = where.y - size.y;
		setDirty();

		if(tickPosition.x < tickRadiusReal)
			tickPosition.x = tickRadiusReal;
		else if(tickPosition.x > (size.getWidth()-tickRadiusReal))
			tickPosition.x = (size.getWidth()-tickRadiusReal);

		if(tickPosition.y < tickRadiusReal)
			tickPosition.y = tickRadiusReal;
		else if(tickPosition.y > (size.getHeight()-tickRadiusReal))
			tickPosition.y = (size.getHeight()-tickRadiusReal);

		xVal = (float)tickPosition.x/(float)(size.getWidth()-(2*tickRadiusReal));
		yVal = (float)tickPosition.y/(float)(size.getHeight()-(2*tickRadiusReal));

		updateValues();
		if(listener)
			listener->valueChanged(context, this);

		doIdleStuff();
		getMouseLocation(context, where);
		button = context->getMouseButtons();
	}
	endEdit();
}

//----------------------------------------------------------------------------
void ModWheel::setValue1(float val)
{
	value1 = val;
	//updateTick();
}

//----------------------------------------------------------------------------
void ModWheel::setValue2(float val)
{
	value2 = val;
	//updateTick();
}

//----------------------------------------------------------------------------
void ModWheel::setValue3(float val)
{
	value3 = val;
	//updateTick();
}

//----------------------------------------------------------------------------
void ModWheel::setValueX(float val)
{
	int tickRadiusReal = static_cast<int>(tickRadius+1.0f);

	xVal = val;

	tickPosition.x = (int)(xVal * (float)(size.getWidth()-(2*tickRadiusReal)));
	setDirty();
}

//----------------------------------------------------------------------------
void ModWheel::setValueY(float val)
{
	int tickRadiusReal = static_cast<int>(tickRadius+1.0f);

	yVal = val;

	tickPosition.y = (int)(yVal * (float)(size.getHeight()-(2*tickRadiusReal)));
	setDirty();
}

//----------------------------------------------------------------------------
void ModWheel::updateValues()
{
	int tempX, tempY;
	float actualRadius;
	float radValue;
	float midRad = 0.0f;
	float tempValue1 = 0.0f;
	float tempValue2 = 0.0f;
	float tempValue3 = 0.0f;
	float lowerTopY = getLowerTop();
	float leftAmount, rightAmount, lowerAmount;
	const CPoint middle((size.right-size.left)/2, (size.bottom-size.top)/2);

	//First calculate how much the tick is inside the main circle.
	tempX = (tickPosition.x-middle.x);
	tempY = (tickPosition.y-middle.y);
	actualRadius = sqrtf((float)(tempX*tempX) + (float)(tempY*tempY));

	if((actualRadius+tickRadius) <= circleRadius)
		radValue = 1.0f;
	else if((actualRadius-tickRadius) > circleRadius)
		radValue = 0.0f;
	else
		radValue = (circleRadius-(actualRadius-tickRadius))/(tickRadius*2.0f);


	//Now determine how much the tick is inside each of the 3 sections.

	//If it's in the middle of the x axis.
	if(tickPosition.x == middle.x)
	{
		float tempf = ((float)tickPosition.y-tickRadius);
		//Is this right?
		lowerAmount = ((((float)tickPosition.y+tickRadius) - lowerTopY)/
					   (tickRadius*2.0f));
		tempValue3 = (lowerAmount * 0.33f) * radValue * 2.0f;

		tempValue1 = (1.0f-lowerAmount) * 0.33f * radValue * 2.0f;
		tempValue2 = (1.0f-lowerAmount) * 0.33f * radValue * 2.0f;

		/*if((tempf > (lowerTopY-tickRadius)) &&
		   (tempf < (lowerTopY+tickRadius)))*/
		if((tempf > (21.0f-6.0f)) &&
		   (tempf < (21.0f+6.0f)))
		{
			/*tempf -= (lowerTopY-tickRadius);
			tempf /= tickRadius;

			if(tempf > 1.0f)
			{
				--tempf;
				tempf = 1.0f-tempf;
			}*/

			//midRad = tempf;
			midRad = 1.0f;
		}
	}

	//If it's entirely inside the lower triangle.
	if(((float)tickPosition.y-tickRadius) > lowerTopY)
	{
		value1 = 0.0f;
		value2 = 0.0f;
		value3 = radValue;
	}
	//If it's partly in the lower triangle.
	else if(((float)tickPosition.y+tickRadius) > lowerTopY)
	{
		//Is this right?
		lowerAmount = ((((float)tickPosition.y+tickRadius) - lowerTopY)/
					   (tickRadius*2.0f));
		value3 = ((lowerAmount * radValue) * (1.0f-midRad)) +
				 (tempValue3 * midRad);

		//If it's not in the right triangle.
		if(((float)tickPosition.x+tickRadius) < 21.0f)
		{
			value1 = (((1.0f-lowerAmount) * radValue) * (1.0f-midRad)) +
					 (tempValue1 * midRad);
			value2 = 0.0f;
		}
		//If it's not in the left triangle.
		else if(((float)tickPosition.x-tickRadius) >= 21.0f)
		{
			value1 = 0.0f;
			value2 = (((1.0f-lowerAmount) * radValue) * (1.0f-midRad)) +
					 (tempValue2 * midRad);
		}
		//If it's partly in all 3 triangles.
		else
		{
			leftAmount = (21.0f-((float)tickPosition.x-tickRadius))/(tickRadius*2.0f);
			rightAmount = 1.0f-leftAmount;

			value1 = (((leftAmount * (1.0f-lowerAmount)) * radValue) * (1.0f-midRad)) +
					 (tempValue1 * midRad);
			value2 = (((rightAmount * (1.0f-lowerAmount)) * radValue) * (1.0f-midRad)) +
					 (tempValue2 * midRad);
		}
	}
	//It's not in the lower triangle at all.
	else
	{
		value3 = 0.0f;

		//If it's not in the right triangle.
		if(((float)tickPosition.x+tickRadius) < 21.0f)
		{
			value1 = radValue;
			value2 = 0.0f;
		}
		//If it's not in the left triangle.
		else if(((float)tickPosition.x-tickRadius) >= 21.0f)
		{
			value1 = 0.0f;
			value2 = radValue;
		}
		//If it's partly in both top triangles.
		else
		{
			leftAmount = (21.0f-((float)tickPosition.x-tickRadius))/(tickRadius*2.0f);
			rightAmount = 1.0f-leftAmount;

			value1 = ((leftAmount * radValue) * (1.0f-midRad)) +
					 (tempValue1*midRad);
			value2 = ((rightAmount * radValue) * (1.0f-midRad)) +
					 (tempValue2*midRad);
		}
	}
}

//----------------------------------------------------------------------------
void ModWheel::updateTick()
{
	if((value1 == 0.0f) && (value2 == 0.0f) && (value3 == 0.0f))
	{
		tickPosition.x = 0;
		tickPosition.y = 0;
	}
	

	setDirty();
}
