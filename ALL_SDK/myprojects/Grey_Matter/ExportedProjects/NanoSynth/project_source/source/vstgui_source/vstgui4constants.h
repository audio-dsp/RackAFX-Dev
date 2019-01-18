#ifndef VSTGUI43CONSTANTS
#define VSTGUI43CONSTANTS

// --- this seemed to get lost in 4.3?...?
#include "vstgui4/vstgui/lib/cdrawcontext.h"
#include "vstgui4/vstgui/vstgui.h"

using namespace VSTGUI;

enum {rbVertical4, rbVertHoriz2};
enum {segButtonSimple, segButtonBitmaps};
enum {textButton, bitmapButton, smallToggle, smallProphet};

#define VSTGUI_43 

#ifdef VSTGUI_43
	#define CControlListener IControlListener
	#define vstguiOutSize uint32_t
	#define vstguiInSize  uint32_t

	#define VSTGUI43_SWAP(x,y) y

	// --- CXYPad
	#define bounceValues(x,y) boundValues(x,y)


#else	
	#define VSTGUI43_SWAP(x,y) x
	#define vstguiOutSize int32_t
	#define vstguiInSize int32_t
#endif



#endif
