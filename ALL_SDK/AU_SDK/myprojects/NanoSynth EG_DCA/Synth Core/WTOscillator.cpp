#include "WTOscillator.h"

CWTOscillator::CWTOscillator(void)
{
	// --- clear out arrays
	memset(m_pSawTables, 0, NUM_TABLES*sizeof(double*));
	memset(m_pTriangleTables, 0, NUM_TABLES*sizeof(double*));

	// --- init variables
	m_dReadIndex = 0.0;
	m_dWT_inc = 0.0;
	m_nCurrentTableIndex = 0;

	// --- setup correction factors (empirical)
	m_dSquareCorrFactor[0] = 0.5;
	m_dSquareCorrFactor[1] = 0.5;
	m_dSquareCorrFactor[2] = 0.5;
	m_dSquareCorrFactor[3] = 0.49;
	m_dSquareCorrFactor[4] = 0.48;
	m_dSquareCorrFactor[5] = 0.468;
	m_dSquareCorrFactor[6] = 0.43;
	m_dSquareCorrFactor[7] = 0.34;
	m_dSquareCorrFactor[8] = 0.25;

	// --- default to SINE
	m_pCurrentTable = &m_dSineTable[0];
}

CWTOscillator::~CWTOscillator(void)
{
	destroyWaveTables();
}

void CWTOscillator::reset()
{
	COscillator::reset();

	// --- back to top of buffer
	m_dReadIndex = 0.0;
}

void CWTOscillator::startOscillator()
{
	reset();
	m_bNoteOn = true;
}

void CWTOscillator::stopOscillator()
{
	m_bNoteOn = false;
}

void CWTOscillator::update()
{
	COscillator::update();

	// --- calculate the inc value
	m_dWT_inc = WT_LENGTH*m_dInc;

	// --- select the table
	selectTable();
}

void CWTOscillator::setSampleRate(double dFs)
{
	bool bNewSR = m_dSampleRate != dFs ? true : false;
	
	// --- base class first
	COscillator::setSampleRate(dFs);

	// --- recrate the tables only if sample rate has changed
	if(bNewSR)
	{
		// --- then recrate
		destroyWaveTables();
		createWaveTables();
	}
}


void CWTOscillator::createWaveTables()
{
	// create the tables
	//
	// SINE: only need one table
	for(int i = 0; i < WT_LENGTH; i++)
	{
		// sample the sinusoid, WT_LENGTH points
		// sin(wnT) = sin(2pi*i/WT_LENGTH)
		m_dSineTable[i] = sin(((double)i/WT_LENGTH)*(2*pi));
	}

	// SAW, TRIANGLE: need 10 tables
	double dSeedFreq = 27.5; // Note A0, bottom of piano
	for(int j = 0; j < NUM_TABLES; j++)
	{
		double* pSawTable = new double[WT_LENGTH];
		memset(pSawTable, 0, WT_LENGTH*sizeof(double));

		double* pTriTable = new double[WT_LENGTH];
		memset(pTriTable, 0, WT_LENGTH*sizeof(double));

		int nHarms = (int)((m_dSampleRate/2.0/dSeedFreq) - 1.0);
		int nHalfHarms = (int)((float)nHarms/2.0);

		double dMaxSaw = 0;
		double dMaxTri = 0;

		for(int i = 0; i < WT_LENGTH; i++)
		{
			// sawtooth: += (-1)^g+1(1/g)sin(wnT)
			for(int g = 1; g <= nHarms; g++)
			{
				// Lanczos Sigma Factor
				double x = g*pi/nHarms;
				double sigma = sin(x)/x;

				// only apply to partials above fundamental
				if(g == 1)
					sigma = 1.0;

				double n = double(g);
				pSawTable[i] += pow((float)-1.0,(float)(g+1))*(1.0/n)*sigma*sin(2.0*pi*i*n/WT_LENGTH);
			}

			// triangle: += (-1)^g(1/(2g+1+^2)sin(w(2n+1)T)
			// NOTE: the limit is nHalfHarms here because of the way the sum is constructed
			// (look at the (2n+1) components
			for(int g = 0; g <= nHalfHarms; g++)
			{
				double n = double(g);
				pTriTable[i] += pow((float)-1.0, (float)n)*(1.0/pow((float)(2*n + 1),(float)2.0))*sin(2.0*pi*(2.0*n + 1)*i/WT_LENGTH);
			}

			// store the max values
			if(i == 0)
			{
				dMaxSaw = pSawTable[i];
				dMaxTri = pTriTable[i];
			}
			else
			{
				// test and store
				if(pSawTable[i] > dMaxSaw)
					dMaxSaw = pSawTable[i];

				if(pTriTable[i] > dMaxTri)
					dMaxTri = pTriTable[i];
			}
		}
		// normalize
		for(int i = 0; i < WT_LENGTH; i++)
		{
			// normalize it
			pSawTable[i] /= dMaxSaw;
			pTriTable[i] /= dMaxTri;
		}

		// store
		m_pSawTables[j] = pSawTable;
		m_pTriangleTables[j] = pTriTable;

		dSeedFreq *= 2.0;
	}
}

void CWTOscillator::destroyWaveTables()
{
	for(int i = 0; i < NUM_TABLES; i++)
	{
		double* p = m_pSawTables[i];
		if(p)
		{
			delete [] p;
			m_pSawTables[i] = 0;
		}

		p = m_pTriangleTables[i];
		if(p)
		{
			delete [] p;
			m_pTriangleTables[i] = 0;
		}
	}
}

// get table index based on current m_dFo
int CWTOscillator::getTableIndex()
{
	if(m_uWaveform == SINE)
		return -1;

	double dSeedFreq = 27.5; // Note A0, bottom of piano
	for(int j = 0; j < NUM_TABLES; j++)
	{
		if(m_dFo <= dSeedFreq)
		{
			return j;
		}

		dSeedFreq *= 2.0;
	}

	return -1;
}

void CWTOscillator::selectTable()
{
	m_nCurrentTableIndex = getTableIndex();

	// if the frequency is high enough, the sine table will be returned
	// even for non-sinusoidal waves; anything about 10548 Hz is one
	// harmonic only (sine)
	if(m_nCurrentTableIndex < 0)
	{
		m_pCurrentTable = &m_dSineTable[0];
		return;
	}

	// choose table
	if(m_uWaveform == SAW1 || m_uWaveform == SAW2 || m_uWaveform == SAW3 || m_uWaveform == SQUARE)
		m_pCurrentTable = m_pSawTables[m_nCurrentTableIndex];
	else if(m_uWaveform == TRI)
		m_pCurrentTable = m_pTriangleTables[m_nCurrentTableIndex];
}


double CWTOscillator::doWaveTable(double& dReadIndex, double dWT_inc)
{
	double dOut = 0;

	// apply phase modulation, if any
	double dModReadIndex = dReadIndex + m_dPhaseMod*WT_LENGTH;

	// check for multi-wrapping on new read index
	checkWrapIndex(dModReadIndex);

	// get INT part
	int nReadIndex = abs((int)dModReadIndex);

	// get FRAC part
	float fFrac = dModReadIndex - nReadIndex;

	// setup second index for interpolation; wrap the buffer if needed
	int nReadIndexNext = nReadIndex + 1 > WT_LENGTH-1 ? 0 :  nReadIndex + 1;

	// interpolate the output
	dOut = dLinTerp(0, 1, m_pCurrentTable[nReadIndex], m_pCurrentTable[nReadIndexNext], fFrac);

	// add the increment for next time
	dReadIndex += dWT_inc;

	// check for wrap
	checkWrapIndex(dReadIndex);

	return dOut;
}

double CWTOscillator::doSquareWave()
{
	double dPW = m_dPulseWidth/100.0;
	double dPWIndex = m_dReadIndex + dPW*WT_LENGTH;

	// --- render first sawtooth using dReadIndex
	double dSaw1 = doWaveTable(m_dReadIndex, m_dWT_inc);

	// --- find the phase shifted output
	if(m_dWT_inc >= 0)
	{
		if(dPWIndex >= WT_LENGTH)
			dPWIndex = dPWIndex - WT_LENGTH;
	}
	else
	{
		if(dPWIndex < 0)
			dPWIndex = WT_LENGTH + dPWIndex;
	}

	// --- render second sawtooth using dPWIndex (shifted)
	double dSaw2 = doWaveTable(dPWIndex, m_dWT_inc);

	// --- find the correction factor from the table
	double dSqAmp = m_dSquareCorrFactor[m_nCurrentTableIndex];

	// --- then subtract
	double dOut = dSqAmp*dSaw1 -  dSqAmp*dSaw2;

	// --- calculate the DC correction factor
	double dCorr = 1.0/dPW;
	if(dPW < 0.5)
		dCorr = 1.0/(1.0-dPW);

	// --- apply correction
	dOut *= dCorr;

	return dOut;
}

double CWTOscillator::doOscillate(double* pAuxOutput)
{
	if(!m_bNoteOn)
	{
		if(pAuxOutput)
			*pAuxOutput = 0.0;

		return 0.0;
	}

	// if square, it has its own routine
	if(m_uWaveform == SQUARE && m_nCurrentTableIndex >= 0)
	{
		double dOut = doSquareWave();
		if(pAuxOutput)
			*pAuxOutput = dOut;

		return dOut;
	}

	// --- get output
	double dOutSample = doWaveTable(m_dReadIndex, m_dWT_inc);

	// mono oscillator
	if(pAuxOutput)
		*pAuxOutput = dOutSample*m_dAmplitude*m_dAmpMod;

	return dOutSample*m_dAmplitude*m_dAmpMod;
}