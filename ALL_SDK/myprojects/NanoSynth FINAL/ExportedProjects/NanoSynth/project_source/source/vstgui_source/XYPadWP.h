#pragma once
#include "vstgui/lib/controls/cxypad.h"
#include "vstgui4constants.h"

namespace VSTGUI {

/* ------------------------------------------------------
     CXYPadWP
     Custom VSTGUI Object by Will Pirkle
     Created with RackAFX(TM) Plugin Development Software
     www.willpirkle.com
 -------------------------------------------------------*/

class CXYPadWP : public CXYPad
{
public:
	CXYPadWP(const CRect& size = CRect (0, 0, 0, 0));
    bool m_bDraggingPuck;

protected:
	int32_t tagX;
	int32_t tagY;

	// --- for easy trackpad and XY stuff
public:
	void setTagX(int32_t tag){tagX = tag;}
	int32_t getTagX(){return tagX;}

	void setTagY(int32_t tag){tagY = tag;}
	int32_t getTagY(){return tagY;}

	bool m_bIsJoystickPad;

	// --- overrides
	void draw(CDrawContext* context) override;
	CMouseEventResult onMouseMoved(CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp(CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseDown(CPoint& where, const CButtonState& buttons) override;

	virtual void setValue(float val) override;

	inline int pointInPolygon(int nvert, float *vertx, float *verty, float testx, float testy)
	{
		int i, j, c = 0;
		for (i = 0, j = nvert-1; i < nvert; j = i++)
		{
			if ( ((verty[i]>testy) != (verty[j]>testy)) &&
				(testx < (vertx[j]-vertx[i]) * (testy-verty[i]) / (verty[j]-verty[i]) + vertx[i]) )
			c = !c;
		}
	 return c;
	}

protected:
	float m_fVertX[4];
	float m_fVertY[4];

	float m_fLastX;
	float m_fLastY;

};
}
