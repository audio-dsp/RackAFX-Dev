//	TempoSyncReverserEditor.h - Plugin's gui.
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

#ifndef TEMPOSYNCREVERSEREDITOR_H_
#define TEMPOSYNCREVERSEREDITOR_H_


// include VSTGUI
#ifndef __vstgui__
#include "vstgui.h"
#endif

//-----------------------------------------------------------------------------
class TempoSyncReverserEditor : public AEffGUIEditor, public CControlListener
{
public:
	TempoSyncReverserEditor(AudioEffect *effect);
	virtual ~TempoSyncReverserEditor();

	virtual void setParameter(long index, float value);
	virtual void valueChanged(CDrawContext* context, CControl* control);

protected:
	virtual long open(void *ptr);
	virtual void close();

private:
	void setNoteValRadioButtons();

	CKickButton *Reverse;
	COnOffButton *Quantise;

	COptionMenu *MIDINote;
	COptionMenu *MIDICC;

	COnOffButton *Note4;
	COnOffButton *Note3;
	COnOffButton *Note2;
	COnOffButton *Note1;
	COnOffButton *Note1_2;
	COnOffButton *Note1_3;
	COnOffButton *Note1_4;
	COnOffButton *Note1_6;
	COnOffButton *Note1_8;
	COnOffButton *Note1_12;
	COnOffButton *Note1_16;
	COnOffButton *Note1_32;

	// Bitmaps
	CBitmap *hBack;
	CBitmap *hReverse;
	CBitmap *hQuantise;

	CBitmap *hNote4;
	CBitmap *hNote3;
	CBitmap *hNote2;
	CBitmap *hNote1;
	CBitmap *hNote1_2;
	CBitmap *hNote1_3;
	CBitmap *hNote1_4;
	CBitmap *hNote1_6;
	CBitmap *hNote1_8;
	CBitmap *hNote1_12;
	CBitmap *hNote1_16;
	CBitmap *hNote1_32;
};

#endif