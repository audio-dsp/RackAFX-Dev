//	TempoSyncReverserEditor.cpp - Plugin's gui.
//	--------------------------------------------------------------------------
//	Copyright (c) 2004-2006 Niall Moody
//	
//	Permission is hereby granted, free of charge, to any person obtaining a
//	copy of this software and associated documentation files (the "Software"),
//	to deal in the Software without restriction, including without limitation
//	the rights to use, copy, modify, merge, publish, distribute, sublicense,
//	and/or sell copies of the Software, and to permit persons to whom the
//	Software is furnished to do so, subject to the following conditions:

//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.

//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//	THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//	DEALINGS IN THE SOFTWARE.
//	--------------------------------------------------------------------------

#include "TempoSyncReverserEditor.h"
#include "TempoSyncReverser.h"
#include "NoteNames.h"

#include <stdio.h>

inline float round(float inval)
{
	float tempf;

	tempf = inval - static_cast<float>(static_cast<int>(inval));
	if(tempf < 0.5f)
		return static_cast<float>(static_cast<int>(inval));
	else
		return static_cast<float>(static_cast<int>(inval))+1.0f;
}

//-----------------------------------------------------------------------------
// resource id's
enum {
	// bitmaps
	kBack = 128,
	kReverseButton,
	k4,
	k3,
	k2,
	k1,
	k1_2,
	k1_3,
	k1_4,
	k1_6,
	k1_8,
	k1_12,
	k1_16,
	k1_32,
	kQuantiseButton
};

//-----------------------------------------------------------------------------
// TempoSyncReverserEditor class implementation
//-----------------------------------------------------------------------------
TempoSyncReverserEditor::TempoSyncReverserEditor(AudioEffect *effect)
 : AEffGUIEditor(effect) 
{
	Reverse = 0;
	Quantise = 0;
	MIDINote = 0;
	MIDICC= 0;
	Note4 = 0;
	Note3 = 0;
	Note2 = 0;
	Note1 = 0;
	Note1_2 = 0;
	Note1_3 = 0;
	Note1_4 = 0;
	Note1_6 = 0;
	Note1_8 = 0;
	Note1_12 = 0;
	Note1_16 = 0;
	Note1_32 = 0;

	hReverse = 0;
	hQuantise = 0;
	hNote4 = 0;
	hNote3 = 0;
	hNote2 = 0;
	hNote1 = 0;
	hNote1_2 = 0;
	hNote1_3 = 0;
	hNote1_4 = 0;
	hNote1_6 = 0;
	hNote1_8 = 0;
	hNote1_12 = 0;
	hNote1_16 = 0;
	hNote1_32 = 0;

	// load the background bitmap
	// we don't need to load all bitmaps, this could be done when open is called
	hBack  = new CBitmap(kBack);

	// init the size of the plugin
	rect.left   = 0;
	rect.top    = 0;
	rect.right  = (short)hBack->getWidth();
	rect.bottom = (short)hBack->getHeight();
}

//-----------------------------------------------------------------------------
TempoSyncReverserEditor::~TempoSyncReverserEditor()
{
	// free background bitmap
	if(hBack)
		hBack->forget();
	hBack = 0;
}

//-----------------------------------------------------------------------------
long TempoSyncReverserEditor::open(void *ptr)
{
	int i;
	char tempstr[5];
	CPoint point;
	CRect size;

	// !!! always call this !!!
	AEffGUIEditor::open(ptr);
	
	// load some bitmaps
	if(!hReverse)
		hReverse = new CBitmap(kReverseButton);
	if(!hQuantise)
		hQuantise = new CBitmap(kQuantiseButton);
	if(!hNote4)
		hNote4 = new CBitmap(k4);
	if(!hNote3)
		hNote3 = new CBitmap(k3);
	if(!hNote2)
		hNote2 = new CBitmap(k2);
	if(!hNote1)
		hNote1 = new CBitmap(k1);
	if(!hNote1_2)
		hNote1_2 = new CBitmap(k1_2);
	if(!hNote1_3)
		hNote1_3 = new CBitmap(k1_3);
	if(!hNote1_4)
		hNote1_4 = new CBitmap(k1_4);
	if(!hNote1_6)
		hNote1_6 = new CBitmap(k1_6);
	if(!hNote1_8)
		hNote1_8 = new CBitmap(k1_8);
	if(!hNote1_12)
		hNote1_12 = new CBitmap(k1_12);
	if(!hNote1_16)
		hNote1_16 = new CBitmap(k1_16);
	if(!hNote1_32)
		hNote1_32 = new CBitmap(k1_32);

	//--init background frame-----------------------------------------------
	size(0, 0, hBack->getWidth(), hBack->getHeight());
	frame = new CFrame(size, ptr, this);
	frame->setBackground(hBack);

	size(173, 146, (173+hReverse->getWidth()), (146+(hReverse->getHeight()/2)));
#ifdef WIN32
	Reverse = new CKickButton(size, this, kReverse, (hReverse->getHeight()/2), hReverse, CPoint(0, 0));
	Reverse->setValue(effect->getParameter(kReverse));
#else
        point(0, 0);
	Reverse = new CKickButton(size, this, kMacReverse, (hReverse->getHeight()/2), hReverse, point);
	Reverse->setValue(effect->getParameter(kMacReverse));
#endif
	Reverse->setTransparency(false);
	frame->addView(Reverse);

	size(255, 118, (255+hQuantise->getWidth()), (118+(hQuantise->getHeight()/2)));
	Quantise = new COnOffButton(size, this, kQuantise, hQuantise);
	Quantise->setValue(effect->getParameter(kQuantise));
	Quantise->setTransparency(false);
	frame->addView(Quantise);

	//--Options Menus--------
	CColor backColour={224, 224, 224, 0};
	CColor foreColour={128, 128, 128, 0};
	size(222, 63, 273, 79);
	MIDINote = new COptionMenu(size, this, kMIDINote);
	MIDINote->setBackColor(backColour);
	MIDINote->setFrameColor(backColour);
	MIDINote->setFontColor(foreColour);
	MIDINote->setNbItemsPerColumn(32);

	size(222, 92, 273, 108);
	MIDICC = new COptionMenu(size, this, kMIDICC);
	MIDICC->setBackColor(backColour);
	MIDICC->setFrameColor(backColour);
	MIDICC->setFontColor(foreColour);
	MIDICC->setNbItemsPerColumn(32);

	for(i=0;i<128;i++)
	{
		MIDINote->addEntry(const_cast<char *>(noteNames[i]));
		if(i==0)
			MIDICC->addEntry("Off");
		else
		{
			sprintf(tempstr, "%d", (i-1));
			MIDICC->addEntry(tempstr);
		}
	}
	MIDINote->setCurrent(static_cast<long>(round(127.0f*effect->getParameter(kMIDINote))));
	MIDICC->setCurrent(static_cast<long>(round(127.0f*effect->getParameter(kMIDICC))));
	frame->addView(MIDINote);
	frame->addView(MIDICC);

	//--Note Values--------
	size(34, 61, (34+hNote4->getWidth()), (61+(hNote4->getHeight()/2)));
	Note4 = new COnOffButton(size, this, kNoteVal, hNote4);
	Note4->setTransparency(false);
	frame->addView(Note4);

	size(34, 78, (34+hNote3->getWidth()), (78+(hNote3->getHeight()/2)));
	Note3 = new COnOffButton(size, this, kNoteVal, hNote3);
	Note3->setTransparency(false);
	frame->addView(Note3);

	size(34, 95, (34+hNote2->getWidth()), (95+(hNote2->getHeight()/2)));
	Note2 = new COnOffButton(size, this, kNoteVal, hNote2);
	Note2->setTransparency(false);
	frame->addView(Note2);

	size(34, 112, (34+hNote1->getWidth()), (112+(hNote1->getHeight()/2)));
	Note1 = new COnOffButton(size, this, kNoteVal, hNote1);
	Note1->setTransparency(false);
	frame->addView(Note1);

	size(34, 129, (34+hNote1_2->getWidth()), (129+(hNote1_2->getHeight()/2)));
	Note1_2 = new COnOffButton(size, this, kNoteVal, hNote1_2);
	Note1_2->setTransparency(false);
	frame->addView(Note1_2);

	size(34, 146, (34+hNote1_3->getWidth()), (146+(hNote1_3->getHeight()/2)));
	Note1_3 = new COnOffButton(size, this, kNoteVal, hNote1_3);
	Note1_3->setTransparency(false);
	frame->addView(Note1_3);

	size(88, 61, (88+hNote1_4->getWidth()), (61+(hNote1_4->getHeight()/2)));
	Note1_4 = new COnOffButton(size, this, kNoteVal, hNote1_4);
	Note1_4->setTransparency(false);
	frame->addView(Note1_4);

	size(88, 78, (88+hNote1_6->getWidth()), (78+(hNote1_6->getHeight()/2)));
	Note1_6 = new COnOffButton(size, this, kNoteVal, hNote1_6);
	Note1_6->setTransparency(false);
	frame->addView(Note1_6);

	size(88, 95, (88+hNote1_8->getWidth()), (95+(hNote1_8->getHeight()/2)));
	Note1_8 = new COnOffButton(size, this, kNoteVal, hNote1_8);
	Note1_8->setTransparency(false);
	frame->addView(Note1_8);

	size(88, 112, (88+hNote1_12->getWidth()), (112+(hNote1_12->getHeight()/2)));
	Note1_12 = new COnOffButton(size, this, kNoteVal, hNote1_12);
	Note1_12->setTransparency(false);
	frame->addView(Note1_12);

	size(88, 129, (88+hNote1_16->getWidth()), (129+(hNote1_16->getHeight()/2)));
	Note1_16 = new COnOffButton(size, this, kNoteVal, hNote1_16);
	Note1_16->setTransparency(false);
	frame->addView(Note1_16);

	size(88, 146, (88+hNote1_32->getWidth()), (146+(hNote1_32->getHeight()/2)));
	Note1_32 = new COnOffButton(size, this, kNoteVal, hNote1_32);
	Note1_32->setTransparency(false);
	frame->addView(Note1_32);

	setNoteValRadioButtons();

	return true;
}

//-----------------------------------------------------------------------------
void TempoSyncReverserEditor::close()
{
	delete frame;
	frame = 0;

	if(hReverse)
	{
		hReverse->forget();
		hReverse = 0;
	}
	if(hQuantise)
	{
		hQuantise->forget();
		hQuantise = 0;
	}
	if(hNote4)
	{
		hNote4->forget();
		hNote4 = 0;
	}
	if(hNote3)
	{
		hNote3->forget();
		hNote3 = 0;
	}
	if(hNote2)
	{
		hNote2->forget();
		hNote2 = 0;
	}
	if(hNote1)
	{
		hNote1->forget();
		hNote1 = 0;
	}
	if(hNote1_2)
	{
		hNote1_2->forget();
		hNote1_2 = 0;
	}
	if(hNote1_3)
	{
		hNote1_3->forget();
		hNote1_3 = 0;
	}
	if(hNote1_4)
	{
		hNote1_4->forget();
		hNote1_4 = 0;
	}
	if(hNote1_6)
	{
		hNote1_6->forget();
		hNote1_6 = 0;
	}
	if(hNote1_8)
	{
		hNote1_8->forget();
		hNote1_8 = 0;
	}
	if(hNote1_12)
	{
		hNote1_12->forget();
		hNote1_12 = 0;
	}
	if(hNote1_16)
	{
		hNote1_16->forget();
		hNote1_16 = 0;
	}
	if(hNote1_32)
	{
		hNote1_32->forget();
		hNote1_32 = 0;
	}
}

//-----------------------------------------------------------------------------
void TempoSyncReverserEditor::setParameter(long index, float value)
{
	if(!frame)
		return;

	// called from Template
	switch(index)
	{
#ifdef WIN32
		case kReverse:
#else
                case kMacReverse:
#endif
			Reverse->setValue(effect->getParameter(index));
			break;
		case kQuantise:
			Quantise->setValue(effect->getParameter(index));
			break;
		case kNoteVal:
			setNoteValRadioButtons();
			break;
		case kMIDINote:
			MIDINote->setCurrent(static_cast<long>(round(127.0f*effect->getParameter(kMIDINote))));
			break;
		case kMIDICC:
			MIDICC->setCurrent(static_cast<long>(round(127.0f*effect->getParameter(kMIDICC))));
			break;
	}
	postUpdate();
}

//-----------------------------------------------------------------------------
void TempoSyncReverserEditor::valueChanged(CDrawContext* context, CControl* control)
{
	float tempf;
	const float numValues = 12.0f;
	long tag = control->getTag();

	switch(tag)
	{
#ifdef WIN32
		case kReverse:
#else
                case kMacReverse:
#endif
		case kQuantise:
			effect->setParameterAutomated(tag, control->getValue());
			control->update(context);
			break;
		case kNoteVal:
			if(control == Note1_32)
			{
				effect->setParameterAutomated(tag, 0.0f);
				control->update(context);
			}
			else if(control == Note1_16)
			{
				effect->setParameterAutomated(tag, 1.0f/numValues);
				control->update(context);
			}
			else if(control == Note1_12)
			{
				effect->setParameterAutomated(tag, 2.0f/numValues);
				control->update(context);
			}
			else if(control == Note1_8)
			{
				effect->setParameterAutomated(tag, 3.0f/numValues);
				control->update(context);
			}
			else if(control == Note1_6)
			{
				effect->setParameterAutomated(tag, 4.0f/numValues);
				control->update(context);
			}
			else if(control == Note1_4)
			{
				effect->setParameterAutomated(tag, 5.0f/numValues);
				control->update(context);
			}
			else if(control == Note1_3)
			{
				effect->setParameterAutomated(tag, 6.0f/numValues);
				control->update(context);
			}
			else if(control == Note1_2)
			{
				effect->setParameterAutomated(tag, 7.0f/numValues);
				control->update(context);
			}
			else if(control == Note1)
			{
				effect->setParameterAutomated(tag, 8.0f/numValues);
				control->update(context);
			}
			else if(control == Note2)
			{
				effect->setParameterAutomated(tag, 9.0f/numValues);
				control->update(context);
			}
			else if(control == Note3)
			{
				effect->setParameterAutomated(tag, 10.0f/numValues);
				control->update(context);
			}
			else if(control == Note4)
			{
					effect->setParameterAutomated(tag, 11.0f/numValues);
					control->update(context);
			}
			break;
		case kMIDINote:
		case kMIDICC:
			tempf = static_cast<float>(static_cast<COptionMenu *>(control)->getCurrent());
			tempf /= 127.0f;
			effect->setParameterAutomated(tag, tempf);
			control->update(context);
			break;
	}
}

//-----------------------------------------------------------------------------
void TempoSyncReverserEditor::setNoteValRadioButtons()
{
	float tempf;
	const float numValues = 12.0f;

	tempf = effect->getParameter(kNoteVal);

	if(tempf < (1.0f/numValues))
	{
		Note1_32->setValue(1.0f);
		Note1_16->setValue(0.0f);
		Note1_12->setValue(0.0f);
		Note1_8->setValue(0.0f);
		Note1_6->setValue(0.0f);
		Note1_4->setValue(0.0f);
		Note1_3->setValue(0.0f);
		Note1_2->setValue(0.0f);
		Note1->setValue(0.0f);
		Note2->setValue(0.0f);
		Note3->setValue(0.0f);
		Note4->setValue(0.0f);
	}
	else if(tempf < (2.0f/numValues))
	{
		Note1_32->setValue(0.0f);
		Note1_16->setValue(1.0f);
		Note1_12->setValue(0.0f);
		Note1_8->setValue(0.0f);
		Note1_6->setValue(0.0f);
		Note1_4->setValue(0.0f);
		Note1_3->setValue(0.0f);
		Note1_2->setValue(0.0f);
		Note1->setValue(0.0f);
		Note2->setValue(0.0f);
		Note3->setValue(0.0f);
		Note4->setValue(0.0f);
	}
	else if(tempf < (3.0f/numValues))
	{
		Note1_32->setValue(0.0f);
		Note1_16->setValue(0.0f);
		Note1_12->setValue(1.0f);
		Note1_8->setValue(0.0f);
		Note1_6->setValue(0.0f);
		Note1_4->setValue(0.0f);
		Note1_3->setValue(0.0f);
		Note1_2->setValue(0.0f);
		Note1->setValue(0.0f);
		Note2->setValue(0.0f);
		Note3->setValue(0.0f);
		Note4->setValue(0.0f);
	}
	else if(tempf < (4.0f/numValues))
	{
		Note1_32->setValue(0.0f);
		Note1_16->setValue(0.0f);
		Note1_12->setValue(0.0f);
		Note1_8->setValue(1.0f);
		Note1_6->setValue(0.0f);
		Note1_4->setValue(0.0f);
		Note1_3->setValue(0.0f);
		Note1_2->setValue(0.0f);
		Note1->setValue(0.0f);
		Note2->setValue(0.0f);
		Note3->setValue(0.0f);
		Note4->setValue(0.0f);
	}
	else if(tempf < (5.0f/numValues))
	{
		Note1_32->setValue(0.0f);
		Note1_16->setValue(0.0f);
		Note1_12->setValue(0.0f);
		Note1_8->setValue(0.0f);
		Note1_6->setValue(1.0f);
		Note1_4->setValue(0.0f);
		Note1_3->setValue(0.0f);
		Note1_2->setValue(0.0f);
		Note1->setValue(0.0f);
		Note2->setValue(0.0f);
		Note3->setValue(0.0f);
		Note4->setValue(0.0f);
	}
	else if(tempf < (6.0f/numValues))
	{
		Note1_32->setValue(0.0f);
		Note1_16->setValue(0.0f);
		Note1_12->setValue(0.0f);
		Note1_8->setValue(0.0f);
		Note1_6->setValue(0.0f);
		Note1_4->setValue(1.0f);
		Note1_3->setValue(0.0f);
		Note1_2->setValue(0.0f);
		Note1->setValue(0.0f);
		Note2->setValue(0.0f);
		Note3->setValue(0.0f);
		Note4->setValue(0.0f);
	}
	else if(tempf < (7.0f/numValues))
	{
		Note1_32->setValue(0.0f);
		Note1_16->setValue(0.0f);
		Note1_12->setValue(0.0f);
		Note1_8->setValue(0.0f);
		Note1_6->setValue(0.0f);
		Note1_4->setValue(0.0f);
		Note1_3->setValue(1.0f);
		Note1_2->setValue(0.0f);
		Note1->setValue(0.0f);
		Note2->setValue(0.0f);
		Note3->setValue(0.0f);
		Note4->setValue(0.0f);
	}
	else if(tempf < (8.0f/numValues))
	{
		Note1_32->setValue(0.0f);
		Note1_16->setValue(0.0f);
		Note1_12->setValue(0.0f);
		Note1_8->setValue(0.0f);
		Note1_6->setValue(0.0f);
		Note1_4->setValue(0.0f);
		Note1_3->setValue(0.0f);
		Note1_2->setValue(1.0f);
		Note1->setValue(0.0f);
		Note2->setValue(0.0f);
		Note3->setValue(0.0f);
		Note4->setValue(0.0f);
	}
	else if(tempf < (9.0f/numValues))
	{
		Note1_32->setValue(0.0f);
		Note1_16->setValue(0.0f);
		Note1_12->setValue(0.0f);
		Note1_8->setValue(0.0f);
		Note1_6->setValue(0.0f);
		Note1_4->setValue(0.0f);
		Note1_3->setValue(0.0f);
		Note1_2->setValue(0.0f);
		Note1->setValue(1.0f);
		Note2->setValue(0.0f);
		Note3->setValue(0.0f);
		Note4->setValue(0.0f);
	}
	else if(tempf < (10.0f/numValues))
	{
		Note1_32->setValue(0.0f);
		Note1_16->setValue(0.0f);
		Note1_12->setValue(0.0f);
		Note1_8->setValue(0.0f);
		Note1_6->setValue(0.0f);
		Note1_4->setValue(0.0f);
		Note1_3->setValue(0.0f);
		Note1_2->setValue(0.0f);
		Note1->setValue(0.0f);
		Note2->setValue(1.0f);
		Note3->setValue(0.0f);
		Note4->setValue(0.0f);
	}
	else if(tempf < (11.0f/numValues))
	{
		Note1_32->setValue(0.0f);
		Note1_16->setValue(0.0f);
		Note1_12->setValue(0.0f);
		Note1_8->setValue(0.0f);
		Note1_6->setValue(0.0f);
		Note1_4->setValue(0.0f);
		Note1_3->setValue(0.0f);
		Note1_2->setValue(0.0f);
		Note1->setValue(0.0f);
		Note2->setValue(0.0f);
		Note3->setValue(1.0f);
		Note4->setValue(0.0f);
	}
	else
	{
		Note1_32->setValue(0.0f);
		Note1_16->setValue(0.0f);
		Note1_12->setValue(0.0f);
		Note1_8->setValue(0.0f);
		Note1_6->setValue(0.0f);
		Note1_4->setValue(0.0f);
		Note1_3->setValue(0.0f);
		Note1_2->setValue(0.0f);
		Note1->setValue(0.0f);
		Note2->setValue(0.0f);
		Note3->setValue(0.0f);
		Note4->setValue(1.0f);
	}
}