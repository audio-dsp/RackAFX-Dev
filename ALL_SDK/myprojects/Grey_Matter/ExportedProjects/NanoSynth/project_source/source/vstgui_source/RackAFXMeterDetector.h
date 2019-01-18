#pragma once

#include "pluginconstants.h"

//
// http://www.musicdsp.org/archive.php?classid=0#205
//
//const float DIGITAL_TC = -2.0; // log(1%)
//const float ANALOG_TC = -0.43533393574791066201247090699309; // (log(36.7%)
//const float METER_UPDATE_INTERVAL_MSEC = 15.0;
//const float METER_MIN_DB = -60.0;
namespace VSTGUI {

class CRackAFXMeterDetector
{
public:
	CRackAFXMeterDetector(void);
	~CRackAFXMeterDetector(void);

public:

	// Call the Init Function to initialize and setup all at once; this can be called as many times
	// as you want
	void init(float samplerate, float attack_in_ms, float release_in_ms, bool bAnalogTC, UINT uDetect, bool bLogDetector);

	// these functions allow you to change modes and attack/release one at a time during
	// realtime operation
	void setTCModeAnalog(bool bAnalogTC); // {m_bAnalogTC = bAnalogTC;}

	// THEN do these after init
	void setAttackTime(float attack_in_ms);
	void setReleaseTime(float release_in_ms);

	// Use these "codes"
	// DETECT PEAK   = 0
	// DETECT MS	 = 1
	// DETECT RMS	 = 2
	//
	void setDetectMode(UINT uDetect) {m_uDetectMode = uDetect;}
	
	void setSampleRate(float f) 
	{
		m_fSampleRate = f;
	
		setAttackTime(m_fAttackTime_mSec);
		setReleaseTime(m_fReleaseTime_mSec);
	}
	
	void setLogDetect(bool b) {m_bLogDetector = b;}

	// call this to detect; it returns the peak ms or rms value at that instant
	float detect(float fInput);

	// call this from your prepareForPlay() function each time to reset the detector
	void prepareForPlay();
	UINT  m_uDetectMode;

protected:
	int  m_nSample;
	float m_fAttackTime;
	float m_fReleaseTime;
	float m_fAttackTime_mSec;
	float m_fReleaseTime_mSec;
	float m_fSampleRate;
	float m_fEnvelope;
	bool  m_bAnalogTC;
	bool  m_bLogDetector;
};
}