#pragma once

#include <string>
#include "vstguidefaults.h"

/* ------------------------------------------------------
 RAFX GUI Messages
 Created with RackAFX(TM) Plugin Development Software 
 www.willpirkle.com
 -------------------------------------------------------*/

using namespace std;
	
enum {knobGroup, sliderGroup, twoStateSwitchGroup, radioButtonGroup, assignButtonGroup, vumeterGroup, LCDGroup, JSGroup, textLabel, imageControl}; // add more...
enum {allControls, allNormalControls, lcdControls}; // add more...
enum {fullControlGroup, noEditControl, noOptionMenus}; // add more...

const UINT KNOBGROUP = knobGroup;
const UINT SLIDERGROUP = sliderGroup;
const UINT TWOSTATESWITCHGROUP = twoStateSwitchGroup;
const UINT RADIOBUTTONGROUP = radioButtonGroup;
const UINT ASSIGNBUTTONGROUP = assignButtonGroup;
const UINT VUMETERGROUP = vumeterGroup;
const UINT LCDGROUP = LCDGroup;
const UINT JSGROUP = JSGroup;
const UINT TEXTLABEL = textLabel;
const UINT IMAGECONTROL = imageControl;

#define HIDE_VISIBLE_EDITORS	      WM_USER + 1001        
#define SHOW_KNOB_GROUP_EDITOR        WM_USER + 1002        
#define UPDATE_KNOB_GROUP		      WM_USER + 1003
#define SHOW_SLIDER_GROUP_EDITOR      WM_USER + 1004        
#define UPDATE_SLIDER_GROUP		      WM_USER + 1005 // need?
#define SHOW_2SS_GROUP_EDITOR		  WM_USER + 1006        
#define SHOW_RADIO_GROUP_EDITOR		  WM_USER + 1007        
#define SHOW_ASS_BUTTON_EDITOR		  WM_USER + 1008        
#define SHOW_VUMETER_EDITOR			  WM_USER + 1009        
#define SHOW_LCD_EDITOR				  WM_USER + 1010        
#define SHOW_JS_EDITOR				  WM_USER + 1011    
#define SHOW_TEXTLABEL_EDITOR		  WM_USER + 1012    

#define REMOVE_KNOB_GROUP			  WM_USER + 1013    
#define REMOVE_SLIDER_GROUP			  WM_USER + 1014    
#define REMOVE_RB_GROUP				  WM_USER + 1015    
#define REMOVE_2SS_GROUP			  WM_USER + 1016    
#define REMOVE_METER_GROUP			  WM_USER + 1017    
#define REMOVE_ASSBUTT_GROUP		  WM_USER + 1018    
#define REMOVE_LCD_GROUP		      WM_USER + 1019    
#define REMOVE_JS_GROUP		          WM_USER + 1020    
#define REMOVE_TEXTLABEL		      WM_USER + 1021    
#define SHOW_IMAGE_CHOOSER			  WM_USER + 1022    
#define REMOVE_IMAGE				  WM_USER + 1023

#define GetRValue(rgb)      (LOBYTE(rgb))
#define GetGValue(rgb)      (LOBYTE(((WORD)(rgb)) >> 8))
#define GetBValue(rgb)      (LOBYTE((rgb)>>16))

typedef unsigned long DWORD;
typedef DWORD   COLORREF;
typedef unsigned int        UINT;
typedef unsigned char		UCHAR;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;

#if MAC
typedef int INT_PTR, *PINT_PTR;
typedef  unsigned int UINT_PTR, *PUINT_PTR;

typedef long LONG_PTR, *PLONG_PTR;
typedef  unsigned long ULONG_PTR, *PULONG_PTR;

typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;
#endif

#define MAKEWORD(a, b)      ((WORD)(((BYTE)(((DWORD_PTR)(a)) & 0xff)) | ((WORD)((BYTE)(((DWORD_PTR)(b)) & 0xff))) << 8))
#define MAKELONG(a, b)      ((LONG)(((WORD)(((DWORD_PTR)(a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD_PTR)(b)) & 0xffff))) << 16))
#define LOWORD(l)           ((WORD)(((DWORD_PTR)(l)) & 0xffff))
#define HIWORD(l)           ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))
#define LOBYTE(w)           ((BYTE)(((DWORD_PTR)(w)) & 0xff))
#define HIBYTE(w)           ((BYTE)((((DWORD_PTR)(w)) >> 8) & 0xff))
typedef struct
{
	string origin;
	string size;
	string tag;
	string title;
	string sTextColorName;
	string sFontName;

	COLORREF cTextColor;

	int nWeight;
	bool bBold;
	bool bItalic;
	bool bStrike;
	bool bUnder;
	bool bTransparent;
	
	string customView;
	string subController;
	string textAlign;

	void* nodeRoot; // xmlnode
	
} LABEL_STRUCT;

typedef struct
{
	string bitmapName;
	string origin;
	string size;
	string tag;

	string customView;
	string subController;

	string htOneImage;
	string imageCount;
	void* nodeRoot; // xmlnode
	UINT groupStyle;

} KNOB_STRUCT;

typedef struct
{
	string bitmapName;
	string origin;
	string size;
	string tag;
	void* nodeRoot; // xmlnode

} TWOSS_STRUCT;

typedef struct
{
	string controlName;
	bool isLCDControl;
	int numControls;
	int controlIndex;
	int lcdControlIndex;
	void* nodeRoot; // xmlnode

} PLUGIN_CONTROL_LCD;

typedef struct
{
	string bitmapName;
	string origin;
	string size;
	string tag;
	string className;
	bool latching;
	void* nodeRoot; // xmlnode

} ASS_BUTTON_STRUCT;

typedef struct
{
	string bitmapName;
	string origin;
	string size;
	string tag;
	string max; // num_images - 1
	
	string htOneImage;
	string imageCount;
	
	// atts for each button label
	LABEL_STRUCT btnLabelAtts[8];
	int nButtonLabelCount;
	int nButtonLabelIndex;

	//std::vector<std::string> m_ButtonLabelList;
	void* nodeRoot; // xmlnode

} RADIO_BUTTON_STRUCT;

typedef struct
{
	string bitmapName;
	string handleBitmapName;
	string origin;
	string size;
	string tag;

	string handleOffset;
	bool switchSlider;
	string switchCount;
	
	string customView;
	string subController;

	void* nodeRoot; // xmlnode
	UINT groupStyle;

} SLIDER_STRUCT;

typedef struct
{
	string bitmapName;
	string offBitmapName;
	string invertedBitmapName;
	string invertedOffBitmapName;
	string origin;
	string size;
	string tag;
	string numLEDs;

	bool bInverted;
	void* nodeRoot; // xmlnode

} VUMETER_STRUCT;

typedef struct
{
	string origin;
	string size;
	string tag;
	string title;

	string sTextColorName;
	COLORREF cTextColor;

	string sFontName;
	int nWeight;
	bool bBold;
	bool bItalic;
	bool bStrike;
	bool bUnder;
	bool bTransparent;

	// --- edit stuff
	string sBackColorName;
	COLORREF cBackColor;

	string sFrameColorName;
	COLORREF cFrameColor;
	
	int nFrameWidth;
	bool bShowFrame;

	int nRoundRadius;
	bool bRoundCorners;
	bool bTransparentEdit;
		
	string customView;
	string subController;
	string textAlign;

	string sTextInset; // (x,y)

	// --- for OMG ONly
	string maxValue;

	void* nodeRoot; // xmlnode

} EDIT_STRUCT;

typedef struct
{
	EDIT_STRUCT omgAAtts;
	EDIT_STRUCT omgBAtts;
	EDIT_STRUCT omgCAtts;
	EDIT_STRUCT omgDAtts;
	
	LABEL_STRUCT AtextLabel;
	LABEL_STRUCT BtextLabel;
	LABEL_STRUCT CtextLabel;
	LABEL_STRUCT DtextLabel;

	// --- these are abcd around diamond
	LABEL_STRUCT A_Static;
	LABEL_STRUCT B_Static;
	LABEL_STRUCT C_Static;
	LABEL_STRUCT D_Static;

	string sJSBackColorName;
	COLORREF cJSBackColor;

	string sJSPuckColorName;
	COLORREF cJSPuckColor;
	void* nodeRoot; // xmlnode
	
	string origin;
	string size;
	string tag;
	string customView;
	string subController;
	UINT groupStyle;

} JOYSTICK_STRUCT;

typedef struct
{
	string name;		 // may be ""
	string templateName; // may be ""

	string origin;
	string size;
	
	string transparent;
	string bitmap;

	void* nodeViewRoot; // xmlnode
	void* nodeTemplateRoot; // xmlnode

} CONTAINER_STRUCT;

typedef struct
{
	KNOB_STRUCT		alphaKnob;
	KNOB_STRUCT		valueKnob;
	LABEL_STRUCT	topTextLabel;
	EDIT_STRUCT		editBox;
	LABEL_STRUCT	bottomTextLabel;

	void* nodeRoot; // xmlnode
	CONTAINER_STRUCT	vcInnerAtts;

} LCD_STRUCT;


typedef struct
{
	CONTAINER_STRUCT	vcAtts;
	KNOB_STRUCT			knobAtts;
	LABEL_STRUCT		labelAtts;
	EDIT_STRUCT			editAtts;
	void* nodeRoot; // xmlnode, not used here (yet?)

} KNOB_GROUP_STRUCT;

typedef struct
{
	CONTAINER_STRUCT	vcAtts;
	SLIDER_STRUCT		sliderAtts;
	LABEL_STRUCT		labelAtts;
	EDIT_STRUCT			editAtts;
	void* nodeRoot; // xmlnode, not used here (yet?)

} SLIDER_GROUP_STRUCT;


typedef struct
{
	CONTAINER_STRUCT	vcAtts;
	RADIO_BUTTON_STRUCT	rbAtts;
	LABEL_STRUCT		labelAtts;
	void* nodeRoot; // xmlnode, not used here (yet?)

} RB_GROUP_STRUCT;

typedef struct
{
	CONTAINER_STRUCT	vcAtts;
	VUMETER_STRUCT		vuAtts;
	LABEL_STRUCT		labelAtts;
	void* nodeRoot; // xmlnode, not used here (yet?)

} METER_GROUP_STRUCT;

typedef struct
{
	CONTAINER_STRUCT	vcAtts; // outer
	CONTAINER_STRUCT	vcInnerAtts;
	LCD_STRUCT			lcdAtts;
	void* nodeRoot; // xmlnode, not used here (yet?)

} LCD_GROUP_STRUCT;

typedef struct
{
	CONTAINER_STRUCT	vcAtts; // outer
	JOYSTICK_STRUCT		jsAtts;

	void* nodeRoot; // xmlnode, not used here (yet?)

} JOYSTICK_GROUP_STRUCT;

typedef struct
{
	CONTAINER_STRUCT	vcAtts;
	TWOSS_STRUCT		twoSSAtts;
	LABEL_STRUCT		labelAtts;
	void* nodeRoot; // xmlnode, not used here (yet?)

} TWOSS_GROUP_STRUCT;


typedef struct
{
	CONTAINER_STRUCT	vcAtts;
	ASS_BUTTON_STRUCT	assButton1Atts;
	ASS_BUTTON_STRUCT	assButton2Atts;
	ASS_BUTTON_STRUCT	assButton3Atts;
	LABEL_STRUCT		assButton1labelAtts;
	LABEL_STRUCT		assButton2labelAtts;
	LABEL_STRUCT		assButton3labelAtts;
	void* nodeRoot; // xmlnode, not used here (yet?)

} ASSBUTT_GROUP_STRUCT;

typedef struct
{
	bool ab1_Latching;
	bool ab2_Latching;
	bool ab3_Latching;

	string ab1_Title;
	string ab2_Title;
	string ab3_Title;
} ASSBUTT_CREATION_STRUCT;

typedef struct
{
	int nListACount;
	int nListBCount;
	int nListCCount;
	int nListDCount;

} JOYSTICK_CREATION_STRUCT;

// remember to update inline void clearControlAttributes() on knobgroupeditor when you add shit
typedef struct
{
	// --- most conrols have one of each
	KNOB_STRUCT			knobAtts;
	SLIDER_STRUCT		sliderAtts;
	EDIT_STRUCT			editAtts;
	LABEL_STRUCT		labelAtts;
	CONTAINER_STRUCT	vcAtts;
	TWOSS_STRUCT		twoSSAtts;
	RADIO_BUTTON_STRUCT rbAtts;
	VUMETER_STRUCT		vuAtts;

	// --- but keeping these in trio, easier
	ASS_BUTTON_STRUCT	assButton1Atts;
	ASS_BUTTON_STRUCT	assButton2Atts;
	ASS_BUTTON_STRUCT	assButton3Atts;
	LABEL_STRUCT		assButton1labelAtts;
	LABEL_STRUCT		assButton2labelAtts;
	LABEL_STRUCT		assButton3labelAtts;
	LCD_STRUCT			lcdAtts;
	int					nButtonIndex; // 0,1,2

	JOYSTICK_STRUCT		joystickAtts;

	// --- decode for what this is
	UINT				uGroupType;

} CONTROL_GROUP_STRUCT;
