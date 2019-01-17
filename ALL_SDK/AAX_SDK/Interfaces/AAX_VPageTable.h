/*================================================================================================*/
/*
 *	Copyright 2016-2017 by Avid Technology, Inc.
 *	All rights reserved.
 *
 *	CONFIDENTIAL: This document contains confidential information.  Do not
 *	read or examine this document unless you are an Avid Technology employee
 *	or have signed a non-disclosure agreement with Avid Technology which protects
 *	the	confidentiality of this document.  DO NOT DISCLOSE ANY INFORMATION
 *	CONTAINED IN THIS DOCUMENT TO ANY THIRD-PARTY WITHOUT THE PRIOR WRITTEN	CONSENT
 *	OF Avid Technology, INC.
 */

#ifndef AAXLibrary_AAX_VPageTable_h
#define AAXLibrary_AAX_VPageTable_h

#include "AAX_IPageTable.h"
#include "AAX_IACFPageTable.h"
#include "ACFPtr.h"

/**
 *	\brief Version-managed concrete \ref AAX_IPageTable class
 *
 */
class AAX_VPageTable : public AAX_IPageTable
{
public:
	AAX_VPageTable( IACFUnknown* pUnknown );
	virtual ~AAX_VPageTable();
	
	// AAX_IACFPageTable
	virtual AAX_Result Clear() AAX_OVERRIDE; ///< \copydoc AAX_IPageTable::Clear()
	virtual AAX_Result Empty(AAX_CBoolean& oEmpty) const AAX_OVERRIDE; ///< \copydoc AAX_IPageTable::Empty()
	virtual AAX_Result GetNumPages(int32_t& oNumPages) const AAX_OVERRIDE; ///< \copydoc AAX_IPageTable::GetNumPages()
	virtual AAX_Result InsertPage(int32_t iPage) AAX_OVERRIDE; ///< \copydoc AAX_IPageTable::InsertPage()
	virtual AAX_Result RemovePage(int32_t iPage) AAX_OVERRIDE; ///< \copydoc AAX_IPageTable::RemovePage()
	virtual AAX_Result GetNumMappedParameterIDs(int32_t iPage, int32_t& oNumParameterIdentifiers) const AAX_OVERRIDE; ///< \copydoc AAX_IPageTable::GetNumMappedParameterIDs()
	virtual AAX_Result ClearMappedParameter(int32_t iPage, int32_t iIndex) AAX_OVERRIDE; ///< \copydoc AAX_IPageTable::ClearMappedParameter()
	virtual AAX_Result GetMappedParameterID(int32_t iPage, int32_t iIndex, AAX_IString& oParameterIdentifier) const AAX_OVERRIDE; ///< \copydoc AAX_IPageTable::GetMappedParameterID()
	virtual AAX_Result MapParameterID(AAX_CParamID iParameterIdentifier, int32_t iPage, int32_t iIndex) AAX_OVERRIDE; ///< \copydoc AAX_IPageTable::MapParameterID()
	
	// AAX_IACFPageTable_V2
	virtual AAX_Result GetNumParametersWithNameVariations(int32_t& oNumParameterIdentifiers) const AAX_OVERRIDE; ///< \copydoc AAX_IPageTable::GetNumParametersWithNameVariations()
	virtual AAX_Result GetNameVariationParameterIDAtIndex(int32_t iIndex, AAX_IString& oParameterIdentifier) const AAX_OVERRIDE; ///< \copydoc AAX_IPageTable::GetNameVariationParameterIDAtIndex()
	virtual AAX_Result GetNumNameVariationsForParameter(AAX_CPageTableParamID iParameterIdentifier, int32_t& oNumVariations) const AAX_OVERRIDE; ///< \copydoc AAX_IPageTable::GetNumNameVariationsForParameter()
	virtual AAX_Result GetParameterNameVariationAtIndex(AAX_CPageTableParamID iParameterIdentifier, int32_t iIndex, AAX_IString& oNameVariation, int32_t& oLength) const AAX_OVERRIDE; ///< \copydoc AAX_IPageTable::GetParameterNameVariationAtIndex()
	virtual AAX_Result GetParameterNameVariationOfLength(AAX_CPageTableParamID iParameterIdentifier, int32_t iLength, AAX_IString& oNameVariation) const AAX_OVERRIDE; ///< \copydoc AAX_IPageTable::GetParameterNameVariationOfLength()
	virtual AAX_Result ClearParameterNameVariations() AAX_OVERRIDE; ///< \copydoc AAX_IPageTable::ClearParameterNameVariations()
	virtual AAX_Result ClearNameVariationsForParameter(AAX_CPageTableParamID iParameterIdentifier) AAX_OVERRIDE; ///< \copydoc AAX_IPageTable::ClearNameVariationsForParameter()
	virtual AAX_Result SetParameterNameVariation(AAX_CPageTableParamID iParameterIdentifier, const AAX_IString& iNameVariation, int32_t iLength) AAX_OVERRIDE; ///< \copydoc AAX_IPageTable::SetParameterNameVariation()
	
	// AAX_VPageTable
	
	/** Returns the latest supported versioned ACF interface (e.g. an \ref AAX_IACFPageTable) which
	 is wrapped by this \ref AAX_IPageTable
	 */
	const IACFUnknown* AsUnknown() const
	{
		return mIPageTable.inArg();
	}
	
	/** \copydoc AAX_VPageTable::AsUnknown() const
	 */
	IACFUnknown* AsUnknown()
	{
		return mIPageTable.inArg();
	}
	
	bool IsSupported() const { return !mIPageTable.isNull(); }
	
private:
	ACFPtr<AAX_IACFPageTable>		mIPageTable;
	ACFPtr<AAX_IACFPageTable_V2>	mIPageTable2;
};

#endif
