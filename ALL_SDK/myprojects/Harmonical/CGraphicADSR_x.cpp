//	CGraphicADSR.cpp - A graphical ADSR (x because it replaces an older one I
//					   made, w/out anti-aliasing).
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

#include "CGraphicADSR_x.h"

//----------------------------------------------------------------------------
#ifdef WIN32
__forceinline void SetColourAlpha(CColor *col, CColor bcol, int alpha)
#else
inline void SetColourAlpha(CColor *col, CColor bcol, int alpha)
#endif
{
	float red1, red2, green1, green2, blue1, blue2, alph;

	red1 = (float)col->red;
	green1 = (float)col->green;
	blue1 = (float)col->blue;

	red2 = (float)bcol.red;
	green2 = (float)bcol.green;
	blue2 = (float)bcol.blue;

	/*if(*col == bcol)
	{
		red1 += 64;
		green1 += 64;
		blue1 += 64;
	}*/

	alph = ((float)alpha/255.0f);

	if(red1 > red2)
		red1 = red1 - red2;
	else
		red1 = red2 - red1;
	col->red = (unsigned char)(red2 + (red1*alph));
	if(green1 > green2)
		green1 = green1 - green2;
	else
		green1 = green2 - green1;
	col->green = (unsigned char)(green2 + (green1*alph));
	if(blue1 > blue2)
		blue1 = blue1 - blue2;
	else
		blue1 = blue2 - blue1;
	col->blue = (unsigned char)(blue2 + (blue1*alph));
}

#ifdef WIN32
__forceinline void DrawAALine(CDrawContext *context, CPoint p1, CPoint p2, CColor col, CColor bcol)
#else
inline void DrawAALine(CDrawContext *context, CPoint p1, CPoint p2, CColor col, CColor bcol)
#endif
{
	int x0 = p1.x;
	int x1 = p2.x;
	int y0 = p1.y;
	int y1 = p2.y;
	int x, y, xx, yy, v;
	int dx = x1 - x0;
	int dy = y1 - y0;
	CPoint temp(0, 0);
	CColor tcol = col;

	if(p1 == p2)
		return;
    
	if(abs(dx) > abs(dy)) //for each x pixel
	{
		if(x0 > x1) //for increasing x
		{
			x = x0;  x0 = x1;  x1 = x;
			y = y0;  y0 = y1;  y1 = y;
		}
		dy = (dy << 8) / dx; //oversample y by 256
		yy = y0 << 8;
    
		for(x=x0; x<x1; x++) //draw line
		{
			v = yy & 0xFF;
			int y = yy >> 8;
			yy = yy + dy;

			tcol = col;
			SetColourAlpha(&tcol, bcol, 255);
			temp(x, y);
			context->drawPoint(temp, tcol);

			tcol = col;
			SetColourAlpha(&tcol, bcol, v);
			temp(x, y+1);
			context->drawPoint(temp, tcol);

			tcol = col;
			SetColourAlpha(&tcol, bcol, (255-v));
			temp(x, y-1);
			context->drawPoint(temp, tcol);
		}
	}
	else //for each y pixel
	{
		if(y0 > y1) //for increasing y
		{
			y = y0;  y0 = y1;  y1 = y;
			x = x0;  x0 = x1;  x1 = x;
		}
		dx = (dx << 8) / dy;  //oversample x by 256
		xx = x0 << 8;

		for(y=y0; y<y1; y++)
		{
			v = xx & 0xFF;
			x = xx >> 8;
			xx = xx + dx;

			tcol = col;
			SetColourAlpha(&tcol, bcol, 255);
			temp(x, y);
			context->drawPoint(temp, tcol);

			tcol = col;
			SetColourAlpha(&tcol, bcol, v);
			temp(x+1, y);
			context->drawPoint(temp, tcol);

			tcol = col;
			SetColourAlpha(&tcol, bcol, (255-v));
			temp(x-1, y);
			context->drawPoint(temp, tcol);
		}
	}
}

CGraphicADSR_x::CGraphicADSR_x(CRect &size,
						   CControlListener *listener,
						   int tag_A,
						   int tag_D,
						   int tag_S,
						   int tag_R,
						   CColor BackColour,
						   CColor ForeColour,
						   CBitmap *pHandle,
						   CBitmap *pBackground):
			   CControl(size,
						listener,
						tag_A,
						pBackground)
{
	this->tag_A = tag_A;
	this->tag_D = tag_D;
	this->tag_S = tag_S;
	this->tag_R = tag_R;

	value_A = 1.0f;
	value_D = 1.0f;
	value_S = 0.5f;
	value_R = 1.0f;
	//oc = 0;

	pBackOffs(0, 0);

	point_start(0, (size.height()-5)); //size.height(), not 0, because co-ords go top -> bottom

	backg = BackColour;
	foreg = ForeColour;

	/*if(pBackground)
		this->pBackground = pBackground;*/

	this->pHandle = pHandle;

	if(this->pHandle)
	{
		this->pHandle->remember();
		HandleSize(0, 0, this->pHandle->getWidth(), this->pHandle->getHeight());
	}
	else
	{
		HandleSize(0, 0, 10, 10);
	}

	//setBounds();

	point_start(5, (size.height()-5)); //size.height(), not 0, because co-ords go top -> bottom
	point_end((this->size.width()-5), (size.height()-5)); //size.height(), not 0, because co-ords go top -> bottom
	point_start.offset(size.left, size.top);
	point_end.offset(size.left, size.top);
	setPoints(0);
	setPoints(1);
	setPoints(2);
	setPoints(3);
}

CGraphicADSR_x::~CGraphicADSR_x()
{
	if(pHandle)
		pHandle->forget();
}

void CGraphicADSR_x::SetHandleSize(int width)
{
	if(!pHandle)
	{
		HandleSize(0, 0, width, width);
		point_start((width/2), (size.height()-(width/2)));
		point_end((this->size.width()-(width/2)), (size.height()-(width/2)));
		point_start.offset(size.left, size.top);
		point_end.offset(size.left, size.top);

		setPoints(0);
		setPoints(1);
		setPoints(2);
		setPoints(3);
	}
}

void CGraphicADSR_x::setBounds()
{
	/*int temp1;
	float temp2;

	temp1 = (size.width()-10)/4;

	xbounds_A = temp1;
	xbounds_D = temp1;
	xbounds_R = temp1;

	temp2 = (float)((float)size.height()-10) * value_S;
	ybounds_D = ((float)size.height()-10) - (int)temp2;
	ybounds_R = (int)temp2;*/
}

void CGraphicADSR_x::setPoints(int seg)
{
	switch(seg)
	{
		case 0:
			point_A.x = (int)(value_A * ((size.width()-HandleSize.width())/4)) + (HandleSize.width()/2) + size.left;
			point_A.y = (HandleSize.width()/2) + size.top;
			point_D.x = (int)(value_D * ((size.width()-HandleSize.width())/4)) + point_A.x;
			break;
		case 1:
			point_D.x = (int)(value_D * ((size.width()-HandleSize.width())/4)) + point_A.x;
			break;
		case 2:
			point_D.y = (size.height()-(HandleSize.height()/2)) - (int)(value_S * (size.height()-HandleSize.height())) + size.top;
			point_R.y = (size.height()-(HandleSize.height()/2)) - (int)(value_S * (size.height()-HandleSize.height())) + size.top;
			break;
		case 3:
			point_R.x = (size.width()-(HandleSize.width()/2)) - (int)(value_R * ((size.width()-HandleSize.width())/4)) + size.left;
			break;
	}
}

void CGraphicADSR_x::draw(CDrawContext *context)
{
	int temp1, temp2;

	CColor temp = {0, 0, 0, 0};
	CRect recadendrum(0, 0, 3, 3);
	CPoint tempPoint1(0, (getHeight()-1));
	CPoint tempPoint2(0, (getHeight()-1));

	//Draw background.
	if(pBackground)
    {
		//CRect temp98(0, 0, pBackground->getWidth(), pBackground->getHeight());
		context->setFillColor(backg);
		recadendrum.left = -1;
		recadendrum.right = getWidth() + 1;
		recadendrum.top = -1;
		recadendrum.bottom = getHeight() + 1;
		context->fillRect(recadendrum);
		recadendrum.offset(size.left, size.top);
		pBackground->draw(context, recadendrum, pBackOffs);
    }
	else
	{
		context->setFillColor(backg);
		recadendrum.left = -1;
		recadendrum.right = getWidth() + 1;
		recadendrum.top = -1;
		recadendrum.bottom = getHeight() + 1;
		recadendrum.offset(size.left, size.top);
		context->fillRect(recadendrum);
	}

	//Draw the lines.
	context->setFrameColor(foreg);
	context->setFillColor(foreg);

	context->moveTo(point_start);

	DrawAntiAliasedLine(context, point_start, point_A, foreg);
	DrawAntiAliasedLine(context, point_A, point_D, foreg);

	if(point_R.y >= ((float)size.bottom-(HandleSize.height()/2)))
		tempPoint1(point_R.x, (size.bottom-(HandleSize.height()/2)));
	else
		tempPoint1 = point_R;
	if(tempPoint1.x >= (size.right-(HandleSize.width()/2)))
		tempPoint1.x = size.right - (HandleSize.width()/2);
	DrawAntiAliasedLine(context, point_end, tempPoint1, foreg);
	tempPoint2 = tempPoint1;

	if(point_D.y >= ((float)size.bottom-(HandleSize.height()/2)))
		tempPoint1(point_D.x, (size.bottom-(HandleSize.height()/2)));
	else
		tempPoint1 = point_D;
	DrawAntiAliasedLine(context, tempPoint2, tempPoint1, foreg);

	//Draw the handles/drag points.
	if(pHandle) //point_start
	{
		temp1 = pHandle->getWidth()/2;
		temp2 = pHandle->getHeight()/2;

		recadendrum = HandleSize;
		recadendrum.offset(0, (getHeight()-HandleSize.height()));
		recadendrum.offset(size.left, size.top);
		pHandle->draw(context, recadendrum);
	}
	else
	{
		recadendrum = HandleSize;
		recadendrum.offset(0, (getHeight()-HandleSize.height())); //-1, not 0, because fillRect ignores the frame
		recadendrum.offset(size.left, size.top);
		context->fillRect(recadendrum);
	}

	if(pHandle)
	{
		temp1 = pHandle->getWidth()/2;
		temp2 = pHandle->getHeight()/2;

		recadendrum = HandleSize;
		if(point_A.x < ((HandleSize.width()/2)+size.left))
			recadendrum.offset(0, 0);
		else
			recadendrum.offset((point_A.x-temp1), 0);
		recadendrum.offset(size.left, size.top);
		pHandle->draw(context, recadendrum);
	}
	else
	{
		recadendrum = HandleSize;
		if(point_A.x < ((HandleSize.width()/2)+size.left))
			recadendrum.offset(-1, -1);
		else
			recadendrum.offset(((point_A.x-size.left)-(HandleSize.width()/2)), 0); //4, not 5, because fillRect ignores the frame
		recadendrum.offset(size.left, size.top);
		context->fillRect(recadendrum);
	}

	if(pHandle)
	{
		temp1 = pHandle->getWidth()/2;
		temp2 = pHandle->getHeight()/2;

		recadendrum = HandleSize;
		if(point_D.y < (pHandle->getHeight()+size.top))
			recadendrum.offset((point_D.x-temp1), 0);
		else if(point_D.y > ((size.height()-pHandle->getHeight())+size.top))
			recadendrum.offset((point_D.x-(long)temp1), (size.height()-pHandle->getHeight()));
		else
			recadendrum.offset((point_D.x-temp1), (point_D.y-temp2));
		recadendrum.offset(size.left, size.top);
		pHandle->draw(context, recadendrum);
	}
	else
	{
		recadendrum = HandleSize;
		if(point_D.y < (size.top + (HandleSize.height()/2)))
			recadendrum.offset((point_D.x-(HandleSize.width()/2)), size.top);
		else if(point_D.y > (size.bottom-(HandleSize.height()/2)))
			recadendrum.offset((point_D.x-(HandleSize.width()/2)), (size.bottom-HandleSize.height()));
		else
			recadendrum.offset((point_D.x-(HandleSize.width()/2)), (point_D.y-(HandleSize.height()/2)));
		context->fillRect(recadendrum);
	}

	if(pHandle)
	{
		temp1 = pHandle->getWidth()/2;
		temp2 = pHandle->getHeight()/2;

		recadendrum = HandleSize;
		if(point_R.y < (pHandle->getHeight()+size.top))
			recadendrum.offset((point_R.x-temp1), 0);
		else if(point_R.y > ((getHeight()-pHandle->getHeight())+size.top))
			recadendrum.offset((point_R.x-temp1), (getHeight()-pHandle->getHeight()));
		else
			recadendrum.offset((point_R.x-temp1), (point_R.y-temp2));
		recadendrum.offset(size.left, size.top);
		pHandle->draw(context, recadendrum);
	}
	else
	{
		recadendrum = HandleSize;
		if(point_R.y < (size.top + (HandleSize.height()/2)))
			recadendrum.offset((point_R.x-(HandleSize.width()/2)), size.top);
		else if(point_R.y > (size.bottom-(HandleSize.height()/2)))
			recadendrum.offset((point_R.x-(HandleSize.width()/2)), (size.bottom-HandleSize.height()));
		else
			recadendrum.offset((point_R.x-(HandleSize.width()/2)), (point_R.y-(HandleSize.height()/2)));
		context->fillRect(recadendrum);
	}

	if(pHandle)
	{
		temp1 = pHandle->getWidth()/2;
		temp2 = pHandle->getHeight()/2;

		recadendrum = HandleSize;
		recadendrum.offset((getWidth()-HandleSize.width()), (getHeight()-HandleSize.height()));
		recadendrum.offset(size.left, size.top);
		pHandle->draw(context, recadendrum);
	}
	else
	{
		recadendrum = HandleSize;
		recadendrum.offset((getWidth()-HandleSize.width()), (getHeight()-HandleSize.height()));
		recadendrum.offset(size.left, size.top);
		context->fillRect(recadendrum);
	}
}

void CGraphicADSR_x::draw_old(CDrawContext *context)
{
	int temp1, temp2;

	CColor temp = {0, 0, 0, 0};
	CRect recadendrum(0, 0, 3, 3);
	CPoint tempPoint1(0, (getHeight()-1));
	CPoint tempPoint2(0, (getHeight()-1));

#ifndef VSTGUI3
	COffscreenContext *oc = new COffscreenContext(getParent(), getWidth(), getHeight(), backg);
#else
	COffscreenContext *oc = new COffscreenContext(getFrame(), getWidth(), getHeight(), backg);
#endif
        
	//oc->setDrawMode(kAntialias);

	if(pBackground)
    {
		//CRect temp98(0, 0, pBackground->getWidth(), pBackground->getHeight());
		oc->setFillColor(backg);
		recadendrum.left = -1;
		recadendrum.right = getWidth() + 1;
		recadendrum.top = -1;
		recadendrum.bottom = getHeight() + 1;
		oc->fillRect(recadendrum);
		pBackground->draw(oc, recadendrum, pBackOffs);
    }
	else
	{
		oc->setFillColor(backg);
		recadendrum.left = -1;
		recadendrum.right = getWidth() + 1;
		recadendrum.top = -1;
		recadendrum.bottom = getHeight() + 1;
		oc->fillRect(recadendrum);
	}
		//i = 0;

	oc->setFrameColor(foreg);
	oc->setFillColor(foreg);

	oc->moveTo(point_start);

	//anti_aliasedline(point_start, point_A, foreg, oc);
	//anti_aliasedline(point_A, point_D, foreg, oc);

	//oc->lineTo(point_A);
	DrawAntiAliasedLine(oc, point_start, point_A, foreg);
	//oc->lineTo(point_D);
	DrawAntiAliasedLine(oc, point_A, point_D, foreg);
	//oc->moveTo(point_end);

	if(point_R.y == ((float)size.height()-10))
		tempPoint1.x = point_R.x;
	else
		tempPoint1 = point_R;
	if(tempPoint1.x == (size.width()-10))
		tempPoint1.x--;
	//oc->lineTo(tempPoint1);
	DrawAntiAliasedLine(oc, point_end, tempPoint1, foreg);
	//anti_aliasedline(point_end, tempPoint1, foreg, oc);
	tempPoint2 = tempPoint1;

	if(point_D.y == ((float)size.height()-10))
		tempPoint1.x = point_D.x;
	else
		tempPoint1 = point_D;
	//oc->lineTo(tempPoint1);
	DrawAntiAliasedLine(oc, tempPoint2, tempPoint1, foreg);
	//anti_aliasedline(tempPoint2, tempPoint1, foreg, oc);

	if(pHandle) //point_start
	{
		temp1 = pHandle->getWidth()/2;
		temp2 = pHandle->getHeight()/2;

		recadendrum = HandleSize;
		recadendrum.offset(0, (getHeight()-HandleSize.height()));
		pHandle->draw(oc, recadendrum);
	}
	else
	{
		recadendrum = HandleSize;
		recadendrum.offset(0, (getHeight()-HandleSize.height())); //-1, not 0, because fillRect ignores the frame
		oc->fillRect(recadendrum);
	}

	if(pHandle)
	{
		temp1 = pHandle->getWidth()/2;
		temp2 = pHandle->getHeight()/2;

		recadendrum = HandleSize;
		if(point_A.x < (HandleSize.width()/2))
			recadendrum.offset(0, 0);
		else
			recadendrum.offset((point_A.x-temp1), 0);
		pHandle->draw(oc, recadendrum);
	}
	else
	{
		recadendrum = HandleSize;
		if(point_A.x < (HandleSize.width()/2))
			recadendrum.offset(-1, -1);
		else
			recadendrum.offset((point_A.x-(HandleSize.width()/2)), 0); //4, not 5, because fillRect ignores the frame
		oc->fillRect(recadendrum);
	}

	if(pHandle)
	{
		temp1 = pHandle->getWidth()/2;
		temp2 = pHandle->getHeight()/2;

		recadendrum = HandleSize;
		if(point_D.y < pHandle->getHeight())
			recadendrum.offset((point_D.x-temp1), 0);
		else if(point_D.y > (size.height()-pHandle->getHeight()))
			recadendrum.offset((point_D.x-(long)temp1), (size.height()-pHandle->getHeight()));
		else
			recadendrum.offset((point_D.x-temp1), (point_D.y-temp2));
		pHandle->draw(oc, recadendrum);
	}
	else
	{
		recadendrum = HandleSize;
		if(point_D.y < (HandleSize.height()/2))
			recadendrum.offset((point_D.x-(HandleSize.width()/2)), -1);
		else if(point_D.y > (getHeight()-(HandleSize.height()/2)))
			recadendrum.offset((point_D.x-(HandleSize.width()/2)), (getHeight()-HandleSize.height()));
		else
			recadendrum.offset((point_D.x-(HandleSize.width()/2)), (point_D.y-(HandleSize.height()/2)));
		oc->fillRect(recadendrum);
	}

	if(pHandle)
	{
		temp1 = pHandle->getWidth()/2;
		temp2 = pHandle->getHeight()/2;

		recadendrum = HandleSize;
		if(point_R.y < pHandle->getHeight())
			recadendrum.offset((point_R.x-temp1), 0);
		else if(point_R.y > (getHeight()-pHandle->getHeight()))
			recadendrum.offset((point_R.x-temp1), (getHeight()-pHandle->getHeight()));
		else
			recadendrum.offset((point_R.x-temp1), (point_R.y-temp2));
		pHandle->draw(oc, recadendrum);
	}
	else
	{
		recadendrum = HandleSize;
		if(point_R.y < (HandleSize.height()/2))
			recadendrum.offset((point_R.x-(HandleSize.width()/2)), -1);
		else if(point_R.y > (getHeight()-(HandleSize.height()/2)))
			recadendrum.offset((point_R.x-(HandleSize.width()/2)), (getHeight()-HandleSize.height()));
		else
			recadendrum.offset((point_R.x-(HandleSize.width()/2)), (point_R.y-(HandleSize.height()/2)));
		if(point_R.x > (getWidth()-(HandleSize.width()/2)))
			recadendrum.offset(-(HandleSize.width()/2), 0);
		oc->fillRect(recadendrum);
	}

	if(pHandle)
	{
		temp1 = pHandle->getWidth()/2;
		temp2 = pHandle->getHeight()/2;

		recadendrum = HandleSize;
		recadendrum.offset((getWidth()-HandleSize.width()), (getHeight()-HandleSize.height()));
		pHandle->draw(oc, recadendrum);
	}
	else
	{
		recadendrum = HandleSize;
		recadendrum.offset((getWidth()-HandleSize.width()), (getHeight()-HandleSize.height()));
		oc->fillRect(recadendrum);
	}

	oc->copyFrom(context, size);

	delete oc;
}

//-----------------------------------------------------------------------------
void CGraphicADSR_x::mouse(CDrawContext *context, CPoint &where, long button)
{
	long delta, delta2, actualPos, actualPos2;
	CRect rect;
	int WhichHandle = 0;
	CPoint tempA, tempD, tempR;

	tempA.x = point_A.x - size.left;
	tempA.y = point_A.y - size.top;
	tempD.x = point_D.x - size.left;
	tempD.y = point_D.y - size.top;
	tempR.x = point_R.x - size.left;
	tempR.y = point_R.y - size.top;

	if(!bMouseEnabled)
		return;

	if(button == -1)
		button = context->getMouseButtons();

	if(listener&&(button&(kAlt|kShift|kControl|kApple)))
	{
		if(listener->controlModifierClicked(context, this, button) != 0)
			return;
	}

	if(!(button & kLButton))
		return;

	delta = size.left;
	delta2 = size.top;

	//--Attack--
	actualPos = tempA.x + size.left;

	rect.left   = actualPos - (HandleSize.width()/2);
	rect.top    = size.top;
	rect.right  = rect.left + HandleSize.width();
	rect.bottom = rect.top + HandleSize.height();

	if(!where.isInside(rect))
	{
		//return;
	}
	else
	{
		WhichHandle = 1;
		delta += where.h - actualPos;
		setTag(tag_A);
	}

	//--Decay/Sustain--
	actualPos = tempD.x + size.left;
	actualPos2 = tempD.y + size.top;

	rect.left   = actualPos - (HandleSize.width()/2);
	rect.top    = actualPos2 - (HandleSize.height()/2);
	rect.right  = rect.left + HandleSize.width();
	rect.bottom = rect.top + HandleSize.height();

	if(!where.isInside(rect))
	{
		//return;
	}
	else
	{
		WhichHandle = 2;
		delta += where.h - actualPos + tempA.x;
		delta2 += where.v - actualPos2;
		setTag(tag_D);
	}

	//--Release/Sustain--
	actualPos = tempR.x + size.left;
	actualPos2 = tempR.y + size.top;

	rect.left   = actualPos - (HandleSize.width()/2);
	rect.top    = actualPos2 - (HandleSize.height()/2);
	rect.right  = rect.left + HandleSize.width();
	rect.bottom = rect.top + HandleSize.height();

	if(!where.isInside(rect))
	{
		//return;
	}
	else
	{
		WhichHandle = 3;
		delta += (size.width()-((size.width()-HandleSize.width())/4))+(where.h - actualPos);
		delta2 += where.v - actualPos2;
		setTag(tag_R);
	}

	if(WhichHandle == 0)
		return;

/*#ifndef VSTGUI3
	getParent()->beginEdit(tag);
	getParent()->setEditView(this);
#else*/
	beginEdit();
//#endif
	while(1)
	{
		button = context->getMouseButtons();
		if(!(button&kLButton))
			break;

		if(WhichHandle == 1) //attack
		{
			value_A = (float)(where.h-delta)/(float)((size.width()-HandleSize.width())/4);
			if(value_A > 1.0f)
				value_A = 1.0f;
			else if(value_A < 0.0f)
				value_A = 0.0f;

			point_A.x = (int)(value_A * ((size.width()-HandleSize.width())/4)) + (HandleSize.width()/2) + size.left;

			point_D.x = (int)(value_D * ((size.width()-HandleSize.width())/4)) + point_A.x;
		}
		else if(WhichHandle == 2) //decay
		{
			value_D = (float)(where.h-delta)/(float)((size.width()-HandleSize.width())/4);
			if(value_D > 1.0f)
				value_D = 1.0f;
			else if(value_D < 0.0f)
				value_D = 0.0f;
			value_S = 1.0f-(float)(where.v-delta2)/(float)(size.height()-HandleSize.height());
			if(value_S > 1.0f)
				value_S = 1.0f;
			else if(value_S < 0.0f)
				value_S = 0.0f;

			point_D.x = (int)(value_D * ((size.width()-HandleSize.width())/4)) + point_A.x;
			//point_D.y = (int)(value_S * (size.height()-HandleSize.height())) + (HandleSize.width()/2);
			point_D.y = (size.height()-(HandleSize.height()/2)) - (int)(value_S * (size.height()-HandleSize.height())) + size.top;

			point_R.y = point_D.y;
		}
		else if(WhichHandle == 3) //release
		{
			value_R = 1.0f-(float)(where.h-delta)/(float)((size.width()-HandleSize.width())/4);
			if(value_R > 1.0f)
				value_R = 1.0f;
			else if(value_R < 0.0f)
				value_R = 0.0f;
			value_S = 1.0f-((float)(where.v-delta2)/(float)(size.height()-HandleSize.height()));
			if(value_S > 1.0f)
				value_S = 1.0f;
			else if(value_S < 0.0f)
				value_S = 0.0f;

			point_R.x = (size.width()-(HandleSize.width()/2)) - (int)(value_R * ((size.width()-HandleSize.width())/4)) + size.left;
			point_R.y = (size.height()-(HandleSize.height()/2)) - (int)(value_S * (size.height()-HandleSize.height())) + size.top;

			point_D.y = point_R.y;
		}

		setDirty();

		if(listener)
			listener->valueChanged(context, this);
		getMouseLocation(context, where);
		doIdleStuff();
	}
/*#ifndef VSTGUI3
	getParent()->endEdit(tag);
#else*/
	endEdit();
//#endif
}

/*//-----------------------------------------------------------------------------
bool CGraphicADSR_x::attached(CView *parent)
{
	if(oc)
		delete oc;

	oc = new COffscreenContext(getParent(), getWidth(), getHeight(), backg);
		
	return CControl::attached(parent);
}

//-----------------------------------------------------------------------------
bool CGraphicADSR_x::removed(CView *parent)
{
	if(oc)
	{
		delete oc;
		oc = 0;
	}
	return CControl::removed(parent);
}*/

//-----------------------------------------------------------------------------
void CGraphicADSR_x::DrawAntiAliasedLine(CDrawContext *context,
										 CPoint p1,
										 CPoint p2,
										 CColor col)
{
	int x0 = p1.x;
	int x1 = p2.x;
	int y0 = p1.y;
	int y1 = p2.y;
	int x, y, xx, yy, v;
	int dx = x1 - x0;
	int dy = y1 - y0;
	CPoint temp(0, 0);
	CColor tcol = col;

	if(p1 == p2)
		return;
    
	if(abs(dx) > abs(dy)) //for each x pixel
	{
		if(x0 > x1) //for increasing x
		{
			x = x0;  x0 = x1;  x1 = x;
			y = y0;  y0 = y1;  y1 = y;
		}
		dy = (dy << 8) / dx; //oversample y by 256
		yy = y0 << 8;
    
		for(x=x0; x<x1; x++) //draw line
		{
			v = yy & 0xFF;
			int y = yy >> 8;
			yy = yy + dy;

			/*tcol = col;
			SetColourAlpha(&tcol, bcol, 255);
			temp(x, y);
			context->drawPoint(temp, tcol);*/
			tcol = col;
			tcol.alpha = 255;
			temp(x, y);
#ifdef WIN32
			BlendPixel(context, temp, tcol);
#else
			context->drawPoint(temp, tcol);
#endif

			/*tcol = col;
			SetColourAlpha(&tcol, bcol, v);
			temp(x, y+1);
			context->drawPoint(temp, tcol);*/
			tcol = col;
			tcol.alpha = v;
			temp(x, y+1);
#ifdef WIN32
			BlendPixel(context, temp, tcol);
#else
			context->drawPoint(temp, tcol);
#endif

			/*tcol = col;
			SetColourAlpha(&tcol, bcol, (255-v));
			temp(x, y-1);
			context->drawPoint(temp, tcol);*/
			tcol = col;
			tcol.alpha = (255-v);
			temp(x, y-1);
#ifdef WIN32
			BlendPixel(context, temp, tcol);
#else
			context->drawPoint(temp, tcol);
#endif
		}
	}
	else //for each y pixel
	{
		if(y0 > y1) //for increasing y
		{
			y = y0;  y0 = y1;  y1 = y;
			x = x0;  x0 = x1;  x1 = x;
		}
		dx = (dx << 8) / dy;  //oversample x by 256
		xx = x0 << 8;

		for(y=y0; y<y1; y++)
		{
			v = xx & 0xFF;
			x = xx >> 8;
			xx = xx + dx;

			/*tcol = col;
			SetColourAlpha(&tcol, bcol, 255);
			temp(x, y);
			context->drawPoint(temp, tcol);*/
			tcol = col;
			tcol.alpha = 255;
			temp(x, y);
#ifdef WIN32
			BlendPixel(context, temp, tcol);
#else
			context->drawPoint(temp, tcol);
#endif

			/*tcol = col;
			SetColourAlpha(&tcol, bcol, v);
			temp(x+1, y);
			context->drawPoint(temp, tcol);*/
			tcol = col;
			tcol.alpha = v;
			temp(x+1, y);
#ifdef WIN32
			BlendPixel(context, temp, tcol);
#else
			context->drawPoint(temp, tcol);
#endif

			/*tcol = col;
			SetColourAlpha(&tcol, bcol, (255-v));
			temp(x-1, y);
			context->drawPoint(temp, tcol);*/
			tcol = col;
			tcol.alpha = (255-v);
			temp(x-1, y);
#ifdef WIN32
			BlendPixel(context, temp, tcol);
#else
			context->drawPoint(temp, tcol);
#endif
		}
	}
}

//-----------------------------------------------------------------------------
void CGraphicADSR_x::BlendPixel(CDrawContext *context, CPoint point, CColor Colour)
{
	CColor current_colour, dest_colour;
	int temp;

	current_colour = context->getPoint(point);

	temp = current_colour.red + (int)((float)(Colour.red-current_colour.red) * ((float)Colour.alpha/255.0f));
	if(temp > 255)
		temp = 255;
	dest_colour.red = temp;

	temp = current_colour.green + (int)((float)(Colour.green-current_colour.green) * ((float)Colour.alpha/255.0f));
	if(temp > 255)
		temp = 255;
	dest_colour.green = temp;

	temp = current_colour.blue + (int)((float)(Colour.blue-current_colour.blue) * ((float)Colour.alpha/255.0f));
	if(temp > 255)
		temp = 255;
	dest_colour.blue = temp;

	dest_colour.alpha = current_colour.alpha;

	context->drawPoint(point, dest_colour);
}
