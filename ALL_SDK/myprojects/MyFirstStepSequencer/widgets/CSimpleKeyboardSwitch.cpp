//	CSimpleKeyboardSwitch.cpp - Each key acts as a switch, has a tag & value
//								for every key.
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

#include "CSimpleKeyboardSwitch.h"

//----------------------------------------------------------------------------
CSimpleKeyboardSwitch::CSimpleKeyboardSwitch(CRect &size,
											 CControlListener *listener,
											 long *tag,
											 CColor bgcol,
											 CColor fgcol,
											 int numoctaves):
							 CSimpleKeyboard(size,
											 listener,
											 bgcol,
											 fgcol,
											 numoctaves)
{
	int i;

	Tags = new long[(12*NumOctaves)];
	Values = new bool[(12*NumOctaves)];

	for(i=0;i<(12*NumOctaves);i++)
	{
		Tags[i] = tag[i];
		Values[i] = false;
	}

	setTag(Tags[0]);
}

//----------------------------------------------------------------------------
CSimpleKeyboardSwitch::~CSimpleKeyboardSwitch()
{
	delete [] Tags;
	delete [] Values;
}

//-----------------------------------------------------------------------------
void CSimpleKeyboardSwitch::draw(CDrawContext *pContext)
{
	DrawKeyboard(pContext, Values);
}

//-----------------------------------------------------------------------------
void CSimpleKeyboardSwitch::mouse(CDrawContext *pContext, CPoint& where, long buttons)
{
	CRect rect;
	int numwhitekeys, numblackkeys;
	int i, i_black;
	int temp1, temp2, temp3;
	bool onthree = false;
	int blackcount;
	int current_octave = 0;
	bool already_black = false;

	if(!bMouseEnabled)
		return;

	long button = pContext->getMouseButtons();

	numwhitekeys = 7 * NumOctaves;
	numblackkeys = 5 * NumOctaves;

	temp1 = (size.right-size.left)/numwhitekeys;

	//rect(0, 0, temp1, ((size.bottom-1)-size.top));
	rect = size;
	rect.right += temp1;

	for(i=1;i<numwhitekeys;i++)
	{

		rect.left += temp1;
		rect.right += temp1;
	}

	temp3 = (int)(0.65f * temp1);
	temp2 = (int)(0.65f * ((size.bottom-1)-size.top));

	//rect(0, 0, temp3, temp2);
	rect = size;
	rect.bottom = rect.y + temp2;
	rect.left += temp1-(temp3/2);
	rect.right = rect.left + temp3;

	//rect.offset(size.left, size.top);

	blackcount = 0;
	i_black = numwhitekeys;	//bacause Values is (allthewhitekeys...alltheblackkeys)
	for(i=0;i<(numwhitekeys);i++)
	{
		if((!onthree)&&(blackcount < 2))
		{
			if(rect.pointInside(where))
			{
				if(Values[i_black])
					Values[i_black] = false;
				else
					Values[i_black] = true;

				setTag(Tags[i_black]);
				already_black = true;
				setDirty();
				doIdleStuff();
				if(listener)
					listener->valueChanged(pContext, this);

				break;
			}
			rect.left += temp1;
			rect.right = rect.left + temp3;

			i_black++;
			//i_black++;
			blackcount++;
		}
		else if((!onthree)&&(blackcount < 3))
		{
			rect.left += temp1;
			rect.right = rect.left + temp3;

			//i_black++;
			blackcount = 0;
			onthree = true;
		}
		else if((onthree)&&(blackcount < 3))
		{

			if(rect.pointInside(where))
			{
				if(Values[i_black])
					Values[i_black] = false;
				else
					Values[i_black] = true;

				setTag(Tags[i_black]);
				already_black = true;
				setDirty();
				doIdleStuff();
				if(listener)
					listener->valueChanged(pContext, this);

				break;
			}
			rect.left += temp1;
			rect.right = rect.left + temp3;

			i_black++;
			//i_black++;

			blackcount++;
			//j++;
		}
		else
		{
			rect.left += temp1;
			rect.right = rect.left + temp3;

			//i_black++;
			blackcount = 0;
			onthree = false;
		}
	}
	if(!already_black)
	{
		temp1 = (size.right-size.left)/numwhitekeys;
		rect(0, 0, temp1, ((size.bottom-1)-size.top));
		rect.offset(size.left, size.top);
		for(i=0;i<numwhitekeys;i++)
		{
			if(rect.pointInside(where))
			{
				if(Values[i])
					Values[i] = false;
				else
					Values[i] = true;

				setTag(Tags[i]);
				setDirty();
				doIdleStuff();
				if(listener)
					listener->valueChanged(pContext, this);

				break;
			}

			rect.left += temp1;
			rect.right += temp1;
		}
	}
}
