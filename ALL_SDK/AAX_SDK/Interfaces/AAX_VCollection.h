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
 *	\file  AAX_VCollection.h
 *
 *	\brief Version-managed concrete Collection  class
 *
 */ 
/*================================================================================================*/

#ifndef AAX_VCOLLECTION_H
#define AAX_VCOLLECTION_H

#include "AAX.h"
#include "AAX_ICollection.h"
#include "AAX_IACFCollection.h"
#include "AAX_VDescriptionHost.h"
#include "acfunknown.h"
#include "ACFPtr.h"
#include <set>

class IACFUnknown;
class IACFPluginDefinition;
class AAX_IACFCollection;
class AAX_IEffectDescriptor;

/**
 *	\brief Version-managed concrete \ref AAX_ICollection class
 *
 */
class AAX_VCollection : public AAX_ICollection
{
public:
	AAX_VCollection (IACFUnknown * pUnkHost);
	virtual ~AAX_VCollection ();
	
	/** \copydoc AAX_ICollection::NewDescriptor()
	 *	
	 *	This implementation retains each generated \ref AAX_IEffectDescriptor and destroys the descriptor upon AAX_VCollection destruction
	 */
	virtual AAX_IEffectDescriptor *	NewDescriptor () AAX_OVERRIDE; ///< \copydoc AAX_ICollection::NewDescriptor()
	virtual AAX_Result				AddEffect ( const char * inEffectID, AAX_IEffectDescriptor * inEffectDescriptor ) AAX_OVERRIDE; ///< \copydoc AAX_ICollection::AddEffect()
	virtual AAX_Result				SetManufacturerName( const char* inPackageName ) AAX_OVERRIDE; ///< \copydoc AAX_ICollection::SetManufacturerName()
	virtual AAX_Result				AddPackageName( const char *inPackageName ) AAX_OVERRIDE; ///< \copydoc AAX_ICollection::AddPackageName()
	virtual AAX_Result				SetPackageVersion( uint32_t inVersion ) AAX_OVERRIDE; ///< \copydoc AAX_ICollection::SetPackageVersion()
	virtual AAX_IPropertyMap *		NewPropertyMap () AAX_OVERRIDE; ///< \copydoc AAX_ICollection::NewPropertyMap()
	virtual AAX_Result				SetProperties ( AAX_IPropertyMap * inProperties ) AAX_OVERRIDE; ///< \copydoc AAX_ICollection::SetProperties()
	
	virtual AAX_IDescriptionHost* DescriptionHost() AAX_OVERRIDE; ///< \copydoc AAX_ICollection::DescriptionHost()
	virtual const AAX_IDescriptionHost* DescriptionHost() const AAX_OVERRIDE; ///< \copydoc AAX_ICollection::DescriptionHost() const
	virtual IACFDefinition* HostDefinition() const AAX_OVERRIDE; ///< \copydoc AAX_ICollection::HostDefinition() const
	
	IACFPluginDefinition*			GetIUnknown(void) const;

private:
	ACFPtr<IACFUnknown>					mUnkHost;
	ACFPtr<AAX_IACFCollection>			mIACFCollection;
	AAX_VDescriptionHost				mDescriptionHost;
	std::set<AAX_IEffectDescriptor *>	mEffectDescriptors;
    std::set<AAX_IPropertyMap *>		mPropertyMaps;	
};

#endif
