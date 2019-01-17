#include "VuMeterWP.h"

namespace VSTGUI {

CVuMeterWP::CVuMeterWP(const CRect& size, CBitmap* onBitmap, CBitmap* offBitmap, int32_t nbLed, bool bInverted, bool bAnalogVU, int32_t style)
: CVuMeter(size, onBitmap, offBitmap, nbLed, style)
{
	m_bInverted = bInverted;
	m_bAnalogVU = bAnalogVU;
	subPixMaps = 80;
	heightOfOneImage = 65;
	m_dZero_dB_Frame = 52;
}

CVuMeterWP::~CVuMeterWP(void)
{
}

void CVuMeterWP::setViewSize(const CRect& newSize, bool invalid)
{
	CControl::setViewSize (newSize, invalid);

	rectOn  = getViewSize();
	rectOff = getViewSize();
}

void CVuMeterWP::draw(CDrawContext *_pContext)
{
	if (!getOnBitmap())
		return;

	if(!m_bAnalogVU)
	{
		CRect _rectOn (rectOn);
		CRect _rectOff (rectOff);
		CPoint pointOn;
		CPoint pointOff;
		CDrawContext *pContext = _pContext;

		bounceValue();
        float newValue = 0.f;

        // --- for LED's as on/off single LEDs
        if (nbLed == 2)
        {
            if (value < 0.5f)
                newValue = 0.f;
            else
                newValue = 1.f;
        }
        else
        {
            newValue = getOldValue() - decreaseValue;
            if (newValue < value)
                newValue = value;
            setOldValue(newValue);

            // --- apply detector *after* storing value
            newValue = m_Detector.detect(newValue);
        }

        if (style & kHorizontal)
		{
			if(!m_bInverted)
			{
				CCoord tmp = (CCoord)(((int32_t)(nbLed * (getMin () + newValue) + 0.5f) / (float)nbLed) * getOnBitmap()->getWidth ());
				// http://ehc.ac/p/vstgui/mailman/vstgui-devel/?page=43
				// [Vstgui-devel] CVuMeter display bug on Windows
				tmp = (CCoord)((long)(tmp + 0.5f));
				if(tmp < 1.0) tmp = 0.0;

				pointOff(tmp, 0);
				_rectOff.left += tmp;
				_rectOn.right = tmp + rectOn.left;
			}
			else
			{
				CCoord tmp = (CCoord)(((int32_t)(nbLed * (getMax () - newValue) + 0.5f) / (float)nbLed) * getOnBitmap()->getWidth ());
				// http://ehc.ac/p/vstgui/mailman/vstgui-devel/?page=43
				// [Vstgui-devel] CVuMeter display bug on Windows
				tmp = (CCoord)((long)(tmp + 0.5f));
				if(tmp < 1.0) tmp = 0.0;

				pointOn(tmp, 0);
				_rectOn.left += tmp;
				_rectOff.right = tmp + _rectOff.left;
			}
		}
		else
		{
			if(!m_bInverted)
			{
				CCoord tmp = (CCoord)(((int32_t)(nbLed * (getMax () - newValue) + 0.5f) / (float)nbLed) * getOnBitmap()->getHeight ());
				// http://ehc.ac/p/vstgui/mailman/vstgui-devel/?page=43
				// [Vstgui-devel] CVuMeter display bug on Windows
				tmp = (CCoord)((long)(tmp + 0.5f));
				if(tmp < 1.0) tmp = 0.0;

				pointOn(0, tmp);
				_rectOff.bottom = tmp + rectOff.top;
				_rectOn.top += tmp;
			}
			else
			{
				CCoord tmp = (CCoord)(((int32_t)(nbLed * (newValue) + 0.5f) / (float)nbLed) * getOnBitmap()->getHeight ());
				// http://ehc.ac/p/vstgui/mailman/vstgui-devel/?page=43
				// [Vstgui-devel] CVuMeter display bug on Windows
				tmp = (CCoord)((long)(tmp + 0.5f));
				if(tmp < 1.0) tmp = 0.0;

				pointOff (0, tmp);
				_rectOn.bottom = tmp + _rectOn.top;
				_rectOff.top     += tmp;
			}
		}
		if(getOffBitmap())
			getOffBitmap()->draw (pContext, _rectOff, pointOff);

		getOnBitmap()->draw(pContext, _rectOn, pointOn);
	}
	else
	{
		if(getDrawBackground())
		{
            bounceValue();

            float newValue = getOldValue() - decreaseValue;
            if (newValue < value)
                newValue = value;
            setOldValue(newValue);

            // --- apply detector *after* storing value
            newValue = m_Detector.detect(newValue);


			CPoint where (0, 0);
			if (value >= 0.f && heightOfOneImage > 0.)
			{
				CCoord tmp = heightOfOneImage * (subPixMaps - 1);
				if(m_bInverted)
				{
					double dTop = m_dZero_dB_Frame/subPixMaps;
					where.y = floor ((dTop - newValue*dTop) * tmp);
				}
				else
					where.y = floor (newValue * tmp);
				where.y -= (int32_t)where.y % (int32_t)heightOfOneImage;
			}

			getDrawBackground()->draw (_pContext, getViewSize (), where);
		}
	}

	setDirty (false);
}


}
