//	TwoTickKnob.cpp - CKnob subclass with an extra tick.
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

#include "TwoTickKnob.h"

#include <cmath>

//-----------------------------------------------------------------------------
TwoTickKnob::TwoTickKnob(const CRect &size,
						 CControlListener *listener,
						 long tag, 
						 CBitmap *background,
						 CBitmap *handle,
						 CBitmap *extra,
						 const CPoint &offset):
CKnob(size, listener, tag, background, handle, offset),
extraBitmap(extra),
extraPosition(0.0f)
{
	if(extraBitmap)
		extraBitmap->remember();
}

//-----------------------------------------------------------------------------
TwoTickKnob::~TwoTickKnob()
{
	if(extraBitmap)
		extraBitmap->forget();
}

//-----------------------------------------------------------------------------
void TwoTickKnob::draw(CDrawContext *pContext)
{
	if(pBackground)
	{
		if(bTransparencyEnabled)
			pBackground->drawTransparent(pContext, size, offset);
		else
			pBackground->draw(pContext, size, offset);
	}
	drawHandle(pContext);
	drawExtraTick(pContext);
	setDirty(false);
}

//-----------------------------------------------------------------------------
void TwoTickKnob::setExtraTick(float val)
{
	extraPosition = val;
}

//-----------------------------------------------------------------------------
void TwoTickKnob::drawExtraTick(CDrawContext *pContext)
{
	CPoint where;
	extraToPoint(where);

	if(extraBitmap)
	{
		long width  = (long)extraBitmap->getWidth();
		long height = (long)extraBitmap->getHeight();
		where.offset(size.left - width / 2, size.top - height / 2);

		CRect handleSize(0, 0, width, height);
		handleSize.offset(where.h, where.v);
		extraBitmap->drawTransparent(pContext, handleSize);
	}
	else
	{
		CPoint origin(size.width () / 2, size.height () / 2);
		
		where.offset(size.left - 1, size.top);
		origin.offset(size.left - 1, size.top);
		pContext->setFrameColor(colorShadowHandle);
		pContext->moveTo(where);
		pContext->lineTo(origin);
		
		where.offset(1, -1);
		origin.offset(1, -1);
		pContext->setFrameColor(colorHandle);
		pContext->moveTo(where);
		pContext->lineTo(origin);
	}
}

//-----------------------------------------------------------------------------
void TwoTickKnob::extraToPoint(CPoint &point) const
{
	float alpha = (extraPosition - bCoef) / aCoef;
	point.h = (long)(radius + cosf (alpha) * (radius - inset) + 0.5f);
	point.v = (long)(radius - sinf (alpha) * (radius - inset) + 0.5f);
}
