#include "XYPadWP.h"

namespace VSTGUI {
    
/* ------------------------------------------------------
     CXYPadWP
     Custom VSTGUI Object by Will Pirkle
     Created with RackAFX(TM) Plugin Development Software
     www.willpirkle.com
 -------------------------------------------------------*/

CXYPadWP::CXYPadWP(const CRect& size)
: CXYPad(size)
{
	m_fVertX[0] = 0.0;
	m_fVertY[0] = 0.5;

	m_fVertX[1] = 0.5;
	m_fVertY[1] = 0.0;

	m_fVertX[2] = 1.0;
	m_fVertY[2] = 0.5;

	m_fVertX[3] = 0.5;
	m_fVertY[3] = 1.0;

	m_fLastX = 0.0;
	m_fLastY = 0.0;

	tagX = -1;
	tagY = -1;

	m_bIsJoystickPad = false;
	m_bDraggingPuck = false;
}

void CXYPadWP::setValue(float val)
{
	if(m_bIsJoystickPad)
	{
		float x, y;
		calculateXY(val, x, y);
		if(!m_bDraggingPuck)
			y = -1.0*y + 1.0;
        
		// --- this bounds to the diamond
		if(pointInPolygon(4, &m_fVertX[0], &m_fVertY[0], x, y))
		{
			val = calculateValue(x, y);
			CXYPad::setValue(val);
		}/// else ignore
	}
	else if(m_bDraggingPuck)
		CXYPad::setValue(val);
	else
	{
		float x, y;
		calculateXY(val, x, y);
		y = -1.0*y + 1.0;
		boundValues(x, y);
		val = calculateValue(x, y);
		CXYPad::setValue(val);
	}
}

CMouseEventResult CXYPadWP::onMouseDown(CPoint& where, const CButtonState& buttons)
{
	if(buttons.isLeftButton())
	{
		mouseChangeStartPoint = where;
		mouseChangeStartPoint.offset (-getViewSize ().left - getRoundRectRadius() / 2., -getViewSize ().top - getRoundRectRadius() / 2.);
		
		m_bDraggingPuck = true;
		onMouseMoved (where, buttons);

		if(!isEditing())
		{
			beginEdit ();
		}

		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

CMouseEventResult CXYPadWP::onMouseUp(CPoint& where, const CButtonState& buttons)
{	
	CXYPad::onMouseUp(where, buttons);
	m_bDraggingPuck = false;
	return kMouseEventHandled;
}
CMouseEventResult CXYPadWP::onMouseMoved(CPoint& where, const CButtonState& buttons)
{
	if(!m_bIsJoystickPad)
		return CXYPad::onMouseMoved(where, buttons);

	if(buttons.isLeftButton ())
	{
		if(stopTrackingOnMouseExit)
		{
			if(!hitTest(where, buttons))
			{
				endEdit ();
				return kMouseMoveEventHandledButDontNeedMoreEvents;
			}
		}

		float x, y;
		CCoord width = getWidth() - getRoundRectRadius ();
		CCoord height = getHeight() - getRoundRectRadius ();
		where.offset(-getViewSize ().left - getRoundRectRadius() / 2., -getViewSize ().top - getRoundRectRadius() / 2.);

		x = (float)(where.x / width);
		y = (float)(where.y / height);

		// --- this bounds to the diamond
		if(!pointInPolygon(4, &m_fVertX[0], &m_fVertY[0], x, y))
		{
            x = m_fLastX;
            y = m_fLastY;

			setValue(calculateValue (x, y));
			if(listener && isDirty())
				listener->valueChanged(this);

			invalid ();
			lastMouseChangePoint = where;
			return kMouseEventHandled;
		}
		
		boundValues (x, y);
		
		m_fLastX = x;
		m_fLastY = y;

		setValue(calculateValue (x, y));
		if(listener && isDirty())
			listener->valueChanged(this);

		invalid ();
		lastMouseChangePoint = where;
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

void CXYPadWP::draw(CDrawContext* context)
{
	if(!m_bIsJoystickPad)
		return CXYPad::draw(context);

	const CRect& xyRect = getViewSize();
    
#ifdef VSTGUI_43
    CDrawContext::PointList pointList;
    CPoint pVertex0( xyRect.left, xyRect.top + 0.5*(float)(xyRect.getHeight()) );
    CPoint pVertex1( xyRect.left + 0.5*(float)(xyRect.getWidth()), xyRect.top );
    CPoint pVertex2( xyRect.right, xyRect.top + 0.5*(float)(xyRect.getHeight()) );
    CPoint pVertex3( xyRect.left + 0.5*(float)(xyRect.getWidth()), xyRect.bottom );
    CPoint pVertex4( xyRect.left, xyRect.top + 0.5*(float)(xyRect.getHeight()) );
    
    pointList.push_back(pVertex0);
    pointList.push_back(pVertex1);
    pointList.push_back(pVertex2);
    pointList.push_back(pVertex3);
    pointList.push_back(pVertex4);
    
    context->setLineWidth(2); // 2/win and 1/mac
    context->setFillColor(getBackColor());
    context->setFrameColor(getFrameColor());
    context->drawPolygon(pointList, kDrawFilledAndStroked);
    
    // --- now the axes(?)
    float centerX = xyRect.left + xyRect.getWidth()/2.0;
    float centerY = xyRect.top + xyRect.getHeight()/2.0;
    
    context->setLineWidth(1);
    const CPoint p1(centerX, xyRect.top);
    const CPoint p2(centerX, xyRect.bottom);
    const CPoint p3(xyRect.left, centerY);
    const CPoint p4(xyRect.right, centerY);
    context->drawLine(p1, p2);
    context->drawLine(p3, p4);
    
#else
    CPoint* pPoints = new CPoint[5];
    
    pPoints[0].x = xyRect.left;
    pPoints[0].y = xyRect.top + 0.5*(float)(xyRect.height());
    pPoints[1].x = xyRect.left + 0.5*(float)(xyRect.width());
    pPoints[1].y = xyRect.top;
    pPoints[2].x = xyRect.right;
    pPoints[2].y = xyRect.top + 0.5*(float)(xyRect.height());
    pPoints[3].x = xyRect.left + 0.5*(float)(xyRect.width());
    pPoints[3].y = xyRect.bottom;
    pPoints[4].x = xyRect.left;
    pPoints[4].y = xyRect.top + 0.5*(float)(xyRect.height());
    
    const CPoint* points = &pPoints[0];
    // --- draw polygon outline and fill
    context->setLineWidth(2); // 2/win and 1/mac
    context->setFillColor(getBackColor());
    context->setFrameColor(getFrameColor());
    context->drawPolygon(points, 5, kDrawFilledAndStroked);
    
    // --- now the axes(?)
    float centerX = xyRect.left + xyRect.width()/2.0;
    float centerY = xyRect.top + xyRect.height()/2.0;
    
    context->setLineWidth(1);
    const CPoint p1(centerX, xyRect.top);
    const CPoint p2(centerX, xyRect.bottom);
    const CPoint p3(xyRect.left, centerY);
    const CPoint p4(xyRect.right, centerY);
    
    context->moveTo(p1);
    context->lineTo(p2);
    context->moveTo(p3);
    context->lineTo(p4);
    delete pPoints;
    
#endif
    
	// --- this draws the puck
	float x, y;
	calculateXY(getValue(), x, y);

	CCoord width = getWidth() - getRoundRectRadius ();
	CCoord height = getHeight() - getRoundRectRadius ();
	CRect r(x*width, y*height, x*width, y*height);
	r.inset(-getRoundRectRadius()/2., -getRoundRectRadius()/2.);
	r.offset(getViewSize().left + getRoundRectRadius()/2., getViewSize().top + getRoundRectRadius()/2.);
	context->setFillColor(getFontColor());
	context->setDrawMode(kAntiAliasing);
	context->drawEllipse(r, kDrawFilled);
	setDirty(false);
}
}