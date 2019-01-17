#include "RAFXSwitchView.h"

namespace VSTGUI {

CRAFXSwitchView::CRAFXSwitchView(const CRect& size)
: CViewContainer(size)
{
	m_nTabControllerTag = -1;
	animationTime = 120; // msec
	currentViewIndex = -1; // not set
}

CRAFXSwitchView::~CRAFXSwitchView(void)
{
	removeAll();
}
	
void  CRAFXSwitchView::hideCurrentView()
{
	CView* oldView = getView(currentViewIndex);	
	if(oldView)
		oldView->setVisible(false);
}

void CRAFXSwitchView::setCurrentViewIndex(int viewIndex)
{
	if(viewIndex >= (int)getNbViews()) 
		return;
	CView* view = getView(viewIndex);
    CView* oldView = currentViewIndex >= 0 ? getView(currentViewIndex) : NULL;
	if(oldView == view) 
		return;

	if(animationTime)
	{
		if(getFrame())
			getFrame()->getAnimator()->removeAnimation(this, "CRAFXSwitchView::setCurrentViewIndex");
		if(isAttached() && oldView && getFrame())
		{
			getFrame()->getAnimator()->addAnimation(this, "CRAFXSwitchView::setCurrentViewIndex", 
													new ViewVisibilityAnimation(oldView, view, ViewVisibilityAnimation::kAlphaValueFade), 
													new Animation::LinearTimingFunction(animationTime));
		}
		else
		{
			if(oldView)
				oldView->setVisible(false);
			view->setVisible(true);
		}
	}
	else
	{
		if(oldView)
			oldView->setVisible(false);
		view->setVisible(true);
	}

	currentViewIndex = viewIndex;
	invalid ();
}


ViewVisibilityAnimation::ViewVisibilityAnimation(CView* oldView, CView* newView, AnimationStyle style)
: newView(newView)
, oldView(oldView)
, style(style)
{
	newViewValueEnd = 1.0;//newView->getAlphaValue();
	newView->setAlphaValue(0.f);
}

//-----------------------------------------------------------------------------
ViewVisibilityAnimation::~ViewVisibilityAnimation()
{
}

//-----------------------------------------------------------------------------
void ViewVisibilityAnimation::animationStart(CView* view, IdStringPtr name)
{
	newView->setVisible(true);
	if(style == kAlphaValueFade)
	{
		oldViewValueStart = oldView->getAlphaValue();
	}
	else
	{
		newView->setAlphaValue(newViewValueEnd);
	}
}

//-----------------------------------------------------------------------------
void ViewVisibilityAnimation::animationTick (CView* view, IdStringPtr name, float pos)
{
	switch(style)
	{
		case kAlphaValueFade:
		{
			float alpha = oldViewValueStart - (oldViewValueStart * pos);
			oldView->setAlphaValue(alpha);
			alpha = newViewValueEnd * pos;
			newView->setAlphaValue(alpha);
			break;
		}
		case kPushInFromLeft:
		{
			CRect viewSize (newView->getViewSize());
			CCoord leftOrigin = oldView->getViewSize().left;
			CCoord offset = viewSize.getWidth() * (1.f - pos);
			viewSize.offset(-viewSize.left, 0);
			viewSize.offset(leftOrigin - offset, 0);
			newView->invalid();
			newView->setViewSize(viewSize);
			newView->setMouseableArea(viewSize);
			newView->invalid();
			break;
		}
		case kPushInFromRight:
		{
			CRect viewSize (newView->getViewSize());
			CCoord rightOrigin = oldView->getViewSize().left + oldView->getViewSize ().getWidth ();
			CCoord offset = viewSize.getWidth () * pos;
			viewSize.offset(-viewSize.left, 0);
			viewSize.offset(rightOrigin - offset, 0);
			newView->invalid();
			newView->setViewSize(viewSize);
			newView->setMouseableArea(viewSize);
			newView->invalid();
			break;
		}
		case kPushInFromTop:
		{
			CRect viewSize (newView->getViewSize());
			CCoord topOrigin = oldView->getViewSize().top;
			CCoord offset = viewSize.getHeight() * (1.f - pos);
			viewSize.offset(0, -viewSize.top);
			viewSize.offset(0, topOrigin - offset);
			newView->invalid();
			newView->setViewSize(viewSize);
			newView->setMouseableArea(viewSize);
			newView->invalid();
			break;
		}
		case kPushInFromBottom:
		{
			CRect viewSize (newView->getViewSize ());
			CCoord bottomOrigin = oldView->getViewSize().top + oldView->getViewSize ().getHeight ();
			CCoord offset = viewSize.getHeight () * pos;
			viewSize.offset(0, -viewSize.top);
			viewSize.offset(0, bottomOrigin - offset);
			newView->invalid();
			newView->setViewSize(viewSize);
			newView->setMouseableArea(viewSize);
			newView->invalid();
			break;
		}
	}
}

//-----------------------------------------------------------------------------
void ViewVisibilityAnimation::animationFinished(CView* view, IdStringPtr name, bool wasCanceled)
{
	if(wasCanceled)
	{
		animationTick(0, 0, 1.f);
	}
	newView->invalid();
	oldView->setVisible(false);
}


}