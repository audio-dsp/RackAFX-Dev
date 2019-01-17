#ifndef __ctextlabelwp__
#define __ctextlabelwp__

#include "vstgui/lib/controls/ctextlabel.h"
#include "vstgui4constants.h"

namespace VSTGUI {
using namespace std;

/* ------------------------------------------------------
     CTextLabelWP, CHorizontalSliderWP
     Custom VSTGUI Objects by Will Pirkle
     Created with RackAFX(TM) Plugin Development Software
     www.willpirkle.com
 -------------------------------------------------------*/
class CTextLabelWP : public CTextLabel
{
public:
	CTextLabelWP (const CRect& size, UTF8StringPtr txt = 0, CBitmap* background = 0, const int32_t style = 0);
	CTextLabelWP (const CTextLabelWP& textLabel);

	virtual void setText(UTF8StringPtr txt);			
	virtual const UTF8String& getText() const;				

	string m_sUnits;
	void setUnitsString(char* pUints){trimString(pUints); m_sUnits.assign(pUints);}
		
	inline bool trimString(char* str)
	{
		while(*str == ' ' || *str == '\t' || *str == '\n')
			str++;

		int len = strlen(str);
		if(len <= 0) return false;

		while(len >= 0 && (str[len - 1] == ' ' || str[len - 1] == '\t' || *str == '\n'))
		{
			*(str + len - 1) = '\0';
			len--;
		}

		return true;
	}

	CLASS_METHODS(CTextLabelWP, CTextLabel)
protected:
	//~CTextLabelWP (); // destructor is non-virtual
};

} // namespace

class CTextEditWP : public CTextEdit
{
public:
	CTextEditWP(const CRect& size, IControlListener* listener, int32_t tag, UTF8StringPtr txt = 0, CBitmap* background = 0, const int32_t style = 0);
	CTextEditWP(const CTextEditWP& textEdit);
		
	string m_sUnits;
	void setUnitsString(char* pUints){trimString(pUints); m_sUnits.assign(pUints);}
		
	inline bool trimString(char* str)
	{
		while(*str == ' ' || *str == '\t' || *str == '\n')
			str++;

		int len = strlen(str);
		if(len <= 0) return false;

		while(len >= 0 && (str[len - 1] == ' ' || str[len - 1] == '\t' || *str == '\n'))
		{
			*(str + len - 1) = '\0';
			len--;
		}

		return true;
	}
	
	virtual void setText (UTF8StringPtr txt);

	CLASS_METHODS(CTextEditWP, CTextEdit)
protected:
};

#endif
