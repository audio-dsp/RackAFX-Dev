//	NiallsAbout.h - An about box that is only displayed as long as the mouse
//					button is held down.
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

#ifndef NIALLSABOUT_H_
#define NIALLSABOUT_H_

///	About box which is only displayed as long as the mouse button is held down.
/*!
	Places a button on the parent CFrame - when the mouse is clicked on this,
	and only as long as the button is clicked, an image will be displayed
	which covers the entire frame.
 */
class NiallsAbout : public CControl
{
  public:
	///	Constructor.
	/*!
		\param size The position and size of the button clicked to display the
		about box.
		\param button The image for the button clicked to display the about
		box.
		\param about The image of the about box displayed when the button is
		clicked.  This will always be displayed from 0,0 in the parent frame,
		and should be the same size as the background if you want it to look
		good.
	 */
	NiallsAbout(const CRect& size, CBitmap *button, CBitmap *about);
	///	Destructor.
	~NiallsAbout();

	///	Draws the button and about box (if the button is clicked).
	void draw(CDrawContext *context);

	///	Handles mouse events in the widget.
	/*!
		\param context The CDrawContext from where the event was generated.
		\param position The position within the widget where the mouse was
		when the event was generated.
		\param buttons The button clicked to generate the event?
	 */
	void mouse(CDrawContext *context, CPoint& position, long buttons = -1);
  private:
	///	The image for the button.
	CBitmap *buttonImage;
	///	The image for the actual about box.
	CBitmap *aboutImage;

	///	Whether or not the mouse button is currently down.
	bool displayingAboutBox;
};

#endif
