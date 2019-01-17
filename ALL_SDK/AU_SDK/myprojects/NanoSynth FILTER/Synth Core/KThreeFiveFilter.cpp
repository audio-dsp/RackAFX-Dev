#include "KThreeFiveFilter.h"

CKThreeFiveFilter::CKThreeFiveFilter(void)
{
	// --- init
	m_dK = 0.01;
	m_dAlpha0 = 0;

	// --- set filter types
	m_LPF1.m_uFilterType = LPF1;
	m_LPF2.m_uFilterType = LPF1;
	m_HPF1.m_uFilterType = HPF1;
	m_HPF2.m_uFilterType = HPF1;

	// --- default filter type
	m_uFilterType = LPF2;

	// --- flush everything
	reset();
}

CKThreeFiveFilter::~CKThreeFiveFilter(void)
{
}

void CKThreeFiveFilter::reset()
{
	// --- flush everything
	m_LPF1.reset();
	m_LPF2.reset();
	m_HPF1.reset();
	m_HPF2.reset();
}

// --- decode the Q value; Q on UI is 1->10
void CKThreeFiveFilter::setQControl(double dQControl)
{
	// this maps dQControl = 1->10 to K = 0.01 -> 2
	m_dK = (2.0 - 0.01)*(dQControl - 1.0)/(10.0 - 1.0) + 0.01;
}

void CKThreeFiveFilter::update()
{	
	// --- do any modulation first
	CFilter::update();

	// prewarp for BZT
	double wd = 2*pi*m_dFc;          
	double T  = 1/m_dSampleRate;             
	double wa = (2/T)*tan(wd*T/2); 
	double g  = wa*T/2;    

	// G - the feedforward coeff in the VA One Pole
	//     same for LPF, HPF
	double G = g/(1.0 + g);

	// set alphas; same for LPF, HPF
	m_LPF1.m_dAlpha = G;
	m_LPF2.m_dAlpha = G;
	m_HPF1.m_dAlpha = G;
	m_HPF2.m_dAlpha = G;
	
	// set m_dAlpha0 variable; same for LPF, HPF
	m_dAlpha0 = 1.0/(1.0 - m_dK*G + m_dK*G*G);

	if(m_uFilterType == LPF2)
	{
		m_LPF2.m_dBeta = (m_dK - m_dK*G)/(1.0 + g);
		m_HPF1.m_dBeta = -1.0/(1.0 + g);
	}
	else // HPF
	{
		m_HPF2.m_dBeta = -1.0*G/(1.0 + g);
		m_LPF1.m_dBeta = 1.0/(1.0 + g);
	}
}

double CKThreeFiveFilter::doFilter(double xn)
{
	// return xn if filter not supported
	if(m_uFilterType != LPF2 && m_uFilterType != HPF2)
		return xn;

	double y = 0.0;

	// two filters to implement
	if(m_uFilterType == LPF2)
	{
		// process input through LPF1
		double y1 = m_LPF1.doFilter(xn);

		// form S35
		double S35 = m_HPF1.getFeedbackOutput() + m_LPF2.getFeedbackOutput(); 
		
		// calculate u
		double u = m_dAlpha0*(y1 + S35);
		
		// NAIVE NLP
		if(m_uNLP == ON)
		{
			// Regular Version
			u = fasttanh(m_dSaturation*u);
		}

		// feed it to LPF2
		y = m_dK*m_LPF2.doFilter(u);
			
		// feed y to HPF
		m_HPF1.doFilter(y);
	}
	else // HPF
	{
		// process input through HPF1
		double y1 = m_HPF1.doFilter(xn);

		// then: form feedback and feed forward values (read before write)
		double S35 = m_HPF2.getFeedbackOutput() + m_LPF1.getFeedbackOutput();

		// calculate u
		double u = m_dAlpha0*y1 + S35;

		// form output
		y = m_dK*u;

		// NAIVE NLP
		if(m_uNLP == ON)
			y = fasttanh(m_dSaturation*y);

		// process y through feedback BPF
		m_LPF1.doFilter(m_HPF2.doFilter(y));
	}

	// auto-normalize
	if(m_dK > 0)
		y *= 1/m_dK;

	return y;
}
