//	StateButton.h - A button with states, which change with left and right
//					clicks.
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

#ifndef STATEBUTTON_H_
#define STATEBUTTON_H_

#include <vector>

///	A button with states, which change with left and right clicks.
/*!
	Essentially just lets you cycle through it's stacked images.
 */
class StateButton : public CControl
{
  public:
	///	Constructor.
	/*!
		\param size The size and position of the widget.
		\param image The stacked image used to draw the widget.
		\param numImages The number of images stacked in image.  Note: this
		should include the disabled image, if you have one.
		\param listener The change listener for the widget.
		\param tag The widget's tag - usually related to the plugin parameter
		it alters.
		\param backgroundImage A background image to draw behind the widget.
	 */
	StateButton(const CRect& size,
				CBitmap *image,
				long numImages,
				CControlListener *listener = 0,
				long tag = 0,
				CBitmap *backgroundImage = 0);
	///	Destructor.
	~StateButton();

	///	Draws the widget.
	/*!
		\param context The context to draw into.
	 */
	void draw(CDrawContext *context);

	///	Handles mouse events in the widget.
	/*!
		\param context The CDrawContext from where the event was generated.
		\param position The position within the widget where the mouse was
		when the event was generated.
		\param buttons The button clicked to generate the event?
	 */
	void mouse(CDrawContext *context, CPoint& position, long buttons = -1);
	///	The button's state can be changed with the mouse wheel.
	bool onWheel(CDrawContext *context,
				 const CPoint &where,
				 float distance);

	///	We override this so we can update the button's state.
	void setValue(float val);

	///	Returns the widget's current state.
	long getCurrentState() const {return currentState;};
	///	Sets the 'state ignored' list.
	/*!
		We keep a list of states which we want to be ignored.  What this means
		is, when the user reaches a state that should be ignored, the widget
		will just skip over it, and move on to the next state that is not on
		the ignored list.

		Note that the widget will not let you set every state to be ignored - 
		it will always keep one back, so an image is still displayed on
		screen.

		This method lets you set multiple states at once, in a bit array
		(i.e. 110101 would mean 'ignore states 2 and 4').  This will only work
		if there are 32 or less states in the widget.
	 */
	void setStatesIgnored(long states);
	///	Sets a state to be ignored or not.
	/*!
		\param state The state to set ignored or not.
		\param isIgnored If true, state should be ignored, else state should
		not be ignored.

		\sa setStatesIgnored()
	 */
	void setStateIgnored(long state, bool isIgnored = true);

	///	Sets whether the button has a disabled state.
	/*!
		Call setDisabled() to set the actual state.
		\sa setDisabled()
	 */
	void setHasDisabledState(bool val);
	///	Sets whether or not the button's currently in a disabled state.
	/*!
		\param val True = disabled, false = enabled.

		This will only have an effect if you have previously called
		setHasDisabledState(true).  What the disabled state means is that the
		button will see the last image in it's stack as a disabled image, to
		be displayed when the button is disabled.  It will not be shown when
		the button is enabled.
	 */
	void setDisabled(bool val);

	///	Can be called to set a new image for the button.
	/*!
		Note: It is assumed that the new image will be the same dimensions as
		the one passed in to the constructor.
	 */
	void setImage(CBitmap *newImage);
  private:
	///	Sets the current state.
	void setState(long val);

	///	The image used to draw the widget.
	CBitmap *image;
	///	The number of sub-images that are stacked in image.
	long numStackedImages;
	///	The height of the sub-images.
	int sizeOfOneImage;

	///	The widget's current state.
	/*!
		The range for this is 0->(numStackedImage-1).
	 */
	long currentState;
	///	List of which states to ignore.
	/*!
		\sa setStatesIgnored()
	 */
	std::vector<bool> ignoredStates;

	///	Whether or not the button \emph has a disabled state.
	bool hasDisabledState;
	///	Whether or not the button is currently in a disabled state.
	bool isDisabled;
};

#endif
