/*================================================================================================*/
/*
 *
 *	Copyright 2013-2017 by Avid Technology, Inc.
 *	All rights reserved.
 *	
 *	CONFIDENTIAL: This document contains confidential information.  Do not 
 *	read or examine this document unless you are an Avid Technology employee
 *	or have signed a non-disclosure agreement with Avid Technology which protects
 *	the	confidentiality of this document.  DO NOT DISCLOSE ANY INFORMATION 
 *	CONTAINED IN THIS DOCUMENT TO ANY THIRD-PARTY WITHOUT THE PRIOR WRITTEN	CONSENT 
 *	OF Avid Technology, INC.
 *
 */

/**  
 *	\file  AAX_VViewContainer.h
 *
 *	\brief Version-managed concrete ViewContainer class
 *
 */ 
/*================================================================================================*/

#ifndef AAX_VVIEWCONTAINER_H
#define AAX_VVIEWCONTAINER_H

#include "AAX_IViewContainer.h"
#include "AAX_IACFViewContainer.h"
#include "ACFPtr.h"


class IACFUnknown;

/**
 *	\brief Version-managed concrete \ref AAX_IViewContainer class
 *
 */
class AAX_VViewContainer : public AAX_IViewContainer
{
public:
	AAX_VViewContainer( IACFUnknown * pUnknown );
	virtual ~AAX_VViewContainer();
	
    // AAX_IACFViewContainer
    
	// Getters
	virtual int32_t		GetType () AAX_OVERRIDE; ///< \copydoc AAX_IViewContainer::GetType()
    virtual void *		GetPtr () AAX_OVERRIDE; ///< \copydoc AAX_IViewContainer::GetPtr()
	virtual AAX_Result	GetModifiers ( uint32_t * outModifiers ) AAX_OVERRIDE; ///< \copydoc AAX_IViewContainer::GetModifiers()

	// Setters
    virtual AAX_Result	SetViewSize ( AAX_Point & inSize ) AAX_OVERRIDE; ///< \copydoc AAX_IViewContainer::SetViewSize()
	virtual AAX_Result	HandleParameterMouseDown ( AAX_CParamID inParamID, uint32_t inModifiers ) AAX_OVERRIDE; ///< \copydoc AAX_IViewContainer::HandleParameterMouseDown()
	virtual AAX_Result	HandleParameterMouseDrag ( AAX_CParamID inParamID, uint32_t inModifiers ) AAX_OVERRIDE; ///< \copydoc AAX_IViewContainer::HandleParameterMouseDrag()
	virtual AAX_Result	HandleParameterMouseUp   ( AAX_CParamID inParamID, uint32_t inModifiers ) AAX_OVERRIDE; ///< \copydoc AAX_IViewContainer::HandleParameterMouseUp()
	virtual AAX_Result	HandleMultipleParametersMouseDown ( const AAX_CParamID* inParamIDs, uint32_t inNumOfParams, uint32_t inModifiers ) AAX_OVERRIDE; ///< \copydoc AAX_IViewContainer::HandleMultipleParametersMouseDown()
	virtual AAX_Result	HandleMultipleParametersMouseDrag ( const AAX_CParamID* inParamIDs, uint32_t inNumOfParams, uint32_t inModifiers ) AAX_OVERRIDE; ///< \copydoc AAX_IViewContainer::HandleMultipleParametersMouseDrag()
	virtual AAX_Result	HandleMultipleParametersMouseUp   ( const AAX_CParamID* inParamIDs, uint32_t inNumOfParams, uint32_t inModifiers ) AAX_OVERRIDE; ///< \copydoc AAX_IViewContainer::HandleMultipleParametersMouseUp()
	
private:
	ACFPtr<AAX_IACFViewContainer>	    mIViewContainer;
	ACFPtr<AAX_IACFViewContainer_V2>	mIViewContainerV2;
};


#endif //AAX_IAUTOMATIONDELEGATE_H
