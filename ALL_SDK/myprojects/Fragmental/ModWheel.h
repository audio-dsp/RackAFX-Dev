//	ModWheel.h - VSTGUI 'mod wheel' control.
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

#ifndef MODWHEEL_H_
#define MODWHEEL_H_

#include "vstcontrols.h"
#include "HelperStuff.h"

///	VSTGUI 'mod wheel' control.
/*!
	Essentially this is a circle, divided pie-chart style into 3 sections.
	The user controls a smaller circle which can move freely inside and
	without the main circle.  The control has 3 parameters corresponding
	to the 3 sections in the main circle.  The current value of those
	parameters is determined by how much the smaller circle covers each of
	those sections.

	I'm afraid it's not really coded to be a general-purpose control - there's
	quite a few hard-coded values in here...
 */
class ModWheel : public CControl
{
  public:
	///	Constructor.
	/*!
		\param size The bounds of the control.
		\param listener The CControlListener to call when our value changes.
		\param tag1 Tag for the first parameter.
		\param tag2 Tag for the second parameter.
		\param tag3 Tag for the third parameter.
		\param tickImage Image of the 'tick' that the use drags around.
	 */
	ModWheel(CRect& size,
			 CControlListener *listener,
			 int tag1,
			 int tag2,
			 int tag3,
			 int tagX,
			 int tagY,
			 CBitmap *tickImage);
	///	Destructor.
	~ModWheel();

	///	Draws the control.
	void draw(CDrawContext *context);

	///	Handles mouse events.
	void mouse(CDrawContext *context, CPoint &where, long button = -1);

	///	Sets the 1st value.
	void setValue1(float val);
	///	Sets the 2nd value.
	void setValue2(float val);
	///	Sets the 3rd value.
	void setValue3(float val);
	///	Sets the tick's x position.
	void setValueX(float val);
	///	Sets the tick's y position.
	void setValueY(float val);

	///	Returns the 1st value.
	float getValue1() const {return value1;};
	///	Returns the 2nd value.
	float getValue2() const {return value2;};
	///	Returns the 3rd value.
	float getValue3() const {return value3;};
	///	Returns the x pos value.
	float getValueX() const {return xVal;};
	///	Returns the y pos value.
	float getValueY() const {return yVal;};

	///	Returns the 1st tag.
	int getTag1() const {return tag1;};
	///	Returns the 2nd tag.
	int getTag2() const {return tag2;};
	///	Returns the 3rd tag.
	int getTag3() const {return tag3;};
	///	Returns the X tag.
	int getTagX() const {return tagX;};
	///	Returns the Y tag.
	int getTagY() const {return tagY;};
  private:
	///	Updates the control's parameters when the user drags the tick.
	void updateValues();
	///	Updates the tick's position according to the 3 values.
	void updateTick();
	///	Returns the height of the lower triangle for the tick's current x position.
	strictinline float getLowerTop()
	{
		float retval;
		float halfSize = (float)(size.right-size.left)*0.5f;
		float tempf = (float)tickPosition.x/halfSize;

		if(tempf > 1.0f)
		{
			--tempf;
			tempf = 1.0f-tempf;
		}

		retval = (tempf*halfSize) + ((1.0f-tempf)*(circleRadius*2.0f));

		return retval;
	};

	///	The image of the tick.
	CBitmap *tickImage;

	///	The current position of the tick.
	CPoint tickPosition;
	///	The radius of the tick.
	float tickRadius;

	///	The radius of the main circle.
	float circleRadius;

	///	The value of the 1st parameter.
	float value1;
	///	The value of the 2nd parameter.
	float value2;
	///	The value of the 3rd parameter.
	float value3;
	///	The float value of the x position of the tick.
	float xVal;
	///	The float value of the y position of the tick.
	float yVal;

	///	The tag for the 1st parameter.
	int tag1;
	///	The tag for the 2nd parameter.
	int tag2;
	///	The tag for the 3rd parameter.
	int tag3;
	///	The tag for xVal.
	int tagX;
	///	The tag for yVal.
	int tagY;

	///	If true, draw debug lines.
	bool debugLines;
};

#endif
