//	RoutingsEditor.h - The VSTGL editor handling the effects routings.
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

#ifndef ROUTINGSEDITOR_H_
#define ROUTINGSEDITOR_H_

//Stupid MSVC v6 warning about the STL...
#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include "VSTGLEditor.h"
#include "VSTGLTimer.h"
#include "HelperStuff.h"

///	The VSTGL editor handling the effects routings.
class RoutingsEditor : public VSTGLEditor
{
  public:
	///	Constructor.
	RoutingsEditor(AudioEffect *effect);
	///	Destructor.
	~RoutingsEditor();

	///	Called when the editor is to be opened.
	void guiOpen();
	///	Called when the editor is to be closed.
	void guiClose();

	///	Called when the user clicks somewhere.
	void onMouseDown(int button, int x, int y);
	///	Called when the user moves the mouse.
	void onMouseMove(int x, int y);
	///	Called when the user releases a mouse button.
	void onMouseUp(int button, int x, int y);

	///	Called to draw the editor.
	void draw();
	///	Called from MainEditor to update the graphics.
	void timerCallback();

	///	Called from the plugin when a parameter is updated.
	void setParameter(int index, float value);
  private:
	///	Class used to represent and draw a bezier curve.
	class BezierCurve
	{
	  public:
		///	Constructor.
		BezierCurve();
		///	Destructor.
		~BezierCurve();

		///	Simple struct representing a point.
		struct BezierPoint
		{
			///	Default constructor.
			BezierPoint():
			x(0.0f),
			y(0.0f)
			{
			}
			///	Constructor.
			BezierPoint(float xVal, float yVal):
			x(xVal),
			y(yVal)
			{
			}

			float x;
			float y;
		};

		///	Sets the points and controls, updates coefficients.
		void setPoints(BezierPoint p1,
					   BezierPoint p2,
					   BezierPoint c1,
					   BezierPoint c2);
		///	Sets whether the 

		///	Draws the curve.
		void draw();
	  private:
		///	Updates the coefficients.
		void updateCoefficients();

		///	Points for the curve.
		BezierPoint points[2];
		///	Controls for the curve.
		BezierPoint controls[2];
		///	a coefficents.
		BezierPoint a;
		///	b coefficents.
		BezierPoint b;
		///	c coefficents.
		BezierPoint c;

		///	Number of iterations to draw the curve with.
		int numIterations;
	};

	///	Simple struct representing a single slider.
	struct Slider
	{
		///	Default constructor.
		Slider():
		x(1),
		y(0),
		routing(0)
		{
		};

		///	Left position of the slider rectangle.
		int x;
		///	Top position of the slider rectangle.
		int y;
		///	Current routing setting for the slider.
		int routing;
	};

	///	Used to determine if the mouse is within a slider.
	strictinline bool isInside(int x, int y, const Slider& slider)
	{
		bool retval = false;

		if((x > slider.x) && (x < (slider.x+28)) &&
		   (y > slider.y) && (y < (slider.y+20)))
			retval = true;

		return retval;
	};
	///	Used to update the bezier curves' positions.
	void updateBezierCurves();
	///	Helper method to round a float to an int.
	inline int round(float val)
	{
		int retval;

		retval = (int)val;

		if((val-(float)retval) > 0.5f)
			++retval;

		return retval;
	};

	///	The five sliders.
	Slider sliders[6];
	///	Bezier curves to connect all the sliders.
	BezierCurve lines[9];
	///	Transparency for the bezier curves.
	float lineTrans;
	///	Whether the indexed line should be displayed.
	bool lineVisible[9];

	///	Non-zero if the user is dragging a slider.
	int sliderSelected;

	///	The owning effect.
	AudioEffect *mainPlugin;
};

#endif
