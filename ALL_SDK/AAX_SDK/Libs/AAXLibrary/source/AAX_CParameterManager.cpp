/*================================================================================================*/
/*
 *	Copyright 2009-2015, 2018 by Avid Technology, Inc.
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
 *  \author David Tremblay
 *
 */
/*================================================================================================*/

#include "AAX_CParameterManager.h"
#include "AAX_IAutomationDelegate.h"
#include "AAX_CParameter.h"
#include "AAX_Assert.h"


AAX_CParameterManager::AAX_CParameterManager()	:
	mParameters(),
    mParametersMap(),
	mAutomationDelegate(NULL)
{
	
}

AAX_CParameterManager::~AAX_CParameterManager()
{
	RemoveAllParameters();
}

void AAX_CParameterManager::Initialize(AAX_IAutomationDelegate* inAutomationDelegate)
{
	mAutomationDelegate = inAutomationDelegate;
}

int32_t	AAX_CParameterManager::NumParameters()	const
{
	return int32_t(mParameters.size());
}

void		AAX_CParameterManager::AddParameter(AAX_IParameter*	param)
{
	//If the parameter is null, just return.
	if (param == 0)
	{
		AAX_TRACE(kAAX_Trace_Priority_Normal, "AAX_CParameterManager::AddParameter() - Attempt to add a null parameter into AAX_CParameterManager");
		return;
	}
	
	//Make sure that unique identifier is not already being used.  
	if (GetParameterByID(param->Identifier()) != 0)
	{
		AAX_TRACE(kAAX_Trace_Priority_Normal, "AAX_CParameterManager::AddParameter() - Duplicate AAX_IParameter ID Inserted into AAX_CParameterManager");
		return;
	}
	
	//Setup the automation delegate.
	param->SetAutomationDelegate( mAutomationDelegate );
	
	//Add the parameter.
	mParameters.push_back(param);
    mParametersMap.insert(std::map<AAX_CParamID, AAX_IParameter*>::value_type(param->Identifier(), param));
}

void		AAX_CParameterManager::RemoveParameterByID(AAX_CParamID identifier)
{
	int32_t parameterIndex = GetParameterIndex(identifier);
	if (parameterIndex < 0 || parameterIndex >= NumParameters())
		return;

	AAX_IParameter* param = mParameters[static_cast<size_t>(parameterIndex)];			
	mParameters.erase(mParameters.begin() + parameterIndex);		//remove it from the vector.
	mParametersMap.erase(identifier);
    delete param;													//make sure to actually delete it.
}

void		AAX_CParameterManager::RemoveParameter(AAX_IParameter*	param)
{
	if (param)
		RemoveParameterByID(param->Identifier());
}

void		AAX_CParameterManager::RemoveAllParameters()
{
	int32_t numParameters = NumParameters();
	for(int32_t index=0; index < numParameters; index++)
	{
		delete mParameters[static_cast<size_t>(index)];
	}
	mParameters.clear();
    mParametersMap.clear();
}

AAX_IParameter*		AAX_CParameterManager::GetParameterByID(AAX_CParamID identifier)
{
	if ( identifier )
	{
		std::map<std::string, AAX_IParameter*>::const_iterator iter = mParametersMap.find(identifier);
		if ( iter != mParametersMap.end() )
			return iter->second;
	}

    return NULL;
}

const AAX_IParameter*		AAX_CParameterManager::GetParameterByID(AAX_CParamID identifier) const
{
	if ( identifier )
	{
		std::map<std::string, AAX_IParameter*>::const_iterator iter = mParametersMap.find(identifier);
		if ( iter != mParametersMap.end() )
			return iter->second;
	}

    return NULL;
}

AAX_IParameter*			AAX_CParameterManager::GetParameterByName(const char*  name)
{
	return const_cast<AAX_IParameter*>(const_cast<const AAX_CParameterManager*>(this)->GetParameterByName(name));
}

const AAX_IParameter*	AAX_CParameterManager::GetParameterByName(const char*  name) const
{
	AAX_IParameter* foundParam = 0;
	for (std::vector<AAX_IParameter*>::const_iterator iter = mParameters.begin(); (0 == foundParam) && (iter != mParameters.end()); ++iter)
	{
		if ((*iter) && (AAX_CString(name) == (*iter)->Name()))
			foundParam = *iter;
	}
	return foundParam;
}

AAX_IParameter*			AAX_CParameterManager::GetParameter(int32_t index)
{
	if (index < 0 || index >= NumParameters())
		return 0;

	AAX_IParameter* param = mParameters[static_cast<size_t>(index)];
	return param;
}

const AAX_IParameter*	AAX_CParameterManager::GetParameter(int32_t index) const
{
	if (index < 0 || index >= NumParameters())
		return 0;

	AAX_IParameter* param = mParameters[static_cast<size_t>(index)];
	return param;
}

int32_t	AAX_CParameterManager::GetParameterIndex(AAX_CParamID identifier) const
{
	if (identifier == 0)
		return -1;
	
	int32_t numParameters = NumParameters();
	for (int32_t i=0; i < numParameters; i++)
	{
		AAX_IParameter* param = mParameters[static_cast<size_t>(i)];
		if (strcmp(param->Identifier(), identifier) == 0)
			return i;		
	}
	return -1;
}




