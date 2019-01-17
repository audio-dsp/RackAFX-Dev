/*
	RackAFX(TM)
	Applications Programming Interface
	Derived Class Object Implementation
*/


#include "SpectralDynamics.h"


/* constructor()
	You can initialize variables here.
	You can also allocate memory here as long is it does not
	require the plugin to be fully instantiated. If so, allocate in init()

*/
CSpectralDynamics::CSpectralDynamics()
{
	// Added by RackAFX - DO NOT REMOVE
	//
	// initUI() for GUI controls: this must be called before initializing/using any GUI variables
	initUI();
	// END initUI()

	// built in initialization
	m_PlugInName = "SpectralDynamics";

	// Default to Stereo Operation:
	// Change this if you want to support more/less channels
	m_uMaxInputChannels = 2;
	m_uMaxOutputChannels = 2;

	// use of MIDI controllers to adjust sliders/knobs
	m_bEnableMIDIControl = true;		// by default this is enabled

	// custom GUI stuff
	m_bLinkGUIRowsAndButtons = false;	// change this if you want to force-link

	// DO NOT CHANGE let RackAFX change it for you; use Edit Project to alter
	m_bUseCustomVSTGUI = false;

	// for a user (not RackAFX) generated GUI - advanced you must compile your own resources
	// DO NOT CHANGE let RackAFX change it for you; use Edit Project to alter
	m_bUserCustomGUI = false;

	// output only - SYNTH - plugin DO NOT CHANGE let RackAFX change it for you; use Edit Project to alter
	m_bOutputOnlyPlugIn = false;

	// Finish initializations here

}


/* destructor()
	Destroy variables allocated in the contructor()

*/
CSpectralDynamics::~CSpectralDynamics(void)
{


}

/*
initialize()
	Called by the client after creation; the parent window handle is now valid
	so you can use the Plug-In -> Host functions here (eg sendUpdateUI())

	NOTE: This function is called whenever the plug-in gets a new Client UI
	      e.g. loading and unloading User Plug-Ins.

	      Therefore, do not place One Time Initialization code here, place it in
	      the END of the constructor.

	      This function is really designed only for letting you communicate back
	      with the Host GUI via sendUpdateUI()
	See the website www.willpirkle.com for more details
*/
bool __stdcall CSpectralDynamics::initialize()
{
	// Add your code here

	return true;
}

// calculate the compressor G(n) value from the Detector output
float CSpectralDynamics::calcCompressorGain(float fDetectorValue, float fThreshold, float fRatio, float fKneeWidth, bool bLimit)
{
	// slope variable
	float CS = 1.0 - 1.0/fRatio; 

	// limiting is infinite ratio thus CS->1.0
	if(bLimit)
		CS = 1;
	
	// soft-knee with detection value in range?
	if(fKneeWidth > 0 && fDetectorValue > (fThreshold - fKneeWidth/2.0) && fDetectorValue < fThreshold + fKneeWidth/2.0)
	{
		// setup for Lagrange
		double x[2];
		double y[2];
		x[0] = fThreshold - fKneeWidth/2.0;
		x[1] = fThreshold + fKneeWidth/2.0;
		x[1] = min(0, x[1]); // top limit is 0dBFS
		y[0] = 0;	// CS = 0
		y[1] = CS;	// CS = 0
		
		// interpolate
		CS = lagrpol(&x[0], &y[0], 2, fDetectorValue);
	}
	// compute gain; threshold and detection values are in dB	
	float yG = (fThreshold - fDetectorValue)*CS;
	
	// clamp; this allows ratios of 1:1 to still operate
	yG = min(0, yG);
	
	// convert back to linear
	return pow(10.0, yG/20.0); 
}


// calculate the downward expander G(n) value from the Detector output
float CSpectralDynamics::calcDownwardExpanderGain(float fDetectorValue, float fThreshold, float fRatio, float fKneeWidth, bool bGate)
{
	// slope variable
	float ES = 1.0/fRatio - 1; // just to make it like the book...

	// gating is infinite ratio; ES -> -1.0
	if(bGate)
		ES = -1;

	// soft-knee with detection value in range?
	if(fKneeWidth > 0 && fDetectorValue > (fThreshold - fKneeWidth/2.0) && fDetectorValue < fThreshold + fKneeWidth/2.0)
	{
		// setup for Lagrange
		double x[2];
		double y[2];
		x[0] = fThreshold - fKneeWidth/2.0;
		x[1] = fThreshold + fKneeWidth/2.0;
		x[1] = min(0, x[1]); // top limit is 0dBFS
		y[0] = ES;	// current ES
		y[1] = 0;	// 1:1 ratio
		
		// interpolate the value
		ES = lagrpol(&x[0], &y[0], 2, fDetectorValue);
	}

	// compute gain; threshold and detection values are in dB	
	float yG = (fThreshold - fDetectorValue)*ES;
	
	// clamp; this allows ratios of 1:1 to still operate
	yG = min(0, yG);
	
	// convert back to linear
	return pow(10.0, yG/20.0); 
}

// all filters have same cutoff frequency; only need LPF and HPF calcs
void CSpectralDynamics::calculateFilterBankCoeffs(float fCutoffFreq)
{
	// Shared Factors:
	float omega_c = pi*fCutoffFreq;
	float theta_c = pi*fCutoffFreq/(float)m_nSampleRate;

	float k = omega_c/tan(theta_c);
	float k_squared = k*k;

	float omega_c_squared = omega_c*omega_c;

	float fDenominator = k_squared + omega_c_squared + 2.0*k*omega_c;
	
	float fb1_Num = -2.0*k_squared + 2.0*omega_c_squared;
	float fb2_Num = -2.0*k*omega_c + k_squared + omega_c_squared;

	// the LPF coeffs
	float a0 = omega_c_squared/fDenominator;
	float a1 = 2.0*omega_c_squared/fDenominator;
	float a2 = a0;	
	float b1 = fb1_Num/fDenominator; 
	float b2 = fb2_Num/fDenominator;

	// set the LPFs
	m_LeftLPF.m_f_a0 = a0;
	m_LeftLPF.m_f_a1 = a1;
	m_LeftLPF.m_f_a2 = a2;
	m_LeftLPF.m_f_b1 = b1;
	m_LeftLPF.m_f_b2 = b2;
	
	// right
	m_RightLPF.m_f_a0 = a0;
	m_RightLPF.m_f_a1 = a1;
	m_RightLPF.m_f_a2 = a2;
	m_RightLPF.m_f_b1 = b1;
	m_RightLPF.m_f_b2 = b2;


	// the HPF coeffs
	a0 = k_squared/fDenominator;
	a1 = -2.0*k_squared/fDenominator;
	a2 = a0;	
	b1 = fb1_Num/fDenominator; 
	b2 = fb2_Num/fDenominator;

	// set the HPFs
	m_LeftHPF.m_f_a0 = a0;
	m_LeftHPF.m_f_a1 = a1;
	m_LeftHPF.m_f_a2 = a2;
	m_LeftHPF.m_f_b1 = b1;
	m_LeftHPF.m_f_b2 = b2;
	
	// right
	m_RightHPF.m_f_a0 = a0;
	m_RightHPF.m_f_a1 = a1;
	m_RightHPF.m_f_a2 = a2;
	m_RightHPF.m_f_b1 = b1;
	m_RightHPF.m_f_b2 = b2;
}


/* prepareForPlay()
	Called by the client after Play() is initiated but before audio streams

	You can perform buffer flushes and per-run intializations.
	You can check the following variables and use them if needed:

	m_nNumWAVEChannels;
	m_nSampleRate;
	m_nBitDepth;

	NOTE: the above values are only valid during prepareForPlay() and
		  processAudioFrame() because the user might change to another wave file,
		  or use the sound card, oscillators, or impulse response mechanisms

    NOTE: if you allocte memory in this function, destroy it in ::destroy() above
*/
bool __stdcall CSpectralDynamics::prepareForPlay()
{
	// Add your code here:
	// Add your code here:
	// Flush the filters
	m_LeftLPF.flushDelays();
	m_LeftHPF.flushDelays();
	m_RightLPF.flushDelays();
	m_RightHPF.flushDelays();
		
	// calculate the Coeffs all at once!
	calculateFilterBankCoeffs(m_fFilterBankCutoff);

	// init detectors
	// set all params at once with this function; false = Digital Time Constant NOT 
	// NOTE: Setting detector for linear operaton so we can sum the results; 
	//       we have to convert back to log manually
	if(m_uTimeConstant == Digital)
	{
		m_Left_LF_Detector.init((float)m_nSampleRate, m_LF_Attack_mSec, m_LF_Release_mSec, false, DETECT_MODE_RMS, false);
		m_Right_LF_Detector.init((float)m_nSampleRate, m_LF_Attack_mSec, m_LF_Release_mSec, false, DETECT_MODE_RMS, false);
		
		m_Left_HF_Detector.init((float)m_nSampleRate, m_HF_Attack_mSec, m_HF_Release_mSec, false, DETECT_MODE_RMS, false);

		m_Right_HF_Detector.init((float)m_nSampleRate, m_HF_Attack_mSec, m_HF_Release_mSec, false, DETECT_MODE_RMS, false);
	}
	else
	{
		m_Left_LF_Detector.init((float)m_nSampleRate, m_LF_Attack_mSec, m_LF_Release_mSec, true, DETECT_MODE_RMS, false);

		m_Right_LF_Detector.init((float)m_nSampleRate, m_LF_Attack_mSec, m_LF_Release_mSec, true, DETECT_MODE_RMS, false);
		
		m_Left_HF_Detector.init((float)m_nSampleRate, m_HF_Attack_mSec, m_HF_Release_mSec, true, DETECT_MODE_RMS, false);

		m_Right_HF_Detector.init((float)m_nSampleRate, m_HF_Attack_mSec, m_HF_Release_mSec, true, DETECT_MODE_RMS, false);
	}

	return true;
}


/* processAudioFrame

// ALL VALUES IN AND OUT ON THE RANGE OF -1.0 TO + 1.0

LEFT INPUT = pInputBuffer[0];
RIGHT INPUT = pInputBuffer[1]

LEFT INPUT = pInputBuffer[0]
LEFT OUTPUT = pOutputBuffer[1]

*/
bool __stdcall CSpectralDynamics::processAudioFrame(float* pInputBuffer, float* pOutputBuffer, UINT uNumInputChannels, UINT uNumOutputChannels)
{
	// Do LEFT (MONO) Channel; there is always at least one input/one output
	float fLeftInput = pInputBuffer[0];
	float fRightInput = uNumInputChannels == 1 ? fLeftInput : pInputBuffer[1];
	
	// setup the input and output gains
	float fLFGain = pow(10.0, m_LF_DetectorGain_dB/20.0);
	float fHFGain = pow(10.0, m_HF_DetectorGain_dB/20.0);

	float fLFOutputGain = pow(10.0, m_LF_MakeUpGain_dB/20.0);
	float fHFOutputGain = pow(10.0, m_HF_MakeUpGain_dB/20.0);
	
	// split the signal into m_Left LF and HF parts
	float fLeft_LF_Out = m_LeftLPF.doBiQuad(fLeftInput*fLFGain);
	float fLeft_HF_Out = m_LeftHPF.doBiQuad(fLeftInput*fHFGain);
	
	// invert ONE of the outputs for proper recombination
	fLeft_HF_Out *= -1.0;

	// send these to the detectors: NOTE OUTPUTS ARE LINEAR
	float fLeft_LF_Detector = m_Left_LF_Detector.detect(fLeft_LF_Out);
	float fLeft_HF_Detector = m_Left_HF_Detector.detect(fLeft_HF_Out); 
	
	// split the signal into m_Left LF and HF parts
	float fRight_LF_Out = m_RightLPF.doBiQuad(fRightInput*fLFGain);
	float fRight_HF_Out = m_RightHPF.doBiQuad(fRightInput*fHFGain);
	
	// invert ONE of the outputs for proper recombination
	fRight_HF_Out *= -1.0;

	// send these to the detectors: NOTE OUTPUTS ARE LINEAR
	float fRight_LF_Detector = m_Right_LF_Detector.detect(fRight_LF_Out);
	float fRight_HF_Detector = m_Right_HF_Detector.detect(fRight_HF_Out); 

	// This is the stereo linking of the detector paths; 
	// The detectors were set for linear operation so we could sum them
	// they must be converted back to dB before use in the gain calculation
	float LFDetectorSum = 0.5*(fRight_LF_Detector, fLeft_LF_Detector);
	float HFDetectorSum = 0.5*(fRight_HF_Detector, fLeft_HF_Detector);
	
	// convert back to dB
	LFDetectorSum = 20.0*log10(LFDetectorSum);
	HFDetectorSum = 20.0*log10(HFDetectorSum);

	// sum for input metering
	m_fMeterLFIn =0.5*(fLeft_LF_Out + fRight_LF_Out);
	m_fMeterHFIn = 0.5*(fLeft_HF_Out + fRight_HF_Out);

	// calculate the gain factors
	float fGn = 1.0;
		
	// --- LF BAND ---------------
	// branch: all are LF: detect, thresh, ratio
	if(m_uProcessorType == COMP)
		fGn = calcCompressorGain(LFDetectorSum, m_LF_Threshold, m_LF_Ratio, m_fKneeWidth, false);
	else if(m_uProcessorType == LIMIT)
		fGn = calcCompressorGain(LFDetectorSum, m_LF_Threshold, m_LF_Ratio, m_fKneeWidth, true);
	else if(m_uProcessorType == EXPAND)
		fGn = calcDownwardExpanderGain(LFDetectorSum, m_LF_Threshold, m_LF_Ratio, m_fKneeWidth, false);
	else if(m_uProcessorType == GATE)
		fGn = calcDownwardExpanderGain(LFDetectorSum, m_LF_Threshold, m_LF_Ratio, m_fKneeWidth, true);
	
	// create left and right LF outputs
	float fLFOutputL = fGn*fLeft_LF_Out*fLFOutputGain;
	float fLFOutputR = fGn*fRight_LF_Out*fLFOutputGain;
	
	// gain reduction meter
	m_fMeterLFGr = 1.0 - fGn;

	// --- HF BAND ---------------
	// branch: all are HF: detect, thresh, ratio
	if(m_uProcessorType == COMP)
		fGn = calcCompressorGain(HFDetectorSum, m_HF_Threshold, m_HF_Ratio, m_fKneeWidth, false);
	else if(m_uProcessorType == LIMIT)
		fGn = calcCompressorGain(HFDetectorSum, m_HF_Threshold, m_HF_Ratio, m_fKneeWidth, true);
	else if(m_uProcessorType == EXPAND)
		fGn = calcDownwardExpanderGain(HFDetectorSum, m_HF_Threshold, m_HF_Ratio, m_fKneeWidth, false);
	else if(m_uProcessorType == GATE)
		fGn = calcDownwardExpanderGain(HFDetectorSum, m_HF_Threshold, m_HF_Ratio, m_fKneeWidth, true);

	// create left and right HF outputs
	float fHFOutputL = fGn*fLeft_HF_Out*fHFOutputGain;
	float fHFOutputR = fGn*fRight_HF_Out*fHFOutputGain;

	// meter output
	m_fMeterLFOut = 0.5*(fLFOutputL + fLFOutputR);
	m_fMeterHFOut = 0.5*(fHFOutputL + fHFOutputR);
	
	// meter GR
	m_fMeterHFGr = 1.0 - fGn;

	// sum outputs
	float fLeftOutput = fLFOutputL + fHFOutputL;
	float fRightOutput = fLFOutputR + fHFOutputR;

	// write the outputs
	pOutputBuffer[0] = fLeftOutput;

	// Mono-In, Stereo-Out (AUX Effect)
	if(uNumInputChannels == 1 && uNumOutputChannels == 2)
		pOutputBuffer[1] = fRightOutput;

	// Stereo-In, Stereo-Out (INSERT Effect)
	if(uNumInputChannels == 2 && uNumOutputChannels == 2)
		pOutputBuffer[1] = fRightOutput;

	return true;
}


/* ADDED BY RACKAFX -- DO NOT EDIT THIS CODE!!! ----------------------------------- //
   	**--0x2983--**

	Variable Name                    Index
-----------------------------------------------
	m_fFilterBankCutoff               0
	m_HF_DetectorGain_dB              1
	m_HF_Threshold                    2
	m_HF_Attack_mSec                  3
	m_HF_Release_mSec                 4
	m_HF_Ratio                        5
	m_HF_MakeUpGain_dB                6
	m_fKneeWidth                      10
	m_LF_DetectorGain_dB              11
	m_LF_Threshold                    12
	m_LF_Attack_mSec                  13
	m_LF_Release_mSec                 14
	m_LF_Ratio                        15
	m_LF_MakeUpGain_dB                16
	m_uProcessorType                  41
	m_uTimeConstant                   42
	                                  50
	                                  51
	                                  54
	                                  55
	                                  58
	                                  59

	Assignable Buttons               Index
-----------------------------------------------
	B1                                50
	B2                                51
	B3                                52

-----------------------------------------------
Joystick Drop List Boxes          Index
-----------------------------------------------
	 Drop List A                     60
	 Drop List B                     61
	 Drop List C                     62
	 Drop List D                     63

-----------------------------------------------

	**--0xFFDD--**
// ------------------------------------------------------------------------------- */
// Add your UI Handler code here ------------------------------------------------- //
//
bool __stdcall CSpectralDynamics::userInterfaceChange(int nControlIndex)
{
	// decode the control index, or delete the switch and use brute force calls
	switch(nControlIndex)
	{
		// filter cutoff
		case 0:
		{
			calculateFilterBankCoeffs(m_fFilterBankCutoff);
			break;
		}
		
		// HF Attack
		case 3:
		{
			m_Left_HF_Detector.setAttackTime(m_HF_Attack_mSec);
			m_Right_HF_Detector.setAttackTime(m_HF_Attack_mSec);
			break;
		}
	
		// LF Attack
		case 13:
		{
			m_Left_LF_Detector.setAttackTime(m_LF_Attack_mSec);
			m_Right_LF_Detector.setAttackTime(m_LF_Attack_mSec);
			break;
		}

		// HF Release
		case 4:
		{
			m_Left_HF_Detector.setReleaseTime(m_HF_Release_mSec);
			m_Right_HF_Detector.setReleaseTime(m_HF_Release_mSec);
			break;
		}
	
		// LF Release
		case 14:
		{
			m_Left_LF_Detector.setReleaseTime(m_LF_Release_mSec);
			m_Right_LF_Detector.setReleaseTime(m_LF_Release_mSec);
			break;
		}
		
		// Time Constant
		case 42:
		{
			if(m_uTimeConstant == Digital)
			{
				m_Left_HF_Detector.setTCModeAnalog(false);
				m_Right_LF_Detector.setTCModeAnalog(false);
				m_Right_HF_Detector.setTCModeAnalog(false);
				m_Right_LF_Detector.setTCModeAnalog(false);
			}
			else
			{
				m_Left_HF_Detector.setTCModeAnalog(true);
				m_Right_LF_Detector.setTCModeAnalog(true);
				m_Right_HF_Detector.setTCModeAnalog(true);
				m_Right_LF_Detector.setTCModeAnalog(true);
			}
		}
		default:
			break;
	}

	return true;
}


/* joystickControlChange

	Indicates the user moved the joystick point; the variables are the relative mixes
	of each axis; the values will add up to 1.0

			B
			|
		A -	x -	C
			|
			D

	The point in the very center (x) would be:
	fControlA = 0.25
	fControlB = 0.25
	fControlC = 0.25
	fControlD = 0.25

	AC Mix = projection on X Axis (0 -> 1)
	BD Mix = projection on Y Axis (0 -> 1)
*/
bool __stdcall CSpectralDynamics::joystickControlChange(float fControlA, float fControlB, float fControlC, float fControlD, float fACMix, float fBDMix)
{
	// add your code here

	return true;
}



/* processAudioBuffer

// ALL VALUES IN AND OUT ON THE RANGE OF -1.0 TO + 1.0

	The I/O buffers are interleaved depending on the number of channels. If uNumChannels = 2, then the
	buffer is L/R/L/R/L/R etc...

	if uNumChannels = 6 then the buffer is L/R/C/Sub/BL/BR etc...

	It is up to you to decode and de-interleave the data.

	For advanced users only!!
*/
bool __stdcall CSpectralDynamics::processRackAFXAudioBuffer(float* pInputBuffer, float* pOutputBuffer,
													   UINT uNumInputChannels, UINT uNumOutputChannels,
													   UINT uBufferSize)
{

	for(UINT i=0; i<uBufferSize; i++)
	{
		// pass through code
		pOutputBuffer[i] = pInputBuffer[i];
	}


	return true;
}



/* processVSTAudioBuffer

// ALL VALUES IN AND OUT ON THE RANGE OF -1.0 TO + 1.0

	The I/O buffers are interleaved depending on the number of channels. If uNumChannels = 2, then the
	buffer is L/R/L/R/L/R etc...

	if uNumChannels = 6 then the buffer is L/R/C/Sub/BL/BR etc...

	It is up to you to decode and de-interleave the data.

	The VST input and output buffers are pointers-to-pointers. The pp buffers are the same depth as uNumChannels, so
	if uNumChannels = 2, then ppInputs would contain two pointers,

		ppInputs[0] = a pointer to the LEFT buffer of data
		ppInputs[1] = a pointer to the RIGHT buffer of data

	Similarly, ppOutputs would have 2 pointers, one for left and one for right.

*/
bool __stdcall CSpectralDynamics::processVSTAudioBuffer(float** ppInputs, float** ppOutputs,
													UINT uNumChannels, int uNumFrames)
{
	// PASS Through example for Stereo interleaved data

	// MONO First
    float* in1  =  ppInputs[0];
    float* out1 =  ppOutputs[0];
  	float* in2;
    float* out2;

 	// if STEREO,
   	if(uNumChannels == 2)
   	{
    	in2  =  ppInputs[1];
     	out2 = ppOutputs[1];
	}

	// loop and pass input to output by de-referencing ptrs
	while (--uNumFrames >= 0)
    {
        (*out1++) = (*in1++);

        if(uNumChannels == 2)
        	(*out2++) = (*in2++);
    }

	// all OK
	return true;
}

bool __stdcall CSpectralDynamics::midiNoteOn(UINT uChannel, UINT uMIDINote, UINT uVelocity)
{
	return true;
}

bool __stdcall CSpectralDynamics::midiNoteOff(UINT uChannel, UINT uMIDINote, UINT uVelocity, bool bAllNotesOff)
{
	return true;
}

// uModValue = 0->127
bool __stdcall CSpectralDynamics::midiModWheel(UINT uChannel, UINT uModValue)
{
	return true;
}

// nActualPitchBendValue 		= -8192 -> +8191, 0 at center
// fNormalizedPitchBendValue 	= -1.0  -> +1.0,  0 at center
bool __stdcall CSpectralDynamics::midiPitchBend(UINT uChannel, int nActualPitchBendValue, float fNormalizedPitchBendValue)
{
	return true;
}

// MIDI Clock
// http://home.roadrunner.com/~jgglatt/tech/midispec/clock.htm
/* There are 24 MIDI Clocks in every quarter note. (12 MIDI Clocks in an eighth note, 6 MIDI Clocks in a 16th, etc).
   Therefore, when a slave device counts down the receipt of 24 MIDI Clock messages, it knows that one quarter note
   has passed. When the slave counts off another 24 MIDI Clock messages, it knows that another quarter note has passed.
   Etc. Of course, the rate that the master sends these messages is based upon the master's tempo.

   For example, for a tempo of 120 BPM (ie, there are 120 quarter notes in every minute), the master sends a MIDI clock
   every 20833 microseconds. (ie, There are 1,000,000 microseconds in a second. Therefore, there are 60,000,000
   microseconds in a minute. At a tempo of 120 BPM, there are 120 quarter notes per minute. There are 24 MIDI clocks
   in each quarter note. Therefore, there should be 24 * 120 MIDI Clocks per minute.
   So, each MIDI Clock is sent at a rate of 60,000,000/(24 * 120) microseconds).
*/
bool __stdcall CSpectralDynamics::midiClock()
{

	return true;
}

// any midi message other than note on, note off, pitchbend, mod wheel or clock
bool __stdcall CSpectralDynamics::midiMessage(unsigned char cChannel, unsigned char cStatus, unsigned char
						   				  cData1, unsigned char cData2)
{
	return true;
}


// DO NOT DELETE THIS FUNCTION --------------------------------------------------- //
bool __stdcall CSpectralDynamics::initUI()
{
	// ADDED BY RACKAFX -- DO NOT EDIT THIS CODE!!! ------------------------------ //
	if(m_UIControlList.count() > 0)
		return true;

// **--0xDEA7--**

	m_fFilterBankCutoff = 1000.000000;
	CUICtrl ui0;
	ui0.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui0.uControlId = 0;
	ui0.bLogSlider = false;
	ui0.bExpSlider = false;
	ui0.fUserDisplayDataLoLimit = 100.000000;
	ui0.fUserDisplayDataHiLimit = 8000.000000;
	ui0.uUserDataType = floatData;
	ui0.fInitUserIntValue = 0;
	ui0.fInitUserFloatValue = 1000.000000;
	ui0.fInitUserDoubleValue = 0;
	ui0.fInitUserUINTValue = 0;
	ui0.m_pUserCookedIntData = NULL;
	ui0.m_pUserCookedFloatData = &m_fFilterBankCutoff;
	ui0.m_pUserCookedDoubleData = NULL;
	ui0.m_pUserCookedUINTData = NULL;
	ui0.cControlUnits = "Hz                                                              ";
	ui0.cVariableName = "m_fFilterBankCutoff";
	ui0.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui0.dPresetData[0] = 0.000000;ui0.dPresetData[1] = 0.000000;ui0.dPresetData[2] = 0.000000;ui0.dPresetData[3] = 0.000000;ui0.dPresetData[4] = 0.000000;ui0.dPresetData[5] = 0.000000;ui0.dPresetData[6] = 0.000000;ui0.dPresetData[7] = 0.000000;ui0.dPresetData[8] = 0.000000;ui0.dPresetData[9] = 0.000000;ui0.dPresetData[10] = 0.000000;ui0.dPresetData[11] = 0.000000;ui0.dPresetData[12] = 0.000000;ui0.dPresetData[13] = 0.000000;ui0.dPresetData[14] = 0.000000;ui0.dPresetData[15] = 0.000000;
	ui0.cControlName = "fc";
	ui0.bOwnerControl = false;
	ui0.bMIDIControl = true;
	ui0.uMIDIControlCommand = 176;
	ui0.uMIDIControlName = 74;
	ui0.uMIDIControlChannel = 0;
	ui0.nGUIRow = 1;
	ui0.nGUIColumn = 1;
	ui0.uControlTheme[0] = 1; ui0.uControlTheme[1] = 9; ui0.uControlTheme[2] = 0; ui0.uControlTheme[3] = 0; ui0.uControlTheme[4] = 0; ui0.uControlTheme[5] = 1; ui0.uControlTheme[6] = 0; ui0.uControlTheme[7] = 65535; ui0.uControlTheme[8] = 0; ui0.uControlTheme[9] = 11119017; ui0.uControlTheme[10] = 2; ui0.uControlTheme[11] = 12632256; ui0.uControlTheme[12] = 0; ui0.uControlTheme[13] = 6316128; ui0.uControlTheme[14] = 3; ui0.uControlTheme[15] = 8421504; ui0.uControlTheme[16] = 14772545; ui0.uControlTheme[17] = 0; ui0.uControlTheme[18] = 0; ui0.uControlTheme[19] = 0; ui0.uControlTheme[20] = 0; ui0.uControlTheme[21] = 0; ui0.uControlTheme[22] = 0; ui0.uControlTheme[23] = 4; ui0.uControlTheme[24] = 1; ui0.uControlTheme[25] = 0; ui0.uControlTheme[26] = 0; ui0.uControlTheme[27] = 0; ui0.uControlTheme[28] = 0; ui0.uControlTheme[29] = 0; ui0.uControlTheme[30] = 0; ui0.uControlTheme[31] = 0; 
	ui0.uFluxCapControl[0] = 0; ui0.uFluxCapControl[1] = 0; ui0.uFluxCapControl[2] = 0; ui0.uFluxCapControl[3] = 0; ui0.uFluxCapControl[4] = 0; ui0.uFluxCapControl[5] = 0; ui0.uFluxCapControl[6] = 0; ui0.uFluxCapControl[7] = 0; ui0.uFluxCapControl[8] = 0; ui0.uFluxCapControl[9] = 0; ui0.uFluxCapControl[10] = 0; ui0.uFluxCapControl[11] = 0; ui0.uFluxCapControl[12] = 0; ui0.uFluxCapControl[13] = 0; ui0.uFluxCapControl[14] = 0; ui0.uFluxCapControl[15] = 0; ui0.uFluxCapControl[16] = 0; ui0.uFluxCapControl[17] = 0; ui0.uFluxCapControl[18] = 0; ui0.uFluxCapControl[19] = 0; ui0.uFluxCapControl[20] = 0; ui0.uFluxCapControl[21] = 0; ui0.uFluxCapControl[22] = 0; ui0.uFluxCapControl[23] = 0; ui0.uFluxCapControl[24] = 0; ui0.uFluxCapControl[25] = 0; ui0.uFluxCapControl[26] = 0; ui0.uFluxCapControl[27] = 0; ui0.uFluxCapControl[28] = 0; ui0.uFluxCapControl[29] = 0; ui0.uFluxCapControl[30] = 0; ui0.uFluxCapControl[31] = 0; ui0.uFluxCapControl[32] = 0; ui0.uFluxCapControl[33] = 0; ui0.uFluxCapControl[34] = 0; ui0.uFluxCapControl[35] = 0; ui0.uFluxCapControl[36] = 0; ui0.uFluxCapControl[37] = 0; ui0.uFluxCapControl[38] = 0; ui0.uFluxCapControl[39] = 0; ui0.uFluxCapControl[40] = 0; ui0.uFluxCapControl[41] = 0; ui0.uFluxCapControl[42] = 0; ui0.uFluxCapControl[43] = 0; ui0.uFluxCapControl[44] = 0; ui0.uFluxCapControl[45] = 0; ui0.uFluxCapControl[46] = 0; ui0.uFluxCapControl[47] = 0; ui0.uFluxCapControl[48] = 0; ui0.uFluxCapControl[49] = 0; ui0.uFluxCapControl[50] = 0; ui0.uFluxCapControl[51] = 0; ui0.uFluxCapControl[52] = 0; ui0.uFluxCapControl[53] = 0; ui0.uFluxCapControl[54] = 0; ui0.uFluxCapControl[55] = 0; ui0.uFluxCapControl[56] = 0; ui0.uFluxCapControl[57] = 0; ui0.uFluxCapControl[58] = 0; ui0.uFluxCapControl[59] = 0; ui0.uFluxCapControl[60] = 0; ui0.uFluxCapControl[61] = 0; ui0.uFluxCapControl[62] = 0; ui0.uFluxCapControl[63] = 0; 
	ui0.fFluxCapData[0] = 0.000000; ui0.fFluxCapData[1] = 0.000000; ui0.fFluxCapData[2] = 0.000000; ui0.fFluxCapData[3] = 0.000000; ui0.fFluxCapData[4] = 0.000000; ui0.fFluxCapData[5] = 0.000000; ui0.fFluxCapData[6] = 0.000000; ui0.fFluxCapData[7] = 0.000000; ui0.fFluxCapData[8] = 0.000000; ui0.fFluxCapData[9] = 0.000000; ui0.fFluxCapData[10] = 0.000000; ui0.fFluxCapData[11] = 0.000000; ui0.fFluxCapData[12] = 0.000000; ui0.fFluxCapData[13] = 0.000000; ui0.fFluxCapData[14] = 0.000000; ui0.fFluxCapData[15] = 0.000000; ui0.fFluxCapData[16] = 0.000000; ui0.fFluxCapData[17] = 0.000000; ui0.fFluxCapData[18] = 0.000000; ui0.fFluxCapData[19] = 0.000000; ui0.fFluxCapData[20] = 0.000000; ui0.fFluxCapData[21] = 0.000000; ui0.fFluxCapData[22] = 0.000000; ui0.fFluxCapData[23] = 0.000000; ui0.fFluxCapData[24] = 0.000000; ui0.fFluxCapData[25] = 0.000000; ui0.fFluxCapData[26] = 0.000000; ui0.fFluxCapData[27] = 0.000000; ui0.fFluxCapData[28] = 0.000000; ui0.fFluxCapData[29] = 0.000000; ui0.fFluxCapData[30] = 0.000000; ui0.fFluxCapData[31] = 0.000000; ui0.fFluxCapData[32] = 0.000000; ui0.fFluxCapData[33] = 0.000000; ui0.fFluxCapData[34] = 0.000000; ui0.fFluxCapData[35] = 0.000000; ui0.fFluxCapData[36] = 0.000000; ui0.fFluxCapData[37] = 0.000000; ui0.fFluxCapData[38] = 0.000000; ui0.fFluxCapData[39] = 0.000000; ui0.fFluxCapData[40] = 0.000000; ui0.fFluxCapData[41] = 0.000000; ui0.fFluxCapData[42] = 0.000000; ui0.fFluxCapData[43] = 0.000000; ui0.fFluxCapData[44] = 0.000000; ui0.fFluxCapData[45] = 0.000000; ui0.fFluxCapData[46] = 0.000000; ui0.fFluxCapData[47] = 0.000000; ui0.fFluxCapData[48] = 0.000000; ui0.fFluxCapData[49] = 0.000000; ui0.fFluxCapData[50] = 0.000000; ui0.fFluxCapData[51] = 0.000000; ui0.fFluxCapData[52] = 0.000000; ui0.fFluxCapData[53] = 0.000000; ui0.fFluxCapData[54] = 0.000000; ui0.fFluxCapData[55] = 0.000000; ui0.fFluxCapData[56] = 0.000000; ui0.fFluxCapData[57] = 0.000000; ui0.fFluxCapData[58] = 0.000000; ui0.fFluxCapData[59] = 0.000000; ui0.fFluxCapData[60] = 0.000000; ui0.fFluxCapData[61] = 0.000000; ui0.fFluxCapData[62] = 0.000000; ui0.fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(ui0);


	m_HF_DetectorGain_dB = 0.000000;
	CUICtrl ui1;
	ui1.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui1.uControlId = 1;
	ui1.bLogSlider = false;
	ui1.bExpSlider = false;
	ui1.fUserDisplayDataLoLimit = 0.000000;
	ui1.fUserDisplayDataHiLimit = 20.000000;
	ui1.uUserDataType = floatData;
	ui1.fInitUserIntValue = 0;
	ui1.fInitUserFloatValue = 0.000000;
	ui1.fInitUserDoubleValue = 0;
	ui1.fInitUserUINTValue = 0;
	ui1.m_pUserCookedIntData = NULL;
	ui1.m_pUserCookedFloatData = &m_HF_DetectorGain_dB;
	ui1.m_pUserCookedDoubleData = NULL;
	ui1.m_pUserCookedUINTData = NULL;
	ui1.cControlUnits = "dB                                                              ";
	ui1.cVariableName = "m_HF_DetectorGain_dB";
	ui1.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui1.dPresetData[0] = 0.000000;ui1.dPresetData[1] = 0.000000;ui1.dPresetData[2] = 0.000000;ui1.dPresetData[3] = 0.000000;ui1.dPresetData[4] = 0.000000;ui1.dPresetData[5] = 0.000000;ui1.dPresetData[6] = 0.000000;ui1.dPresetData[7] = 0.000000;ui1.dPresetData[8] = 0.000000;ui1.dPresetData[9] = 0.000000;ui1.dPresetData[10] = 0.000000;ui1.dPresetData[11] = 0.000000;ui1.dPresetData[12] = 0.000000;ui1.dPresetData[13] = 0.000000;ui1.dPresetData[14] = 0.000000;ui1.dPresetData[15] = 0.000000;
	ui1.cControlName = "HF Sen";
	ui1.bOwnerControl = false;
	ui1.bMIDIControl = true;
	ui1.uMIDIControlCommand = 176;
	ui1.uMIDIControlName = 71;
	ui1.uMIDIControlChannel = 0;
	ui1.nGUIRow = 1;
	ui1.nGUIColumn = 2;
	ui1.uControlTheme[0] = 0; ui1.uControlTheme[1] = 9; ui1.uControlTheme[2] = 0; ui1.uControlTheme[3] = 0; ui1.uControlTheme[4] = 0; ui1.uControlTheme[5] = 1; ui1.uControlTheme[6] = 0; ui1.uControlTheme[7] = 65535; ui1.uControlTheme[8] = 0; ui1.uControlTheme[9] = 11119017; ui1.uControlTheme[10] = 1; ui1.uControlTheme[11] = 12632256; ui1.uControlTheme[12] = 0; ui1.uControlTheme[13] = 6316128; ui1.uControlTheme[14] = 3; ui1.uControlTheme[15] = 8421504; ui1.uControlTheme[16] = 14772545; ui1.uControlTheme[17] = 0; ui1.uControlTheme[18] = 0; ui1.uControlTheme[19] = 0; ui1.uControlTheme[20] = 0; ui1.uControlTheme[21] = 0; ui1.uControlTheme[22] = 0; ui1.uControlTheme[23] = 85; ui1.uControlTheme[24] = 1; ui1.uControlTheme[25] = 0; ui1.uControlTheme[26] = 0; ui1.uControlTheme[27] = 0; ui1.uControlTheme[28] = 0; ui1.uControlTheme[29] = 0; ui1.uControlTheme[30] = 0; ui1.uControlTheme[31] = 0; 
	ui1.uFluxCapControl[0] = 0; ui1.uFluxCapControl[1] = 0; ui1.uFluxCapControl[2] = 0; ui1.uFluxCapControl[3] = 0; ui1.uFluxCapControl[4] = 0; ui1.uFluxCapControl[5] = 0; ui1.uFluxCapControl[6] = 0; ui1.uFluxCapControl[7] = 0; ui1.uFluxCapControl[8] = 0; ui1.uFluxCapControl[9] = 0; ui1.uFluxCapControl[10] = 0; ui1.uFluxCapControl[11] = 0; ui1.uFluxCapControl[12] = 0; ui1.uFluxCapControl[13] = 0; ui1.uFluxCapControl[14] = 0; ui1.uFluxCapControl[15] = 0; ui1.uFluxCapControl[16] = 0; ui1.uFluxCapControl[17] = 0; ui1.uFluxCapControl[18] = 0; ui1.uFluxCapControl[19] = 0; ui1.uFluxCapControl[20] = 0; ui1.uFluxCapControl[21] = 0; ui1.uFluxCapControl[22] = 0; ui1.uFluxCapControl[23] = 0; ui1.uFluxCapControl[24] = 0; ui1.uFluxCapControl[25] = 0; ui1.uFluxCapControl[26] = 0; ui1.uFluxCapControl[27] = 0; ui1.uFluxCapControl[28] = 0; ui1.uFluxCapControl[29] = 0; ui1.uFluxCapControl[30] = 0; ui1.uFluxCapControl[31] = 0; ui1.uFluxCapControl[32] = 0; ui1.uFluxCapControl[33] = 0; ui1.uFluxCapControl[34] = 0; ui1.uFluxCapControl[35] = 0; ui1.uFluxCapControl[36] = 0; ui1.uFluxCapControl[37] = 0; ui1.uFluxCapControl[38] = 0; ui1.uFluxCapControl[39] = 0; ui1.uFluxCapControl[40] = 0; ui1.uFluxCapControl[41] = 0; ui1.uFluxCapControl[42] = 0; ui1.uFluxCapControl[43] = 0; ui1.uFluxCapControl[44] = 0; ui1.uFluxCapControl[45] = 0; ui1.uFluxCapControl[46] = 0; ui1.uFluxCapControl[47] = 0; ui1.uFluxCapControl[48] = 0; ui1.uFluxCapControl[49] = 0; ui1.uFluxCapControl[50] = 0; ui1.uFluxCapControl[51] = 0; ui1.uFluxCapControl[52] = 0; ui1.uFluxCapControl[53] = 0; ui1.uFluxCapControl[54] = 0; ui1.uFluxCapControl[55] = 0; ui1.uFluxCapControl[56] = 0; ui1.uFluxCapControl[57] = 0; ui1.uFluxCapControl[58] = 0; ui1.uFluxCapControl[59] = 0; ui1.uFluxCapControl[60] = 0; ui1.uFluxCapControl[61] = 0; ui1.uFluxCapControl[62] = 0; ui1.uFluxCapControl[63] = 0; 
	ui1.fFluxCapData[0] = 0.000000; ui1.fFluxCapData[1] = 0.000000; ui1.fFluxCapData[2] = 0.000000; ui1.fFluxCapData[3] = 0.000000; ui1.fFluxCapData[4] = 0.000000; ui1.fFluxCapData[5] = 0.000000; ui1.fFluxCapData[6] = 0.000000; ui1.fFluxCapData[7] = 0.000000; ui1.fFluxCapData[8] = 0.000000; ui1.fFluxCapData[9] = 0.000000; ui1.fFluxCapData[10] = 0.000000; ui1.fFluxCapData[11] = 0.000000; ui1.fFluxCapData[12] = 0.000000; ui1.fFluxCapData[13] = 0.000000; ui1.fFluxCapData[14] = 0.000000; ui1.fFluxCapData[15] = 0.000000; ui1.fFluxCapData[16] = 0.000000; ui1.fFluxCapData[17] = 0.000000; ui1.fFluxCapData[18] = 0.000000; ui1.fFluxCapData[19] = 0.000000; ui1.fFluxCapData[20] = 0.000000; ui1.fFluxCapData[21] = 0.000000; ui1.fFluxCapData[22] = 0.000000; ui1.fFluxCapData[23] = 0.000000; ui1.fFluxCapData[24] = 0.000000; ui1.fFluxCapData[25] = 0.000000; ui1.fFluxCapData[26] = 0.000000; ui1.fFluxCapData[27] = 0.000000; ui1.fFluxCapData[28] = 0.000000; ui1.fFluxCapData[29] = 0.000000; ui1.fFluxCapData[30] = 0.000000; ui1.fFluxCapData[31] = 0.000000; ui1.fFluxCapData[32] = 0.000000; ui1.fFluxCapData[33] = 0.000000; ui1.fFluxCapData[34] = 0.000000; ui1.fFluxCapData[35] = 0.000000; ui1.fFluxCapData[36] = 0.000000; ui1.fFluxCapData[37] = 0.000000; ui1.fFluxCapData[38] = 0.000000; ui1.fFluxCapData[39] = 0.000000; ui1.fFluxCapData[40] = 0.000000; ui1.fFluxCapData[41] = 0.000000; ui1.fFluxCapData[42] = 0.000000; ui1.fFluxCapData[43] = 0.000000; ui1.fFluxCapData[44] = 0.000000; ui1.fFluxCapData[45] = 0.000000; ui1.fFluxCapData[46] = 0.000000; ui1.fFluxCapData[47] = 0.000000; ui1.fFluxCapData[48] = 0.000000; ui1.fFluxCapData[49] = 0.000000; ui1.fFluxCapData[50] = 0.000000; ui1.fFluxCapData[51] = 0.000000; ui1.fFluxCapData[52] = 0.000000; ui1.fFluxCapData[53] = 0.000000; ui1.fFluxCapData[54] = 0.000000; ui1.fFluxCapData[55] = 0.000000; ui1.fFluxCapData[56] = 0.000000; ui1.fFluxCapData[57] = 0.000000; ui1.fFluxCapData[58] = 0.000000; ui1.fFluxCapData[59] = 0.000000; ui1.fFluxCapData[60] = 0.000000; ui1.fFluxCapData[61] = 0.000000; ui1.fFluxCapData[62] = 0.000000; ui1.fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(ui1);


	m_HF_Threshold = 0.000000;
	CUICtrl ui2;
	ui2.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui2.uControlId = 2;
	ui2.bLogSlider = false;
	ui2.bExpSlider = false;
	ui2.fUserDisplayDataLoLimit = -60.000000;
	ui2.fUserDisplayDataHiLimit = 0.000000;
	ui2.uUserDataType = floatData;
	ui2.fInitUserIntValue = 0;
	ui2.fInitUserFloatValue = 0.000000;
	ui2.fInitUserDoubleValue = 0;
	ui2.fInitUserUINTValue = 0;
	ui2.m_pUserCookedIntData = NULL;
	ui2.m_pUserCookedFloatData = &m_HF_Threshold;
	ui2.m_pUserCookedDoubleData = NULL;
	ui2.m_pUserCookedUINTData = NULL;
	ui2.cControlUnits = "dB                                                              ";
	ui2.cVariableName = "m_HF_Threshold";
	ui2.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui2.dPresetData[0] = 0.000000;ui2.dPresetData[1] = 0.000000;ui2.dPresetData[2] = 0.000000;ui2.dPresetData[3] = 0.000000;ui2.dPresetData[4] = 0.000000;ui2.dPresetData[5] = 0.000000;ui2.dPresetData[6] = 0.000000;ui2.dPresetData[7] = 0.000000;ui2.dPresetData[8] = 0.000000;ui2.dPresetData[9] = 0.000000;ui2.dPresetData[10] = 0.000000;ui2.dPresetData[11] = 0.000000;ui2.dPresetData[12] = 0.000000;ui2.dPresetData[13] = 0.000000;ui2.dPresetData[14] = 0.000000;ui2.dPresetData[15] = 0.000000;
	ui2.cControlName = "HF Thresh";
	ui2.bOwnerControl = false;
	ui2.bMIDIControl = true;
	ui2.uMIDIControlCommand = 176;
	ui2.uMIDIControlName = 81;
	ui2.uMIDIControlChannel = 0;
	ui2.nGUIRow = 1;
	ui2.nGUIColumn = 3;
	ui2.uControlTheme[0] = 0; ui2.uControlTheme[1] = 9; ui2.uControlTheme[2] = 0; ui2.uControlTheme[3] = 0; ui2.uControlTheme[4] = 0; ui2.uControlTheme[5] = 1; ui2.uControlTheme[6] = 0; ui2.uControlTheme[7] = 65535; ui2.uControlTheme[8] = 0; ui2.uControlTheme[9] = 11119017; ui2.uControlTheme[10] = 1; ui2.uControlTheme[11] = 12632256; ui2.uControlTheme[12] = 0; ui2.uControlTheme[13] = 6316128; ui2.uControlTheme[14] = 3; ui2.uControlTheme[15] = 8421504; ui2.uControlTheme[16] = 14772545; ui2.uControlTheme[17] = 0; ui2.uControlTheme[18] = 0; ui2.uControlTheme[19] = 0; ui2.uControlTheme[20] = 0; ui2.uControlTheme[21] = 0; ui2.uControlTheme[22] = 0; ui2.uControlTheme[23] = 151; ui2.uControlTheme[24] = 1; ui2.uControlTheme[25] = 0; ui2.uControlTheme[26] = 0; ui2.uControlTheme[27] = 0; ui2.uControlTheme[28] = 0; ui2.uControlTheme[29] = 0; ui2.uControlTheme[30] = 0; ui2.uControlTheme[31] = 0; 
	ui2.uFluxCapControl[0] = 0; ui2.uFluxCapControl[1] = 0; ui2.uFluxCapControl[2] = 0; ui2.uFluxCapControl[3] = 0; ui2.uFluxCapControl[4] = 0; ui2.uFluxCapControl[5] = 0; ui2.uFluxCapControl[6] = 0; ui2.uFluxCapControl[7] = 0; ui2.uFluxCapControl[8] = 0; ui2.uFluxCapControl[9] = 0; ui2.uFluxCapControl[10] = 0; ui2.uFluxCapControl[11] = 0; ui2.uFluxCapControl[12] = 0; ui2.uFluxCapControl[13] = 0; ui2.uFluxCapControl[14] = 0; ui2.uFluxCapControl[15] = 0; ui2.uFluxCapControl[16] = 0; ui2.uFluxCapControl[17] = 0; ui2.uFluxCapControl[18] = 0; ui2.uFluxCapControl[19] = 0; ui2.uFluxCapControl[20] = 0; ui2.uFluxCapControl[21] = 0; ui2.uFluxCapControl[22] = 0; ui2.uFluxCapControl[23] = 0; ui2.uFluxCapControl[24] = 0; ui2.uFluxCapControl[25] = 0; ui2.uFluxCapControl[26] = 0; ui2.uFluxCapControl[27] = 0; ui2.uFluxCapControl[28] = 0; ui2.uFluxCapControl[29] = 0; ui2.uFluxCapControl[30] = 0; ui2.uFluxCapControl[31] = 0; ui2.uFluxCapControl[32] = 0; ui2.uFluxCapControl[33] = 0; ui2.uFluxCapControl[34] = 0; ui2.uFluxCapControl[35] = 0; ui2.uFluxCapControl[36] = 0; ui2.uFluxCapControl[37] = 0; ui2.uFluxCapControl[38] = 0; ui2.uFluxCapControl[39] = 0; ui2.uFluxCapControl[40] = 0; ui2.uFluxCapControl[41] = 0; ui2.uFluxCapControl[42] = 0; ui2.uFluxCapControl[43] = 0; ui2.uFluxCapControl[44] = 0; ui2.uFluxCapControl[45] = 0; ui2.uFluxCapControl[46] = 0; ui2.uFluxCapControl[47] = 0; ui2.uFluxCapControl[48] = 0; ui2.uFluxCapControl[49] = 0; ui2.uFluxCapControl[50] = 0; ui2.uFluxCapControl[51] = 0; ui2.uFluxCapControl[52] = 0; ui2.uFluxCapControl[53] = 0; ui2.uFluxCapControl[54] = 0; ui2.uFluxCapControl[55] = 0; ui2.uFluxCapControl[56] = 0; ui2.uFluxCapControl[57] = 0; ui2.uFluxCapControl[58] = 0; ui2.uFluxCapControl[59] = 0; ui2.uFluxCapControl[60] = 0; ui2.uFluxCapControl[61] = 0; ui2.uFluxCapControl[62] = 0; ui2.uFluxCapControl[63] = 0; 
	ui2.fFluxCapData[0] = 0.000000; ui2.fFluxCapData[1] = 0.000000; ui2.fFluxCapData[2] = 0.000000; ui2.fFluxCapData[3] = 0.000000; ui2.fFluxCapData[4] = 0.000000; ui2.fFluxCapData[5] = 0.000000; ui2.fFluxCapData[6] = 0.000000; ui2.fFluxCapData[7] = 0.000000; ui2.fFluxCapData[8] = 0.000000; ui2.fFluxCapData[9] = 0.000000; ui2.fFluxCapData[10] = 0.000000; ui2.fFluxCapData[11] = 0.000000; ui2.fFluxCapData[12] = 0.000000; ui2.fFluxCapData[13] = 0.000000; ui2.fFluxCapData[14] = 0.000000; ui2.fFluxCapData[15] = 0.000000; ui2.fFluxCapData[16] = 0.000000; ui2.fFluxCapData[17] = 0.000000; ui2.fFluxCapData[18] = 0.000000; ui2.fFluxCapData[19] = 0.000000; ui2.fFluxCapData[20] = 0.000000; ui2.fFluxCapData[21] = 0.000000; ui2.fFluxCapData[22] = 0.000000; ui2.fFluxCapData[23] = 0.000000; ui2.fFluxCapData[24] = 0.000000; ui2.fFluxCapData[25] = 0.000000; ui2.fFluxCapData[26] = 0.000000; ui2.fFluxCapData[27] = 0.000000; ui2.fFluxCapData[28] = 0.000000; ui2.fFluxCapData[29] = 0.000000; ui2.fFluxCapData[30] = 0.000000; ui2.fFluxCapData[31] = 0.000000; ui2.fFluxCapData[32] = 0.000000; ui2.fFluxCapData[33] = 0.000000; ui2.fFluxCapData[34] = 0.000000; ui2.fFluxCapData[35] = 0.000000; ui2.fFluxCapData[36] = 0.000000; ui2.fFluxCapData[37] = 0.000000; ui2.fFluxCapData[38] = 0.000000; ui2.fFluxCapData[39] = 0.000000; ui2.fFluxCapData[40] = 0.000000; ui2.fFluxCapData[41] = 0.000000; ui2.fFluxCapData[42] = 0.000000; ui2.fFluxCapData[43] = 0.000000; ui2.fFluxCapData[44] = 0.000000; ui2.fFluxCapData[45] = 0.000000; ui2.fFluxCapData[46] = 0.000000; ui2.fFluxCapData[47] = 0.000000; ui2.fFluxCapData[48] = 0.000000; ui2.fFluxCapData[49] = 0.000000; ui2.fFluxCapData[50] = 0.000000; ui2.fFluxCapData[51] = 0.000000; ui2.fFluxCapData[52] = 0.000000; ui2.fFluxCapData[53] = 0.000000; ui2.fFluxCapData[54] = 0.000000; ui2.fFluxCapData[55] = 0.000000; ui2.fFluxCapData[56] = 0.000000; ui2.fFluxCapData[57] = 0.000000; ui2.fFluxCapData[58] = 0.000000; ui2.fFluxCapData[59] = 0.000000; ui2.fFluxCapData[60] = 0.000000; ui2.fFluxCapData[61] = 0.000000; ui2.fFluxCapData[62] = 0.000000; ui2.fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(ui2);


	m_HF_Attack_mSec = 20.000000;
	CUICtrl ui3;
	ui3.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui3.uControlId = 3;
	ui3.bLogSlider = false;
	ui3.bExpSlider = false;
	ui3.fUserDisplayDataLoLimit = 1.000000;
	ui3.fUserDisplayDataHiLimit = 300.000000;
	ui3.uUserDataType = floatData;
	ui3.fInitUserIntValue = 0;
	ui3.fInitUserFloatValue = 20.000000;
	ui3.fInitUserDoubleValue = 0;
	ui3.fInitUserUINTValue = 0;
	ui3.m_pUserCookedIntData = NULL;
	ui3.m_pUserCookedFloatData = &m_HF_Attack_mSec;
	ui3.m_pUserCookedDoubleData = NULL;
	ui3.m_pUserCookedUINTData = NULL;
	ui3.cControlUnits = "mS                                                              ";
	ui3.cVariableName = "m_HF_Attack_mSec";
	ui3.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui3.dPresetData[0] = 0.000000;ui3.dPresetData[1] = 0.000000;ui3.dPresetData[2] = 0.000000;ui3.dPresetData[3] = 0.000000;ui3.dPresetData[4] = 0.000000;ui3.dPresetData[5] = 0.000000;ui3.dPresetData[6] = 0.000000;ui3.dPresetData[7] = 0.000000;ui3.dPresetData[8] = 0.000000;ui3.dPresetData[9] = 0.000000;ui3.dPresetData[10] = 0.000000;ui3.dPresetData[11] = 0.000000;ui3.dPresetData[12] = 0.000000;ui3.dPresetData[13] = 0.000000;ui3.dPresetData[14] = 0.000000;ui3.dPresetData[15] = 0.000000;
	ui3.cControlName = "HF Attack";
	ui3.bOwnerControl = false;
	ui3.bMIDIControl = false;
	ui3.uMIDIControlCommand = 176;
	ui3.uMIDIControlName = 3;
	ui3.uMIDIControlChannel = 0;
	ui3.nGUIRow = 1;
	ui3.nGUIColumn = 4;
	ui3.uControlTheme[0] = 0; ui3.uControlTheme[1] = 9; ui3.uControlTheme[2] = 0; ui3.uControlTheme[3] = 0; ui3.uControlTheme[4] = 0; ui3.uControlTheme[5] = 1; ui3.uControlTheme[6] = 0; ui3.uControlTheme[7] = 65535; ui3.uControlTheme[8] = 0; ui3.uControlTheme[9] = 11119017; ui3.uControlTheme[10] = 1; ui3.uControlTheme[11] = 12632256; ui3.uControlTheme[12] = 0; ui3.uControlTheme[13] = 6316128; ui3.uControlTheme[14] = 3; ui3.uControlTheme[15] = 8421504; ui3.uControlTheme[16] = 14772545; ui3.uControlTheme[17] = 0; ui3.uControlTheme[18] = 0; ui3.uControlTheme[19] = 0; ui3.uControlTheme[20] = 0; ui3.uControlTheme[21] = 0; ui3.uControlTheme[22] = 0; ui3.uControlTheme[23] = 220; ui3.uControlTheme[24] = 1; ui3.uControlTheme[25] = 0; ui3.uControlTheme[26] = 0; ui3.uControlTheme[27] = 0; ui3.uControlTheme[28] = 0; ui3.uControlTheme[29] = 0; ui3.uControlTheme[30] = 0; ui3.uControlTheme[31] = 0; 
	ui3.uFluxCapControl[0] = 0; ui3.uFluxCapControl[1] = 0; ui3.uFluxCapControl[2] = 0; ui3.uFluxCapControl[3] = 0; ui3.uFluxCapControl[4] = 0; ui3.uFluxCapControl[5] = 0; ui3.uFluxCapControl[6] = 0; ui3.uFluxCapControl[7] = 0; ui3.uFluxCapControl[8] = 0; ui3.uFluxCapControl[9] = 0; ui3.uFluxCapControl[10] = 0; ui3.uFluxCapControl[11] = 0; ui3.uFluxCapControl[12] = 0; ui3.uFluxCapControl[13] = 0; ui3.uFluxCapControl[14] = 0; ui3.uFluxCapControl[15] = 0; ui3.uFluxCapControl[16] = 0; ui3.uFluxCapControl[17] = 0; ui3.uFluxCapControl[18] = 0; ui3.uFluxCapControl[19] = 0; ui3.uFluxCapControl[20] = 0; ui3.uFluxCapControl[21] = 0; ui3.uFluxCapControl[22] = 0; ui3.uFluxCapControl[23] = 0; ui3.uFluxCapControl[24] = 0; ui3.uFluxCapControl[25] = 0; ui3.uFluxCapControl[26] = 0; ui3.uFluxCapControl[27] = 0; ui3.uFluxCapControl[28] = 0; ui3.uFluxCapControl[29] = 0; ui3.uFluxCapControl[30] = 0; ui3.uFluxCapControl[31] = 0; ui3.uFluxCapControl[32] = 0; ui3.uFluxCapControl[33] = 0; ui3.uFluxCapControl[34] = 0; ui3.uFluxCapControl[35] = 0; ui3.uFluxCapControl[36] = 0; ui3.uFluxCapControl[37] = 0; ui3.uFluxCapControl[38] = 0; ui3.uFluxCapControl[39] = 0; ui3.uFluxCapControl[40] = 0; ui3.uFluxCapControl[41] = 0; ui3.uFluxCapControl[42] = 0; ui3.uFluxCapControl[43] = 0; ui3.uFluxCapControl[44] = 0; ui3.uFluxCapControl[45] = 0; ui3.uFluxCapControl[46] = 0; ui3.uFluxCapControl[47] = 0; ui3.uFluxCapControl[48] = 0; ui3.uFluxCapControl[49] = 0; ui3.uFluxCapControl[50] = 0; ui3.uFluxCapControl[51] = 0; ui3.uFluxCapControl[52] = 0; ui3.uFluxCapControl[53] = 0; ui3.uFluxCapControl[54] = 0; ui3.uFluxCapControl[55] = 0; ui3.uFluxCapControl[56] = 0; ui3.uFluxCapControl[57] = 0; ui3.uFluxCapControl[58] = 0; ui3.uFluxCapControl[59] = 0; ui3.uFluxCapControl[60] = 0; ui3.uFluxCapControl[61] = 0; ui3.uFluxCapControl[62] = 0; ui3.uFluxCapControl[63] = 0; 
	ui3.fFluxCapData[0] = 0.000000; ui3.fFluxCapData[1] = 0.000000; ui3.fFluxCapData[2] = 0.000000; ui3.fFluxCapData[3] = 0.000000; ui3.fFluxCapData[4] = 0.000000; ui3.fFluxCapData[5] = 0.000000; ui3.fFluxCapData[6] = 0.000000; ui3.fFluxCapData[7] = 0.000000; ui3.fFluxCapData[8] = 0.000000; ui3.fFluxCapData[9] = 0.000000; ui3.fFluxCapData[10] = 0.000000; ui3.fFluxCapData[11] = 0.000000; ui3.fFluxCapData[12] = 0.000000; ui3.fFluxCapData[13] = 0.000000; ui3.fFluxCapData[14] = 0.000000; ui3.fFluxCapData[15] = 0.000000; ui3.fFluxCapData[16] = 0.000000; ui3.fFluxCapData[17] = 0.000000; ui3.fFluxCapData[18] = 0.000000; ui3.fFluxCapData[19] = 0.000000; ui3.fFluxCapData[20] = 0.000000; ui3.fFluxCapData[21] = 0.000000; ui3.fFluxCapData[22] = 0.000000; ui3.fFluxCapData[23] = 0.000000; ui3.fFluxCapData[24] = 0.000000; ui3.fFluxCapData[25] = 0.000000; ui3.fFluxCapData[26] = 0.000000; ui3.fFluxCapData[27] = 0.000000; ui3.fFluxCapData[28] = 0.000000; ui3.fFluxCapData[29] = 0.000000; ui3.fFluxCapData[30] = 0.000000; ui3.fFluxCapData[31] = 0.000000; ui3.fFluxCapData[32] = 0.000000; ui3.fFluxCapData[33] = 0.000000; ui3.fFluxCapData[34] = 0.000000; ui3.fFluxCapData[35] = 0.000000; ui3.fFluxCapData[36] = 0.000000; ui3.fFluxCapData[37] = 0.000000; ui3.fFluxCapData[38] = 0.000000; ui3.fFluxCapData[39] = 0.000000; ui3.fFluxCapData[40] = 0.000000; ui3.fFluxCapData[41] = 0.000000; ui3.fFluxCapData[42] = 0.000000; ui3.fFluxCapData[43] = 0.000000; ui3.fFluxCapData[44] = 0.000000; ui3.fFluxCapData[45] = 0.000000; ui3.fFluxCapData[46] = 0.000000; ui3.fFluxCapData[47] = 0.000000; ui3.fFluxCapData[48] = 0.000000; ui3.fFluxCapData[49] = 0.000000; ui3.fFluxCapData[50] = 0.000000; ui3.fFluxCapData[51] = 0.000000; ui3.fFluxCapData[52] = 0.000000; ui3.fFluxCapData[53] = 0.000000; ui3.fFluxCapData[54] = 0.000000; ui3.fFluxCapData[55] = 0.000000; ui3.fFluxCapData[56] = 0.000000; ui3.fFluxCapData[57] = 0.000000; ui3.fFluxCapData[58] = 0.000000; ui3.fFluxCapData[59] = 0.000000; ui3.fFluxCapData[60] = 0.000000; ui3.fFluxCapData[61] = 0.000000; ui3.fFluxCapData[62] = 0.000000; ui3.fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(ui3);


	m_HF_Release_mSec = 1000.000000;
	CUICtrl ui4;
	ui4.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui4.uControlId = 4;
	ui4.bLogSlider = false;
	ui4.bExpSlider = false;
	ui4.fUserDisplayDataLoLimit = 20.000000;
	ui4.fUserDisplayDataHiLimit = 5000.000000;
	ui4.uUserDataType = floatData;
	ui4.fInitUserIntValue = 0;
	ui4.fInitUserFloatValue = 1000.000000;
	ui4.fInitUserDoubleValue = 0;
	ui4.fInitUserUINTValue = 0;
	ui4.m_pUserCookedIntData = NULL;
	ui4.m_pUserCookedFloatData = &m_HF_Release_mSec;
	ui4.m_pUserCookedDoubleData = NULL;
	ui4.m_pUserCookedUINTData = NULL;
	ui4.cControlUnits = "mS                                                              ";
	ui4.cVariableName = "m_HF_Release_mSec";
	ui4.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui4.dPresetData[0] = 0.000000;ui4.dPresetData[1] = 0.000000;ui4.dPresetData[2] = 0.000000;ui4.dPresetData[3] = 0.000000;ui4.dPresetData[4] = 0.000000;ui4.dPresetData[5] = 0.000000;ui4.dPresetData[6] = 0.000000;ui4.dPresetData[7] = 0.000000;ui4.dPresetData[8] = 0.000000;ui4.dPresetData[9] = 0.000000;ui4.dPresetData[10] = 0.000000;ui4.dPresetData[11] = 0.000000;ui4.dPresetData[12] = 0.000000;ui4.dPresetData[13] = 0.000000;ui4.dPresetData[14] = 0.000000;ui4.dPresetData[15] = 0.000000;
	ui4.cControlName = "HF Rel";
	ui4.bOwnerControl = false;
	ui4.bMIDIControl = false;
	ui4.uMIDIControlCommand = 176;
	ui4.uMIDIControlName = 3;
	ui4.uMIDIControlChannel = 0;
	ui4.nGUIRow = 1;
	ui4.nGUIColumn = 5;
	ui4.uControlTheme[0] = 0; ui4.uControlTheme[1] = 9; ui4.uControlTheme[2] = 0; ui4.uControlTheme[3] = 0; ui4.uControlTheme[4] = 0; ui4.uControlTheme[5] = 1; ui4.uControlTheme[6] = 0; ui4.uControlTheme[7] = 65535; ui4.uControlTheme[8] = 0; ui4.uControlTheme[9] = 11119017; ui4.uControlTheme[10] = 1; ui4.uControlTheme[11] = 12632256; ui4.uControlTheme[12] = 0; ui4.uControlTheme[13] = 6316128; ui4.uControlTheme[14] = 3; ui4.uControlTheme[15] = 8421504; ui4.uControlTheme[16] = 14772545; ui4.uControlTheme[17] = 0; ui4.uControlTheme[18] = 0; ui4.uControlTheme[19] = 0; ui4.uControlTheme[20] = 0; ui4.uControlTheme[21] = 0; ui4.uControlTheme[22] = 0; ui4.uControlTheme[23] = 293; ui4.uControlTheme[24] = 1; ui4.uControlTheme[25] = 0; ui4.uControlTheme[26] = 0; ui4.uControlTheme[27] = 0; ui4.uControlTheme[28] = 0; ui4.uControlTheme[29] = 0; ui4.uControlTheme[30] = 0; ui4.uControlTheme[31] = 0; 
	ui4.uFluxCapControl[0] = 0; ui4.uFluxCapControl[1] = 0; ui4.uFluxCapControl[2] = 0; ui4.uFluxCapControl[3] = 0; ui4.uFluxCapControl[4] = 0; ui4.uFluxCapControl[5] = 0; ui4.uFluxCapControl[6] = 0; ui4.uFluxCapControl[7] = 0; ui4.uFluxCapControl[8] = 0; ui4.uFluxCapControl[9] = 0; ui4.uFluxCapControl[10] = 0; ui4.uFluxCapControl[11] = 0; ui4.uFluxCapControl[12] = 0; ui4.uFluxCapControl[13] = 0; ui4.uFluxCapControl[14] = 0; ui4.uFluxCapControl[15] = 0; ui4.uFluxCapControl[16] = 0; ui4.uFluxCapControl[17] = 0; ui4.uFluxCapControl[18] = 0; ui4.uFluxCapControl[19] = 0; ui4.uFluxCapControl[20] = 0; ui4.uFluxCapControl[21] = 0; ui4.uFluxCapControl[22] = 0; ui4.uFluxCapControl[23] = 0; ui4.uFluxCapControl[24] = 0; ui4.uFluxCapControl[25] = 0; ui4.uFluxCapControl[26] = 0; ui4.uFluxCapControl[27] = 0; ui4.uFluxCapControl[28] = 0; ui4.uFluxCapControl[29] = 0; ui4.uFluxCapControl[30] = 0; ui4.uFluxCapControl[31] = 0; ui4.uFluxCapControl[32] = 0; ui4.uFluxCapControl[33] = 0; ui4.uFluxCapControl[34] = 0; ui4.uFluxCapControl[35] = 0; ui4.uFluxCapControl[36] = 0; ui4.uFluxCapControl[37] = 0; ui4.uFluxCapControl[38] = 0; ui4.uFluxCapControl[39] = 0; ui4.uFluxCapControl[40] = 0; ui4.uFluxCapControl[41] = 0; ui4.uFluxCapControl[42] = 0; ui4.uFluxCapControl[43] = 0; ui4.uFluxCapControl[44] = 0; ui4.uFluxCapControl[45] = 0; ui4.uFluxCapControl[46] = 0; ui4.uFluxCapControl[47] = 0; ui4.uFluxCapControl[48] = 0; ui4.uFluxCapControl[49] = 0; ui4.uFluxCapControl[50] = 0; ui4.uFluxCapControl[51] = 0; ui4.uFluxCapControl[52] = 0; ui4.uFluxCapControl[53] = 0; ui4.uFluxCapControl[54] = 0; ui4.uFluxCapControl[55] = 0; ui4.uFluxCapControl[56] = 0; ui4.uFluxCapControl[57] = 0; ui4.uFluxCapControl[58] = 0; ui4.uFluxCapControl[59] = 0; ui4.uFluxCapControl[60] = 0; ui4.uFluxCapControl[61] = 0; ui4.uFluxCapControl[62] = 0; ui4.uFluxCapControl[63] = 0; 
	ui4.fFluxCapData[0] = 0.000000; ui4.fFluxCapData[1] = 0.000000; ui4.fFluxCapData[2] = 0.000000; ui4.fFluxCapData[3] = 0.000000; ui4.fFluxCapData[4] = 0.000000; ui4.fFluxCapData[5] = 0.000000; ui4.fFluxCapData[6] = 0.000000; ui4.fFluxCapData[7] = 0.000000; ui4.fFluxCapData[8] = 0.000000; ui4.fFluxCapData[9] = 0.000000; ui4.fFluxCapData[10] = 0.000000; ui4.fFluxCapData[11] = 0.000000; ui4.fFluxCapData[12] = 0.000000; ui4.fFluxCapData[13] = 0.000000; ui4.fFluxCapData[14] = 0.000000; ui4.fFluxCapData[15] = 0.000000; ui4.fFluxCapData[16] = 0.000000; ui4.fFluxCapData[17] = 0.000000; ui4.fFluxCapData[18] = 0.000000; ui4.fFluxCapData[19] = 0.000000; ui4.fFluxCapData[20] = 0.000000; ui4.fFluxCapData[21] = 0.000000; ui4.fFluxCapData[22] = 0.000000; ui4.fFluxCapData[23] = 0.000000; ui4.fFluxCapData[24] = 0.000000; ui4.fFluxCapData[25] = 0.000000; ui4.fFluxCapData[26] = 0.000000; ui4.fFluxCapData[27] = 0.000000; ui4.fFluxCapData[28] = 0.000000; ui4.fFluxCapData[29] = 0.000000; ui4.fFluxCapData[30] = 0.000000; ui4.fFluxCapData[31] = 0.000000; ui4.fFluxCapData[32] = 0.000000; ui4.fFluxCapData[33] = 0.000000; ui4.fFluxCapData[34] = 0.000000; ui4.fFluxCapData[35] = 0.000000; ui4.fFluxCapData[36] = 0.000000; ui4.fFluxCapData[37] = 0.000000; ui4.fFluxCapData[38] = 0.000000; ui4.fFluxCapData[39] = 0.000000; ui4.fFluxCapData[40] = 0.000000; ui4.fFluxCapData[41] = 0.000000; ui4.fFluxCapData[42] = 0.000000; ui4.fFluxCapData[43] = 0.000000; ui4.fFluxCapData[44] = 0.000000; ui4.fFluxCapData[45] = 0.000000; ui4.fFluxCapData[46] = 0.000000; ui4.fFluxCapData[47] = 0.000000; ui4.fFluxCapData[48] = 0.000000; ui4.fFluxCapData[49] = 0.000000; ui4.fFluxCapData[50] = 0.000000; ui4.fFluxCapData[51] = 0.000000; ui4.fFluxCapData[52] = 0.000000; ui4.fFluxCapData[53] = 0.000000; ui4.fFluxCapData[54] = 0.000000; ui4.fFluxCapData[55] = 0.000000; ui4.fFluxCapData[56] = 0.000000; ui4.fFluxCapData[57] = 0.000000; ui4.fFluxCapData[58] = 0.000000; ui4.fFluxCapData[59] = 0.000000; ui4.fFluxCapData[60] = 0.000000; ui4.fFluxCapData[61] = 0.000000; ui4.fFluxCapData[62] = 0.000000; ui4.fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(ui4);


	m_HF_Ratio = 1.000000;
	CUICtrl ui5;
	ui5.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui5.uControlId = 5;
	ui5.bLogSlider = false;
	ui5.bExpSlider = false;
	ui5.fUserDisplayDataLoLimit = 1.000000;
	ui5.fUserDisplayDataHiLimit = 20.000000;
	ui5.uUserDataType = floatData;
	ui5.fInitUserIntValue = 0;
	ui5.fInitUserFloatValue = 1.000000;
	ui5.fInitUserDoubleValue = 0;
	ui5.fInitUserUINTValue = 0;
	ui5.m_pUserCookedIntData = NULL;
	ui5.m_pUserCookedFloatData = &m_HF_Ratio;
	ui5.m_pUserCookedDoubleData = NULL;
	ui5.m_pUserCookedUINTData = NULL;
	ui5.cControlUnits = "                                                                ";
	ui5.cVariableName = "m_HF_Ratio";
	ui5.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui5.dPresetData[0] = 0.000000;ui5.dPresetData[1] = 0.000000;ui5.dPresetData[2] = 0.000000;ui5.dPresetData[3] = 0.000000;ui5.dPresetData[4] = 0.000000;ui5.dPresetData[5] = 0.000000;ui5.dPresetData[6] = 0.000000;ui5.dPresetData[7] = 0.000000;ui5.dPresetData[8] = 0.000000;ui5.dPresetData[9] = 0.000000;ui5.dPresetData[10] = 0.000000;ui5.dPresetData[11] = 0.000000;ui5.dPresetData[12] = 0.000000;ui5.dPresetData[13] = 0.000000;ui5.dPresetData[14] = 0.000000;ui5.dPresetData[15] = 0.000000;
	ui5.cControlName = "HF Ratio";
	ui5.bOwnerControl = false;
	ui5.bMIDIControl = false;
	ui5.uMIDIControlCommand = 176;
	ui5.uMIDIControlName = 3;
	ui5.uMIDIControlChannel = 0;
	ui5.nGUIRow = 1;
	ui5.nGUIColumn = 6;
	ui5.uControlTheme[0] = 0; ui5.uControlTheme[1] = 9; ui5.uControlTheme[2] = 0; ui5.uControlTheme[3] = 0; ui5.uControlTheme[4] = 0; ui5.uControlTheme[5] = 1; ui5.uControlTheme[6] = 0; ui5.uControlTheme[7] = 65535; ui5.uControlTheme[8] = 0; ui5.uControlTheme[9] = 11119017; ui5.uControlTheme[10] = 1; ui5.uControlTheme[11] = 12632256; ui5.uControlTheme[12] = 0; ui5.uControlTheme[13] = 6316128; ui5.uControlTheme[14] = 3; ui5.uControlTheme[15] = 8421504; ui5.uControlTheme[16] = 14772545; ui5.uControlTheme[17] = 0; ui5.uControlTheme[18] = 0; ui5.uControlTheme[19] = 0; ui5.uControlTheme[20] = 0; ui5.uControlTheme[21] = 0; ui5.uControlTheme[22] = 0; ui5.uControlTheme[23] = 362; ui5.uControlTheme[24] = 1; ui5.uControlTheme[25] = 0; ui5.uControlTheme[26] = 0; ui5.uControlTheme[27] = 0; ui5.uControlTheme[28] = 0; ui5.uControlTheme[29] = 0; ui5.uControlTheme[30] = 0; ui5.uControlTheme[31] = 0; 
	ui5.uFluxCapControl[0] = 0; ui5.uFluxCapControl[1] = 0; ui5.uFluxCapControl[2] = 0; ui5.uFluxCapControl[3] = 0; ui5.uFluxCapControl[4] = 0; ui5.uFluxCapControl[5] = 0; ui5.uFluxCapControl[6] = 0; ui5.uFluxCapControl[7] = 0; ui5.uFluxCapControl[8] = 0; ui5.uFluxCapControl[9] = 0; ui5.uFluxCapControl[10] = 0; ui5.uFluxCapControl[11] = 0; ui5.uFluxCapControl[12] = 0; ui5.uFluxCapControl[13] = 0; ui5.uFluxCapControl[14] = 0; ui5.uFluxCapControl[15] = 0; ui5.uFluxCapControl[16] = 0; ui5.uFluxCapControl[17] = 0; ui5.uFluxCapControl[18] = 0; ui5.uFluxCapControl[19] = 0; ui5.uFluxCapControl[20] = 0; ui5.uFluxCapControl[21] = 0; ui5.uFluxCapControl[22] = 0; ui5.uFluxCapControl[23] = 0; ui5.uFluxCapControl[24] = 0; ui5.uFluxCapControl[25] = 0; ui5.uFluxCapControl[26] = 0; ui5.uFluxCapControl[27] = 0; ui5.uFluxCapControl[28] = 0; ui5.uFluxCapControl[29] = 0; ui5.uFluxCapControl[30] = 0; ui5.uFluxCapControl[31] = 0; ui5.uFluxCapControl[32] = 0; ui5.uFluxCapControl[33] = 0; ui5.uFluxCapControl[34] = 0; ui5.uFluxCapControl[35] = 0; ui5.uFluxCapControl[36] = 0; ui5.uFluxCapControl[37] = 0; ui5.uFluxCapControl[38] = 0; ui5.uFluxCapControl[39] = 0; ui5.uFluxCapControl[40] = 0; ui5.uFluxCapControl[41] = 0; ui5.uFluxCapControl[42] = 0; ui5.uFluxCapControl[43] = 0; ui5.uFluxCapControl[44] = 0; ui5.uFluxCapControl[45] = 0; ui5.uFluxCapControl[46] = 0; ui5.uFluxCapControl[47] = 0; ui5.uFluxCapControl[48] = 0; ui5.uFluxCapControl[49] = 0; ui5.uFluxCapControl[50] = 0; ui5.uFluxCapControl[51] = 0; ui5.uFluxCapControl[52] = 0; ui5.uFluxCapControl[53] = 0; ui5.uFluxCapControl[54] = 0; ui5.uFluxCapControl[55] = 0; ui5.uFluxCapControl[56] = 0; ui5.uFluxCapControl[57] = 0; ui5.uFluxCapControl[58] = 0; ui5.uFluxCapControl[59] = 0; ui5.uFluxCapControl[60] = 0; ui5.uFluxCapControl[61] = 0; ui5.uFluxCapControl[62] = 0; ui5.uFluxCapControl[63] = 0; 
	ui5.fFluxCapData[0] = 0.000000; ui5.fFluxCapData[1] = 0.000000; ui5.fFluxCapData[2] = 0.000000; ui5.fFluxCapData[3] = 0.000000; ui5.fFluxCapData[4] = 0.000000; ui5.fFluxCapData[5] = 0.000000; ui5.fFluxCapData[6] = 0.000000; ui5.fFluxCapData[7] = 0.000000; ui5.fFluxCapData[8] = 0.000000; ui5.fFluxCapData[9] = 0.000000; ui5.fFluxCapData[10] = 0.000000; ui5.fFluxCapData[11] = 0.000000; ui5.fFluxCapData[12] = 0.000000; ui5.fFluxCapData[13] = 0.000000; ui5.fFluxCapData[14] = 0.000000; ui5.fFluxCapData[15] = 0.000000; ui5.fFluxCapData[16] = 0.000000; ui5.fFluxCapData[17] = 0.000000; ui5.fFluxCapData[18] = 0.000000; ui5.fFluxCapData[19] = 0.000000; ui5.fFluxCapData[20] = 0.000000; ui5.fFluxCapData[21] = 0.000000; ui5.fFluxCapData[22] = 0.000000; ui5.fFluxCapData[23] = 0.000000; ui5.fFluxCapData[24] = 0.000000; ui5.fFluxCapData[25] = 0.000000; ui5.fFluxCapData[26] = 0.000000; ui5.fFluxCapData[27] = 0.000000; ui5.fFluxCapData[28] = 0.000000; ui5.fFluxCapData[29] = 0.000000; ui5.fFluxCapData[30] = 0.000000; ui5.fFluxCapData[31] = 0.000000; ui5.fFluxCapData[32] = 0.000000; ui5.fFluxCapData[33] = 0.000000; ui5.fFluxCapData[34] = 0.000000; ui5.fFluxCapData[35] = 0.000000; ui5.fFluxCapData[36] = 0.000000; ui5.fFluxCapData[37] = 0.000000; ui5.fFluxCapData[38] = 0.000000; ui5.fFluxCapData[39] = 0.000000; ui5.fFluxCapData[40] = 0.000000; ui5.fFluxCapData[41] = 0.000000; ui5.fFluxCapData[42] = 0.000000; ui5.fFluxCapData[43] = 0.000000; ui5.fFluxCapData[44] = 0.000000; ui5.fFluxCapData[45] = 0.000000; ui5.fFluxCapData[46] = 0.000000; ui5.fFluxCapData[47] = 0.000000; ui5.fFluxCapData[48] = 0.000000; ui5.fFluxCapData[49] = 0.000000; ui5.fFluxCapData[50] = 0.000000; ui5.fFluxCapData[51] = 0.000000; ui5.fFluxCapData[52] = 0.000000; ui5.fFluxCapData[53] = 0.000000; ui5.fFluxCapData[54] = 0.000000; ui5.fFluxCapData[55] = 0.000000; ui5.fFluxCapData[56] = 0.000000; ui5.fFluxCapData[57] = 0.000000; ui5.fFluxCapData[58] = 0.000000; ui5.fFluxCapData[59] = 0.000000; ui5.fFluxCapData[60] = 0.000000; ui5.fFluxCapData[61] = 0.000000; ui5.fFluxCapData[62] = 0.000000; ui5.fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(ui5);


	m_HF_MakeUpGain_dB = 0.000000;
	CUICtrl ui6;
	ui6.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui6.uControlId = 6;
	ui6.bLogSlider = false;
	ui6.bExpSlider = false;
	ui6.fUserDisplayDataLoLimit = 0.000000;
	ui6.fUserDisplayDataHiLimit = 20.000000;
	ui6.uUserDataType = floatData;
	ui6.fInitUserIntValue = 0;
	ui6.fInitUserFloatValue = 0.000000;
	ui6.fInitUserDoubleValue = 0;
	ui6.fInitUserUINTValue = 0;
	ui6.m_pUserCookedIntData = NULL;
	ui6.m_pUserCookedFloatData = &m_HF_MakeUpGain_dB;
	ui6.m_pUserCookedDoubleData = NULL;
	ui6.m_pUserCookedUINTData = NULL;
	ui6.cControlUnits = "dB                                                              ";
	ui6.cVariableName = "m_HF_MakeUpGain_dB";
	ui6.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui6.dPresetData[0] = 0.000000;ui6.dPresetData[1] = 0.000000;ui6.dPresetData[2] = 0.000000;ui6.dPresetData[3] = 0.000000;ui6.dPresetData[4] = 0.000000;ui6.dPresetData[5] = 0.000000;ui6.dPresetData[6] = 0.000000;ui6.dPresetData[7] = 0.000000;ui6.dPresetData[8] = 0.000000;ui6.dPresetData[9] = 0.000000;ui6.dPresetData[10] = 0.000000;ui6.dPresetData[11] = 0.000000;ui6.dPresetData[12] = 0.000000;ui6.dPresetData[13] = 0.000000;ui6.dPresetData[14] = 0.000000;ui6.dPresetData[15] = 0.000000;
	ui6.cControlName = "HF Gain";
	ui6.bOwnerControl = false;
	ui6.bMIDIControl = false;
	ui6.uMIDIControlCommand = 176;
	ui6.uMIDIControlName = 3;
	ui6.uMIDIControlChannel = 0;
	ui6.nGUIRow = 1;
	ui6.nGUIColumn = 7;
	ui6.uControlTheme[0] = 0; ui6.uControlTheme[1] = 9; ui6.uControlTheme[2] = 0; ui6.uControlTheme[3] = 2; ui6.uControlTheme[4] = 0; ui6.uControlTheme[5] = 1; ui6.uControlTheme[6] = 0; ui6.uControlTheme[7] = 65535; ui6.uControlTheme[8] = 0; ui6.uControlTheme[9] = 11119017; ui6.uControlTheme[10] = 1; ui6.uControlTheme[11] = 12632256; ui6.uControlTheme[12] = 0; ui6.uControlTheme[13] = 6316128; ui6.uControlTheme[14] = 3; ui6.uControlTheme[15] = 8421504; ui6.uControlTheme[16] = 14772545; ui6.uControlTheme[17] = 0; ui6.uControlTheme[18] = 0; ui6.uControlTheme[19] = 0; ui6.uControlTheme[20] = 0; ui6.uControlTheme[21] = 0; ui6.uControlTheme[22] = 0; ui6.uControlTheme[23] = 432; ui6.uControlTheme[24] = 1; ui6.uControlTheme[25] = 0; ui6.uControlTheme[26] = 0; ui6.uControlTheme[27] = 0; ui6.uControlTheme[28] = 0; ui6.uControlTheme[29] = 0; ui6.uControlTheme[30] = 0; ui6.uControlTheme[31] = 0; 
	ui6.uFluxCapControl[0] = 0; ui6.uFluxCapControl[1] = 0; ui6.uFluxCapControl[2] = 0; ui6.uFluxCapControl[3] = 0; ui6.uFluxCapControl[4] = 0; ui6.uFluxCapControl[5] = 0; ui6.uFluxCapControl[6] = 0; ui6.uFluxCapControl[7] = 0; ui6.uFluxCapControl[8] = 0; ui6.uFluxCapControl[9] = 0; ui6.uFluxCapControl[10] = 0; ui6.uFluxCapControl[11] = 0; ui6.uFluxCapControl[12] = 0; ui6.uFluxCapControl[13] = 0; ui6.uFluxCapControl[14] = 0; ui6.uFluxCapControl[15] = 0; ui6.uFluxCapControl[16] = 0; ui6.uFluxCapControl[17] = 0; ui6.uFluxCapControl[18] = 0; ui6.uFluxCapControl[19] = 0; ui6.uFluxCapControl[20] = 0; ui6.uFluxCapControl[21] = 0; ui6.uFluxCapControl[22] = 0; ui6.uFluxCapControl[23] = 0; ui6.uFluxCapControl[24] = 0; ui6.uFluxCapControl[25] = 0; ui6.uFluxCapControl[26] = 0; ui6.uFluxCapControl[27] = 0; ui6.uFluxCapControl[28] = 0; ui6.uFluxCapControl[29] = 0; ui6.uFluxCapControl[30] = 0; ui6.uFluxCapControl[31] = 0; ui6.uFluxCapControl[32] = 0; ui6.uFluxCapControl[33] = 0; ui6.uFluxCapControl[34] = 0; ui6.uFluxCapControl[35] = 0; ui6.uFluxCapControl[36] = 0; ui6.uFluxCapControl[37] = 0; ui6.uFluxCapControl[38] = 0; ui6.uFluxCapControl[39] = 0; ui6.uFluxCapControl[40] = 0; ui6.uFluxCapControl[41] = 0; ui6.uFluxCapControl[42] = 0; ui6.uFluxCapControl[43] = 0; ui6.uFluxCapControl[44] = 0; ui6.uFluxCapControl[45] = 0; ui6.uFluxCapControl[46] = 0; ui6.uFluxCapControl[47] = 0; ui6.uFluxCapControl[48] = 0; ui6.uFluxCapControl[49] = 0; ui6.uFluxCapControl[50] = 0; ui6.uFluxCapControl[51] = 0; ui6.uFluxCapControl[52] = 0; ui6.uFluxCapControl[53] = 0; ui6.uFluxCapControl[54] = 0; ui6.uFluxCapControl[55] = 0; ui6.uFluxCapControl[56] = 0; ui6.uFluxCapControl[57] = 0; ui6.uFluxCapControl[58] = 0; ui6.uFluxCapControl[59] = 0; ui6.uFluxCapControl[60] = 0; ui6.uFluxCapControl[61] = 0; ui6.uFluxCapControl[62] = 0; ui6.uFluxCapControl[63] = 0; 
	ui6.fFluxCapData[0] = 0.000000; ui6.fFluxCapData[1] = 0.000000; ui6.fFluxCapData[2] = 0.000000; ui6.fFluxCapData[3] = 0.000000; ui6.fFluxCapData[4] = 0.000000; ui6.fFluxCapData[5] = 0.000000; ui6.fFluxCapData[6] = 0.000000; ui6.fFluxCapData[7] = 0.000000; ui6.fFluxCapData[8] = 0.000000; ui6.fFluxCapData[9] = 0.000000; ui6.fFluxCapData[10] = 0.000000; ui6.fFluxCapData[11] = 0.000000; ui6.fFluxCapData[12] = 0.000000; ui6.fFluxCapData[13] = 0.000000; ui6.fFluxCapData[14] = 0.000000; ui6.fFluxCapData[15] = 0.000000; ui6.fFluxCapData[16] = 0.000000; ui6.fFluxCapData[17] = 0.000000; ui6.fFluxCapData[18] = 0.000000; ui6.fFluxCapData[19] = 0.000000; ui6.fFluxCapData[20] = 0.000000; ui6.fFluxCapData[21] = 0.000000; ui6.fFluxCapData[22] = 0.000000; ui6.fFluxCapData[23] = 0.000000; ui6.fFluxCapData[24] = 0.000000; ui6.fFluxCapData[25] = 0.000000; ui6.fFluxCapData[26] = 0.000000; ui6.fFluxCapData[27] = 0.000000; ui6.fFluxCapData[28] = 0.000000; ui6.fFluxCapData[29] = 0.000000; ui6.fFluxCapData[30] = 0.000000; ui6.fFluxCapData[31] = 0.000000; ui6.fFluxCapData[32] = 0.000000; ui6.fFluxCapData[33] = 0.000000; ui6.fFluxCapData[34] = 0.000000; ui6.fFluxCapData[35] = 0.000000; ui6.fFluxCapData[36] = 0.000000; ui6.fFluxCapData[37] = 0.000000; ui6.fFluxCapData[38] = 0.000000; ui6.fFluxCapData[39] = 0.000000; ui6.fFluxCapData[40] = 0.000000; ui6.fFluxCapData[41] = 0.000000; ui6.fFluxCapData[42] = 0.000000; ui6.fFluxCapData[43] = 0.000000; ui6.fFluxCapData[44] = 0.000000; ui6.fFluxCapData[45] = 0.000000; ui6.fFluxCapData[46] = 0.000000; ui6.fFluxCapData[47] = 0.000000; ui6.fFluxCapData[48] = 0.000000; ui6.fFluxCapData[49] = 0.000000; ui6.fFluxCapData[50] = 0.000000; ui6.fFluxCapData[51] = 0.000000; ui6.fFluxCapData[52] = 0.000000; ui6.fFluxCapData[53] = 0.000000; ui6.fFluxCapData[54] = 0.000000; ui6.fFluxCapData[55] = 0.000000; ui6.fFluxCapData[56] = 0.000000; ui6.fFluxCapData[57] = 0.000000; ui6.fFluxCapData[58] = 0.000000; ui6.fFluxCapData[59] = 0.000000; ui6.fFluxCapData[60] = 0.000000; ui6.fFluxCapData[61] = 0.000000; ui6.fFluxCapData[62] = 0.000000; ui6.fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(ui6);


	m_fKneeWidth = 0.000000;
	CUICtrl ui7;
	ui7.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui7.uControlId = 10;
	ui7.bLogSlider = false;
	ui7.bExpSlider = false;
	ui7.fUserDisplayDataLoLimit = 0.000000;
	ui7.fUserDisplayDataHiLimit = 20.000000;
	ui7.uUserDataType = floatData;
	ui7.fInitUserIntValue = 0;
	ui7.fInitUserFloatValue = 0.000000;
	ui7.fInitUserDoubleValue = 0;
	ui7.fInitUserUINTValue = 0;
	ui7.m_pUserCookedIntData = NULL;
	ui7.m_pUserCookedFloatData = &m_fKneeWidth;
	ui7.m_pUserCookedDoubleData = NULL;
	ui7.m_pUserCookedUINTData = NULL;
	ui7.cControlUnits = "dB                                                              ";
	ui7.cVariableName = "m_fKneeWidth";
	ui7.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui7.dPresetData[0] = 0.000000;ui7.dPresetData[1] = 0.000000;ui7.dPresetData[2] = 0.000000;ui7.dPresetData[3] = 0.000000;ui7.dPresetData[4] = 0.000000;ui7.dPresetData[5] = 0.000000;ui7.dPresetData[6] = 0.000000;ui7.dPresetData[7] = 0.000000;ui7.dPresetData[8] = 0.000000;ui7.dPresetData[9] = 0.000000;ui7.dPresetData[10] = 0.000000;ui7.dPresetData[11] = 0.000000;ui7.dPresetData[12] = 0.000000;ui7.dPresetData[13] = 0.000000;ui7.dPresetData[14] = 0.000000;ui7.dPresetData[15] = 0.000000;
	ui7.cControlName = "Knee Width";
	ui7.bOwnerControl = false;
	ui7.bMIDIControl = false;
	ui7.uMIDIControlCommand = 176;
	ui7.uMIDIControlName = 3;
	ui7.uMIDIControlChannel = 0;
	ui7.nGUIRow = 1;
	ui7.nGUIColumn = 10;
	ui7.uControlTheme[0] = 0; ui7.uControlTheme[1] = 9; ui7.uControlTheme[2] = 0; ui7.uControlTheme[3] = 1; ui7.uControlTheme[4] = 0; ui7.uControlTheme[5] = 1; ui7.uControlTheme[6] = 0; ui7.uControlTheme[7] = 65535; ui7.uControlTheme[8] = 0; ui7.uControlTheme[9] = 11119017; ui7.uControlTheme[10] = 1; ui7.uControlTheme[11] = 12632256; ui7.uControlTheme[12] = 0; ui7.uControlTheme[13] = 6316128; ui7.uControlTheme[14] = 3; ui7.uControlTheme[15] = 8421504; ui7.uControlTheme[16] = 14772545; ui7.uControlTheme[17] = 0; ui7.uControlTheme[18] = 0; ui7.uControlTheme[19] = 0; ui7.uControlTheme[20] = 0; ui7.uControlTheme[21] = 0; ui7.uControlTheme[22] = 0; ui7.uControlTheme[23] = 225; ui7.uControlTheme[24] = 107; ui7.uControlTheme[25] = 0; ui7.uControlTheme[26] = 0; ui7.uControlTheme[27] = 0; ui7.uControlTheme[28] = 0; ui7.uControlTheme[29] = 0; ui7.uControlTheme[30] = 0; ui7.uControlTheme[31] = 0; 
	ui7.uFluxCapControl[0] = 0; ui7.uFluxCapControl[1] = 0; ui7.uFluxCapControl[2] = 0; ui7.uFluxCapControl[3] = 0; ui7.uFluxCapControl[4] = 0; ui7.uFluxCapControl[5] = 0; ui7.uFluxCapControl[6] = 0; ui7.uFluxCapControl[7] = 0; ui7.uFluxCapControl[8] = 0; ui7.uFluxCapControl[9] = 0; ui7.uFluxCapControl[10] = 0; ui7.uFluxCapControl[11] = 0; ui7.uFluxCapControl[12] = 0; ui7.uFluxCapControl[13] = 0; ui7.uFluxCapControl[14] = 0; ui7.uFluxCapControl[15] = 0; ui7.uFluxCapControl[16] = 0; ui7.uFluxCapControl[17] = 0; ui7.uFluxCapControl[18] = 0; ui7.uFluxCapControl[19] = 0; ui7.uFluxCapControl[20] = 0; ui7.uFluxCapControl[21] = 0; ui7.uFluxCapControl[22] = 0; ui7.uFluxCapControl[23] = 0; ui7.uFluxCapControl[24] = 0; ui7.uFluxCapControl[25] = 0; ui7.uFluxCapControl[26] = 0; ui7.uFluxCapControl[27] = 0; ui7.uFluxCapControl[28] = 0; ui7.uFluxCapControl[29] = 0; ui7.uFluxCapControl[30] = 0; ui7.uFluxCapControl[31] = 0; ui7.uFluxCapControl[32] = 0; ui7.uFluxCapControl[33] = 0; ui7.uFluxCapControl[34] = 0; ui7.uFluxCapControl[35] = 0; ui7.uFluxCapControl[36] = 0; ui7.uFluxCapControl[37] = 0; ui7.uFluxCapControl[38] = 0; ui7.uFluxCapControl[39] = 0; ui7.uFluxCapControl[40] = 0; ui7.uFluxCapControl[41] = 0; ui7.uFluxCapControl[42] = 0; ui7.uFluxCapControl[43] = 0; ui7.uFluxCapControl[44] = 0; ui7.uFluxCapControl[45] = 0; ui7.uFluxCapControl[46] = 0; ui7.uFluxCapControl[47] = 0; ui7.uFluxCapControl[48] = 0; ui7.uFluxCapControl[49] = 0; ui7.uFluxCapControl[50] = 0; ui7.uFluxCapControl[51] = 0; ui7.uFluxCapControl[52] = 0; ui7.uFluxCapControl[53] = 0; ui7.uFluxCapControl[54] = 0; ui7.uFluxCapControl[55] = 0; ui7.uFluxCapControl[56] = 0; ui7.uFluxCapControl[57] = 0; ui7.uFluxCapControl[58] = 0; ui7.uFluxCapControl[59] = 0; ui7.uFluxCapControl[60] = 0; ui7.uFluxCapControl[61] = 0; ui7.uFluxCapControl[62] = 0; ui7.uFluxCapControl[63] = 0; 
	ui7.fFluxCapData[0] = 0.000000; ui7.fFluxCapData[1] = 0.000000; ui7.fFluxCapData[2] = 0.000000; ui7.fFluxCapData[3] = 0.000000; ui7.fFluxCapData[4] = 0.000000; ui7.fFluxCapData[5] = 0.000000; ui7.fFluxCapData[6] = 0.000000; ui7.fFluxCapData[7] = 0.000000; ui7.fFluxCapData[8] = 0.000000; ui7.fFluxCapData[9] = 0.000000; ui7.fFluxCapData[10] = 0.000000; ui7.fFluxCapData[11] = 0.000000; ui7.fFluxCapData[12] = 0.000000; ui7.fFluxCapData[13] = 0.000000; ui7.fFluxCapData[14] = 0.000000; ui7.fFluxCapData[15] = 0.000000; ui7.fFluxCapData[16] = 0.000000; ui7.fFluxCapData[17] = 0.000000; ui7.fFluxCapData[18] = 0.000000; ui7.fFluxCapData[19] = 0.000000; ui7.fFluxCapData[20] = 0.000000; ui7.fFluxCapData[21] = 0.000000; ui7.fFluxCapData[22] = 0.000000; ui7.fFluxCapData[23] = 0.000000; ui7.fFluxCapData[24] = 0.000000; ui7.fFluxCapData[25] = 0.000000; ui7.fFluxCapData[26] = 0.000000; ui7.fFluxCapData[27] = 0.000000; ui7.fFluxCapData[28] = 0.000000; ui7.fFluxCapData[29] = 0.000000; ui7.fFluxCapData[30] = 0.000000; ui7.fFluxCapData[31] = 0.000000; ui7.fFluxCapData[32] = 0.000000; ui7.fFluxCapData[33] = 0.000000; ui7.fFluxCapData[34] = 0.000000; ui7.fFluxCapData[35] = 0.000000; ui7.fFluxCapData[36] = 0.000000; ui7.fFluxCapData[37] = 0.000000; ui7.fFluxCapData[38] = 0.000000; ui7.fFluxCapData[39] = 0.000000; ui7.fFluxCapData[40] = 0.000000; ui7.fFluxCapData[41] = 0.000000; ui7.fFluxCapData[42] = 0.000000; ui7.fFluxCapData[43] = 0.000000; ui7.fFluxCapData[44] = 0.000000; ui7.fFluxCapData[45] = 0.000000; ui7.fFluxCapData[46] = 0.000000; ui7.fFluxCapData[47] = 0.000000; ui7.fFluxCapData[48] = 0.000000; ui7.fFluxCapData[49] = 0.000000; ui7.fFluxCapData[50] = 0.000000; ui7.fFluxCapData[51] = 0.000000; ui7.fFluxCapData[52] = 0.000000; ui7.fFluxCapData[53] = 0.000000; ui7.fFluxCapData[54] = 0.000000; ui7.fFluxCapData[55] = 0.000000; ui7.fFluxCapData[56] = 0.000000; ui7.fFluxCapData[57] = 0.000000; ui7.fFluxCapData[58] = 0.000000; ui7.fFluxCapData[59] = 0.000000; ui7.fFluxCapData[60] = 0.000000; ui7.fFluxCapData[61] = 0.000000; ui7.fFluxCapData[62] = 0.000000; ui7.fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(ui7);


	m_LF_DetectorGain_dB = 0.000000;
	CUICtrl ui8;
	ui8.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui8.uControlId = 11;
	ui8.bLogSlider = false;
	ui8.bExpSlider = false;
	ui8.fUserDisplayDataLoLimit = 0.000000;
	ui8.fUserDisplayDataHiLimit = 20.000000;
	ui8.uUserDataType = floatData;
	ui8.fInitUserIntValue = 0;
	ui8.fInitUserFloatValue = 0.000000;
	ui8.fInitUserDoubleValue = 0;
	ui8.fInitUserUINTValue = 0;
	ui8.m_pUserCookedIntData = NULL;
	ui8.m_pUserCookedFloatData = &m_LF_DetectorGain_dB;
	ui8.m_pUserCookedDoubleData = NULL;
	ui8.m_pUserCookedUINTData = NULL;
	ui8.cControlUnits = "dB                                                              ";
	ui8.cVariableName = "m_LF_DetectorGain_dB";
	ui8.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui8.dPresetData[0] = 0.000000;ui8.dPresetData[1] = 0.000000;ui8.dPresetData[2] = 0.000000;ui8.dPresetData[3] = 0.000000;ui8.dPresetData[4] = 0.000000;ui8.dPresetData[5] = 0.000000;ui8.dPresetData[6] = 0.000000;ui8.dPresetData[7] = 0.000000;ui8.dPresetData[8] = 0.000000;ui8.dPresetData[9] = 0.000000;ui8.dPresetData[10] = 0.000000;ui8.dPresetData[11] = 0.000000;ui8.dPresetData[12] = 0.000000;ui8.dPresetData[13] = 0.000000;ui8.dPresetData[14] = 0.000000;ui8.dPresetData[15] = 0.000000;
	ui8.cControlName = "LF Sens";
	ui8.bOwnerControl = false;
	ui8.bMIDIControl = false;
	ui8.uMIDIControlCommand = 176;
	ui8.uMIDIControlName = 3;
	ui8.uMIDIControlChannel = 0;
	ui8.nGUIRow = 1;
	ui8.nGUIColumn = 8;
	ui8.uControlTheme[0] = 0; ui8.uControlTheme[1] = 9; ui8.uControlTheme[2] = 0; ui8.uControlTheme[3] = 1; ui8.uControlTheme[4] = 0; ui8.uControlTheme[5] = 1; ui8.uControlTheme[6] = 0; ui8.uControlTheme[7] = 65535; ui8.uControlTheme[8] = 0; ui8.uControlTheme[9] = 11119017; ui8.uControlTheme[10] = 1; ui8.uControlTheme[11] = 12632256; ui8.uControlTheme[12] = 0; ui8.uControlTheme[13] = 6316128; ui8.uControlTheme[14] = 3; ui8.uControlTheme[15] = 8421504; ui8.uControlTheme[16] = 14772545; ui8.uControlTheme[17] = 0; ui8.uControlTheme[18] = 0; ui8.uControlTheme[19] = 0; ui8.uControlTheme[20] = 0; ui8.uControlTheme[21] = 0; ui8.uControlTheme[22] = 0; ui8.uControlTheme[23] = 86; ui8.uControlTheme[24] = 106; ui8.uControlTheme[25] = 0; ui8.uControlTheme[26] = 0; ui8.uControlTheme[27] = 0; ui8.uControlTheme[28] = 0; ui8.uControlTheme[29] = 0; ui8.uControlTheme[30] = 0; ui8.uControlTheme[31] = 0; 
	ui8.uFluxCapControl[0] = 0; ui8.uFluxCapControl[1] = 0; ui8.uFluxCapControl[2] = 0; ui8.uFluxCapControl[3] = 0; ui8.uFluxCapControl[4] = 0; ui8.uFluxCapControl[5] = 0; ui8.uFluxCapControl[6] = 0; ui8.uFluxCapControl[7] = 0; ui8.uFluxCapControl[8] = 0; ui8.uFluxCapControl[9] = 0; ui8.uFluxCapControl[10] = 0; ui8.uFluxCapControl[11] = 0; ui8.uFluxCapControl[12] = 0; ui8.uFluxCapControl[13] = 0; ui8.uFluxCapControl[14] = 0; ui8.uFluxCapControl[15] = 0; ui8.uFluxCapControl[16] = 0; ui8.uFluxCapControl[17] = 0; ui8.uFluxCapControl[18] = 0; ui8.uFluxCapControl[19] = 0; ui8.uFluxCapControl[20] = 0; ui8.uFluxCapControl[21] = 0; ui8.uFluxCapControl[22] = 0; ui8.uFluxCapControl[23] = 0; ui8.uFluxCapControl[24] = 0; ui8.uFluxCapControl[25] = 0; ui8.uFluxCapControl[26] = 0; ui8.uFluxCapControl[27] = 0; ui8.uFluxCapControl[28] = 0; ui8.uFluxCapControl[29] = 0; ui8.uFluxCapControl[30] = 0; ui8.uFluxCapControl[31] = 0; ui8.uFluxCapControl[32] = 0; ui8.uFluxCapControl[33] = 0; ui8.uFluxCapControl[34] = 0; ui8.uFluxCapControl[35] = 0; ui8.uFluxCapControl[36] = 0; ui8.uFluxCapControl[37] = 0; ui8.uFluxCapControl[38] = 0; ui8.uFluxCapControl[39] = 0; ui8.uFluxCapControl[40] = 0; ui8.uFluxCapControl[41] = 0; ui8.uFluxCapControl[42] = 0; ui8.uFluxCapControl[43] = 0; ui8.uFluxCapControl[44] = 0; ui8.uFluxCapControl[45] = 0; ui8.uFluxCapControl[46] = 0; ui8.uFluxCapControl[47] = 0; ui8.uFluxCapControl[48] = 0; ui8.uFluxCapControl[49] = 0; ui8.uFluxCapControl[50] = 0; ui8.uFluxCapControl[51] = 0; ui8.uFluxCapControl[52] = 0; ui8.uFluxCapControl[53] = 0; ui8.uFluxCapControl[54] = 0; ui8.uFluxCapControl[55] = 0; ui8.uFluxCapControl[56] = 0; ui8.uFluxCapControl[57] = 0; ui8.uFluxCapControl[58] = 0; ui8.uFluxCapControl[59] = 0; ui8.uFluxCapControl[60] = 0; ui8.uFluxCapControl[61] = 0; ui8.uFluxCapControl[62] = 0; ui8.uFluxCapControl[63] = 0; 
	ui8.fFluxCapData[0] = 0.000000; ui8.fFluxCapData[1] = 0.000000; ui8.fFluxCapData[2] = 0.000000; ui8.fFluxCapData[3] = 0.000000; ui8.fFluxCapData[4] = 0.000000; ui8.fFluxCapData[5] = 0.000000; ui8.fFluxCapData[6] = 0.000000; ui8.fFluxCapData[7] = 0.000000; ui8.fFluxCapData[8] = 0.000000; ui8.fFluxCapData[9] = 0.000000; ui8.fFluxCapData[10] = 0.000000; ui8.fFluxCapData[11] = 0.000000; ui8.fFluxCapData[12] = 0.000000; ui8.fFluxCapData[13] = 0.000000; ui8.fFluxCapData[14] = 0.000000; ui8.fFluxCapData[15] = 0.000000; ui8.fFluxCapData[16] = 0.000000; ui8.fFluxCapData[17] = 0.000000; ui8.fFluxCapData[18] = 0.000000; ui8.fFluxCapData[19] = 0.000000; ui8.fFluxCapData[20] = 0.000000; ui8.fFluxCapData[21] = 0.000000; ui8.fFluxCapData[22] = 0.000000; ui8.fFluxCapData[23] = 0.000000; ui8.fFluxCapData[24] = 0.000000; ui8.fFluxCapData[25] = 0.000000; ui8.fFluxCapData[26] = 0.000000; ui8.fFluxCapData[27] = 0.000000; ui8.fFluxCapData[28] = 0.000000; ui8.fFluxCapData[29] = 0.000000; ui8.fFluxCapData[30] = 0.000000; ui8.fFluxCapData[31] = 0.000000; ui8.fFluxCapData[32] = 0.000000; ui8.fFluxCapData[33] = 0.000000; ui8.fFluxCapData[34] = 0.000000; ui8.fFluxCapData[35] = 0.000000; ui8.fFluxCapData[36] = 0.000000; ui8.fFluxCapData[37] = 0.000000; ui8.fFluxCapData[38] = 0.000000; ui8.fFluxCapData[39] = 0.000000; ui8.fFluxCapData[40] = 0.000000; ui8.fFluxCapData[41] = 0.000000; ui8.fFluxCapData[42] = 0.000000; ui8.fFluxCapData[43] = 0.000000; ui8.fFluxCapData[44] = 0.000000; ui8.fFluxCapData[45] = 0.000000; ui8.fFluxCapData[46] = 0.000000; ui8.fFluxCapData[47] = 0.000000; ui8.fFluxCapData[48] = 0.000000; ui8.fFluxCapData[49] = 0.000000; ui8.fFluxCapData[50] = 0.000000; ui8.fFluxCapData[51] = 0.000000; ui8.fFluxCapData[52] = 0.000000; ui8.fFluxCapData[53] = 0.000000; ui8.fFluxCapData[54] = 0.000000; ui8.fFluxCapData[55] = 0.000000; ui8.fFluxCapData[56] = 0.000000; ui8.fFluxCapData[57] = 0.000000; ui8.fFluxCapData[58] = 0.000000; ui8.fFluxCapData[59] = 0.000000; ui8.fFluxCapData[60] = 0.000000; ui8.fFluxCapData[61] = 0.000000; ui8.fFluxCapData[62] = 0.000000; ui8.fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(ui8);


	m_LF_Threshold = 0.000000;
	CUICtrl ui9;
	ui9.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui9.uControlId = 12;
	ui9.bLogSlider = false;
	ui9.bExpSlider = false;
	ui9.fUserDisplayDataLoLimit = -60.000000;
	ui9.fUserDisplayDataHiLimit = 0.000000;
	ui9.uUserDataType = floatData;
	ui9.fInitUserIntValue = 0;
	ui9.fInitUserFloatValue = 0.000000;
	ui9.fInitUserDoubleValue = 0;
	ui9.fInitUserUINTValue = 0;
	ui9.m_pUserCookedIntData = NULL;
	ui9.m_pUserCookedFloatData = &m_LF_Threshold;
	ui9.m_pUserCookedDoubleData = NULL;
	ui9.m_pUserCookedUINTData = NULL;
	ui9.cControlUnits = "dB                                                              ";
	ui9.cVariableName = "m_LF_Threshold";
	ui9.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui9.dPresetData[0] = 0.000000;ui9.dPresetData[1] = 0.000000;ui9.dPresetData[2] = 0.000000;ui9.dPresetData[3] = 0.000000;ui9.dPresetData[4] = 0.000000;ui9.dPresetData[5] = 0.000000;ui9.dPresetData[6] = 0.000000;ui9.dPresetData[7] = 0.000000;ui9.dPresetData[8] = 0.000000;ui9.dPresetData[9] = 0.000000;ui9.dPresetData[10] = 0.000000;ui9.dPresetData[11] = 0.000000;ui9.dPresetData[12] = 0.000000;ui9.dPresetData[13] = 0.000000;ui9.dPresetData[14] = 0.000000;ui9.dPresetData[15] = 0.000000;
	ui9.cControlName = "LF Threshold";
	ui9.bOwnerControl = false;
	ui9.bMIDIControl = false;
	ui9.uMIDIControlCommand = 176;
	ui9.uMIDIControlName = 3;
	ui9.uMIDIControlChannel = 0;
	ui9.nGUIRow = 1;
	ui9.nGUIColumn = 9;
	ui9.uControlTheme[0] = 0; ui9.uControlTheme[1] = 9; ui9.uControlTheme[2] = 0; ui9.uControlTheme[3] = 1; ui9.uControlTheme[4] = 0; ui9.uControlTheme[5] = 1; ui9.uControlTheme[6] = 0; ui9.uControlTheme[7] = 65535; ui9.uControlTheme[8] = 0; ui9.uControlTheme[9] = 11119017; ui9.uControlTheme[10] = 1; ui9.uControlTheme[11] = 12632256; ui9.uControlTheme[12] = 0; ui9.uControlTheme[13] = 6316128; ui9.uControlTheme[14] = 3; ui9.uControlTheme[15] = 8421504; ui9.uControlTheme[16] = 14772545; ui9.uControlTheme[17] = 0; ui9.uControlTheme[18] = 0; ui9.uControlTheme[19] = 0; ui9.uControlTheme[20] = 0; ui9.uControlTheme[21] = 0; ui9.uControlTheme[22] = 0; ui9.uControlTheme[23] = 156; ui9.uControlTheme[24] = 106; ui9.uControlTheme[25] = 0; ui9.uControlTheme[26] = 0; ui9.uControlTheme[27] = 0; ui9.uControlTheme[28] = 0; ui9.uControlTheme[29] = 0; ui9.uControlTheme[30] = 0; ui9.uControlTheme[31] = 0; 
	ui9.uFluxCapControl[0] = 0; ui9.uFluxCapControl[1] = 0; ui9.uFluxCapControl[2] = 0; ui9.uFluxCapControl[3] = 0; ui9.uFluxCapControl[4] = 0; ui9.uFluxCapControl[5] = 0; ui9.uFluxCapControl[6] = 0; ui9.uFluxCapControl[7] = 0; ui9.uFluxCapControl[8] = 0; ui9.uFluxCapControl[9] = 0; ui9.uFluxCapControl[10] = 0; ui9.uFluxCapControl[11] = 0; ui9.uFluxCapControl[12] = 0; ui9.uFluxCapControl[13] = 0; ui9.uFluxCapControl[14] = 0; ui9.uFluxCapControl[15] = 0; ui9.uFluxCapControl[16] = 0; ui9.uFluxCapControl[17] = 0; ui9.uFluxCapControl[18] = 0; ui9.uFluxCapControl[19] = 0; ui9.uFluxCapControl[20] = 0; ui9.uFluxCapControl[21] = 0; ui9.uFluxCapControl[22] = 0; ui9.uFluxCapControl[23] = 0; ui9.uFluxCapControl[24] = 0; ui9.uFluxCapControl[25] = 0; ui9.uFluxCapControl[26] = 0; ui9.uFluxCapControl[27] = 0; ui9.uFluxCapControl[28] = 0; ui9.uFluxCapControl[29] = 0; ui9.uFluxCapControl[30] = 0; ui9.uFluxCapControl[31] = 0; ui9.uFluxCapControl[32] = 0; ui9.uFluxCapControl[33] = 0; ui9.uFluxCapControl[34] = 0; ui9.uFluxCapControl[35] = 0; ui9.uFluxCapControl[36] = 0; ui9.uFluxCapControl[37] = 0; ui9.uFluxCapControl[38] = 0; ui9.uFluxCapControl[39] = 0; ui9.uFluxCapControl[40] = 0; ui9.uFluxCapControl[41] = 0; ui9.uFluxCapControl[42] = 0; ui9.uFluxCapControl[43] = 0; ui9.uFluxCapControl[44] = 0; ui9.uFluxCapControl[45] = 0; ui9.uFluxCapControl[46] = 0; ui9.uFluxCapControl[47] = 0; ui9.uFluxCapControl[48] = 0; ui9.uFluxCapControl[49] = 0; ui9.uFluxCapControl[50] = 0; ui9.uFluxCapControl[51] = 0; ui9.uFluxCapControl[52] = 0; ui9.uFluxCapControl[53] = 0; ui9.uFluxCapControl[54] = 0; ui9.uFluxCapControl[55] = 0; ui9.uFluxCapControl[56] = 0; ui9.uFluxCapControl[57] = 0; ui9.uFluxCapControl[58] = 0; ui9.uFluxCapControl[59] = 0; ui9.uFluxCapControl[60] = 0; ui9.uFluxCapControl[61] = 0; ui9.uFluxCapControl[62] = 0; ui9.uFluxCapControl[63] = 0; 
	ui9.fFluxCapData[0] = 0.000000; ui9.fFluxCapData[1] = 0.000000; ui9.fFluxCapData[2] = 0.000000; ui9.fFluxCapData[3] = 0.000000; ui9.fFluxCapData[4] = 0.000000; ui9.fFluxCapData[5] = 0.000000; ui9.fFluxCapData[6] = 0.000000; ui9.fFluxCapData[7] = 0.000000; ui9.fFluxCapData[8] = 0.000000; ui9.fFluxCapData[9] = 0.000000; ui9.fFluxCapData[10] = 0.000000; ui9.fFluxCapData[11] = 0.000000; ui9.fFluxCapData[12] = 0.000000; ui9.fFluxCapData[13] = 0.000000; ui9.fFluxCapData[14] = 0.000000; ui9.fFluxCapData[15] = 0.000000; ui9.fFluxCapData[16] = 0.000000; ui9.fFluxCapData[17] = 0.000000; ui9.fFluxCapData[18] = 0.000000; ui9.fFluxCapData[19] = 0.000000; ui9.fFluxCapData[20] = 0.000000; ui9.fFluxCapData[21] = 0.000000; ui9.fFluxCapData[22] = 0.000000; ui9.fFluxCapData[23] = 0.000000; ui9.fFluxCapData[24] = 0.000000; ui9.fFluxCapData[25] = 0.000000; ui9.fFluxCapData[26] = 0.000000; ui9.fFluxCapData[27] = 0.000000; ui9.fFluxCapData[28] = 0.000000; ui9.fFluxCapData[29] = 0.000000; ui9.fFluxCapData[30] = 0.000000; ui9.fFluxCapData[31] = 0.000000; ui9.fFluxCapData[32] = 0.000000; ui9.fFluxCapData[33] = 0.000000; ui9.fFluxCapData[34] = 0.000000; ui9.fFluxCapData[35] = 0.000000; ui9.fFluxCapData[36] = 0.000000; ui9.fFluxCapData[37] = 0.000000; ui9.fFluxCapData[38] = 0.000000; ui9.fFluxCapData[39] = 0.000000; ui9.fFluxCapData[40] = 0.000000; ui9.fFluxCapData[41] = 0.000000; ui9.fFluxCapData[42] = 0.000000; ui9.fFluxCapData[43] = 0.000000; ui9.fFluxCapData[44] = 0.000000; ui9.fFluxCapData[45] = 0.000000; ui9.fFluxCapData[46] = 0.000000; ui9.fFluxCapData[47] = 0.000000; ui9.fFluxCapData[48] = 0.000000; ui9.fFluxCapData[49] = 0.000000; ui9.fFluxCapData[50] = 0.000000; ui9.fFluxCapData[51] = 0.000000; ui9.fFluxCapData[52] = 0.000000; ui9.fFluxCapData[53] = 0.000000; ui9.fFluxCapData[54] = 0.000000; ui9.fFluxCapData[55] = 0.000000; ui9.fFluxCapData[56] = 0.000000; ui9.fFluxCapData[57] = 0.000000; ui9.fFluxCapData[58] = 0.000000; ui9.fFluxCapData[59] = 0.000000; ui9.fFluxCapData[60] = 0.000000; ui9.fFluxCapData[61] = 0.000000; ui9.fFluxCapData[62] = 0.000000; ui9.fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(ui9);


	m_LF_Attack_mSec = 20.000000;
	CUICtrl ui10;
	ui10.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui10.uControlId = 13;
	ui10.bLogSlider = false;
	ui10.bExpSlider = false;
	ui10.fUserDisplayDataLoLimit = 1.000000;
	ui10.fUserDisplayDataHiLimit = 300.000000;
	ui10.uUserDataType = floatData;
	ui10.fInitUserIntValue = 0;
	ui10.fInitUserFloatValue = 20.000000;
	ui10.fInitUserDoubleValue = 0;
	ui10.fInitUserUINTValue = 0;
	ui10.m_pUserCookedIntData = NULL;
	ui10.m_pUserCookedFloatData = &m_LF_Attack_mSec;
	ui10.m_pUserCookedDoubleData = NULL;
	ui10.m_pUserCookedUINTData = NULL;
	ui10.cControlUnits = "mS                                                              ";
	ui10.cVariableName = "m_LF_Attack_mSec";
	ui10.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui10.dPresetData[0] = 0.000000;ui10.dPresetData[1] = 0.000000;ui10.dPresetData[2] = 0.000000;ui10.dPresetData[3] = 0.000000;ui10.dPresetData[4] = 0.000000;ui10.dPresetData[5] = 0.000000;ui10.dPresetData[6] = 0.000000;ui10.dPresetData[7] = 0.000000;ui10.dPresetData[8] = 0.000000;ui10.dPresetData[9] = 0.000000;ui10.dPresetData[10] = 0.000000;ui10.dPresetData[11] = 0.000000;ui10.dPresetData[12] = 0.000000;ui10.dPresetData[13] = 0.000000;ui10.dPresetData[14] = 0.000000;ui10.dPresetData[15] = 0.000000;
	ui10.cControlName = "LF Attack";
	ui10.bOwnerControl = false;
	ui10.bMIDIControl = false;
	ui10.uMIDIControlCommand = 176;
	ui10.uMIDIControlName = 3;
	ui10.uMIDIControlChannel = 0;
	ui10.nGUIRow = 1;
	ui10.nGUIColumn = 10;
	ui10.uControlTheme[0] = 0; ui10.uControlTheme[1] = 9; ui10.uControlTheme[2] = 0; ui10.uControlTheme[3] = 1; ui10.uControlTheme[4] = 0; ui10.uControlTheme[5] = 1; ui10.uControlTheme[6] = 0; ui10.uControlTheme[7] = 65535; ui10.uControlTheme[8] = 0; ui10.uControlTheme[9] = 11119017; ui10.uControlTheme[10] = 1; ui10.uControlTheme[11] = 12632256; ui10.uControlTheme[12] = 0; ui10.uControlTheme[13] = 6316128; ui10.uControlTheme[14] = 3; ui10.uControlTheme[15] = 8421504; ui10.uControlTheme[16] = 14772545; ui10.uControlTheme[17] = 0; ui10.uControlTheme[18] = 0; ui10.uControlTheme[19] = 0; ui10.uControlTheme[20] = 0; ui10.uControlTheme[21] = 0; ui10.uControlTheme[22] = 0; ui10.uControlTheme[23] = 225; ui10.uControlTheme[24] = 107; ui10.uControlTheme[25] = 0; ui10.uControlTheme[26] = 0; ui10.uControlTheme[27] = 0; ui10.uControlTheme[28] = 0; ui10.uControlTheme[29] = 0; ui10.uControlTheme[30] = 0; ui10.uControlTheme[31] = 0; 
	ui10.uFluxCapControl[0] = 0; ui10.uFluxCapControl[1] = 0; ui10.uFluxCapControl[2] = 0; ui10.uFluxCapControl[3] = 0; ui10.uFluxCapControl[4] = 0; ui10.uFluxCapControl[5] = 0; ui10.uFluxCapControl[6] = 0; ui10.uFluxCapControl[7] = 0; ui10.uFluxCapControl[8] = 0; ui10.uFluxCapControl[9] = 0; ui10.uFluxCapControl[10] = 0; ui10.uFluxCapControl[11] = 0; ui10.uFluxCapControl[12] = 0; ui10.uFluxCapControl[13] = 0; ui10.uFluxCapControl[14] = 0; ui10.uFluxCapControl[15] = 0; ui10.uFluxCapControl[16] = 0; ui10.uFluxCapControl[17] = 0; ui10.uFluxCapControl[18] = 0; ui10.uFluxCapControl[19] = 0; ui10.uFluxCapControl[20] = 0; ui10.uFluxCapControl[21] = 0; ui10.uFluxCapControl[22] = 0; ui10.uFluxCapControl[23] = 0; ui10.uFluxCapControl[24] = 0; ui10.uFluxCapControl[25] = 0; ui10.uFluxCapControl[26] = 0; ui10.uFluxCapControl[27] = 0; ui10.uFluxCapControl[28] = 0; ui10.uFluxCapControl[29] = 0; ui10.uFluxCapControl[30] = 0; ui10.uFluxCapControl[31] = 0; ui10.uFluxCapControl[32] = 0; ui10.uFluxCapControl[33] = 0; ui10.uFluxCapControl[34] = 0; ui10.uFluxCapControl[35] = 0; ui10.uFluxCapControl[36] = 0; ui10.uFluxCapControl[37] = 0; ui10.uFluxCapControl[38] = 0; ui10.uFluxCapControl[39] = 0; ui10.uFluxCapControl[40] = 0; ui10.uFluxCapControl[41] = 0; ui10.uFluxCapControl[42] = 0; ui10.uFluxCapControl[43] = 0; ui10.uFluxCapControl[44] = 0; ui10.uFluxCapControl[45] = 0; ui10.uFluxCapControl[46] = 0; ui10.uFluxCapControl[47] = 0; ui10.uFluxCapControl[48] = 0; ui10.uFluxCapControl[49] = 0; ui10.uFluxCapControl[50] = 0; ui10.uFluxCapControl[51] = 0; ui10.uFluxCapControl[52] = 0; ui10.uFluxCapControl[53] = 0; ui10.uFluxCapControl[54] = 0; ui10.uFluxCapControl[55] = 0; ui10.uFluxCapControl[56] = 0; ui10.uFluxCapControl[57] = 0; ui10.uFluxCapControl[58] = 0; ui10.uFluxCapControl[59] = 0; ui10.uFluxCapControl[60] = 0; ui10.uFluxCapControl[61] = 0; ui10.uFluxCapControl[62] = 0; ui10.uFluxCapControl[63] = 0; 
	ui10.fFluxCapData[0] = 0.000000; ui10.fFluxCapData[1] = 0.000000; ui10.fFluxCapData[2] = 0.000000; ui10.fFluxCapData[3] = 0.000000; ui10.fFluxCapData[4] = 0.000000; ui10.fFluxCapData[5] = 0.000000; ui10.fFluxCapData[6] = 0.000000; ui10.fFluxCapData[7] = 0.000000; ui10.fFluxCapData[8] = 0.000000; ui10.fFluxCapData[9] = 0.000000; ui10.fFluxCapData[10] = 0.000000; ui10.fFluxCapData[11] = 0.000000; ui10.fFluxCapData[12] = 0.000000; ui10.fFluxCapData[13] = 0.000000; ui10.fFluxCapData[14] = 0.000000; ui10.fFluxCapData[15] = 0.000000; ui10.fFluxCapData[16] = 0.000000; ui10.fFluxCapData[17] = 0.000000; ui10.fFluxCapData[18] = 0.000000; ui10.fFluxCapData[19] = 0.000000; ui10.fFluxCapData[20] = 0.000000; ui10.fFluxCapData[21] = 0.000000; ui10.fFluxCapData[22] = 0.000000; ui10.fFluxCapData[23] = 0.000000; ui10.fFluxCapData[24] = 0.000000; ui10.fFluxCapData[25] = 0.000000; ui10.fFluxCapData[26] = 0.000000; ui10.fFluxCapData[27] = 0.000000; ui10.fFluxCapData[28] = 0.000000; ui10.fFluxCapData[29] = 0.000000; ui10.fFluxCapData[30] = 0.000000; ui10.fFluxCapData[31] = 0.000000; ui10.fFluxCapData[32] = 0.000000; ui10.fFluxCapData[33] = 0.000000; ui10.fFluxCapData[34] = 0.000000; ui10.fFluxCapData[35] = 0.000000; ui10.fFluxCapData[36] = 0.000000; ui10.fFluxCapData[37] = 0.000000; ui10.fFluxCapData[38] = 0.000000; ui10.fFluxCapData[39] = 0.000000; ui10.fFluxCapData[40] = 0.000000; ui10.fFluxCapData[41] = 0.000000; ui10.fFluxCapData[42] = 0.000000; ui10.fFluxCapData[43] = 0.000000; ui10.fFluxCapData[44] = 0.000000; ui10.fFluxCapData[45] = 0.000000; ui10.fFluxCapData[46] = 0.000000; ui10.fFluxCapData[47] = 0.000000; ui10.fFluxCapData[48] = 0.000000; ui10.fFluxCapData[49] = 0.000000; ui10.fFluxCapData[50] = 0.000000; ui10.fFluxCapData[51] = 0.000000; ui10.fFluxCapData[52] = 0.000000; ui10.fFluxCapData[53] = 0.000000; ui10.fFluxCapData[54] = 0.000000; ui10.fFluxCapData[55] = 0.000000; ui10.fFluxCapData[56] = 0.000000; ui10.fFluxCapData[57] = 0.000000; ui10.fFluxCapData[58] = 0.000000; ui10.fFluxCapData[59] = 0.000000; ui10.fFluxCapData[60] = 0.000000; ui10.fFluxCapData[61] = 0.000000; ui10.fFluxCapData[62] = 0.000000; ui10.fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(ui10);


	m_LF_Release_mSec = 1000.000000;
	CUICtrl ui11;
	ui11.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui11.uControlId = 14;
	ui11.bLogSlider = false;
	ui11.bExpSlider = false;
	ui11.fUserDisplayDataLoLimit = 20.000000;
	ui11.fUserDisplayDataHiLimit = 5000.000000;
	ui11.uUserDataType = floatData;
	ui11.fInitUserIntValue = 0;
	ui11.fInitUserFloatValue = 1000.000000;
	ui11.fInitUserDoubleValue = 0;
	ui11.fInitUserUINTValue = 0;
	ui11.m_pUserCookedIntData = NULL;
	ui11.m_pUserCookedFloatData = &m_LF_Release_mSec;
	ui11.m_pUserCookedDoubleData = NULL;
	ui11.m_pUserCookedUINTData = NULL;
	ui11.cControlUnits = "mS                                                              ";
	ui11.cVariableName = "m_LF_Release_mSec";
	ui11.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui11.dPresetData[0] = 0.000000;ui11.dPresetData[1] = 0.000000;ui11.dPresetData[2] = 0.000000;ui11.dPresetData[3] = 0.000000;ui11.dPresetData[4] = 0.000000;ui11.dPresetData[5] = 0.000000;ui11.dPresetData[6] = 0.000000;ui11.dPresetData[7] = 0.000000;ui11.dPresetData[8] = 0.000000;ui11.dPresetData[9] = 0.000000;ui11.dPresetData[10] = 0.000000;ui11.dPresetData[11] = 0.000000;ui11.dPresetData[12] = 0.000000;ui11.dPresetData[13] = 0.000000;ui11.dPresetData[14] = 0.000000;ui11.dPresetData[15] = 0.000000;
	ui11.cControlName = "LF Rel";
	ui11.bOwnerControl = false;
	ui11.bMIDIControl = false;
	ui11.uMIDIControlCommand = 176;
	ui11.uMIDIControlName = 3;
	ui11.uMIDIControlChannel = 0;
	ui11.nGUIRow = 2;
	ui11.nGUIColumn = 1;
	ui11.uControlTheme[0] = 0; ui11.uControlTheme[1] = 9; ui11.uControlTheme[2] = 0; ui11.uControlTheme[3] = 1; ui11.uControlTheme[4] = 0; ui11.uControlTheme[5] = 1; ui11.uControlTheme[6] = 0; ui11.uControlTheme[7] = 65535; ui11.uControlTheme[8] = 0; ui11.uControlTheme[9] = 11119017; ui11.uControlTheme[10] = 1; ui11.uControlTheme[11] = 12632256; ui11.uControlTheme[12] = 0; ui11.uControlTheme[13] = 6316128; ui11.uControlTheme[14] = 3; ui11.uControlTheme[15] = 8421504; ui11.uControlTheme[16] = 14772545; ui11.uControlTheme[17] = 0; ui11.uControlTheme[18] = 0; ui11.uControlTheme[19] = 0; ui11.uControlTheme[20] = 0; ui11.uControlTheme[21] = 0; ui11.uControlTheme[22] = 0; ui11.uControlTheme[23] = 294; ui11.uControlTheme[24] = 107; ui11.uControlTheme[25] = 0; ui11.uControlTheme[26] = 0; ui11.uControlTheme[27] = 0; ui11.uControlTheme[28] = 0; ui11.uControlTheme[29] = 0; ui11.uControlTheme[30] = 0; ui11.uControlTheme[31] = 0; 
	ui11.uFluxCapControl[0] = 0; ui11.uFluxCapControl[1] = 0; ui11.uFluxCapControl[2] = 0; ui11.uFluxCapControl[3] = 0; ui11.uFluxCapControl[4] = 0; ui11.uFluxCapControl[5] = 0; ui11.uFluxCapControl[6] = 0; ui11.uFluxCapControl[7] = 0; ui11.uFluxCapControl[8] = 0; ui11.uFluxCapControl[9] = 0; ui11.uFluxCapControl[10] = 0; ui11.uFluxCapControl[11] = 0; ui11.uFluxCapControl[12] = 0; ui11.uFluxCapControl[13] = 0; ui11.uFluxCapControl[14] = 0; ui11.uFluxCapControl[15] = 0; ui11.uFluxCapControl[16] = 0; ui11.uFluxCapControl[17] = 0; ui11.uFluxCapControl[18] = 0; ui11.uFluxCapControl[19] = 0; ui11.uFluxCapControl[20] = 0; ui11.uFluxCapControl[21] = 0; ui11.uFluxCapControl[22] = 0; ui11.uFluxCapControl[23] = 0; ui11.uFluxCapControl[24] = 0; ui11.uFluxCapControl[25] = 0; ui11.uFluxCapControl[26] = 0; ui11.uFluxCapControl[27] = 0; ui11.uFluxCapControl[28] = 0; ui11.uFluxCapControl[29] = 0; ui11.uFluxCapControl[30] = 0; ui11.uFluxCapControl[31] = 0; ui11.uFluxCapControl[32] = 0; ui11.uFluxCapControl[33] = 0; ui11.uFluxCapControl[34] = 0; ui11.uFluxCapControl[35] = 0; ui11.uFluxCapControl[36] = 0; ui11.uFluxCapControl[37] = 0; ui11.uFluxCapControl[38] = 0; ui11.uFluxCapControl[39] = 0; ui11.uFluxCapControl[40] = 0; ui11.uFluxCapControl[41] = 0; ui11.uFluxCapControl[42] = 0; ui11.uFluxCapControl[43] = 0; ui11.uFluxCapControl[44] = 0; ui11.uFluxCapControl[45] = 0; ui11.uFluxCapControl[46] = 0; ui11.uFluxCapControl[47] = 0; ui11.uFluxCapControl[48] = 0; ui11.uFluxCapControl[49] = 0; ui11.uFluxCapControl[50] = 0; ui11.uFluxCapControl[51] = 0; ui11.uFluxCapControl[52] = 0; ui11.uFluxCapControl[53] = 0; ui11.uFluxCapControl[54] = 0; ui11.uFluxCapControl[55] = 0; ui11.uFluxCapControl[56] = 0; ui11.uFluxCapControl[57] = 0; ui11.uFluxCapControl[58] = 0; ui11.uFluxCapControl[59] = 0; ui11.uFluxCapControl[60] = 0; ui11.uFluxCapControl[61] = 0; ui11.uFluxCapControl[62] = 0; ui11.uFluxCapControl[63] = 0; 
	ui11.fFluxCapData[0] = 0.000000; ui11.fFluxCapData[1] = 0.000000; ui11.fFluxCapData[2] = 0.000000; ui11.fFluxCapData[3] = 0.000000; ui11.fFluxCapData[4] = 0.000000; ui11.fFluxCapData[5] = 0.000000; ui11.fFluxCapData[6] = 0.000000; ui11.fFluxCapData[7] = 0.000000; ui11.fFluxCapData[8] = 0.000000; ui11.fFluxCapData[9] = 0.000000; ui11.fFluxCapData[10] = 0.000000; ui11.fFluxCapData[11] = 0.000000; ui11.fFluxCapData[12] = 0.000000; ui11.fFluxCapData[13] = 0.000000; ui11.fFluxCapData[14] = 0.000000; ui11.fFluxCapData[15] = 0.000000; ui11.fFluxCapData[16] = 0.000000; ui11.fFluxCapData[17] = 0.000000; ui11.fFluxCapData[18] = 0.000000; ui11.fFluxCapData[19] = 0.000000; ui11.fFluxCapData[20] = 0.000000; ui11.fFluxCapData[21] = 0.000000; ui11.fFluxCapData[22] = 0.000000; ui11.fFluxCapData[23] = 0.000000; ui11.fFluxCapData[24] = 0.000000; ui11.fFluxCapData[25] = 0.000000; ui11.fFluxCapData[26] = 0.000000; ui11.fFluxCapData[27] = 0.000000; ui11.fFluxCapData[28] = 0.000000; ui11.fFluxCapData[29] = 0.000000; ui11.fFluxCapData[30] = 0.000000; ui11.fFluxCapData[31] = 0.000000; ui11.fFluxCapData[32] = 0.000000; ui11.fFluxCapData[33] = 0.000000; ui11.fFluxCapData[34] = 0.000000; ui11.fFluxCapData[35] = 0.000000; ui11.fFluxCapData[36] = 0.000000; ui11.fFluxCapData[37] = 0.000000; ui11.fFluxCapData[38] = 0.000000; ui11.fFluxCapData[39] = 0.000000; ui11.fFluxCapData[40] = 0.000000; ui11.fFluxCapData[41] = 0.000000; ui11.fFluxCapData[42] = 0.000000; ui11.fFluxCapData[43] = 0.000000; ui11.fFluxCapData[44] = 0.000000; ui11.fFluxCapData[45] = 0.000000; ui11.fFluxCapData[46] = 0.000000; ui11.fFluxCapData[47] = 0.000000; ui11.fFluxCapData[48] = 0.000000; ui11.fFluxCapData[49] = 0.000000; ui11.fFluxCapData[50] = 0.000000; ui11.fFluxCapData[51] = 0.000000; ui11.fFluxCapData[52] = 0.000000; ui11.fFluxCapData[53] = 0.000000; ui11.fFluxCapData[54] = 0.000000; ui11.fFluxCapData[55] = 0.000000; ui11.fFluxCapData[56] = 0.000000; ui11.fFluxCapData[57] = 0.000000; ui11.fFluxCapData[58] = 0.000000; ui11.fFluxCapData[59] = 0.000000; ui11.fFluxCapData[60] = 0.000000; ui11.fFluxCapData[61] = 0.000000; ui11.fFluxCapData[62] = 0.000000; ui11.fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(ui11);


	m_LF_Ratio = 1.000000;
	CUICtrl ui12;
	ui12.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui12.uControlId = 15;
	ui12.bLogSlider = false;
	ui12.bExpSlider = false;
	ui12.fUserDisplayDataLoLimit = 1.000000;
	ui12.fUserDisplayDataHiLimit = 20.000000;
	ui12.uUserDataType = floatData;
	ui12.fInitUserIntValue = 0;
	ui12.fInitUserFloatValue = 1.000000;
	ui12.fInitUserDoubleValue = 0;
	ui12.fInitUserUINTValue = 0;
	ui12.m_pUserCookedIntData = NULL;
	ui12.m_pUserCookedFloatData = &m_LF_Ratio;
	ui12.m_pUserCookedDoubleData = NULL;
	ui12.m_pUserCookedUINTData = NULL;
	ui12.cControlUnits = "                                                                ";
	ui12.cVariableName = "m_LF_Ratio";
	ui12.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui12.dPresetData[0] = 0.000000;ui12.dPresetData[1] = 0.000000;ui12.dPresetData[2] = 0.000000;ui12.dPresetData[3] = 0.000000;ui12.dPresetData[4] = 0.000000;ui12.dPresetData[5] = 0.000000;ui12.dPresetData[6] = 0.000000;ui12.dPresetData[7] = 0.000000;ui12.dPresetData[8] = 0.000000;ui12.dPresetData[9] = 0.000000;ui12.dPresetData[10] = 0.000000;ui12.dPresetData[11] = 0.000000;ui12.dPresetData[12] = 0.000000;ui12.dPresetData[13] = 0.000000;ui12.dPresetData[14] = 0.000000;ui12.dPresetData[15] = 0.000000;
	ui12.cControlName = "LF Ratio";
	ui12.bOwnerControl = false;
	ui12.bMIDIControl = false;
	ui12.uMIDIControlCommand = 176;
	ui12.uMIDIControlName = 3;
	ui12.uMIDIControlChannel = 0;
	ui12.nGUIRow = 2;
	ui12.nGUIColumn = 2;
	ui12.uControlTheme[0] = 0; ui12.uControlTheme[1] = 9; ui12.uControlTheme[2] = 0; ui12.uControlTheme[3] = 1; ui12.uControlTheme[4] = 0; ui12.uControlTheme[5] = 1; ui12.uControlTheme[6] = 0; ui12.uControlTheme[7] = 65535; ui12.uControlTheme[8] = 0; ui12.uControlTheme[9] = 11119017; ui12.uControlTheme[10] = 1; ui12.uControlTheme[11] = 12632256; ui12.uControlTheme[12] = 0; ui12.uControlTheme[13] = 6316128; ui12.uControlTheme[14] = 3; ui12.uControlTheme[15] = 8421504; ui12.uControlTheme[16] = 14772545; ui12.uControlTheme[17] = 0; ui12.uControlTheme[18] = 0; ui12.uControlTheme[19] = 0; ui12.uControlTheme[20] = 0; ui12.uControlTheme[21] = 0; ui12.uControlTheme[22] = 0; ui12.uControlTheme[23] = 366; ui12.uControlTheme[24] = 107; ui12.uControlTheme[25] = 0; ui12.uControlTheme[26] = 0; ui12.uControlTheme[27] = 0; ui12.uControlTheme[28] = 0; ui12.uControlTheme[29] = 0; ui12.uControlTheme[30] = 0; ui12.uControlTheme[31] = 0; 
	ui12.uFluxCapControl[0] = 0; ui12.uFluxCapControl[1] = 0; ui12.uFluxCapControl[2] = 0; ui12.uFluxCapControl[3] = 0; ui12.uFluxCapControl[4] = 0; ui12.uFluxCapControl[5] = 0; ui12.uFluxCapControl[6] = 0; ui12.uFluxCapControl[7] = 0; ui12.uFluxCapControl[8] = 0; ui12.uFluxCapControl[9] = 0; ui12.uFluxCapControl[10] = 0; ui12.uFluxCapControl[11] = 0; ui12.uFluxCapControl[12] = 0; ui12.uFluxCapControl[13] = 0; ui12.uFluxCapControl[14] = 0; ui12.uFluxCapControl[15] = 0; ui12.uFluxCapControl[16] = 0; ui12.uFluxCapControl[17] = 0; ui12.uFluxCapControl[18] = 0; ui12.uFluxCapControl[19] = 0; ui12.uFluxCapControl[20] = 0; ui12.uFluxCapControl[21] = 0; ui12.uFluxCapControl[22] = 0; ui12.uFluxCapControl[23] = 0; ui12.uFluxCapControl[24] = 0; ui12.uFluxCapControl[25] = 0; ui12.uFluxCapControl[26] = 0; ui12.uFluxCapControl[27] = 0; ui12.uFluxCapControl[28] = 0; ui12.uFluxCapControl[29] = 0; ui12.uFluxCapControl[30] = 0; ui12.uFluxCapControl[31] = 0; ui12.uFluxCapControl[32] = 0; ui12.uFluxCapControl[33] = 0; ui12.uFluxCapControl[34] = 0; ui12.uFluxCapControl[35] = 0; ui12.uFluxCapControl[36] = 0; ui12.uFluxCapControl[37] = 0; ui12.uFluxCapControl[38] = 0; ui12.uFluxCapControl[39] = 0; ui12.uFluxCapControl[40] = 0; ui12.uFluxCapControl[41] = 0; ui12.uFluxCapControl[42] = 0; ui12.uFluxCapControl[43] = 0; ui12.uFluxCapControl[44] = 0; ui12.uFluxCapControl[45] = 0; ui12.uFluxCapControl[46] = 0; ui12.uFluxCapControl[47] = 0; ui12.uFluxCapControl[48] = 0; ui12.uFluxCapControl[49] = 0; ui12.uFluxCapControl[50] = 0; ui12.uFluxCapControl[51] = 0; ui12.uFluxCapControl[52] = 0; ui12.uFluxCapControl[53] = 0; ui12.uFluxCapControl[54] = 0; ui12.uFluxCapControl[55] = 0; ui12.uFluxCapControl[56] = 0; ui12.uFluxCapControl[57] = 0; ui12.uFluxCapControl[58] = 0; ui12.uFluxCapControl[59] = 0; ui12.uFluxCapControl[60] = 0; ui12.uFluxCapControl[61] = 0; ui12.uFluxCapControl[62] = 0; ui12.uFluxCapControl[63] = 0; 
	ui12.fFluxCapData[0] = 0.000000; ui12.fFluxCapData[1] = 0.000000; ui12.fFluxCapData[2] = 0.000000; ui12.fFluxCapData[3] = 0.000000; ui12.fFluxCapData[4] = 0.000000; ui12.fFluxCapData[5] = 0.000000; ui12.fFluxCapData[6] = 0.000000; ui12.fFluxCapData[7] = 0.000000; ui12.fFluxCapData[8] = 0.000000; ui12.fFluxCapData[9] = 0.000000; ui12.fFluxCapData[10] = 0.000000; ui12.fFluxCapData[11] = 0.000000; ui12.fFluxCapData[12] = 0.000000; ui12.fFluxCapData[13] = 0.000000; ui12.fFluxCapData[14] = 0.000000; ui12.fFluxCapData[15] = 0.000000; ui12.fFluxCapData[16] = 0.000000; ui12.fFluxCapData[17] = 0.000000; ui12.fFluxCapData[18] = 0.000000; ui12.fFluxCapData[19] = 0.000000; ui12.fFluxCapData[20] = 0.000000; ui12.fFluxCapData[21] = 0.000000; ui12.fFluxCapData[22] = 0.000000; ui12.fFluxCapData[23] = 0.000000; ui12.fFluxCapData[24] = 0.000000; ui12.fFluxCapData[25] = 0.000000; ui12.fFluxCapData[26] = 0.000000; ui12.fFluxCapData[27] = 0.000000; ui12.fFluxCapData[28] = 0.000000; ui12.fFluxCapData[29] = 0.000000; ui12.fFluxCapData[30] = 0.000000; ui12.fFluxCapData[31] = 0.000000; ui12.fFluxCapData[32] = 0.000000; ui12.fFluxCapData[33] = 0.000000; ui12.fFluxCapData[34] = 0.000000; ui12.fFluxCapData[35] = 0.000000; ui12.fFluxCapData[36] = 0.000000; ui12.fFluxCapData[37] = 0.000000; ui12.fFluxCapData[38] = 0.000000; ui12.fFluxCapData[39] = 0.000000; ui12.fFluxCapData[40] = 0.000000; ui12.fFluxCapData[41] = 0.000000; ui12.fFluxCapData[42] = 0.000000; ui12.fFluxCapData[43] = 0.000000; ui12.fFluxCapData[44] = 0.000000; ui12.fFluxCapData[45] = 0.000000; ui12.fFluxCapData[46] = 0.000000; ui12.fFluxCapData[47] = 0.000000; ui12.fFluxCapData[48] = 0.000000; ui12.fFluxCapData[49] = 0.000000; ui12.fFluxCapData[50] = 0.000000; ui12.fFluxCapData[51] = 0.000000; ui12.fFluxCapData[52] = 0.000000; ui12.fFluxCapData[53] = 0.000000; ui12.fFluxCapData[54] = 0.000000; ui12.fFluxCapData[55] = 0.000000; ui12.fFluxCapData[56] = 0.000000; ui12.fFluxCapData[57] = 0.000000; ui12.fFluxCapData[58] = 0.000000; ui12.fFluxCapData[59] = 0.000000; ui12.fFluxCapData[60] = 0.000000; ui12.fFluxCapData[61] = 0.000000; ui12.fFluxCapData[62] = 0.000000; ui12.fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(ui12);


	m_LF_MakeUpGain_dB = 0.000000;
	CUICtrl ui13;
	ui13.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui13.uControlId = 16;
	ui13.bLogSlider = false;
	ui13.bExpSlider = false;
	ui13.fUserDisplayDataLoLimit = 0.000000;
	ui13.fUserDisplayDataHiLimit = 20.000000;
	ui13.uUserDataType = floatData;
	ui13.fInitUserIntValue = 0;
	ui13.fInitUserFloatValue = 0.000000;
	ui13.fInitUserDoubleValue = 0;
	ui13.fInitUserUINTValue = 0;
	ui13.m_pUserCookedIntData = NULL;
	ui13.m_pUserCookedFloatData = &m_LF_MakeUpGain_dB;
	ui13.m_pUserCookedDoubleData = NULL;
	ui13.m_pUserCookedUINTData = NULL;
	ui13.cControlUnits = "dB                                                              ";
	ui13.cVariableName = "m_LF_MakeUpGain_dB";
	ui13.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui13.dPresetData[0] = 0.000000;ui13.dPresetData[1] = 0.000000;ui13.dPresetData[2] = 0.000000;ui13.dPresetData[3] = 0.000000;ui13.dPresetData[4] = 0.000000;ui13.dPresetData[5] = 0.000000;ui13.dPresetData[6] = 0.000000;ui13.dPresetData[7] = 0.000000;ui13.dPresetData[8] = 0.000000;ui13.dPresetData[9] = 0.000000;ui13.dPresetData[10] = 0.000000;ui13.dPresetData[11] = 0.000000;ui13.dPresetData[12] = 0.000000;ui13.dPresetData[13] = 0.000000;ui13.dPresetData[14] = 0.000000;ui13.dPresetData[15] = 0.000000;
	ui13.cControlName = "LF Gain";
	ui13.bOwnerControl = false;
	ui13.bMIDIControl = false;
	ui13.uMIDIControlCommand = 176;
	ui13.uMIDIControlName = 3;
	ui13.uMIDIControlChannel = 0;
	ui13.nGUIRow = 2;
	ui13.nGUIColumn = 3;
	ui13.uControlTheme[0] = 0; ui13.uControlTheme[1] = 9; ui13.uControlTheme[2] = 0; ui13.uControlTheme[3] = 2; ui13.uControlTheme[4] = 0; ui13.uControlTheme[5] = 1; ui13.uControlTheme[6] = 0; ui13.uControlTheme[7] = 65535; ui13.uControlTheme[8] = 0; ui13.uControlTheme[9] = 11119017; ui13.uControlTheme[10] = 1; ui13.uControlTheme[11] = 12632256; ui13.uControlTheme[12] = 0; ui13.uControlTheme[13] = 6316128; ui13.uControlTheme[14] = 3; ui13.uControlTheme[15] = 8421504; ui13.uControlTheme[16] = 14772545; ui13.uControlTheme[17] = 0; ui13.uControlTheme[18] = 0; ui13.uControlTheme[19] = 0; ui13.uControlTheme[20] = 0; ui13.uControlTheme[21] = 0; ui13.uControlTheme[22] = 0; ui13.uControlTheme[23] = 437; ui13.uControlTheme[24] = 106; ui13.uControlTheme[25] = 0; ui13.uControlTheme[26] = 0; ui13.uControlTheme[27] = 0; ui13.uControlTheme[28] = 0; ui13.uControlTheme[29] = 0; ui13.uControlTheme[30] = 0; ui13.uControlTheme[31] = 0; 
	ui13.uFluxCapControl[0] = 0; ui13.uFluxCapControl[1] = 0; ui13.uFluxCapControl[2] = 0; ui13.uFluxCapControl[3] = 0; ui13.uFluxCapControl[4] = 0; ui13.uFluxCapControl[5] = 0; ui13.uFluxCapControl[6] = 0; ui13.uFluxCapControl[7] = 0; ui13.uFluxCapControl[8] = 0; ui13.uFluxCapControl[9] = 0; ui13.uFluxCapControl[10] = 0; ui13.uFluxCapControl[11] = 0; ui13.uFluxCapControl[12] = 0; ui13.uFluxCapControl[13] = 0; ui13.uFluxCapControl[14] = 0; ui13.uFluxCapControl[15] = 0; ui13.uFluxCapControl[16] = 0; ui13.uFluxCapControl[17] = 0; ui13.uFluxCapControl[18] = 0; ui13.uFluxCapControl[19] = 0; ui13.uFluxCapControl[20] = 0; ui13.uFluxCapControl[21] = 0; ui13.uFluxCapControl[22] = 0; ui13.uFluxCapControl[23] = 0; ui13.uFluxCapControl[24] = 0; ui13.uFluxCapControl[25] = 0; ui13.uFluxCapControl[26] = 0; ui13.uFluxCapControl[27] = 0; ui13.uFluxCapControl[28] = 0; ui13.uFluxCapControl[29] = 0; ui13.uFluxCapControl[30] = 0; ui13.uFluxCapControl[31] = 0; ui13.uFluxCapControl[32] = 0; ui13.uFluxCapControl[33] = 0; ui13.uFluxCapControl[34] = 0; ui13.uFluxCapControl[35] = 0; ui13.uFluxCapControl[36] = 0; ui13.uFluxCapControl[37] = 0; ui13.uFluxCapControl[38] = 0; ui13.uFluxCapControl[39] = 0; ui13.uFluxCapControl[40] = 0; ui13.uFluxCapControl[41] = 0; ui13.uFluxCapControl[42] = 0; ui13.uFluxCapControl[43] = 0; ui13.uFluxCapControl[44] = 0; ui13.uFluxCapControl[45] = 0; ui13.uFluxCapControl[46] = 0; ui13.uFluxCapControl[47] = 0; ui13.uFluxCapControl[48] = 0; ui13.uFluxCapControl[49] = 0; ui13.uFluxCapControl[50] = 0; ui13.uFluxCapControl[51] = 0; ui13.uFluxCapControl[52] = 0; ui13.uFluxCapControl[53] = 0; ui13.uFluxCapControl[54] = 0; ui13.uFluxCapControl[55] = 0; ui13.uFluxCapControl[56] = 0; ui13.uFluxCapControl[57] = 0; ui13.uFluxCapControl[58] = 0; ui13.uFluxCapControl[59] = 0; ui13.uFluxCapControl[60] = 0; ui13.uFluxCapControl[61] = 0; ui13.uFluxCapControl[62] = 0; ui13.uFluxCapControl[63] = 0; 
	ui13.fFluxCapData[0] = 0.000000; ui13.fFluxCapData[1] = 0.000000; ui13.fFluxCapData[2] = 0.000000; ui13.fFluxCapData[3] = 0.000000; ui13.fFluxCapData[4] = 0.000000; ui13.fFluxCapData[5] = 0.000000; ui13.fFluxCapData[6] = 0.000000; ui13.fFluxCapData[7] = 0.000000; ui13.fFluxCapData[8] = 0.000000; ui13.fFluxCapData[9] = 0.000000; ui13.fFluxCapData[10] = 0.000000; ui13.fFluxCapData[11] = 0.000000; ui13.fFluxCapData[12] = 0.000000; ui13.fFluxCapData[13] = 0.000000; ui13.fFluxCapData[14] = 0.000000; ui13.fFluxCapData[15] = 0.000000; ui13.fFluxCapData[16] = 0.000000; ui13.fFluxCapData[17] = 0.000000; ui13.fFluxCapData[18] = 0.000000; ui13.fFluxCapData[19] = 0.000000; ui13.fFluxCapData[20] = 0.000000; ui13.fFluxCapData[21] = 0.000000; ui13.fFluxCapData[22] = 0.000000; ui13.fFluxCapData[23] = 0.000000; ui13.fFluxCapData[24] = 0.000000; ui13.fFluxCapData[25] = 0.000000; ui13.fFluxCapData[26] = 0.000000; ui13.fFluxCapData[27] = 0.000000; ui13.fFluxCapData[28] = 0.000000; ui13.fFluxCapData[29] = 0.000000; ui13.fFluxCapData[30] = 0.000000; ui13.fFluxCapData[31] = 0.000000; ui13.fFluxCapData[32] = 0.000000; ui13.fFluxCapData[33] = 0.000000; ui13.fFluxCapData[34] = 0.000000; ui13.fFluxCapData[35] = 0.000000; ui13.fFluxCapData[36] = 0.000000; ui13.fFluxCapData[37] = 0.000000; ui13.fFluxCapData[38] = 0.000000; ui13.fFluxCapData[39] = 0.000000; ui13.fFluxCapData[40] = 0.000000; ui13.fFluxCapData[41] = 0.000000; ui13.fFluxCapData[42] = 0.000000; ui13.fFluxCapData[43] = 0.000000; ui13.fFluxCapData[44] = 0.000000; ui13.fFluxCapData[45] = 0.000000; ui13.fFluxCapData[46] = 0.000000; ui13.fFluxCapData[47] = 0.000000; ui13.fFluxCapData[48] = 0.000000; ui13.fFluxCapData[49] = 0.000000; ui13.fFluxCapData[50] = 0.000000; ui13.fFluxCapData[51] = 0.000000; ui13.fFluxCapData[52] = 0.000000; ui13.fFluxCapData[53] = 0.000000; ui13.fFluxCapData[54] = 0.000000; ui13.fFluxCapData[55] = 0.000000; ui13.fFluxCapData[56] = 0.000000; ui13.fFluxCapData[57] = 0.000000; ui13.fFluxCapData[58] = 0.000000; ui13.fFluxCapData[59] = 0.000000; ui13.fFluxCapData[60] = 0.000000; ui13.fFluxCapData[61] = 0.000000; ui13.fFluxCapData[62] = 0.000000; ui13.fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(ui13);


	m_uProcessorType = 0;
	CUICtrl ui14;
	ui14.uControlType = FILTER_CONTROL_RADIO_SWITCH_VARIABLE;
	ui14.uControlId = 41;
	ui14.bLogSlider = false;
	ui14.bExpSlider = false;
	ui14.fUserDisplayDataLoLimit = 0.000000;
	ui14.fUserDisplayDataHiLimit = 3.000000;
	ui14.uUserDataType = UINTData;
	ui14.fInitUserIntValue = 0;
	ui14.fInitUserFloatValue = 0;
	ui14.fInitUserDoubleValue = 0;
	ui14.fInitUserUINTValue = 0.000000;
	ui14.m_pUserCookedIntData = NULL;
	ui14.m_pUserCookedFloatData = NULL;
	ui14.m_pUserCookedDoubleData = NULL;
	ui14.m_pUserCookedUINTData = &m_uProcessorType;
	ui14.cControlUnits = "";
	ui14.cVariableName = "m_uProcessorType";
	ui14.cEnumeratedList = "COMP,LIMIT,EXPAND,GATE";
	ui14.dPresetData[0] = 0.000000;ui14.dPresetData[1] = 0.000000;ui14.dPresetData[2] = 0.000000;ui14.dPresetData[3] = 0.000000;ui14.dPresetData[4] = 0.000000;ui14.dPresetData[5] = 0.000000;ui14.dPresetData[6] = 0.000000;ui14.dPresetData[7] = 0.000000;ui14.dPresetData[8] = 0.000000;ui14.dPresetData[9] = 0.000000;ui14.dPresetData[10] = 0.000000;ui14.dPresetData[11] = 0.000000;ui14.dPresetData[12] = 0.000000;ui14.dPresetData[13] = 0.000000;ui14.dPresetData[14] = 0.000000;ui14.dPresetData[15] = 0.000000;
	ui14.cControlName = "Processor";
	ui14.bOwnerControl = false;
	ui14.bMIDIControl = false;
	ui14.uMIDIControlCommand = 176;
	ui14.uMIDIControlName = 3;
	ui14.uMIDIControlChannel = 0;
	ui14.nGUIRow = -1;
	ui14.nGUIColumn = -1;
	ui14.uControlTheme[0] = 0; ui14.uControlTheme[1] = 0; ui14.uControlTheme[2] = 0; ui14.uControlTheme[3] = 0; ui14.uControlTheme[4] = 16777215; ui14.uControlTheme[5] = 0; ui14.uControlTheme[6] = 0; ui14.uControlTheme[7] = 0; ui14.uControlTheme[8] = 0; ui14.uControlTheme[9] = 0; ui14.uControlTheme[10] = 0; ui14.uControlTheme[11] = 0; ui14.uControlTheme[12] = 0; ui14.uControlTheme[13] = 0; ui14.uControlTheme[14] = 0; ui14.uControlTheme[15] = 0; ui14.uControlTheme[16] = 0; ui14.uControlTheme[17] = 0; ui14.uControlTheme[18] = 0; ui14.uControlTheme[19] = 0; ui14.uControlTheme[20] = 0; ui14.uControlTheme[21] = 0; ui14.uControlTheme[22] = 1; ui14.uControlTheme[23] = 623; ui14.uControlTheme[24] = 4294967291; ui14.uControlTheme[25] = 0; ui14.uControlTheme[26] = 0; ui14.uControlTheme[27] = 0; ui14.uControlTheme[28] = 0; ui14.uControlTheme[29] = 0; ui14.uControlTheme[30] = 0; ui14.uControlTheme[31] = 0; 
	ui14.uFluxCapControl[0] = 0; ui14.uFluxCapControl[1] = 0; ui14.uFluxCapControl[2] = 0; ui14.uFluxCapControl[3] = 0; ui14.uFluxCapControl[4] = 0; ui14.uFluxCapControl[5] = 0; ui14.uFluxCapControl[6] = 0; ui14.uFluxCapControl[7] = 0; ui14.uFluxCapControl[8] = 0; ui14.uFluxCapControl[9] = 0; ui14.uFluxCapControl[10] = 0; ui14.uFluxCapControl[11] = 0; ui14.uFluxCapControl[12] = 0; ui14.uFluxCapControl[13] = 0; ui14.uFluxCapControl[14] = 0; ui14.uFluxCapControl[15] = 0; ui14.uFluxCapControl[16] = 0; ui14.uFluxCapControl[17] = 0; ui14.uFluxCapControl[18] = 0; ui14.uFluxCapControl[19] = 0; ui14.uFluxCapControl[20] = 0; ui14.uFluxCapControl[21] = 0; ui14.uFluxCapControl[22] = 0; ui14.uFluxCapControl[23] = 0; ui14.uFluxCapControl[24] = 0; ui14.uFluxCapControl[25] = 0; ui14.uFluxCapControl[26] = 0; ui14.uFluxCapControl[27] = 0; ui14.uFluxCapControl[28] = 0; ui14.uFluxCapControl[29] = 0; ui14.uFluxCapControl[30] = 0; ui14.uFluxCapControl[31] = 0; ui14.uFluxCapControl[32] = 0; ui14.uFluxCapControl[33] = 0; ui14.uFluxCapControl[34] = 0; ui14.uFluxCapControl[35] = 0; ui14.uFluxCapControl[36] = 0; ui14.uFluxCapControl[37] = 0; ui14.uFluxCapControl[38] = 0; ui14.uFluxCapControl[39] = 0; ui14.uFluxCapControl[40] = 0; ui14.uFluxCapControl[41] = 0; ui14.uFluxCapControl[42] = 0; ui14.uFluxCapControl[43] = 0; ui14.uFluxCapControl[44] = 0; ui14.uFluxCapControl[45] = 0; ui14.uFluxCapControl[46] = 0; ui14.uFluxCapControl[47] = 0; ui14.uFluxCapControl[48] = 0; ui14.uFluxCapControl[49] = 0; ui14.uFluxCapControl[50] = 0; ui14.uFluxCapControl[51] = 0; ui14.uFluxCapControl[52] = 0; ui14.uFluxCapControl[53] = 0; ui14.uFluxCapControl[54] = 0; ui14.uFluxCapControl[55] = 0; ui14.uFluxCapControl[56] = 0; ui14.uFluxCapControl[57] = 0; ui14.uFluxCapControl[58] = 0; ui14.uFluxCapControl[59] = 0; ui14.uFluxCapControl[60] = 0; ui14.uFluxCapControl[61] = 0; ui14.uFluxCapControl[62] = 0; ui14.uFluxCapControl[63] = 0; 
	ui14.fFluxCapData[0] = 0.000000; ui14.fFluxCapData[1] = 0.000000; ui14.fFluxCapData[2] = 0.000000; ui14.fFluxCapData[3] = 0.000000; ui14.fFluxCapData[4] = 0.000000; ui14.fFluxCapData[5] = 0.000000; ui14.fFluxCapData[6] = 0.000000; ui14.fFluxCapData[7] = 0.000000; ui14.fFluxCapData[8] = 0.000000; ui14.fFluxCapData[9] = 0.000000; ui14.fFluxCapData[10] = 0.000000; ui14.fFluxCapData[11] = 0.000000; ui14.fFluxCapData[12] = 0.000000; ui14.fFluxCapData[13] = 0.000000; ui14.fFluxCapData[14] = 0.000000; ui14.fFluxCapData[15] = 0.000000; ui14.fFluxCapData[16] = 0.000000; ui14.fFluxCapData[17] = 0.000000; ui14.fFluxCapData[18] = 0.000000; ui14.fFluxCapData[19] = 0.000000; ui14.fFluxCapData[20] = 0.000000; ui14.fFluxCapData[21] = 0.000000; ui14.fFluxCapData[22] = 0.000000; ui14.fFluxCapData[23] = 0.000000; ui14.fFluxCapData[24] = 0.000000; ui14.fFluxCapData[25] = 0.000000; ui14.fFluxCapData[26] = 0.000000; ui14.fFluxCapData[27] = 0.000000; ui14.fFluxCapData[28] = 0.000000; ui14.fFluxCapData[29] = 0.000000; ui14.fFluxCapData[30] = 0.000000; ui14.fFluxCapData[31] = 0.000000; ui14.fFluxCapData[32] = 0.000000; ui14.fFluxCapData[33] = 0.000000; ui14.fFluxCapData[34] = 0.000000; ui14.fFluxCapData[35] = 0.000000; ui14.fFluxCapData[36] = 0.000000; ui14.fFluxCapData[37] = 0.000000; ui14.fFluxCapData[38] = 0.000000; ui14.fFluxCapData[39] = 0.000000; ui14.fFluxCapData[40] = 0.000000; ui14.fFluxCapData[41] = 0.000000; ui14.fFluxCapData[42] = 0.000000; ui14.fFluxCapData[43] = 0.000000; ui14.fFluxCapData[44] = 0.000000; ui14.fFluxCapData[45] = 0.000000; ui14.fFluxCapData[46] = 0.000000; ui14.fFluxCapData[47] = 0.000000; ui14.fFluxCapData[48] = 0.000000; ui14.fFluxCapData[49] = 0.000000; ui14.fFluxCapData[50] = 0.000000; ui14.fFluxCapData[51] = 0.000000; ui14.fFluxCapData[52] = 0.000000; ui14.fFluxCapData[53] = 0.000000; ui14.fFluxCapData[54] = 0.000000; ui14.fFluxCapData[55] = 0.000000; ui14.fFluxCapData[56] = 0.000000; ui14.fFluxCapData[57] = 0.000000; ui14.fFluxCapData[58] = 0.000000; ui14.fFluxCapData[59] = 0.000000; ui14.fFluxCapData[60] = 0.000000; ui14.fFluxCapData[61] = 0.000000; ui14.fFluxCapData[62] = 0.000000; ui14.fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(ui14);


	m_uTimeConstant = 0;
	CUICtrl ui15;
	ui15.uControlType = FILTER_CONTROL_RADIO_SWITCH_VARIABLE;
	ui15.uControlId = 42;
	ui15.bLogSlider = false;
	ui15.bExpSlider = false;
	ui15.fUserDisplayDataLoLimit = 0.000000;
	ui15.fUserDisplayDataHiLimit = 1.000000;
	ui15.uUserDataType = UINTData;
	ui15.fInitUserIntValue = 0;
	ui15.fInitUserFloatValue = 0;
	ui15.fInitUserDoubleValue = 0;
	ui15.fInitUserUINTValue = 0.000000;
	ui15.m_pUserCookedIntData = NULL;
	ui15.m_pUserCookedFloatData = NULL;
	ui15.m_pUserCookedDoubleData = NULL;
	ui15.m_pUserCookedUINTData = &m_uTimeConstant;
	ui15.cControlUnits = "";
	ui15.cVariableName = "m_uTimeConstant";
	ui15.cEnumeratedList = "Digital,Analog";
	ui15.dPresetData[0] = 0.000000;ui15.dPresetData[1] = 0.000000;ui15.dPresetData[2] = 0.000000;ui15.dPresetData[3] = 0.000000;ui15.dPresetData[4] = 0.000000;ui15.dPresetData[5] = 0.000000;ui15.dPresetData[6] = 0.000000;ui15.dPresetData[7] = 0.000000;ui15.dPresetData[8] = 0.000000;ui15.dPresetData[9] = 0.000000;ui15.dPresetData[10] = 0.000000;ui15.dPresetData[11] = 0.000000;ui15.dPresetData[12] = 0.000000;ui15.dPresetData[13] = 0.000000;ui15.dPresetData[14] = 0.000000;ui15.dPresetData[15] = 0.000000;
	ui15.cControlName = "T.C.";
	ui15.bOwnerControl = false;
	ui15.bMIDIControl = false;
	ui15.uMIDIControlCommand = 176;
	ui15.uMIDIControlName = 3;
	ui15.uMIDIControlChannel = 0;
	ui15.nGUIRow = -1;
	ui15.nGUIColumn = -1;
	ui15.uControlTheme[0] = 0; ui15.uControlTheme[1] = 0; ui15.uControlTheme[2] = 0; ui15.uControlTheme[3] = 0; ui15.uControlTheme[4] = 16777215; ui15.uControlTheme[5] = 0; ui15.uControlTheme[6] = 0; ui15.uControlTheme[7] = 0; ui15.uControlTheme[8] = 0; ui15.uControlTheme[9] = 0; ui15.uControlTheme[10] = 0; ui15.uControlTheme[11] = 0; ui15.uControlTheme[12] = 0; ui15.uControlTheme[13] = 0; ui15.uControlTheme[14] = 0; ui15.uControlTheme[15] = 0; ui15.uControlTheme[16] = 0; ui15.uControlTheme[17] = 0; ui15.uControlTheme[18] = 0; ui15.uControlTheme[19] = 0; ui15.uControlTheme[20] = 0; ui15.uControlTheme[21] = 0; ui15.uControlTheme[22] = 1; ui15.uControlTheme[23] = 626; ui15.uControlTheme[24] = 102; ui15.uControlTheme[25] = 0; ui15.uControlTheme[26] = 0; ui15.uControlTheme[27] = 0; ui15.uControlTheme[28] = 0; ui15.uControlTheme[29] = 0; ui15.uControlTheme[30] = 0; ui15.uControlTheme[31] = 0; 
	ui15.uFluxCapControl[0] = 0; ui15.uFluxCapControl[1] = 0; ui15.uFluxCapControl[2] = 0; ui15.uFluxCapControl[3] = 0; ui15.uFluxCapControl[4] = 0; ui15.uFluxCapControl[5] = 0; ui15.uFluxCapControl[6] = 0; ui15.uFluxCapControl[7] = 0; ui15.uFluxCapControl[8] = 0; ui15.uFluxCapControl[9] = 0; ui15.uFluxCapControl[10] = 0; ui15.uFluxCapControl[11] = 0; ui15.uFluxCapControl[12] = 0; ui15.uFluxCapControl[13] = 0; ui15.uFluxCapControl[14] = 0; ui15.uFluxCapControl[15] = 0; ui15.uFluxCapControl[16] = 0; ui15.uFluxCapControl[17] = 0; ui15.uFluxCapControl[18] = 0; ui15.uFluxCapControl[19] = 0; ui15.uFluxCapControl[20] = 0; ui15.uFluxCapControl[21] = 0; ui15.uFluxCapControl[22] = 0; ui15.uFluxCapControl[23] = 0; ui15.uFluxCapControl[24] = 0; ui15.uFluxCapControl[25] = 0; ui15.uFluxCapControl[26] = 0; ui15.uFluxCapControl[27] = 0; ui15.uFluxCapControl[28] = 0; ui15.uFluxCapControl[29] = 0; ui15.uFluxCapControl[30] = 0; ui15.uFluxCapControl[31] = 0; ui15.uFluxCapControl[32] = 0; ui15.uFluxCapControl[33] = 0; ui15.uFluxCapControl[34] = 0; ui15.uFluxCapControl[35] = 0; ui15.uFluxCapControl[36] = 0; ui15.uFluxCapControl[37] = 0; ui15.uFluxCapControl[38] = 0; ui15.uFluxCapControl[39] = 0; ui15.uFluxCapControl[40] = 0; ui15.uFluxCapControl[41] = 0; ui15.uFluxCapControl[42] = 0; ui15.uFluxCapControl[43] = 0; ui15.uFluxCapControl[44] = 0; ui15.uFluxCapControl[45] = 0; ui15.uFluxCapControl[46] = 0; ui15.uFluxCapControl[47] = 0; ui15.uFluxCapControl[48] = 0; ui15.uFluxCapControl[49] = 0; ui15.uFluxCapControl[50] = 0; ui15.uFluxCapControl[51] = 0; ui15.uFluxCapControl[52] = 0; ui15.uFluxCapControl[53] = 0; ui15.uFluxCapControl[54] = 0; ui15.uFluxCapControl[55] = 0; ui15.uFluxCapControl[56] = 0; ui15.uFluxCapControl[57] = 0; ui15.uFluxCapControl[58] = 0; ui15.uFluxCapControl[59] = 0; ui15.uFluxCapControl[60] = 0; ui15.uFluxCapControl[61] = 0; ui15.uFluxCapControl[62] = 0; ui15.uFluxCapControl[63] = 0; 
	ui15.fFluxCapData[0] = 0.000000; ui15.fFluxCapData[1] = 0.000000; ui15.fFluxCapData[2] = 0.000000; ui15.fFluxCapData[3] = 0.000000; ui15.fFluxCapData[4] = 0.000000; ui15.fFluxCapData[5] = 0.000000; ui15.fFluxCapData[6] = 0.000000; ui15.fFluxCapData[7] = 0.000000; ui15.fFluxCapData[8] = 0.000000; ui15.fFluxCapData[9] = 0.000000; ui15.fFluxCapData[10] = 0.000000; ui15.fFluxCapData[11] = 0.000000; ui15.fFluxCapData[12] = 0.000000; ui15.fFluxCapData[13] = 0.000000; ui15.fFluxCapData[14] = 0.000000; ui15.fFluxCapData[15] = 0.000000; ui15.fFluxCapData[16] = 0.000000; ui15.fFluxCapData[17] = 0.000000; ui15.fFluxCapData[18] = 0.000000; ui15.fFluxCapData[19] = 0.000000; ui15.fFluxCapData[20] = 0.000000; ui15.fFluxCapData[21] = 0.000000; ui15.fFluxCapData[22] = 0.000000; ui15.fFluxCapData[23] = 0.000000; ui15.fFluxCapData[24] = 0.000000; ui15.fFluxCapData[25] = 0.000000; ui15.fFluxCapData[26] = 0.000000; ui15.fFluxCapData[27] = 0.000000; ui15.fFluxCapData[28] = 0.000000; ui15.fFluxCapData[29] = 0.000000; ui15.fFluxCapData[30] = 0.000000; ui15.fFluxCapData[31] = 0.000000; ui15.fFluxCapData[32] = 0.000000; ui15.fFluxCapData[33] = 0.000000; ui15.fFluxCapData[34] = 0.000000; ui15.fFluxCapData[35] = 0.000000; ui15.fFluxCapData[36] = 0.000000; ui15.fFluxCapData[37] = 0.000000; ui15.fFluxCapData[38] = 0.000000; ui15.fFluxCapData[39] = 0.000000; ui15.fFluxCapData[40] = 0.000000; ui15.fFluxCapData[41] = 0.000000; ui15.fFluxCapData[42] = 0.000000; ui15.fFluxCapData[43] = 0.000000; ui15.fFluxCapData[44] = 0.000000; ui15.fFluxCapData[45] = 0.000000; ui15.fFluxCapData[46] = 0.000000; ui15.fFluxCapData[47] = 0.000000; ui15.fFluxCapData[48] = 0.000000; ui15.fFluxCapData[49] = 0.000000; ui15.fFluxCapData[50] = 0.000000; ui15.fFluxCapData[51] = 0.000000; ui15.fFluxCapData[52] = 0.000000; ui15.fFluxCapData[53] = 0.000000; ui15.fFluxCapData[54] = 0.000000; ui15.fFluxCapData[55] = 0.000000; ui15.fFluxCapData[56] = 0.000000; ui15.fFluxCapData[57] = 0.000000; ui15.fFluxCapData[58] = 0.000000; ui15.fFluxCapData[59] = 0.000000; ui15.fFluxCapData[60] = 0.000000; ui15.fFluxCapData[61] = 0.000000; ui15.fFluxCapData[62] = 0.000000; ui15.fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(ui15);


	m_fMeterLFIn = 0.0;
	CUICtrl ui16;
	ui16.uControlType = FILTER_CONTROL_LED_METER;
	ui16.uControlId = 50;
	ui16.bLogSlider = false;
	ui16.bExpSlider = false;
	ui16.dPresetData[0] = 0.000000;ui16.dPresetData[1] = 0.000000;ui16.dPresetData[2] = 0.000000;ui16.dPresetData[3] = 0.000000;ui16.dPresetData[4] = 0.000000;ui16.dPresetData[5] = 0.000000;ui16.dPresetData[6] = 0.000000;ui16.dPresetData[7] = 0.000000;ui16.dPresetData[8] = 0.000000;ui16.dPresetData[9] = 0.000000;ui16.dPresetData[10] = 0.000000;ui16.dPresetData[11] = 0.000000;ui16.dPresetData[12] = 0.000000;ui16.dPresetData[13] = 0.000000;ui16.dPresetData[14] = 0.000000;ui16.dPresetData[15] = 0.000000;
	ui16.cControlName = "LFI";
	ui16.bOwnerControl = false;
	ui16.bMIDIControl = false;
	ui16.uMIDIControlCommand = 176;
	ui16.uMIDIControlName = 3;
	ui16.uMIDIControlChannel = 0;
	ui16.bUseMeter = true;
	ui16.bUpsideDownMeter = false;
	ui16.bLogMeter = true;
	ui16.uDetectorMode = 2;
	ui16.uMeterColorScheme = 0;
	ui16.fMeterAttack_ms = 10.000000;
	ui16.fMeterRelease_ms = 500.000000;
	ui16.cMeterVariableName = "m_fMeterLFIn";
	ui16.m_pCurrentMeterValue = &m_fMeterLFIn;
	ui16.nGUIRow = -1;
	ui16.nGUIColumn = -1;
	ui16.uControlTheme[0] = 0; ui16.uControlTheme[1] = 0; ui16.uControlTheme[2] = 0; ui16.uControlTheme[3] = 0; ui16.uControlTheme[4] = 0; ui16.uControlTheme[5] = 0; ui16.uControlTheme[6] = 0; ui16.uControlTheme[7] = 0; ui16.uControlTheme[8] = 0; ui16.uControlTheme[9] = 0; ui16.uControlTheme[10] = 0; ui16.uControlTheme[11] = 0; ui16.uControlTheme[12] = 0; ui16.uControlTheme[13] = 0; ui16.uControlTheme[14] = 0; ui16.uControlTheme[15] = 0; ui16.uControlTheme[16] = 0; ui16.uControlTheme[17] = 0; ui16.uControlTheme[18] = 0; ui16.uControlTheme[19] = 0; ui16.uControlTheme[20] = 0; ui16.uControlTheme[21] = 0; ui16.uControlTheme[22] = 1; ui16.uControlTheme[23] = 514; ui16.uControlTheme[24] = 123; ui16.uControlTheme[25] = 0; ui16.uControlTheme[26] = 0; ui16.uControlTheme[27] = 0; ui16.uControlTheme[28] = 0; ui16.uControlTheme[29] = 0; ui16.uControlTheme[30] = 0; ui16.uControlTheme[31] = 0; 
	ui16.uFluxCapControl[0] = 0; ui16.uFluxCapControl[1] = 0; ui16.uFluxCapControl[2] = 0; ui16.uFluxCapControl[3] = 0; ui16.uFluxCapControl[4] = 0; ui16.uFluxCapControl[5] = 0; ui16.uFluxCapControl[6] = 0; ui16.uFluxCapControl[7] = 0; ui16.uFluxCapControl[8] = 0; ui16.uFluxCapControl[9] = 0; ui16.uFluxCapControl[10] = 0; ui16.uFluxCapControl[11] = 0; ui16.uFluxCapControl[12] = 0; ui16.uFluxCapControl[13] = 0; ui16.uFluxCapControl[14] = 0; ui16.uFluxCapControl[15] = 0; ui16.uFluxCapControl[16] = 0; ui16.uFluxCapControl[17] = 0; ui16.uFluxCapControl[18] = 0; ui16.uFluxCapControl[19] = 0; ui16.uFluxCapControl[20] = 0; ui16.uFluxCapControl[21] = 0; ui16.uFluxCapControl[22] = 0; ui16.uFluxCapControl[23] = 0; ui16.uFluxCapControl[24] = 0; ui16.uFluxCapControl[25] = 0; ui16.uFluxCapControl[26] = 0; ui16.uFluxCapControl[27] = 0; ui16.uFluxCapControl[28] = 0; ui16.uFluxCapControl[29] = 0; ui16.uFluxCapControl[30] = 0; ui16.uFluxCapControl[31] = 0; ui16.uFluxCapControl[32] = 0; ui16.uFluxCapControl[33] = 0; ui16.uFluxCapControl[34] = 0; ui16.uFluxCapControl[35] = 0; ui16.uFluxCapControl[36] = 0; ui16.uFluxCapControl[37] = 0; ui16.uFluxCapControl[38] = 0; ui16.uFluxCapControl[39] = 0; ui16.uFluxCapControl[40] = 0; ui16.uFluxCapControl[41] = 0; ui16.uFluxCapControl[42] = 0; ui16.uFluxCapControl[43] = 0; ui16.uFluxCapControl[44] = 0; ui16.uFluxCapControl[45] = 0; ui16.uFluxCapControl[46] = 0; ui16.uFluxCapControl[47] = 0; ui16.uFluxCapControl[48] = 0; ui16.uFluxCapControl[49] = 0; ui16.uFluxCapControl[50] = 0; ui16.uFluxCapControl[51] = 0; ui16.uFluxCapControl[52] = 0; ui16.uFluxCapControl[53] = 0; ui16.uFluxCapControl[54] = 0; ui16.uFluxCapControl[55] = 0; ui16.uFluxCapControl[56] = 0; ui16.uFluxCapControl[57] = 0; ui16.uFluxCapControl[58] = 0; ui16.uFluxCapControl[59] = 0; ui16.uFluxCapControl[60] = 0; ui16.uFluxCapControl[61] = 0; ui16.uFluxCapControl[62] = 0; ui16.uFluxCapControl[63] = 0; 
	ui16.fFluxCapData[0] = 0.000000; ui16.fFluxCapData[1] = 0.000000; ui16.fFluxCapData[2] = 0.000000; ui16.fFluxCapData[3] = 0.000000; ui16.fFluxCapData[4] = 0.000000; ui16.fFluxCapData[5] = 0.000000; ui16.fFluxCapData[6] = 0.000000; ui16.fFluxCapData[7] = 0.000000; ui16.fFluxCapData[8] = 0.000000; ui16.fFluxCapData[9] = 0.000000; ui16.fFluxCapData[10] = 0.000000; ui16.fFluxCapData[11] = 0.000000; ui16.fFluxCapData[12] = 0.000000; ui16.fFluxCapData[13] = 0.000000; ui16.fFluxCapData[14] = 0.000000; ui16.fFluxCapData[15] = 0.000000; ui16.fFluxCapData[16] = 0.000000; ui16.fFluxCapData[17] = 0.000000; ui16.fFluxCapData[18] = 0.000000; ui16.fFluxCapData[19] = 0.000000; ui16.fFluxCapData[20] = 0.000000; ui16.fFluxCapData[21] = 0.000000; ui16.fFluxCapData[22] = 0.000000; ui16.fFluxCapData[23] = 0.000000; ui16.fFluxCapData[24] = 0.000000; ui16.fFluxCapData[25] = 0.000000; ui16.fFluxCapData[26] = 0.000000; ui16.fFluxCapData[27] = 0.000000; ui16.fFluxCapData[28] = 0.000000; ui16.fFluxCapData[29] = 0.000000; ui16.fFluxCapData[30] = 0.000000; ui16.fFluxCapData[31] = 0.000000; ui16.fFluxCapData[32] = 0.000000; ui16.fFluxCapData[33] = 0.000000; ui16.fFluxCapData[34] = 0.000000; ui16.fFluxCapData[35] = 0.000000; ui16.fFluxCapData[36] = 0.000000; ui16.fFluxCapData[37] = 0.000000; ui16.fFluxCapData[38] = 0.000000; ui16.fFluxCapData[39] = 0.000000; ui16.fFluxCapData[40] = 0.000000; ui16.fFluxCapData[41] = 0.000000; ui16.fFluxCapData[42] = 0.000000; ui16.fFluxCapData[43] = 0.000000; ui16.fFluxCapData[44] = 0.000000; ui16.fFluxCapData[45] = 0.000000; ui16.fFluxCapData[46] = 0.000000; ui16.fFluxCapData[47] = 0.000000; ui16.fFluxCapData[48] = 0.000000; ui16.fFluxCapData[49] = 0.000000; ui16.fFluxCapData[50] = 0.000000; ui16.fFluxCapData[51] = 0.000000; ui16.fFluxCapData[52] = 0.000000; ui16.fFluxCapData[53] = 0.000000; ui16.fFluxCapData[54] = 0.000000; ui16.fFluxCapData[55] = 0.000000; ui16.fFluxCapData[56] = 0.000000; ui16.fFluxCapData[57] = 0.000000; ui16.fFluxCapData[58] = 0.000000; ui16.fFluxCapData[59] = 0.000000; ui16.fFluxCapData[60] = 0.000000; ui16.fFluxCapData[61] = 0.000000; ui16.fFluxCapData[62] = 0.000000; ui16.fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(ui16);


	m_fMeterHFIn = 0.0;
	CUICtrl ui17;
	ui17.uControlType = FILTER_CONTROL_LED_METER;
	ui17.uControlId = 51;
	ui17.bLogSlider = false;
	ui17.bExpSlider = false;
	ui17.dPresetData[0] = 0.000000;ui17.dPresetData[1] = 0.000000;ui17.dPresetData[2] = 0.000000;ui17.dPresetData[3] = 0.000000;ui17.dPresetData[4] = 0.000000;ui17.dPresetData[5] = 0.000000;ui17.dPresetData[6] = 0.000000;ui17.dPresetData[7] = 0.000000;ui17.dPresetData[8] = 0.000000;ui17.dPresetData[9] = 0.000000;ui17.dPresetData[10] = 0.000000;ui17.dPresetData[11] = 0.000000;ui17.dPresetData[12] = 0.000000;ui17.dPresetData[13] = 0.000000;ui17.dPresetData[14] = 0.000000;ui17.dPresetData[15] = 0.000000;
	ui17.cControlName = "HFI";
	ui17.bOwnerControl = false;
	ui17.bMIDIControl = false;
	ui17.uMIDIControlCommand = 176;
	ui17.uMIDIControlName = 3;
	ui17.uMIDIControlChannel = 0;
	ui17.bUseMeter = true;
	ui17.bUpsideDownMeter = false;
	ui17.bLogMeter = true;
	ui17.uDetectorMode = 2;
	ui17.uMeterColorScheme = 0;
	ui17.fMeterAttack_ms = 10.000000;
	ui17.fMeterRelease_ms = 500.000000;
	ui17.cMeterVariableName = "m_fMeterHFIn";
	ui17.m_pCurrentMeterValue = &m_fMeterHFIn;
	ui17.nGUIRow = -1;
	ui17.nGUIColumn = -1;
	ui17.uControlTheme[0] = 0; ui17.uControlTheme[1] = 0; ui17.uControlTheme[2] = 0; ui17.uControlTheme[3] = 0; ui17.uControlTheme[4] = 0; ui17.uControlTheme[5] = 0; ui17.uControlTheme[6] = 0; ui17.uControlTheme[7] = 0; ui17.uControlTheme[8] = 0; ui17.uControlTheme[9] = 0; ui17.uControlTheme[10] = 0; ui17.uControlTheme[11] = 0; ui17.uControlTheme[12] = 0; ui17.uControlTheme[13] = 0; ui17.uControlTheme[14] = 0; ui17.uControlTheme[15] = 0; ui17.uControlTheme[16] = 0; ui17.uControlTheme[17] = 0; ui17.uControlTheme[18] = 0; ui17.uControlTheme[19] = 0; ui17.uControlTheme[20] = 0; ui17.uControlTheme[21] = 0; ui17.uControlTheme[22] = 1; ui17.uControlTheme[23] = 512; ui17.uControlTheme[24] = 13; ui17.uControlTheme[25] = 0; ui17.uControlTheme[26] = 0; ui17.uControlTheme[27] = 0; ui17.uControlTheme[28] = 0; ui17.uControlTheme[29] = 0; ui17.uControlTheme[30] = 0; ui17.uControlTheme[31] = 0; 
	ui17.uFluxCapControl[0] = 0; ui17.uFluxCapControl[1] = 0; ui17.uFluxCapControl[2] = 0; ui17.uFluxCapControl[3] = 0; ui17.uFluxCapControl[4] = 0; ui17.uFluxCapControl[5] = 0; ui17.uFluxCapControl[6] = 0; ui17.uFluxCapControl[7] = 0; ui17.uFluxCapControl[8] = 0; ui17.uFluxCapControl[9] = 0; ui17.uFluxCapControl[10] = 0; ui17.uFluxCapControl[11] = 0; ui17.uFluxCapControl[12] = 0; ui17.uFluxCapControl[13] = 0; ui17.uFluxCapControl[14] = 0; ui17.uFluxCapControl[15] = 0; ui17.uFluxCapControl[16] = 0; ui17.uFluxCapControl[17] = 0; ui17.uFluxCapControl[18] = 0; ui17.uFluxCapControl[19] = 0; ui17.uFluxCapControl[20] = 0; ui17.uFluxCapControl[21] = 0; ui17.uFluxCapControl[22] = 0; ui17.uFluxCapControl[23] = 0; ui17.uFluxCapControl[24] = 0; ui17.uFluxCapControl[25] = 0; ui17.uFluxCapControl[26] = 0; ui17.uFluxCapControl[27] = 0; ui17.uFluxCapControl[28] = 0; ui17.uFluxCapControl[29] = 0; ui17.uFluxCapControl[30] = 0; ui17.uFluxCapControl[31] = 0; ui17.uFluxCapControl[32] = 0; ui17.uFluxCapControl[33] = 0; ui17.uFluxCapControl[34] = 0; ui17.uFluxCapControl[35] = 0; ui17.uFluxCapControl[36] = 0; ui17.uFluxCapControl[37] = 0; ui17.uFluxCapControl[38] = 0; ui17.uFluxCapControl[39] = 0; ui17.uFluxCapControl[40] = 0; ui17.uFluxCapControl[41] = 0; ui17.uFluxCapControl[42] = 0; ui17.uFluxCapControl[43] = 0; ui17.uFluxCapControl[44] = 0; ui17.uFluxCapControl[45] = 0; ui17.uFluxCapControl[46] = 0; ui17.uFluxCapControl[47] = 0; ui17.uFluxCapControl[48] = 0; ui17.uFluxCapControl[49] = 0; ui17.uFluxCapControl[50] = 0; ui17.uFluxCapControl[51] = 0; ui17.uFluxCapControl[52] = 0; ui17.uFluxCapControl[53] = 0; ui17.uFluxCapControl[54] = 0; ui17.uFluxCapControl[55] = 0; ui17.uFluxCapControl[56] = 0; ui17.uFluxCapControl[57] = 0; ui17.uFluxCapControl[58] = 0; ui17.uFluxCapControl[59] = 0; ui17.uFluxCapControl[60] = 0; ui17.uFluxCapControl[61] = 0; ui17.uFluxCapControl[62] = 0; ui17.uFluxCapControl[63] = 0; 
	ui17.fFluxCapData[0] = 0.000000; ui17.fFluxCapData[1] = 0.000000; ui17.fFluxCapData[2] = 0.000000; ui17.fFluxCapData[3] = 0.000000; ui17.fFluxCapData[4] = 0.000000; ui17.fFluxCapData[5] = 0.000000; ui17.fFluxCapData[6] = 0.000000; ui17.fFluxCapData[7] = 0.000000; ui17.fFluxCapData[8] = 0.000000; ui17.fFluxCapData[9] = 0.000000; ui17.fFluxCapData[10] = 0.000000; ui17.fFluxCapData[11] = 0.000000; ui17.fFluxCapData[12] = 0.000000; ui17.fFluxCapData[13] = 0.000000; ui17.fFluxCapData[14] = 0.000000; ui17.fFluxCapData[15] = 0.000000; ui17.fFluxCapData[16] = 0.000000; ui17.fFluxCapData[17] = 0.000000; ui17.fFluxCapData[18] = 0.000000; ui17.fFluxCapData[19] = 0.000000; ui17.fFluxCapData[20] = 0.000000; ui17.fFluxCapData[21] = 0.000000; ui17.fFluxCapData[22] = 0.000000; ui17.fFluxCapData[23] = 0.000000; ui17.fFluxCapData[24] = 0.000000; ui17.fFluxCapData[25] = 0.000000; ui17.fFluxCapData[26] = 0.000000; ui17.fFluxCapData[27] = 0.000000; ui17.fFluxCapData[28] = 0.000000; ui17.fFluxCapData[29] = 0.000000; ui17.fFluxCapData[30] = 0.000000; ui17.fFluxCapData[31] = 0.000000; ui17.fFluxCapData[32] = 0.000000; ui17.fFluxCapData[33] = 0.000000; ui17.fFluxCapData[34] = 0.000000; ui17.fFluxCapData[35] = 0.000000; ui17.fFluxCapData[36] = 0.000000; ui17.fFluxCapData[37] = 0.000000; ui17.fFluxCapData[38] = 0.000000; ui17.fFluxCapData[39] = 0.000000; ui17.fFluxCapData[40] = 0.000000; ui17.fFluxCapData[41] = 0.000000; ui17.fFluxCapData[42] = 0.000000; ui17.fFluxCapData[43] = 0.000000; ui17.fFluxCapData[44] = 0.000000; ui17.fFluxCapData[45] = 0.000000; ui17.fFluxCapData[46] = 0.000000; ui17.fFluxCapData[47] = 0.000000; ui17.fFluxCapData[48] = 0.000000; ui17.fFluxCapData[49] = 0.000000; ui17.fFluxCapData[50] = 0.000000; ui17.fFluxCapData[51] = 0.000000; ui17.fFluxCapData[52] = 0.000000; ui17.fFluxCapData[53] = 0.000000; ui17.fFluxCapData[54] = 0.000000; ui17.fFluxCapData[55] = 0.000000; ui17.fFluxCapData[56] = 0.000000; ui17.fFluxCapData[57] = 0.000000; ui17.fFluxCapData[58] = 0.000000; ui17.fFluxCapData[59] = 0.000000; ui17.fFluxCapData[60] = 0.000000; ui17.fFluxCapData[61] = 0.000000; ui17.fFluxCapData[62] = 0.000000; ui17.fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(ui17);


	m_fMeterLFGr = 0.0;
	CUICtrl ui18;
	ui18.uControlType = FILTER_CONTROL_LED_METER;
	ui18.uControlId = 54;
	ui18.bLogSlider = false;
	ui18.bExpSlider = false;
	ui18.dPresetData[0] = 0.000000;ui18.dPresetData[1] = 0.000000;ui18.dPresetData[2] = 0.000000;ui18.dPresetData[3] = 0.000000;ui18.dPresetData[4] = 0.000000;ui18.dPresetData[5] = 0.000000;ui18.dPresetData[6] = 0.000000;ui18.dPresetData[7] = 0.000000;ui18.dPresetData[8] = 0.000000;ui18.dPresetData[9] = 0.000000;ui18.dPresetData[10] = 0.000000;ui18.dPresetData[11] = 0.000000;ui18.dPresetData[12] = 0.000000;ui18.dPresetData[13] = 0.000000;ui18.dPresetData[14] = 0.000000;ui18.dPresetData[15] = 0.000000;
	ui18.cControlName = "LR";
	ui18.bOwnerControl = false;
	ui18.bMIDIControl = false;
	ui18.uMIDIControlCommand = 176;
	ui18.uMIDIControlName = 3;
	ui18.uMIDIControlChannel = 0;
	ui18.bUseMeter = true;
	ui18.bUpsideDownMeter = true;
	ui18.bLogMeter = false;
	ui18.uDetectorMode = 0;
	ui18.uMeterColorScheme = 4;
	ui18.fMeterAttack_ms = 0.000000;
	ui18.fMeterRelease_ms = 0.000000;
	ui18.cMeterVariableName = "m_fMeterLFGr";
	ui18.m_pCurrentMeterValue = &m_fMeterLFGr;
	ui18.nGUIRow = -1;
	ui18.nGUIColumn = -1;
	ui18.uControlTheme[0] = 0; ui18.uControlTheme[1] = 0; ui18.uControlTheme[2] = 0; ui18.uControlTheme[3] = 0; ui18.uControlTheme[4] = 0; ui18.uControlTheme[5] = 0; ui18.uControlTheme[6] = 0; ui18.uControlTheme[7] = 0; ui18.uControlTheme[8] = 0; ui18.uControlTheme[9] = 0; ui18.uControlTheme[10] = 0; ui18.uControlTheme[11] = 0; ui18.uControlTheme[12] = 0; ui18.uControlTheme[13] = 0; ui18.uControlTheme[14] = 0; ui18.uControlTheme[15] = 0; ui18.uControlTheme[16] = 0; ui18.uControlTheme[17] = 0; ui18.uControlTheme[18] = 0; ui18.uControlTheme[19] = 0; ui18.uControlTheme[20] = 0; ui18.uControlTheme[21] = 0; ui18.uControlTheme[22] = 1; ui18.uControlTheme[23] = 548; ui18.uControlTheme[24] = 123; ui18.uControlTheme[25] = 0; ui18.uControlTheme[26] = 0; ui18.uControlTheme[27] = 0; ui18.uControlTheme[28] = 0; ui18.uControlTheme[29] = 0; ui18.uControlTheme[30] = 0; ui18.uControlTheme[31] = 0; 
	ui18.uFluxCapControl[0] = 0; ui18.uFluxCapControl[1] = 0; ui18.uFluxCapControl[2] = 0; ui18.uFluxCapControl[3] = 0; ui18.uFluxCapControl[4] = 0; ui18.uFluxCapControl[5] = 0; ui18.uFluxCapControl[6] = 0; ui18.uFluxCapControl[7] = 0; ui18.uFluxCapControl[8] = 0; ui18.uFluxCapControl[9] = 0; ui18.uFluxCapControl[10] = 0; ui18.uFluxCapControl[11] = 0; ui18.uFluxCapControl[12] = 0; ui18.uFluxCapControl[13] = 0; ui18.uFluxCapControl[14] = 0; ui18.uFluxCapControl[15] = 0; ui18.uFluxCapControl[16] = 0; ui18.uFluxCapControl[17] = 0; ui18.uFluxCapControl[18] = 0; ui18.uFluxCapControl[19] = 0; ui18.uFluxCapControl[20] = 0; ui18.uFluxCapControl[21] = 0; ui18.uFluxCapControl[22] = 0; ui18.uFluxCapControl[23] = 0; ui18.uFluxCapControl[24] = 0; ui18.uFluxCapControl[25] = 0; ui18.uFluxCapControl[26] = 0; ui18.uFluxCapControl[27] = 0; ui18.uFluxCapControl[28] = 0; ui18.uFluxCapControl[29] = 0; ui18.uFluxCapControl[30] = 0; ui18.uFluxCapControl[31] = 0; ui18.uFluxCapControl[32] = 0; ui18.uFluxCapControl[33] = 0; ui18.uFluxCapControl[34] = 0; ui18.uFluxCapControl[35] = 0; ui18.uFluxCapControl[36] = 0; ui18.uFluxCapControl[37] = 0; ui18.uFluxCapControl[38] = 0; ui18.uFluxCapControl[39] = 0; ui18.uFluxCapControl[40] = 0; ui18.uFluxCapControl[41] = 0; ui18.uFluxCapControl[42] = 0; ui18.uFluxCapControl[43] = 0; ui18.uFluxCapControl[44] = 0; ui18.uFluxCapControl[45] = 0; ui18.uFluxCapControl[46] = 0; ui18.uFluxCapControl[47] = 0; ui18.uFluxCapControl[48] = 0; ui18.uFluxCapControl[49] = 0; ui18.uFluxCapControl[50] = 0; ui18.uFluxCapControl[51] = 0; ui18.uFluxCapControl[52] = 0; ui18.uFluxCapControl[53] = 0; ui18.uFluxCapControl[54] = 0; ui18.uFluxCapControl[55] = 0; ui18.uFluxCapControl[56] = 0; ui18.uFluxCapControl[57] = 0; ui18.uFluxCapControl[58] = 0; ui18.uFluxCapControl[59] = 0; ui18.uFluxCapControl[60] = 0; ui18.uFluxCapControl[61] = 0; ui18.uFluxCapControl[62] = 0; ui18.uFluxCapControl[63] = 0; 
	ui18.fFluxCapData[0] = 0.000000; ui18.fFluxCapData[1] = 0.000000; ui18.fFluxCapData[2] = 0.000000; ui18.fFluxCapData[3] = 0.000000; ui18.fFluxCapData[4] = 0.000000; ui18.fFluxCapData[5] = 0.000000; ui18.fFluxCapData[6] = 0.000000; ui18.fFluxCapData[7] = 0.000000; ui18.fFluxCapData[8] = 0.000000; ui18.fFluxCapData[9] = 0.000000; ui18.fFluxCapData[10] = 0.000000; ui18.fFluxCapData[11] = 0.000000; ui18.fFluxCapData[12] = 0.000000; ui18.fFluxCapData[13] = 0.000000; ui18.fFluxCapData[14] = 0.000000; ui18.fFluxCapData[15] = 0.000000; ui18.fFluxCapData[16] = 0.000000; ui18.fFluxCapData[17] = 0.000000; ui18.fFluxCapData[18] = 0.000000; ui18.fFluxCapData[19] = 0.000000; ui18.fFluxCapData[20] = 0.000000; ui18.fFluxCapData[21] = 0.000000; ui18.fFluxCapData[22] = 0.000000; ui18.fFluxCapData[23] = 0.000000; ui18.fFluxCapData[24] = 0.000000; ui18.fFluxCapData[25] = 0.000000; ui18.fFluxCapData[26] = 0.000000; ui18.fFluxCapData[27] = 0.000000; ui18.fFluxCapData[28] = 0.000000; ui18.fFluxCapData[29] = 0.000000; ui18.fFluxCapData[30] = 0.000000; ui18.fFluxCapData[31] = 0.000000; ui18.fFluxCapData[32] = 0.000000; ui18.fFluxCapData[33] = 0.000000; ui18.fFluxCapData[34] = 0.000000; ui18.fFluxCapData[35] = 0.000000; ui18.fFluxCapData[36] = 0.000000; ui18.fFluxCapData[37] = 0.000000; ui18.fFluxCapData[38] = 0.000000; ui18.fFluxCapData[39] = 0.000000; ui18.fFluxCapData[40] = 0.000000; ui18.fFluxCapData[41] = 0.000000; ui18.fFluxCapData[42] = 0.000000; ui18.fFluxCapData[43] = 0.000000; ui18.fFluxCapData[44] = 0.000000; ui18.fFluxCapData[45] = 0.000000; ui18.fFluxCapData[46] = 0.000000; ui18.fFluxCapData[47] = 0.000000; ui18.fFluxCapData[48] = 0.000000; ui18.fFluxCapData[49] = 0.000000; ui18.fFluxCapData[50] = 0.000000; ui18.fFluxCapData[51] = 0.000000; ui18.fFluxCapData[52] = 0.000000; ui18.fFluxCapData[53] = 0.000000; ui18.fFluxCapData[54] = 0.000000; ui18.fFluxCapData[55] = 0.000000; ui18.fFluxCapData[56] = 0.000000; ui18.fFluxCapData[57] = 0.000000; ui18.fFluxCapData[58] = 0.000000; ui18.fFluxCapData[59] = 0.000000; ui18.fFluxCapData[60] = 0.000000; ui18.fFluxCapData[61] = 0.000000; ui18.fFluxCapData[62] = 0.000000; ui18.fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(ui18);


	m_fMeterHFGr = 0.0;
	CUICtrl ui19;
	ui19.uControlType = FILTER_CONTROL_LED_METER;
	ui19.uControlId = 55;
	ui19.bLogSlider = false;
	ui19.bExpSlider = false;
	ui19.dPresetData[0] = 0.000000;ui19.dPresetData[1] = 0.000000;ui19.dPresetData[2] = 0.000000;ui19.dPresetData[3] = 0.000000;ui19.dPresetData[4] = 0.000000;ui19.dPresetData[5] = 0.000000;ui19.dPresetData[6] = 0.000000;ui19.dPresetData[7] = 0.000000;ui19.dPresetData[8] = 0.000000;ui19.dPresetData[9] = 0.000000;ui19.dPresetData[10] = 0.000000;ui19.dPresetData[11] = 0.000000;ui19.dPresetData[12] = 0.000000;ui19.dPresetData[13] = 0.000000;ui19.dPresetData[14] = 0.000000;ui19.dPresetData[15] = 0.000000;
	ui19.cControlName = "HR";
	ui19.bOwnerControl = false;
	ui19.bMIDIControl = false;
	ui19.uMIDIControlCommand = 176;
	ui19.uMIDIControlName = 3;
	ui19.uMIDIControlChannel = 0;
	ui19.bUseMeter = true;
	ui19.bUpsideDownMeter = true;
	ui19.bLogMeter = false;
	ui19.uDetectorMode = 0;
	ui19.uMeterColorScheme = 1;
	ui19.fMeterAttack_ms = 0.000000;
	ui19.fMeterRelease_ms = 0.000000;
	ui19.cMeterVariableName = "m_fMeterHFGr";
	ui19.m_pCurrentMeterValue = &m_fMeterHFGr;
	ui19.nGUIRow = -1;
	ui19.nGUIColumn = -1;
	ui19.uControlTheme[0] = 0; ui19.uControlTheme[1] = 0; ui19.uControlTheme[2] = 0; ui19.uControlTheme[3] = 0; ui19.uControlTheme[4] = 0; ui19.uControlTheme[5] = 0; ui19.uControlTheme[6] = 0; ui19.uControlTheme[7] = 0; ui19.uControlTheme[8] = 0; ui19.uControlTheme[9] = 0; ui19.uControlTheme[10] = 0; ui19.uControlTheme[11] = 0; ui19.uControlTheme[12] = 0; ui19.uControlTheme[13] = 0; ui19.uControlTheme[14] = 0; ui19.uControlTheme[15] = 0; ui19.uControlTheme[16] = 0; ui19.uControlTheme[17] = 0; ui19.uControlTheme[18] = 0; ui19.uControlTheme[19] = 0; ui19.uControlTheme[20] = 0; ui19.uControlTheme[21] = 0; ui19.uControlTheme[22] = 1; ui19.uControlTheme[23] = 547; ui19.uControlTheme[24] = 13; ui19.uControlTheme[25] = 0; ui19.uControlTheme[26] = 0; ui19.uControlTheme[27] = 0; ui19.uControlTheme[28] = 0; ui19.uControlTheme[29] = 0; ui19.uControlTheme[30] = 0; ui19.uControlTheme[31] = 0; 
	ui19.uFluxCapControl[0] = 0; ui19.uFluxCapControl[1] = 0; ui19.uFluxCapControl[2] = 0; ui19.uFluxCapControl[3] = 0; ui19.uFluxCapControl[4] = 0; ui19.uFluxCapControl[5] = 0; ui19.uFluxCapControl[6] = 0; ui19.uFluxCapControl[7] = 0; ui19.uFluxCapControl[8] = 0; ui19.uFluxCapControl[9] = 0; ui19.uFluxCapControl[10] = 0; ui19.uFluxCapControl[11] = 0; ui19.uFluxCapControl[12] = 0; ui19.uFluxCapControl[13] = 0; ui19.uFluxCapControl[14] = 0; ui19.uFluxCapControl[15] = 0; ui19.uFluxCapControl[16] = 0; ui19.uFluxCapControl[17] = 0; ui19.uFluxCapControl[18] = 0; ui19.uFluxCapControl[19] = 0; ui19.uFluxCapControl[20] = 0; ui19.uFluxCapControl[21] = 0; ui19.uFluxCapControl[22] = 0; ui19.uFluxCapControl[23] = 0; ui19.uFluxCapControl[24] = 0; ui19.uFluxCapControl[25] = 0; ui19.uFluxCapControl[26] = 0; ui19.uFluxCapControl[27] = 0; ui19.uFluxCapControl[28] = 0; ui19.uFluxCapControl[29] = 0; ui19.uFluxCapControl[30] = 0; ui19.uFluxCapControl[31] = 0; ui19.uFluxCapControl[32] = 0; ui19.uFluxCapControl[33] = 0; ui19.uFluxCapControl[34] = 0; ui19.uFluxCapControl[35] = 0; ui19.uFluxCapControl[36] = 0; ui19.uFluxCapControl[37] = 0; ui19.uFluxCapControl[38] = 0; ui19.uFluxCapControl[39] = 0; ui19.uFluxCapControl[40] = 0; ui19.uFluxCapControl[41] = 0; ui19.uFluxCapControl[42] = 0; ui19.uFluxCapControl[43] = 0; ui19.uFluxCapControl[44] = 0; ui19.uFluxCapControl[45] = 0; ui19.uFluxCapControl[46] = 0; ui19.uFluxCapControl[47] = 0; ui19.uFluxCapControl[48] = 0; ui19.uFluxCapControl[49] = 0; ui19.uFluxCapControl[50] = 0; ui19.uFluxCapControl[51] = 0; ui19.uFluxCapControl[52] = 0; ui19.uFluxCapControl[53] = 0; ui19.uFluxCapControl[54] = 0; ui19.uFluxCapControl[55] = 0; ui19.uFluxCapControl[56] = 0; ui19.uFluxCapControl[57] = 0; ui19.uFluxCapControl[58] = 0; ui19.uFluxCapControl[59] = 0; ui19.uFluxCapControl[60] = 0; ui19.uFluxCapControl[61] = 0; ui19.uFluxCapControl[62] = 0; ui19.uFluxCapControl[63] = 0; 
	ui19.fFluxCapData[0] = 0.000000; ui19.fFluxCapData[1] = 0.000000; ui19.fFluxCapData[2] = 0.000000; ui19.fFluxCapData[3] = 0.000000; ui19.fFluxCapData[4] = 0.000000; ui19.fFluxCapData[5] = 0.000000; ui19.fFluxCapData[6] = 0.000000; ui19.fFluxCapData[7] = 0.000000; ui19.fFluxCapData[8] = 0.000000; ui19.fFluxCapData[9] = 0.000000; ui19.fFluxCapData[10] = 0.000000; ui19.fFluxCapData[11] = 0.000000; ui19.fFluxCapData[12] = 0.000000; ui19.fFluxCapData[13] = 0.000000; ui19.fFluxCapData[14] = 0.000000; ui19.fFluxCapData[15] = 0.000000; ui19.fFluxCapData[16] = 0.000000; ui19.fFluxCapData[17] = 0.000000; ui19.fFluxCapData[18] = 0.000000; ui19.fFluxCapData[19] = 0.000000; ui19.fFluxCapData[20] = 0.000000; ui19.fFluxCapData[21] = 0.000000; ui19.fFluxCapData[22] = 0.000000; ui19.fFluxCapData[23] = 0.000000; ui19.fFluxCapData[24] = 0.000000; ui19.fFluxCapData[25] = 0.000000; ui19.fFluxCapData[26] = 0.000000; ui19.fFluxCapData[27] = 0.000000; ui19.fFluxCapData[28] = 0.000000; ui19.fFluxCapData[29] = 0.000000; ui19.fFluxCapData[30] = 0.000000; ui19.fFluxCapData[31] = 0.000000; ui19.fFluxCapData[32] = 0.000000; ui19.fFluxCapData[33] = 0.000000; ui19.fFluxCapData[34] = 0.000000; ui19.fFluxCapData[35] = 0.000000; ui19.fFluxCapData[36] = 0.000000; ui19.fFluxCapData[37] = 0.000000; ui19.fFluxCapData[38] = 0.000000; ui19.fFluxCapData[39] = 0.000000; ui19.fFluxCapData[40] = 0.000000; ui19.fFluxCapData[41] = 0.000000; ui19.fFluxCapData[42] = 0.000000; ui19.fFluxCapData[43] = 0.000000; ui19.fFluxCapData[44] = 0.000000; ui19.fFluxCapData[45] = 0.000000; ui19.fFluxCapData[46] = 0.000000; ui19.fFluxCapData[47] = 0.000000; ui19.fFluxCapData[48] = 0.000000; ui19.fFluxCapData[49] = 0.000000; ui19.fFluxCapData[50] = 0.000000; ui19.fFluxCapData[51] = 0.000000; ui19.fFluxCapData[52] = 0.000000; ui19.fFluxCapData[53] = 0.000000; ui19.fFluxCapData[54] = 0.000000; ui19.fFluxCapData[55] = 0.000000; ui19.fFluxCapData[56] = 0.000000; ui19.fFluxCapData[57] = 0.000000; ui19.fFluxCapData[58] = 0.000000; ui19.fFluxCapData[59] = 0.000000; ui19.fFluxCapData[60] = 0.000000; ui19.fFluxCapData[61] = 0.000000; ui19.fFluxCapData[62] = 0.000000; ui19.fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(ui19);


	m_fMeterLFOut = 0.0;
	CUICtrl ui20;
	ui20.uControlType = FILTER_CONTROL_LED_METER;
	ui20.uControlId = 58;
	ui20.bLogSlider = false;
	ui20.bExpSlider = false;
	ui20.dPresetData[0] = 0.000000;ui20.dPresetData[1] = 0.000000;ui20.dPresetData[2] = 0.000000;ui20.dPresetData[3] = 0.000000;ui20.dPresetData[4] = 0.000000;ui20.dPresetData[5] = 0.000000;ui20.dPresetData[6] = 0.000000;ui20.dPresetData[7] = 0.000000;ui20.dPresetData[8] = 0.000000;ui20.dPresetData[9] = 0.000000;ui20.dPresetData[10] = 0.000000;ui20.dPresetData[11] = 0.000000;ui20.dPresetData[12] = 0.000000;ui20.dPresetData[13] = 0.000000;ui20.dPresetData[14] = 0.000000;ui20.dPresetData[15] = 0.000000;
	ui20.cControlName = "LFO";
	ui20.bOwnerControl = false;
	ui20.bMIDIControl = false;
	ui20.uMIDIControlCommand = 176;
	ui20.uMIDIControlName = 3;
	ui20.uMIDIControlChannel = 0;
	ui20.bUseMeter = true;
	ui20.bUpsideDownMeter = false;
	ui20.bLogMeter = true;
	ui20.uDetectorMode = 2;
	ui20.uMeterColorScheme = 0;
	ui20.fMeterAttack_ms = 10.000000;
	ui20.fMeterRelease_ms = 500.000000;
	ui20.cMeterVariableName = "m_fMeterLFOut";
	ui20.m_pCurrentMeterValue = &m_fMeterLFOut;
	ui20.nGUIRow = -1;
	ui20.nGUIColumn = -1;
	ui20.uControlTheme[0] = 0; ui20.uControlTheme[1] = 0; ui20.uControlTheme[2] = 0; ui20.uControlTheme[3] = 0; ui20.uControlTheme[4] = 0; ui20.uControlTheme[5] = 0; ui20.uControlTheme[6] = 0; ui20.uControlTheme[7] = 0; ui20.uControlTheme[8] = 0; ui20.uControlTheme[9] = 0; ui20.uControlTheme[10] = 0; ui20.uControlTheme[11] = 0; ui20.uControlTheme[12] = 0; ui20.uControlTheme[13] = 0; ui20.uControlTheme[14] = 0; ui20.uControlTheme[15] = 0; ui20.uControlTheme[16] = 0; ui20.uControlTheme[17] = 0; ui20.uControlTheme[18] = 0; ui20.uControlTheme[19] = 0; ui20.uControlTheme[20] = 0; ui20.uControlTheme[21] = 0; ui20.uControlTheme[22] = 1; ui20.uControlTheme[23] = 581; ui20.uControlTheme[24] = 123; ui20.uControlTheme[25] = 0; ui20.uControlTheme[26] = 0; ui20.uControlTheme[27] = 0; ui20.uControlTheme[28] = 0; ui20.uControlTheme[29] = 0; ui20.uControlTheme[30] = 0; ui20.uControlTheme[31] = 0; 
	ui20.uFluxCapControl[0] = 0; ui20.uFluxCapControl[1] = 0; ui20.uFluxCapControl[2] = 0; ui20.uFluxCapControl[3] = 0; ui20.uFluxCapControl[4] = 0; ui20.uFluxCapControl[5] = 0; ui20.uFluxCapControl[6] = 0; ui20.uFluxCapControl[7] = 0; ui20.uFluxCapControl[8] = 0; ui20.uFluxCapControl[9] = 0; ui20.uFluxCapControl[10] = 0; ui20.uFluxCapControl[11] = 0; ui20.uFluxCapControl[12] = 0; ui20.uFluxCapControl[13] = 0; ui20.uFluxCapControl[14] = 0; ui20.uFluxCapControl[15] = 0; ui20.uFluxCapControl[16] = 0; ui20.uFluxCapControl[17] = 0; ui20.uFluxCapControl[18] = 0; ui20.uFluxCapControl[19] = 0; ui20.uFluxCapControl[20] = 0; ui20.uFluxCapControl[21] = 0; ui20.uFluxCapControl[22] = 0; ui20.uFluxCapControl[23] = 0; ui20.uFluxCapControl[24] = 0; ui20.uFluxCapControl[25] = 0; ui20.uFluxCapControl[26] = 0; ui20.uFluxCapControl[27] = 0; ui20.uFluxCapControl[28] = 0; ui20.uFluxCapControl[29] = 0; ui20.uFluxCapControl[30] = 0; ui20.uFluxCapControl[31] = 0; ui20.uFluxCapControl[32] = 0; ui20.uFluxCapControl[33] = 0; ui20.uFluxCapControl[34] = 0; ui20.uFluxCapControl[35] = 0; ui20.uFluxCapControl[36] = 0; ui20.uFluxCapControl[37] = 0; ui20.uFluxCapControl[38] = 0; ui20.uFluxCapControl[39] = 0; ui20.uFluxCapControl[40] = 0; ui20.uFluxCapControl[41] = 0; ui20.uFluxCapControl[42] = 0; ui20.uFluxCapControl[43] = 0; ui20.uFluxCapControl[44] = 0; ui20.uFluxCapControl[45] = 0; ui20.uFluxCapControl[46] = 0; ui20.uFluxCapControl[47] = 0; ui20.uFluxCapControl[48] = 0; ui20.uFluxCapControl[49] = 0; ui20.uFluxCapControl[50] = 0; ui20.uFluxCapControl[51] = 0; ui20.uFluxCapControl[52] = 0; ui20.uFluxCapControl[53] = 0; ui20.uFluxCapControl[54] = 0; ui20.uFluxCapControl[55] = 0; ui20.uFluxCapControl[56] = 0; ui20.uFluxCapControl[57] = 0; ui20.uFluxCapControl[58] = 0; ui20.uFluxCapControl[59] = 0; ui20.uFluxCapControl[60] = 0; ui20.uFluxCapControl[61] = 0; ui20.uFluxCapControl[62] = 0; ui20.uFluxCapControl[63] = 0; 
	ui20.fFluxCapData[0] = 0.000000; ui20.fFluxCapData[1] = 0.000000; ui20.fFluxCapData[2] = 0.000000; ui20.fFluxCapData[3] = 0.000000; ui20.fFluxCapData[4] = 0.000000; ui20.fFluxCapData[5] = 0.000000; ui20.fFluxCapData[6] = 0.000000; ui20.fFluxCapData[7] = 0.000000; ui20.fFluxCapData[8] = 0.000000; ui20.fFluxCapData[9] = 0.000000; ui20.fFluxCapData[10] = 0.000000; ui20.fFluxCapData[11] = 0.000000; ui20.fFluxCapData[12] = 0.000000; ui20.fFluxCapData[13] = 0.000000; ui20.fFluxCapData[14] = 0.000000; ui20.fFluxCapData[15] = 0.000000; ui20.fFluxCapData[16] = 0.000000; ui20.fFluxCapData[17] = 0.000000; ui20.fFluxCapData[18] = 0.000000; ui20.fFluxCapData[19] = 0.000000; ui20.fFluxCapData[20] = 0.000000; ui20.fFluxCapData[21] = 0.000000; ui20.fFluxCapData[22] = 0.000000; ui20.fFluxCapData[23] = 0.000000; ui20.fFluxCapData[24] = 0.000000; ui20.fFluxCapData[25] = 0.000000; ui20.fFluxCapData[26] = 0.000000; ui20.fFluxCapData[27] = 0.000000; ui20.fFluxCapData[28] = 0.000000; ui20.fFluxCapData[29] = 0.000000; ui20.fFluxCapData[30] = 0.000000; ui20.fFluxCapData[31] = 0.000000; ui20.fFluxCapData[32] = 0.000000; ui20.fFluxCapData[33] = 0.000000; ui20.fFluxCapData[34] = 0.000000; ui20.fFluxCapData[35] = 0.000000; ui20.fFluxCapData[36] = 0.000000; ui20.fFluxCapData[37] = 0.000000; ui20.fFluxCapData[38] = 0.000000; ui20.fFluxCapData[39] = 0.000000; ui20.fFluxCapData[40] = 0.000000; ui20.fFluxCapData[41] = 0.000000; ui20.fFluxCapData[42] = 0.000000; ui20.fFluxCapData[43] = 0.000000; ui20.fFluxCapData[44] = 0.000000; ui20.fFluxCapData[45] = 0.000000; ui20.fFluxCapData[46] = 0.000000; ui20.fFluxCapData[47] = 0.000000; ui20.fFluxCapData[48] = 0.000000; ui20.fFluxCapData[49] = 0.000000; ui20.fFluxCapData[50] = 0.000000; ui20.fFluxCapData[51] = 0.000000; ui20.fFluxCapData[52] = 0.000000; ui20.fFluxCapData[53] = 0.000000; ui20.fFluxCapData[54] = 0.000000; ui20.fFluxCapData[55] = 0.000000; ui20.fFluxCapData[56] = 0.000000; ui20.fFluxCapData[57] = 0.000000; ui20.fFluxCapData[58] = 0.000000; ui20.fFluxCapData[59] = 0.000000; ui20.fFluxCapData[60] = 0.000000; ui20.fFluxCapData[61] = 0.000000; ui20.fFluxCapData[62] = 0.000000; ui20.fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(ui20);


	m_fMeterHFOut = 0.0;
	CUICtrl ui21;
	ui21.uControlType = FILTER_CONTROL_LED_METER;
	ui21.uControlId = 59;
	ui21.bLogSlider = false;
	ui21.bExpSlider = false;
	ui21.dPresetData[0] = 0.000000;ui21.dPresetData[1] = 0.000000;ui21.dPresetData[2] = 0.000000;ui21.dPresetData[3] = 0.000000;ui21.dPresetData[4] = 0.000000;ui21.dPresetData[5] = 0.000000;ui21.dPresetData[6] = 0.000000;ui21.dPresetData[7] = 0.000000;ui21.dPresetData[8] = 0.000000;ui21.dPresetData[9] = 0.000000;ui21.dPresetData[10] = 0.000000;ui21.dPresetData[11] = 0.000000;ui21.dPresetData[12] = 0.000000;ui21.dPresetData[13] = 0.000000;ui21.dPresetData[14] = 0.000000;ui21.dPresetData[15] = 0.000000;
	ui21.cControlName = "HFO";
	ui21.bOwnerControl = false;
	ui21.bMIDIControl = false;
	ui21.uMIDIControlCommand = 176;
	ui21.uMIDIControlName = 3;
	ui21.uMIDIControlChannel = 0;
	ui21.bUseMeter = true;
	ui21.bUpsideDownMeter = false;
	ui21.bLogMeter = true;
	ui21.uDetectorMode = 2;
	ui21.uMeterColorScheme = 0;
	ui21.fMeterAttack_ms = 10.000000;
	ui21.fMeterRelease_ms = 500.000000;
	ui21.cMeterVariableName = "m_fMeterHFOut";
	ui21.m_pCurrentMeterValue = &m_fMeterHFOut;
	ui21.nGUIRow = -1;
	ui21.nGUIColumn = -1;
	ui21.uControlTheme[0] = 0; ui21.uControlTheme[1] = 0; ui21.uControlTheme[2] = 0; ui21.uControlTheme[3] = 0; ui21.uControlTheme[4] = 0; ui21.uControlTheme[5] = 0; ui21.uControlTheme[6] = 0; ui21.uControlTheme[7] = 0; ui21.uControlTheme[8] = 0; ui21.uControlTheme[9] = 0; ui21.uControlTheme[10] = 0; ui21.uControlTheme[11] = 0; ui21.uControlTheme[12] = 0; ui21.uControlTheme[13] = 0; ui21.uControlTheme[14] = 0; ui21.uControlTheme[15] = 0; ui21.uControlTheme[16] = 0; ui21.uControlTheme[17] = 0; ui21.uControlTheme[18] = 0; ui21.uControlTheme[19] = 0; ui21.uControlTheme[20] = 0; ui21.uControlTheme[21] = 0; ui21.uControlTheme[22] = 1; ui21.uControlTheme[23] = 579; ui21.uControlTheme[24] = 13; ui21.uControlTheme[25] = 0; ui21.uControlTheme[26] = 0; ui21.uControlTheme[27] = 0; ui21.uControlTheme[28] = 0; ui21.uControlTheme[29] = 0; ui21.uControlTheme[30] = 0; ui21.uControlTheme[31] = 0; 
	ui21.uFluxCapControl[0] = 0; ui21.uFluxCapControl[1] = 0; ui21.uFluxCapControl[2] = 0; ui21.uFluxCapControl[3] = 0; ui21.uFluxCapControl[4] = 0; ui21.uFluxCapControl[5] = 0; ui21.uFluxCapControl[6] = 0; ui21.uFluxCapControl[7] = 0; ui21.uFluxCapControl[8] = 0; ui21.uFluxCapControl[9] = 0; ui21.uFluxCapControl[10] = 0; ui21.uFluxCapControl[11] = 0; ui21.uFluxCapControl[12] = 0; ui21.uFluxCapControl[13] = 0; ui21.uFluxCapControl[14] = 0; ui21.uFluxCapControl[15] = 0; ui21.uFluxCapControl[16] = 0; ui21.uFluxCapControl[17] = 0; ui21.uFluxCapControl[18] = 0; ui21.uFluxCapControl[19] = 0; ui21.uFluxCapControl[20] = 0; ui21.uFluxCapControl[21] = 0; ui21.uFluxCapControl[22] = 0; ui21.uFluxCapControl[23] = 0; ui21.uFluxCapControl[24] = 0; ui21.uFluxCapControl[25] = 0; ui21.uFluxCapControl[26] = 0; ui21.uFluxCapControl[27] = 0; ui21.uFluxCapControl[28] = 0; ui21.uFluxCapControl[29] = 0; ui21.uFluxCapControl[30] = 0; ui21.uFluxCapControl[31] = 0; ui21.uFluxCapControl[32] = 0; ui21.uFluxCapControl[33] = 0; ui21.uFluxCapControl[34] = 0; ui21.uFluxCapControl[35] = 0; ui21.uFluxCapControl[36] = 0; ui21.uFluxCapControl[37] = 0; ui21.uFluxCapControl[38] = 0; ui21.uFluxCapControl[39] = 0; ui21.uFluxCapControl[40] = 0; ui21.uFluxCapControl[41] = 0; ui21.uFluxCapControl[42] = 0; ui21.uFluxCapControl[43] = 0; ui21.uFluxCapControl[44] = 0; ui21.uFluxCapControl[45] = 0; ui21.uFluxCapControl[46] = 0; ui21.uFluxCapControl[47] = 0; ui21.uFluxCapControl[48] = 0; ui21.uFluxCapControl[49] = 0; ui21.uFluxCapControl[50] = 0; ui21.uFluxCapControl[51] = 0; ui21.uFluxCapControl[52] = 0; ui21.uFluxCapControl[53] = 0; ui21.uFluxCapControl[54] = 0; ui21.uFluxCapControl[55] = 0; ui21.uFluxCapControl[56] = 0; ui21.uFluxCapControl[57] = 0; ui21.uFluxCapControl[58] = 0; ui21.uFluxCapControl[59] = 0; ui21.uFluxCapControl[60] = 0; ui21.uFluxCapControl[61] = 0; ui21.uFluxCapControl[62] = 0; ui21.uFluxCapControl[63] = 0; 
	ui21.fFluxCapData[0] = 0.000000; ui21.fFluxCapData[1] = 0.000000; ui21.fFluxCapData[2] = 0.000000; ui21.fFluxCapData[3] = 0.000000; ui21.fFluxCapData[4] = 0.000000; ui21.fFluxCapData[5] = 0.000000; ui21.fFluxCapData[6] = 0.000000; ui21.fFluxCapData[7] = 0.000000; ui21.fFluxCapData[8] = 0.000000; ui21.fFluxCapData[9] = 0.000000; ui21.fFluxCapData[10] = 0.000000; ui21.fFluxCapData[11] = 0.000000; ui21.fFluxCapData[12] = 0.000000; ui21.fFluxCapData[13] = 0.000000; ui21.fFluxCapData[14] = 0.000000; ui21.fFluxCapData[15] = 0.000000; ui21.fFluxCapData[16] = 0.000000; ui21.fFluxCapData[17] = 0.000000; ui21.fFluxCapData[18] = 0.000000; ui21.fFluxCapData[19] = 0.000000; ui21.fFluxCapData[20] = 0.000000; ui21.fFluxCapData[21] = 0.000000; ui21.fFluxCapData[22] = 0.000000; ui21.fFluxCapData[23] = 0.000000; ui21.fFluxCapData[24] = 0.000000; ui21.fFluxCapData[25] = 0.000000; ui21.fFluxCapData[26] = 0.000000; ui21.fFluxCapData[27] = 0.000000; ui21.fFluxCapData[28] = 0.000000; ui21.fFluxCapData[29] = 0.000000; ui21.fFluxCapData[30] = 0.000000; ui21.fFluxCapData[31] = 0.000000; ui21.fFluxCapData[32] = 0.000000; ui21.fFluxCapData[33] = 0.000000; ui21.fFluxCapData[34] = 0.000000; ui21.fFluxCapData[35] = 0.000000; ui21.fFluxCapData[36] = 0.000000; ui21.fFluxCapData[37] = 0.000000; ui21.fFluxCapData[38] = 0.000000; ui21.fFluxCapData[39] = 0.000000; ui21.fFluxCapData[40] = 0.000000; ui21.fFluxCapData[41] = 0.000000; ui21.fFluxCapData[42] = 0.000000; ui21.fFluxCapData[43] = 0.000000; ui21.fFluxCapData[44] = 0.000000; ui21.fFluxCapData[45] = 0.000000; ui21.fFluxCapData[46] = 0.000000; ui21.fFluxCapData[47] = 0.000000; ui21.fFluxCapData[48] = 0.000000; ui21.fFluxCapData[49] = 0.000000; ui21.fFluxCapData[50] = 0.000000; ui21.fFluxCapData[51] = 0.000000; ui21.fFluxCapData[52] = 0.000000; ui21.fFluxCapData[53] = 0.000000; ui21.fFluxCapData[54] = 0.000000; ui21.fFluxCapData[55] = 0.000000; ui21.fFluxCapData[56] = 0.000000; ui21.fFluxCapData[57] = 0.000000; ui21.fFluxCapData[58] = 0.000000; ui21.fFluxCapData[59] = 0.000000; ui21.fFluxCapData[60] = 0.000000; ui21.fFluxCapData[61] = 0.000000; ui21.fFluxCapData[62] = 0.000000; ui21.fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(ui21);


	m_uX_TrackPadIndex = -1;
	m_uY_TrackPadIndex = -1;

	m_AssignButton1Name = "B1";
	m_AssignButton2Name = "B2";
	m_AssignButton3Name = "B3";

	m_bLatchingAssignButton1 = false;
	m_bLatchingAssignButton2 = false;
	m_bLatchingAssignButton3 = false;

	m_nGUIType = -1;
	m_nGUIThemeID = -1;
	m_bUseCustomVSTGUI = false;

	m_uControlTheme[0] = 0; m_uControlTheme[1] = 0; m_uControlTheme[2] = 0; m_uControlTheme[3] = 0; m_uControlTheme[4] = 0; m_uControlTheme[5] = 0; m_uControlTheme[6] = 0; m_uControlTheme[7] = 0; m_uControlTheme[8] = 0; m_uControlTheme[9] = 0; m_uControlTheme[10] = 0; m_uControlTheme[11] = 0; m_uControlTheme[12] = 0; m_uControlTheme[13] = 0; m_uControlTheme[14] = 0; m_uControlTheme[15] = 0; m_uControlTheme[16] = 0; m_uControlTheme[17] = 0; m_uControlTheme[18] = 0; m_uControlTheme[19] = 0; m_uControlTheme[20] = 0; m_uControlTheme[21] = 0; m_uControlTheme[22] = 0; m_uControlTheme[23] = 0; m_uControlTheme[24] = 0; m_uControlTheme[25] = 0; m_uControlTheme[26] = 0; m_uControlTheme[27] = 0; m_uControlTheme[28] = 0; m_uControlTheme[29] = 0; m_uControlTheme[30] = 0; m_uControlTheme[31] = 0; m_uControlTheme[32] = 0; m_uControlTheme[33] = 0; m_uControlTheme[34] = 0; m_uControlTheme[35] = 0; m_uControlTheme[36] = 0; m_uControlTheme[37] = 0; m_uControlTheme[38] = 0; m_uControlTheme[39] = 0; m_uControlTheme[40] = 0; m_uControlTheme[41] = 0; m_uControlTheme[42] = 0; m_uControlTheme[43] = 0; m_uControlTheme[44] = 0; m_uControlTheme[45] = 0; m_uControlTheme[46] = 0; m_uControlTheme[47] = 0; m_uControlTheme[48] = 0; m_uControlTheme[49] = 0; m_uControlTheme[50] = 0; m_uControlTheme[51] = 0; m_uControlTheme[52] = 0; m_uControlTheme[53] = 0; m_uControlTheme[54] = 0; m_uControlTheme[55] = 0; m_uControlTheme[56] = 0; m_uControlTheme[57] = 0; m_uControlTheme[58] = 0; m_uControlTheme[59] = 0; m_uControlTheme[60] = 0; m_uControlTheme[61] = 0; m_uControlTheme[62] = 0; m_uControlTheme[63] = 0; 

	m_uPlugInEx[0] = 0; m_uPlugInEx[1] = 0; m_uPlugInEx[2] = 0; m_uPlugInEx[3] = 0; m_uPlugInEx[4] = 0; m_uPlugInEx[5] = 0; m_uPlugInEx[6] = 0; m_uPlugInEx[7] = 0; m_uPlugInEx[8] = 0; m_uPlugInEx[9] = 0; m_uPlugInEx[10] = 0; m_uPlugInEx[11] = 0; m_uPlugInEx[12] = 0; m_uPlugInEx[13] = 0; m_uPlugInEx[14] = 0; m_uPlugInEx[15] = 0; m_uPlugInEx[16] = 0; m_uPlugInEx[17] = 0; m_uPlugInEx[18] = 0; m_uPlugInEx[19] = 0; m_uPlugInEx[20] = 0; m_uPlugInEx[21] = 0; m_uPlugInEx[22] = 0; m_uPlugInEx[23] = 0; m_uPlugInEx[24] = 0; m_uPlugInEx[25] = 0; m_uPlugInEx[26] = 0; m_uPlugInEx[27] = 0; m_uPlugInEx[28] = 0; m_uPlugInEx[29] = 0; m_uPlugInEx[30] = 0; m_uPlugInEx[31] = 0; m_uPlugInEx[32] = 0; m_uPlugInEx[33] = 0; m_uPlugInEx[34] = 0; m_uPlugInEx[35] = 0; m_uPlugInEx[36] = 0; m_uPlugInEx[37] = 0; m_uPlugInEx[38] = 0; m_uPlugInEx[39] = 0; m_uPlugInEx[40] = 0; m_uPlugInEx[41] = 0; m_uPlugInEx[42] = 0; m_uPlugInEx[43] = 0; m_uPlugInEx[44] = 0; m_uPlugInEx[45] = 0; m_uPlugInEx[46] = 0; m_uPlugInEx[47] = 0; m_uPlugInEx[48] = 0; m_uPlugInEx[49] = 0; m_uPlugInEx[50] = 0; m_uPlugInEx[51] = 0; m_uPlugInEx[52] = 0; m_uPlugInEx[53] = 0; m_uPlugInEx[54] = 0; m_uPlugInEx[55] = 0; m_uPlugInEx[56] = 0; m_uPlugInEx[57] = 0; m_uPlugInEx[58] = 0; m_uPlugInEx[59] = 0; m_uPlugInEx[60] = 0; m_uPlugInEx[61] = 0; m_uPlugInEx[62] = 0; m_uPlugInEx[63] = 0; 
	m_fPlugInEx[0] = 0.000000; m_fPlugInEx[1] = 0.000000; m_fPlugInEx[2] = 0.000000; m_fPlugInEx[3] = 0.000000; m_fPlugInEx[4] = 0.000000; m_fPlugInEx[5] = 0.000000; m_fPlugInEx[6] = 0.000000; m_fPlugInEx[7] = 0.000000; m_fPlugInEx[8] = 0.000000; m_fPlugInEx[9] = 0.000000; m_fPlugInEx[10] = 0.000000; m_fPlugInEx[11] = 0.000000; m_fPlugInEx[12] = 0.000000; m_fPlugInEx[13] = 0.000000; m_fPlugInEx[14] = 0.000000; m_fPlugInEx[15] = 0.000000; m_fPlugInEx[16] = 0.000000; m_fPlugInEx[17] = 0.000000; m_fPlugInEx[18] = 0.000000; m_fPlugInEx[19] = 0.000000; m_fPlugInEx[20] = 0.000000; m_fPlugInEx[21] = 0.000000; m_fPlugInEx[22] = 0.000000; m_fPlugInEx[23] = 0.000000; m_fPlugInEx[24] = 0.000000; m_fPlugInEx[25] = 0.000000; m_fPlugInEx[26] = 0.000000; m_fPlugInEx[27] = 0.000000; m_fPlugInEx[28] = 0.000000; m_fPlugInEx[29] = 0.000000; m_fPlugInEx[30] = 0.000000; m_fPlugInEx[31] = 0.000000; m_fPlugInEx[32] = 0.000000; m_fPlugInEx[33] = 0.000000; m_fPlugInEx[34] = 0.000000; m_fPlugInEx[35] = 0.000000; m_fPlugInEx[36] = 0.000000; m_fPlugInEx[37] = 0.000000; m_fPlugInEx[38] = 0.000000; m_fPlugInEx[39] = 0.000000; m_fPlugInEx[40] = 0.000000; m_fPlugInEx[41] = 0.000000; m_fPlugInEx[42] = 0.000000; m_fPlugInEx[43] = 0.000000; m_fPlugInEx[44] = 0.000000; m_fPlugInEx[45] = 0.000000; m_fPlugInEx[46] = 0.000000; m_fPlugInEx[47] = 0.000000; m_fPlugInEx[48] = 0.000000; m_fPlugInEx[49] = 0.000000; m_fPlugInEx[50] = 0.000000; m_fPlugInEx[51] = 0.000000; m_fPlugInEx[52] = 0.000000; m_fPlugInEx[53] = 0.000000; m_fPlugInEx[54] = 0.000000; m_fPlugInEx[55] = 0.000000; m_fPlugInEx[56] = 0.000000; m_fPlugInEx[57] = 0.000000; m_fPlugInEx[58] = 0.000000; m_fPlugInEx[59] = 0.000000; m_fPlugInEx[60] = 0.000000; m_fPlugInEx[61] = 0.000000; m_fPlugInEx[62] = 0.000000; m_fPlugInEx[63] = 0.000000; 

	m_TextLabels[0] = ""; m_TextLabels[1] = ""; m_TextLabels[2] = ""; m_TextLabels[3] = ""; m_TextLabels[4] = ""; m_TextLabels[5] = ""; m_TextLabels[6] = ""; m_TextLabels[7] = ""; m_TextLabels[8] = ""; m_TextLabels[9] = ""; m_TextLabels[10] = ""; m_TextLabels[11] = ""; m_TextLabels[12] = ""; m_TextLabels[13] = ""; m_TextLabels[14] = ""; m_TextLabels[15] = ""; m_TextLabels[16] = ""; m_TextLabels[17] = ""; m_TextLabels[18] = ""; m_TextLabels[19] = ""; m_TextLabels[20] = ""; m_TextLabels[21] = ""; m_TextLabels[22] = ""; m_TextLabels[23] = ""; m_TextLabels[24] = ""; m_TextLabels[25] = ""; m_TextLabels[26] = ""; m_TextLabels[27] = ""; m_TextLabels[28] = ""; m_TextLabels[29] = ""; m_TextLabels[30] = ""; m_TextLabels[31] = ""; m_TextLabels[32] = ""; m_TextLabels[33] = ""; m_TextLabels[34] = ""; m_TextLabels[35] = ""; m_TextLabels[36] = ""; m_TextLabels[37] = ""; m_TextLabels[38] = ""; m_TextLabels[39] = ""; m_TextLabels[40] = ""; m_TextLabels[41] = ""; m_TextLabels[42] = ""; m_TextLabels[43] = ""; m_TextLabels[44] = ""; m_TextLabels[45] = ""; m_TextLabels[46] = ""; m_TextLabels[47] = ""; m_TextLabels[48] = ""; m_TextLabels[49] = ""; m_TextLabels[50] = ""; m_TextLabels[51] = ""; m_TextLabels[52] = ""; m_TextLabels[53] = ""; m_TextLabels[54] = ""; m_TextLabels[55] = ""; m_TextLabels[56] = ""; m_TextLabels[57] = ""; m_TextLabels[58] = ""; m_TextLabels[59] = ""; m_TextLabels[60] = ""; m_TextLabels[61] = ""; m_TextLabels[62] = ""; m_TextLabels[63] = ""; 

	m_uLabelCX[0] = 0; m_uLabelCX[1] = 0; m_uLabelCX[2] = 0; m_uLabelCX[3] = 0; m_uLabelCX[4] = 0; m_uLabelCX[5] = 0; m_uLabelCX[6] = 0; m_uLabelCX[7] = 0; m_uLabelCX[8] = 0; m_uLabelCX[9] = 0; m_uLabelCX[10] = 0; m_uLabelCX[11] = 0; m_uLabelCX[12] = 0; m_uLabelCX[13] = 0; m_uLabelCX[14] = 0; m_uLabelCX[15] = 0; m_uLabelCX[16] = 0; m_uLabelCX[17] = 0; m_uLabelCX[18] = 0; m_uLabelCX[19] = 0; m_uLabelCX[20] = 0; m_uLabelCX[21] = 0; m_uLabelCX[22] = 0; m_uLabelCX[23] = 0; m_uLabelCX[24] = 0; m_uLabelCX[25] = 0; m_uLabelCX[26] = 0; m_uLabelCX[27] = 0; m_uLabelCX[28] = 0; m_uLabelCX[29] = 0; m_uLabelCX[30] = 0; m_uLabelCX[31] = 0; m_uLabelCX[32] = 0; m_uLabelCX[33] = 0; m_uLabelCX[34] = 0; m_uLabelCX[35] = 0; m_uLabelCX[36] = 0; m_uLabelCX[37] = 0; m_uLabelCX[38] = 0; m_uLabelCX[39] = 0; m_uLabelCX[40] = 0; m_uLabelCX[41] = 0; m_uLabelCX[42] = 0; m_uLabelCX[43] = 0; m_uLabelCX[44] = 0; m_uLabelCX[45] = 0; m_uLabelCX[46] = 0; m_uLabelCX[47] = 0; m_uLabelCX[48] = 0; m_uLabelCX[49] = 0; m_uLabelCX[50] = 0; m_uLabelCX[51] = 0; m_uLabelCX[52] = 0; m_uLabelCX[53] = 0; m_uLabelCX[54] = 0; m_uLabelCX[55] = 0; m_uLabelCX[56] = 0; m_uLabelCX[57] = 0; m_uLabelCX[58] = 0; m_uLabelCX[59] = 0; m_uLabelCX[60] = 0; m_uLabelCX[61] = 0; m_uLabelCX[62] = 0; m_uLabelCX[63] = 0; 
	m_uLabelCY[0] = 0; m_uLabelCY[1] = 0; m_uLabelCY[2] = 0; m_uLabelCY[3] = 0; m_uLabelCY[4] = 0; m_uLabelCY[5] = 0; m_uLabelCY[6] = 0; m_uLabelCY[7] = 0; m_uLabelCY[8] = 0; m_uLabelCY[9] = 0; m_uLabelCY[10] = 0; m_uLabelCY[11] = 0; m_uLabelCY[12] = 0; m_uLabelCY[13] = 0; m_uLabelCY[14] = 0; m_uLabelCY[15] = 0; m_uLabelCY[16] = 0; m_uLabelCY[17] = 0; m_uLabelCY[18] = 0; m_uLabelCY[19] = 0; m_uLabelCY[20] = 0; m_uLabelCY[21] = 0; m_uLabelCY[22] = 0; m_uLabelCY[23] = 0; m_uLabelCY[24] = 0; m_uLabelCY[25] = 0; m_uLabelCY[26] = 0; m_uLabelCY[27] = 0; m_uLabelCY[28] = 0; m_uLabelCY[29] = 0; m_uLabelCY[30] = 0; m_uLabelCY[31] = 0; m_uLabelCY[32] = 0; m_uLabelCY[33] = 0; m_uLabelCY[34] = 0; m_uLabelCY[35] = 0; m_uLabelCY[36] = 0; m_uLabelCY[37] = 0; m_uLabelCY[38] = 0; m_uLabelCY[39] = 0; m_uLabelCY[40] = 0; m_uLabelCY[41] = 0; m_uLabelCY[42] = 0; m_uLabelCY[43] = 0; m_uLabelCY[44] = 0; m_uLabelCY[45] = 0; m_uLabelCY[46] = 0; m_uLabelCY[47] = 0; m_uLabelCY[48] = 0; m_uLabelCY[49] = 0; m_uLabelCY[50] = 0; m_uLabelCY[51] = 0; m_uLabelCY[52] = 0; m_uLabelCY[53] = 0; m_uLabelCY[54] = 0; m_uLabelCY[55] = 0; m_uLabelCY[56] = 0; m_uLabelCY[57] = 0; m_uLabelCY[58] = 0; m_uLabelCY[59] = 0; m_uLabelCY[60] = 0; m_uLabelCY[61] = 0; m_uLabelCY[62] = 0; m_uLabelCY[63] = 0; 

	m_pVectorJSProgram[JS_PROG_INDEX(0,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(0,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(0,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(0,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(0,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(0,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(0,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(1,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(1,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(1,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(1,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(1,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(1,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(1,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(2,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(2,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(2,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(2,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(2,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(2,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(2,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(3,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(3,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(3,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(3,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(3,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(3,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(3,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(4,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(4,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(4,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(4,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(4,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(4,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(4,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(5,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(5,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(5,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(5,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(5,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(5,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(5,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(6,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(6,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(6,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(6,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(6,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(6,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(6,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(7,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(7,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(7,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(7,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(7,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(7,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(7,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(8,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(8,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(8,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(8,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(8,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(8,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(8,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(9,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(9,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(9,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(9,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(9,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(9,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(9,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(10,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(10,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(10,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(10,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(10,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(10,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(10,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(11,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(11,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(11,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(11,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(11,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(11,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(11,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(12,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(12,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(12,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(12,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(12,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(12,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(12,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(13,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(13,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(13,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(13,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(13,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(13,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(13,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(14,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(14,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(14,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(14,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(14,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(14,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(14,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(15,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(15,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(15,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(15,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(15,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(15,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(15,6)] = 0.0000;


	m_JS_XCtrl.cControlName = "MIDI JS X";
	m_JS_XCtrl.uControlId = 0;
	m_JS_XCtrl.bMIDIControl = false;
	m_JS_XCtrl.uMIDIControlCommand = 176;
	m_JS_XCtrl.uMIDIControlName = 16;
	m_JS_XCtrl.uMIDIControlChannel = 0;


	m_JS_YCtrl.cControlName = "MIDI JS Y";
	m_JS_YCtrl.uControlId = 0;
	m_JS_YCtrl.bMIDIControl = false;
	m_JS_YCtrl.uMIDIControlCommand = 176;
	m_JS_YCtrl.uMIDIControlName = 17;
	m_JS_YCtrl.uMIDIControlChannel = 0;


	float* pJSProg = NULL;
	m_PresetNames[0] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[0] = pJSProg;

	m_PresetNames[1] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[1] = pJSProg;

	m_PresetNames[2] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[2] = pJSProg;

	m_PresetNames[3] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[3] = pJSProg;

	m_PresetNames[4] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[4] = pJSProg;

	m_PresetNames[5] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[5] = pJSProg;

	m_PresetNames[6] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[6] = pJSProg;

	m_PresetNames[7] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[7] = pJSProg;

	m_PresetNames[8] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[8] = pJSProg;

	m_PresetNames[9] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[9] = pJSProg;

	m_PresetNames[10] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[10] = pJSProg;

	m_PresetNames[11] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[11] = pJSProg;

	m_PresetNames[12] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[12] = pJSProg;

	m_PresetNames[13] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[13] = pJSProg;

	m_PresetNames[14] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[14] = pJSProg;

	m_PresetNames[15] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[15] = pJSProg;


	// **--0xEDA5--**
// ------------------------------------------------------------------------------- //

	return true;

}



