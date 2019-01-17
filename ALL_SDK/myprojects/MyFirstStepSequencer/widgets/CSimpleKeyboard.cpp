//	CSimpleKeyboard.cpp - Keyboard base class: just draws the keys, basically.
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

#include "CSimpleKeyboard.h"
#include "Round.h"
#include <cmath>

//-----------------------------------------------------------------------------
CSimpleKeyboard::CSimpleKeyboard(CRect &size,
							     CControlListener *listener,
								 CColor bgcol,
								 CColor fgcol,
								 int numoctaves):
						CControl(size,
							     listener)
{
	NumOctaves = numoctaves;

	BackColour = bgcol;
	ForeColour = fgcol;
	FrameColour = kBlackCColor;
}

//-----------------------------------------------------------------------------
void CSimpleKeyboard::draw(CDrawContext *pContext)
{
	bool temp[127];

	for(int i=0;i<127;i++)
		temp[i] = false;

	DrawKeyboard(pContext, temp);
}

//-----------------------------------------------------------------------------
void CSimpleKeyboard::DrawKeyboard(CDrawContext *oc, bool *keys)
{
	int numwhitekeys, numblackkeys;
	int temp1, temp2, temp3;
	int i, j;
	bool onthree = false;
	int blackcount;
	int current_octave = 0;

	CRect rect(0, 0, (size.right-size.left), ((size.bottom-1)-size.top));
	rect = size;
	//oc->setFillColor(kWhiteCColor);
	oc->setFillColor(BackColour);
	oc->fillRect(rect);
	//oc->setFrameColor(kBlackCColor);
	oc->setFrameColor(ForeColour);
	rect.right--;
	oc->drawRect(rect);

	if(NumOctaves > 10)
	{
		numwhitekeys = 74;
		numblackkeys = 53;
	}
	else
	{
		numwhitekeys = (int)round(7.0f * (float)NumOctaves);
		numblackkeys = (int)round(5.0f * (float)NumOctaves);
	}

	temp1 = (int)round(((float)size.right-(float)size.left)/(float)numwhitekeys);

	oc->setFrameColor(FrameColour);

	//rect(0, 0, temp1, ((size.bottom-1)-size.top));
	rect.right = rect.x + temp1;
	rect.right += 1;

	//Draw white keys first.
	for(i=0;i<numwhitekeys;i++)
	{
		if(keys[i])
			oc->setFillColor(kWhiteCColor);
			//oc->setFillColor(BackColour);
		else
			oc->setFillColor(BackColour);
			//oc->setFillColor(kWhiteCColor);

		oc->fillRect(rect);
		oc->drawRect(rect);

		rect.left += temp1;
		rect.right += temp1;
	}

	temp3 = (int)round(0.65f * (float)temp1);
	temp2 = (int)round(0.65f * (((float)size.bottom-1.0f)-(float)size.top));

	//rect(0, 0, temp3, temp2);
	rect = size;
	rect.bottom = rect.y + temp2;
	rect.left += temp1-(temp3/2);
	rect.right = rect.left + temp3;

	//rect.offset(size.left, size.top);

	//OS->fillRect(rect);
	//OS->drawRect(rect);

	//Now draw black keys.
	j = numwhitekeys;
	blackcount = 0;
	for(i=0;i<numwhitekeys;i++)
	{
		if((!onthree)&&(blackcount < 2))
		{
			if(keys[j])
				oc->setFillColor(kBlackCColor);
				//oc->setFillColor(ForeColour);
			else
				oc->setFillColor(ForeColour);
				//oc->setFillColor(kBlueCColor);

			oc->fillRect(rect);
			oc->drawRect(rect);

			rect.left += temp1;
			rect.right = rect.left + temp3;

			blackcount++;
			j++;
		}
		else if((!onthree)&&(blackcount < 3))
		{
			rect.left += temp1;
			rect.right = rect.left + temp3;

			blackcount = 0;
			onthree = true;
		}
		else if((onthree)&&(blackcount < 3))
		{

			if(keys[j])
				oc->setFillColor(kBlackCColor);
				//oc->setFillColor(ForeColour);
			else
				oc->setFillColor(ForeColour);
				//oc->setFillColor(kBlueCColor);

			oc->fillRect(rect);
			oc->drawRect(rect);

			rect.left += temp1;
			rect.right = rect.left + temp3;

			blackcount++;
			j++;
		}
		else
		{
			rect.left += temp1;
			rect.right = rect.left + temp3;

			blackcount = 0;
			onthree = false;
		}
	}

	rect = size;
	oc->setFrameColor(kBlackCColor);
	oc->drawRect(rect);
}
