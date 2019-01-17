//	StateButton.cpp - A button with states, which change with left and right
//					  clicks.
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
#include "StateButton.h"
#include "Round.h"

using namespace std;

//----------------------------------------------------------------------------
StateButton::StateButton(const CRect& size,
						 CBitmap *image,
						 long numImages,
						 CControlListener *listener,
						 long tag,
						 CBitmap *backgroundImage):
CControl(size, listener, tag, backgroundImage),
currentState(0),
hasDisabledState(false),
isDisabled(false)
{
	if(image)
	{
		image->remember();
		this->image = image;
		numStackedImages = numImages;
		sizeOfOneImage = image->getHeight()/numStackedImages;
		ignoredStates.assign(numStackedImages, false);
	}
	else
	{
		this->image = 0;
		numStackedImages = -1;
	}
}

//----------------------------------------------------------------------------
StateButton::~StateButton()
{
	if(image)
		image->forget();
}

//----------------------------------------------------------------------------
void StateButton::draw(CDrawContext *context)
{
	//First, draw background, if it exists.
	if(pBackground)
	{
		if(bTransparencyEnabled)
			pBackground->drawTransparent(context, size, backOffset);
		else
			pBackground->draw(context, size, backOffset);
	}

	//Just in case...
	if(image)
	{
		//Now draw the actual widget.
		if(isDisabled)
		{
			if(bTransparencyEnabled)
				image->drawTransparent(context,
									   size,
									   CPoint(0,
											  ((numStackedImages-1)*sizeOfOneImage)));
			else
				image->draw(context,
							size,
							CPoint(0, ((numStackedImages-1)*sizeOfOneImage)));
		}
		else if(currentState < numStackedImages)
		{
			if(bTransparencyEnabled)
				image->drawTransparent(context,
									   size,
									   CPoint(0,
											  (currentState*sizeOfOneImage)));
			else
				image->draw(context,
							size,
							CPoint(0, (currentState*sizeOfOneImage)));
		}
		else //Just in case...
		{
			if(bTransparencyEnabled)
				image->drawTransparent(context, size); //No offset = image 0.
			else
				image->draw(context, size); //No offset = image 0.
		}
	}

	setDirty(false);
}

//----------------------------------------------------------------------------
void StateButton::mouse(CDrawContext *context, CPoint &position, long buttons)
{
	if(buttons == -1)
		buttons = context->getMouseButtons();

	if(hasDisabledState && isDisabled)
		return;

	if(buttons & kLButton)
	{
		setState(currentState + 1);

		if(hasDisabledState)
			setValue(static_cast<float>(currentState)/static_cast<float>(numStackedImages-2));
		else
			setValue(static_cast<float>(currentState)/static_cast<float>(numStackedImages-1));
		setDirty(true);

		if(listener)
			listener->valueChanged(context, this);
	}
	else if(buttons & kRButton)
	{
		setState(currentState - 1);

		if(hasDisabledState)
			setValue(static_cast<float>(currentState)/static_cast<float>(numStackedImages-2));
		else
			setValue(static_cast<float>(currentState)/static_cast<float>(numStackedImages-1));
		setDirty(true);

		if(listener)
			listener->valueChanged(context, this);
	}
}

//----------------------------------------------------------------------------
bool StateButton::onWheel(CDrawContext *context,
						  const CPoint &where,
						  float distance)
{
	float tempf = getValue();

	if(hasDisabledState)
		tempf += (distance * (1.0f/static_cast<float>(numStackedImages-1)));
	else
		tempf += (distance * (1.0f/static_cast<float>(numStackedImages)));

	if(tempf < 0.0f)
		tempf = 1.0f - fmod(tempf, 1.0f); //?
	else
		tempf = fmod(tempf, 1.0f);

	setValue(tempf);
	if(listener)
		listener->valueChanged(context, this);

	return true; //?
}

//----------------------------------------------------------------------------
void StateButton::setValue(float val)
{
	float tempf;
	value = val;

	if(hasDisabledState)
	{
		tempf = value * static_cast<float>(numStackedImages-2);
		currentState = static_cast<long>(round(tempf));
	}
	else
		currentState = static_cast<long>(round(value * static_cast<float>(numStackedImages-1)));

	//if(!hasDisabledState || (hasDisabledState && !isDisabled)) 
		setDirty();
}

//----------------------------------------------------------------------------
void StateButton::setStatesIgnored(long states)
{
	int i;
	int numIgnored = 0;

	for(i=0;i<numStackedImages;++i)
	{
		//Because states can only hold 32 values at most.
		if(i >= 32)
			break;

		//Need to make sure we always leave one state un-ignored.
		if(numIgnored < (numStackedImages-1))
		{
			//Use the i'th bit to determine whether this state should be ignored
			//or not.
			//(!= 0) to kill the warning about forcing an int to bool...
			ignoredStates[i] = ((states & (1<<i))>>i) != 0;
			if(ignoredStates[i])
				++numIgnored;
		}
	}

	//Update the current state if we have to.
	if(ignoredStates[currentState])
	{
		setState(currentState);
		setDirty(true);
	}
}

//----------------------------------------------------------------------------
void StateButton::setStateIgnored(long state, bool isIgnored)
{
	long i;
	int numIgnored = 0;

	for(i=0;i<ignoredStates.size();++i)
		++numIgnored;

	if(numIgnored < (numStackedImages-1))
		ignoredStates[state] = isIgnored;

	//Update the current state if we have to.
	if(ignoredStates[currentState])
	{
		setState(currentState);
		setDirty(true);
	}
}

//----------------------------------------------------------------------------
void StateButton::setHasDisabledState(bool val)
{
	hasDisabledState = val;
	//Because the effective number of images changes when this happens.
	setValue(getValue());
}

//----------------------------------------------------------------------------
void StateButton::setDisabled(bool val)
{
	if(hasDisabledState)
	{
		isDisabled = val;
		setDirty(true);
	}
}

//----------------------------------------------------------------------------
void StateButton::setState(long val)
{
	bool goingForward = false;

	if(!hasDisabledState)
	{
		if(val == numStackedImages) //Loop back to 0.
		{
			val = 0;
			goingForward = true;
		}
		else if(val < 0)
			val = numStackedImages-1;
	}
	else
	{
		if(val == (numStackedImages-1))
		{
			val = 0;
			goingForward = true;
		}
		else if(val < 0)
			val = numStackedImages-2;
	}

	if((val >= currentState)||(goingForward))
	{
		//Keep going until we find a state which should not be ignored.
		while(ignoredStates[val])
		{
			++val;
			if(!hasDisabledState)
			{
				if(val == numStackedImages)
					val = 0;
			}
			else
			{
				if(val == (numStackedImages-1))
					val = 0;
			}
		}
	}
	else
	{
		//Keep going until we find a state which should not be ignored.
		while(ignoredStates[val])
		{
			--val;
			if(hasDisabledState)
			{
				if(val == -1)
					val = (numStackedImages-2);
			}
			else
			{
				if(val == -1)
					val = (numStackedImages-1);
			}
		}
	}

	currentState = val;
}

//----------------------------------------------------------------------------
void StateButton::setImage(CBitmap *newImage)
{
	if(image)
		image->forget();
	image = newImage;
	if(image)
		image->remember();

	setDirty(true);
}
