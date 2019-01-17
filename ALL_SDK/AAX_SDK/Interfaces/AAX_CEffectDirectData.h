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
 *	\file AAX_CEffectDirectData.h
 *
 *	\brief A default implementation of the AAX_IEffectDirectData interface.
 *
 */ 
/*================================================================================================*/

#pragma once
#ifndef AAX_CEFFECTDIRECTDATA_H
#define AAX_CEFFECTDIRECTDATA_H

#include "AAX_IEffectDirectData.h"



class AAX_IPrivateDataAccess;
class AAX_IEffectParameters;
class AAX_IController;



/** @brief	Default implementation of the AAX_IEffectDirectData interface.
	
	@details
	This class provides a default implementation of the AAX_IEffectDirectData interface.
	
	\ingroup AuxInterface_DirectData
*/
class AAX_CEffectDirectData : public AAX_IEffectDirectData
{
public: ///////////////////////////////////////////////////////////////////////////// AAX_CEffectDirectData

	AAX_CEffectDirectData(
		void);
	
	virtual
	~AAX_CEffectDirectData(
		void);

public: ///////////////////////////////////////////////////////////////////////////// AAX_IEffectDirectData

	/** @name Initialization and uninitialization
	 */
	//@{
	/*!	\brief Non-virtual implementation of AAX_IEfectDirectData::Initialize()
	 *
	 *	This implementation initializes all private AAX_CEffectDirectData
	 *	members and calls Initialize_PrivateDataAccess().  For custom
	 *	initialization, inherited classes should override 
	 *	Initialize_PrivateDataAccess().
	 *
	 *	\param[in] iController
	 *		Unknown pointer that resolves to an AAX_IController.
	 *
	 */
	AAX_Result Initialize (IACFUnknown * iController ) AAX_OVERRIDE AAX_FINAL;
	virtual	AAX_Result Uninitialize (void) AAX_OVERRIDE;
	//@}end Initialization and uninitialization
	
	/** @name Data update callbacks
	 *
	 */
	//@{
	/*!	\brief Non-virtual implementation of AAX_IEfectDirectData::TimerWakeup()
	 *
	 *	This implementation interprets the IACFUnknown and forwards
	 *	the resulting AAX_IPrivateDataAccess to \ref TimerWakeup_PrivateDataAccess()
	 *
	 *	\param[in] iDataAccessInterface
	 *		Unknown pointer that resolves to an AAX_IPrivateDataAccess.  This
	 *		interface is only valid for the duration of this method's execution
	 *		and is discarded when the method returns.
	 *
	 */
	AAX_Result TimerWakeup (IACFUnknown * iDataAccessInterface ) AAX_OVERRIDE;
	//@}end Data update callbacks
	
public: ///////////////////////////////////////////////////////////////////////////// AAX_CEffectDirectData

	/** @name Private member accessors
	 */
	//@{
	/*!
	 *  \brief Returns a pointer to the plug-in's controller interface
	 *
	 *	\todo Change to GetController to match other AAX_CEffect modules
	 */
	AAX_IController* Controller (void);
	/*!
	 *  \brief Returns a pointer to the plug-in's data model interface
	 *
	 *	\todo Change to GetController to match other AAX_CEffect modules
	 */
	AAX_IEffectParameters* EffectParameters (void);
	//@}end Private member accessors

protected: ///////////////////////////////////////////////////////////////////////////// AAX_CEffectDirectData
	
	/** @name AAX_CEffectDirectData virtual interface
	 */
	//@{
	/*!
	 *	\brief Initialization routine for classes that inherit from AAX_CEffectDirectData.
	 *	This method is called by the default Initialize() implementation after all
	 *	internal members have been initialized, and provides a safe location in which to
	 *	perform any additional initialization tasks.
	 *
	 */
	virtual AAX_Result Initialize_PrivateDataAccess();
	/*!
	 *	\brief Callback provided with an AAX_IPrivateDataAccess.  Override this method
	 *	to access the algorithm's private data using the AAX_IPrivateDataAccess interface.
	 *
	 *	\param[in] iPrivateDataAccess
	 *		Pointer to an AAX_IPrivateDataAccess interface.  This interface is only
	 *		valid for the duration of this method.
	 */
	virtual AAX_Result TimerWakeup_PrivateDataAccess(AAX_IPrivateDataAccess* iPrivateDataAccess);
	//@}end AAX_CEffectDirectData virtual interface
	
private:
	AAX_IController*							mController;
	AAX_IEffectParameters*						mEffectParameters;
};


#endif // AAX_CEFFECTDIRECTDATA_H
