/*================================================================================================*/
/*
 *	Copyright 2013-2015 by Avid Technology, Inc.
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
/*================================================================================================*/

#include "AAX_VViewContainer.h"
#include "AAX_UIDs.h"

// ******************************************************************************************
// METHOD:	AAX_VViewContainer
// ******************************************************************************************
AAX_VViewContainer::AAX_VViewContainer( IACFUnknown* pUnknown )
{
	if ( pUnknown )
	{
		pUnknown->QueryInterface ( IID_IAAXViewContainerV1, (void **) &mIViewContainer );
		pUnknown->QueryInterface ( IID_IAAXViewContainerV2, (void **) &mIViewContainerV2 );
	}	
}

// ******************************************************************************************
// METHOD:	~AAX_VViewContainer
// ******************************************************************************************
AAX_VViewContainer::~AAX_VViewContainer()
{
}

// ******************************************************************************************
// METHOD:	GetType
// ******************************************************************************************
int32_t AAX_VViewContainer::GetType ()
{
	int32_t	result = AAX_eViewContainer_Type_NULL;
	
	if ( mIViewContainer )
		result = mIViewContainer->GetType ();
	
	return result;
}

// ******************************************************************************************
// METHOD:	GetPtr
// ******************************************************************************************
void * AAX_VViewContainer::GetPtr ()
{
	void *	result = 0;
	
	if ( mIViewContainer )
		result = mIViewContainer->GetPtr ();
	
	return result;
}

// ******************************************************************************************
// METHOD:	GetModifiers
// ******************************************************************************************
AAX_Result AAX_VViewContainer::GetModifiers ( uint32_t * outModifiers )
{
	AAX_Result	result = AAX_ERROR_UNIMPLEMENTED;
	
	if ( mIViewContainer )
		result = mIViewContainer->GetModifiers ( outModifiers );
	
	return result;
}

// ******************************************************************************************
// METHOD:	HandleParameterMouseDown
// ******************************************************************************************
AAX_Result AAX_VViewContainer::HandleParameterMouseDown ( AAX_CParamID inParamID, uint32_t inModifiers )
{
	AAX_Result	result = AAX_ERROR_UNIMPLEMENTED;
	
	if ( mIViewContainer )
		result = mIViewContainer->HandleParameterMouseDown ( inParamID, inModifiers );
	
	return result;
}

// ******************************************************************************************
// METHOD:	HandleParameterMouseDrag
// ******************************************************************************************
AAX_Result AAX_VViewContainer::HandleParameterMouseDrag ( AAX_CParamID inParamID, uint32_t inModifiers )
{
	AAX_Result	result = AAX_ERROR_UNIMPLEMENTED;
	
	if ( mIViewContainer )
		result = mIViewContainer->HandleParameterMouseDrag ( inParamID, inModifiers );
	
	return result;
}

// ******************************************************************************************
// METHOD:	HandleParameterMouseUp
// ******************************************************************************************
AAX_Result AAX_VViewContainer::HandleParameterMouseUp ( AAX_CParamID inParamID, uint32_t inModifiers )
{
	AAX_Result	result = AAX_ERROR_UNIMPLEMENTED;
	
	if ( mIViewContainer )
		result = mIViewContainer->HandleParameterMouseUp ( inParamID, inModifiers );
	
	return result;
}

// ******************************************************************************************
// METHOD:	SetViewSize
// ******************************************************************************************
AAX_Result AAX_VViewContainer::SetViewSize ( AAX_Point & inSize )
{
	AAX_Result	result = AAX_SUCCESS;
	
	if ( mIViewContainer )
		result = mIViewContainer->SetViewSize ( inSize );
	
	return result;
}

// ******************************************************************************************
// METHOD:	HandleMultipleParametersMouseDown
// ******************************************************************************************
AAX_Result AAX_VViewContainer::HandleMultipleParametersMouseDown ( const AAX_CParamID* inParamIDs, uint32_t iNumOfParams, uint32_t inModifiers )
{
	AAX_Result	result = AAX_ERROR_UNIMPLEMENTED;

	if ( mIViewContainerV2 )
		result = mIViewContainerV2->HandleMultipleParametersMouseDown ( inParamIDs, iNumOfParams, inModifiers );

	return result;
}

// ******************************************************************************************
// METHOD:	HandleMultipleParametersMouseDrag
// ******************************************************************************************
AAX_Result AAX_VViewContainer::HandleMultipleParametersMouseDrag ( const AAX_CParamID* inParamIDs, uint32_t iNumOfParams, uint32_t inModifiers )
{
	AAX_Result	result = AAX_ERROR_UNIMPLEMENTED;

	if ( mIViewContainerV2 )
		result = mIViewContainerV2->HandleMultipleParametersMouseDrag ( inParamIDs, iNumOfParams, inModifiers );

	return result;
}

// ******************************************************************************************
// METHOD:	HandleMultipleParametersMouseUp
// ******************************************************************************************
AAX_Result AAX_VViewContainer::HandleMultipleParametersMouseUp ( const AAX_CParamID* inParamIDs, uint32_t iNumOfParams, uint32_t inModifiers )
{
	AAX_Result	result = AAX_ERROR_UNIMPLEMENTED;

	if ( mIViewContainerV2 )
		result = mIViewContainerV2->HandleMultipleParametersMouseUp ( inParamIDs, iNumOfParams, inModifiers );

	return result;
}
