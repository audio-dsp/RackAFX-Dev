/* ------------------------------------------------------
     CTextLabelWP, CHorizontalSliderWP
     Custom VSTGUI Objects by Will Pirkle
     Created with RackAFX(TM) Plugin Development Software
     www.willpirkle.com
 -------------------------------------------------------*/
#include "TextLabelEditWP.h"

CTextLabelWP::CTextLabelWP (const CRect& size, UTF8StringPtr txt, CBitmap* background, const int32_t style)
: CTextLabel(size, txt, background, style)
{

}

CTextLabelWP::CTextLabelWP(const CTextLabelWP& textLabel)
: CTextLabel(textLabel)
{
	m_sUnits.assign(textLabel.m_sUnits);
}

void CTextLabelWP::setText(UTF8StringPtr txt)			///< set text
{
	CTextLabel::setText(txt);
	                   
	if(m_sUnits.size() > 0)
	{
		string editString(getText());
		editString.append(" ");
		editString.append(m_sUnits);
		CTextLabel::setText(editString.c_str());
	}
}

const UTF8String& CTextLabelWP::getText() const			///< read only access to text
{
	return CTextLabel::getText();
}

CTextEditWP::CTextEditWP(const CRect& size, IControlListener* listener, int32_t tag, UTF8StringPtr txt, CBitmap* background, const int32_t style)
: CTextEdit(size, listener, tag, txt, background, style)
{

}

CTextEditWP::CTextEditWP(const CTextEditWP& textEdit)
: CTextEdit(textEdit)
{

}

void CTextEditWP::setText(UTF8StringPtr txt)			///< set text
{
	CTextEdit::setText(txt);
	                   
	if(m_sUnits.size() > 0)
	{
		string editString(getText());
		editString.append(" ");
		editString.append(m_sUnits);
		CTextEdit::setText(editString.c_str());
	}
}
