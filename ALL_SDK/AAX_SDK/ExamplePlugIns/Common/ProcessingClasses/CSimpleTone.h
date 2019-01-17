/*================================================================================================*/
/*
 *	Copyright 2014-2015, 2018 by Avid Technology, Inc.
 *	All rights reserved.
 *
 *	CONFIDENTIAL: This document contains confidential information.  Do not 
 *	read or examine this document unless you are an Avid Technology employee
 *	or have signed a non-disclosure agreement with Avid Technology which protects
 *	the	confidentiality of this document.  DO NOT DISCLOSE ANY INFORMATION 
 *	CONTAINED IN THIS DOCUMENT TO ANY THIRD-PARTY WITHOUT THE PRIOR WRITTEN	CONSENT 
 *	OF Avid Technology, INC.
 */

/**  
 *	\file   CSimpleTone.h
 *
 *  \author Rob Majors
 */ 
/*================================================================================================*/

#ifndef __DemoMIDI_Synth__CSimpleTone__
#define __DemoMIDI_Synth__CSimpleTone__

// C99 Includes
#include <stdint.h>


class ISimpleTone;
class IToneGeneratorDelegate;


// *******************************************************************************
// IToneGeneratorDelegate
// Pure virtual base class
// *******************************************************************************
class IToneGeneratorDelegate
{
public:
	virtual ~IToneGeneratorDelegate() {}
	virtual float Sample(const ISimpleTone& inDelegator) const = 0;
};


// *******************************************************************************
// ISimpleTone
// Pure virtual base class
// *******************************************************************************
class ISimpleTone
{
public:
	virtual ~ISimpleTone() {}
	
	virtual bool IsEnabled() const = 0;
	virtual float Amplitude() const = 0;
	virtual float Hz() const = 0;
	virtual float Tuning() const = 0;
	virtual float CurPos() const = 0; // Range is [0.0, 2.0]
	virtual void SetMIDINote(unsigned char inMIDINoteVal, unsigned char inMIDIVelocity) = 0; // Use velocity 0 to disable
	virtual void SetFrequency(float inHz) = 0;
	virtual void SetTuning(float inAHz) = 0;
	virtual float ProcessOneSample(const IToneGeneratorDelegate& inDelegate) = 0;
};


// *******************************************************************************
// CSimpleTone
// *******************************************************************************
class CSimpleTone : public ISimpleTone
{
public:
	virtual ~CSimpleTone() {}
	
	CSimpleTone();
	CSimpleTone(int32_t inSampleRateHz, float inAHz);
	CSimpleTone(unsigned char inMIDINoteVal, unsigned char inMIDIVelocity, int32_t inSampleRateHz, float inAHz);
	
	virtual bool IsEnabled() const;
	virtual float Amplitude() const { return mAmplitude; }
	virtual float Hz() const { return mHz; }
	virtual float Tuning() const { return mAHz; }
	virtual float CurPos() const { return mCurPos; }
	virtual void SetMIDINote(unsigned char inMIDINoteVal, unsigned char inMIDIVelocity);
	virtual void SetFrequency(float inHz);
	virtual void SetAmplitude(float inAmplitude); // [0,1]
	virtual void SetTuning(float inAHz);
	virtual float ProcessOneSample(const IToneGeneratorDelegate& inDelegate);
	
private:
	int32_t mSampleRateHz;
	unsigned char mMIDINoteVal;
	float mCurPos;
	float mAHz;
	float mHz;
	float mAmplitude;
};


// *******************************************************************************
// CSawToneGeneratorDelegate
// *******************************************************************************

class CSawToneGeneratorDelegate : public IToneGeneratorDelegate
{
public:
	virtual ~CSawToneGeneratorDelegate() {}
	CSawToneGeneratorDelegate() {}
	
	virtual float Sample(const ISimpleTone& inDelegator) const;
};


// *******************************************************************************
// CTriangleToneGeneratorDelegate
// *******************************************************************************

class CTriangleToneGeneratorDelegate : public IToneGeneratorDelegate
{
public:
	virtual ~CTriangleToneGeneratorDelegate() {}
	CTriangleToneGeneratorDelegate() {}
	
	virtual float Sample(const ISimpleTone& inDelegator) const;
};


// *******************************************************************************
// CSquareToneGeneratorDelegate
// *******************************************************************************

class CSquareToneGeneratorDelegate : public IToneGeneratorDelegate
{
public:
	virtual ~CSquareToneGeneratorDelegate() {}
	CSquareToneGeneratorDelegate() {}
	
	virtual float Sample(const ISimpleTone& inDelegator) const;
};


#endif /* defined(__DemoMIDI_Synth__CSimpleTone__) */
