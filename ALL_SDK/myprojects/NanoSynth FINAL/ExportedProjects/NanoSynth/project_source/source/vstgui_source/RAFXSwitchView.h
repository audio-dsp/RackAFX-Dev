#ifndef __crafxswitchview__
#define __crafxswitchview__

#include "vstgui/vstgui.h"
#include "vstgui/lib/animation/animator.h"
#include "vstgui4constants.h"

#include <vector>
using namespace std;
using namespace Animation;

namespace VSTGUI {
  
/* ------------------------------------------------------
 CRAFXSwitchView
 Custom VSTGUI Object by Will Pirkle
 Created with RackAFX(TM) Plugin Development Software
 www.willpirkle.com
 -------------------------------------------------------*/

class CRAFXSwitchView : public CViewContainer
{
public:
	CRAFXSwitchView(const CRect& size);

	void addSubViewContainer(CViewContainer* pVC)
	{
		CRect viewSize = pVC->getViewSize();
        
#ifdef VSTGUI_43
        const CPoint size(viewSize.getWidth(), viewSize.getHeight());
#else
        const CPoint size(viewSize.width(), viewSize.height());
#endif
		const CPoint origin(0,0);

		CRect newRect(origin, size);
		pVC->setViewSize(newRect);
		pVC->setMouseableArea(newRect);

		// --- add and hide
		addView(pVC);
		pVC->setVisible(false);
	}

	void setCurrentViewIndex(int nViewIndex);
	int getCurrentViewIndex(){return currentViewIndex;}

	int getAnimationTime(){return animationTime;}
	void setAnimationTime(int nAnimationTime){animationTime = nAnimationTime;}

	void setControllerTag(int n){m_nTabControllerTag = n;}
	int getControllerTag(){return m_nTabControllerTag;}

	// for GUI designer only
	void hideCurrentView();

protected:
	virtual ~CRAFXSwitchView(void);
	int32_t animationTime;
	int32_t currentViewIndex;
	int m_nTabControllerTag;
};


class ViewVisibilityAnimation : public IAnimationTarget, public CBaseObject
{
public:
	enum AnimationStyle {
		kAlphaValueFade = 0,
		kPushInFromLeft,
		kPushInFromRight,
		kPushInFromTop,
		kPushInFromBottom
	};

	ViewVisibilityAnimation (CView* oldView, CView* newView, AnimationStyle style = kAlphaValueFade);
	~ViewVisibilityAnimation ();

	void animationStart (CView* view, IdStringPtr name) override;
	void animationTick (CView* view, IdStringPtr name, float pos) override;
	void animationFinished (CView* view, IdStringPtr name, bool wasCanceled) override;
protected:
	CView* newView;
	CView* oldView;
	AnimationStyle style;
	float newViewValueEnd;
	float oldViewValueStart;
};


}

#endif
