/*
	CDelayAPF: implements a delaying APF with a single coefficient g

	Can be used alone or as a base class.


	Copyright (c) 2010 Will Pirkle
	Free for academic use.
*/

// Inherited Base Class functions:
/*
	void init(int nDelayLength);
	void resetDelay();
	void setDelay_mSec(float fmSec);
	void setOutputAttenuation_dB(float fAttendB);

	// NEED TO OVERRIDE
	bool processAudio(float* pInput, float* pOutput);
*/

#pragma once
#include "Delay.h"

// derived class of CDelay
class CDelayAPF : public CDelay
{
public:
	// constructor/destructor
	CDelayAPF(void);
	~CDelayAPF(void);

	// members
protected:
	float m_fAPF_g; // one g coefficient

public:
	// set our g value
	void setAPF_g(float fAPFg){m_fAPF_g = fAPFg;}

	// overrides
	bool processAudio(float* pInput, float* pOutput);

};
