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
 *	\file  AAX_VComponentDescriptor.h
 *
 *	\brief Version-managed concrete ComponentDescriptor class
 *
 */ 
/*================================================================================================*/

#ifndef AAX_VCOMPONENTDESCRIPTOR_H
#define AAX_VCOMPONENTDESCRIPTOR_H

// AAX Includes
#include "AAX_IComponentDescriptor.h"
#include "AAX_IDma.h"
#include "AAX_IACFComponentDescriptor.h"

// ACF Includes
#include "acfunknown.h"
#include "ACFPtr.h"

// Standard Includes
#include <set>


class AAX_IPropertyMap;
class AAX_IACFComponentDescriptor;
class AAX_IACFComponentDescriptorV2;
class IACFUnknown;

/**
 *	\brief Version-managed concrete \ref AAX_IComponentDescriptor class
 *
 */
class AAX_VComponentDescriptor : public AAX_IComponentDescriptor
{
public:
	AAX_VComponentDescriptor ( IACFUnknown * pUnkHost );
	virtual ~AAX_VComponentDescriptor ();
	
	virtual AAX_Result			Clear () AAX_OVERRIDE; ///< \copydoc AAX_IComponentDescriptor::Clear()
	virtual AAX_Result			AddReservedField ( AAX_CFieldIndex inFieldIndex, uint32_t inFieldType ) AAX_OVERRIDE; ///< \copydoc AAX_IComponentDescriptor::AddReservedField()
	virtual AAX_Result			AddAudioIn ( AAX_CFieldIndex inFieldIndex ) AAX_OVERRIDE; ///< \copydoc AAX_IComponentDescriptor::AddAudioIn()
	virtual AAX_Result			AddAudioOut ( AAX_CFieldIndex inFieldIndex ) AAX_OVERRIDE; ///< \copydoc AAX_IComponentDescriptor::AddAudioOut()
	virtual AAX_Result			AddAudioBufferLength ( AAX_CFieldIndex inFieldIndex ) AAX_OVERRIDE; ///< \copydoc AAX_IComponentDescriptor::AddAudioBufferLength()
	virtual AAX_Result			AddSampleRate ( AAX_CFieldIndex inFieldIndex ) AAX_OVERRIDE; ///< \copydoc AAX_IComponentDescriptor::AddSampleRate()
	virtual AAX_Result			AddClock ( AAX_CFieldIndex inFieldIndex ) AAX_OVERRIDE; ///< \copydoc AAX_IComponentDescriptor::AddClock()
	virtual AAX_Result			AddSideChainIn ( AAX_CFieldIndex inFieldIndex ) AAX_OVERRIDE; ///< \copydoc AAX_IComponentDescriptor::AddSideChainIn()

	virtual AAX_Result			AddDataInPort ( AAX_CFieldIndex inFieldIndex, uint32_t inPacketSize, AAX_EDataInPortType inPortType ) AAX_OVERRIDE; ///< \copydoc AAX_IComponentDescriptor::AddDataInPort()
	virtual AAX_Result			AddAuxOutputStem ( AAX_CFieldIndex inFieldIndex, int32_t inStemFormat, const char inNameUTF8[]) AAX_OVERRIDE; ///< \copydoc AAX_IComponentDescriptor::AddAuxOutputStem()
	virtual AAX_Result			AddPrivateData ( AAX_CFieldIndex inFieldIndex, int32_t inDataSize, uint32_t inOptions ) AAX_OVERRIDE; ///< \copydoc AAX_IComponentDescriptor::AddPrivateData()
    virtual AAX_Result          AddTemporaryData( AAX_CFieldIndex inFieldIndex, uint32_t inDataElementSize) AAX_OVERRIDE; ///< \copydoc AAX_IComponentDescriptor::AddTemporaryData()
	virtual AAX_Result			AddDmaInstance ( AAX_CFieldIndex inFieldIndex, AAX_IDma::EMode inDmaMode ) AAX_OVERRIDE; ///< \copydoc AAX_IComponentDescriptor::AddDmaInstance()
	virtual AAX_Result			AddMeters ( AAX_CFieldIndex inFieldIndex, const AAX_CTypeID* inMeterIDs, const uint32_t inMeterCount) AAX_OVERRIDE; ///< \copydoc AAX_IComponentDescriptor::AddMeters()
	virtual AAX_Result			AddMIDINode ( AAX_CFieldIndex inFieldIndex, AAX_EMIDINodeType inNodeType, const char inNodeName[], uint32_t channelMask ) AAX_OVERRIDE; ///< \copydoc AAX_IComponentDescriptor::AddMIDINode()


	/** \copydoc AAX_IComponentDescriptor::NewPropertyMap()
	 *	
	 *	This implementation retains each generated \ref AAX_IPropertyMap and destroys the property map upon \ref AAX_VComponentDescriptor destruction
	 */
	virtual AAX_IPropertyMap *	NewPropertyMap () const AAX_OVERRIDE;
	/** \copydoc AAX_IComponentDescriptor::DuplicatePropertyMap()
	 *	
	 *	This implementation retains each generated \ref AAX_IPropertyMap and destroys the property map upon \ref AAX_VComponentDescriptor destruction
	 */
	virtual AAX_IPropertyMap *	DuplicatePropertyMap (AAX_IPropertyMap* inPropertyMap) const AAX_OVERRIDE;
	/** \copydoc AAX_IComponentDescriptor::AddProcessProc_Native()
	 */
	virtual AAX_Result			AddProcessProc_Native (
		AAX_CProcessProc inProcessProc, 
		AAX_IPropertyMap * inProperties = NULL, 
		AAX_CInstanceInitProc inInstanceInitProc = NULL, 
		AAX_CBackgroundProc inBackgroundProc = NULL, 
		AAX_CSelector * outProcID = NULL ) AAX_OVERRIDE;
	/** \copydoc AAX_IComponentDescriptor::AddProcessProc_TI()
	 */
	virtual AAX_Result			AddProcessProc_TI ( 
		const char inDLLFileNameUTF8[], 
		const char inProcessProcSymbol[], 
		AAX_IPropertyMap * inProperties = NULL,  
		const char	inInstanceInitProcSymbol [] = NULL, 
		const char	inBackgroundProcSymbol [] = NULL, 
		AAX_CSelector * outProcID = NULL ) AAX_OVERRIDE;
	/** \copydoc AAX_IComponentDescriptor::AddProcessProc()
	 */
	virtual AAX_Result AddProcessProc (
		AAX_IPropertyMap* inProperties,
		AAX_CSelector* outProcIDs = NULL,
		int32_t inProcIDsSize = 0) AAX_OVERRIDE;


	IACFUnknown*				GetIUnknown(void) const;

private:
	// Used for backwards compatibility with clients which do not support AddProcessProc
	friend class AAX_VPropertyMap;
	static const std::set<AAX_EProperty>& PointerPropertiesUsedByAddProcessProc();
	
private:
	ACFPtr<IACFUnknown>					mUnkHost;
	ACFPtr<AAX_IACFComponentDescriptor>	mIACFComponentDescriptor;
    ACFPtr<AAX_IACFComponentDescriptor_V2> mIACFComponentDescriptorV2;
    ACFPtr<AAX_IACFComponentDescriptor_V3> mIACFComponentDescriptorV3;
	std::set<AAX_IPropertyMap *>		mPropertyMaps;	
};


#endif // #ifndef _AAX_ICOMPONENTDESCRIPTOR_H_
