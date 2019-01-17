#ifndef	AAX_CRAFXLOGTAPERDELEGATE_H
#define AAX_CRAFXLOGTAPERDELEGATE_H

#include "AAX_ITaperDelegate.h"
#include "AAX_UtilsNative.h"
#include "AAX.h"	//for types

#include <cmath>	//for floor(), log()

template <typename T, int32_t RealPrecision=1000>
class AAX_CRafxLogTaperDelegate : public AAX_ITaperDelegate<T>
{
public:
	AAX_CRafxLogTaperDelegate(T minValue=0, T maxValue=1);

	//Virtual Overrides
	AAX_CRafxLogTaperDelegate<T, RealPrecision>*	Clone() const;
	T		GetMinimumValue()	const						{ return mMinValue; }
	T		GetMaximumValue()	const						{ return mMaxValue; }
	T		ConstrainRealValue(T value)	const;
	T		NormalizedToReal(double normalizedValue) const;
	double	RealToNormalized(T realValue) const;

protected:
	T	Round(double iValue) const;

	// fNormalizedParam = 0->1
	// returns anti-log scaled 0->1 value
	inline float calcLogParameter(float fNormalizedParam) const
	{
		// --- MMA Convex Transform Inverse
		return pow(10.0, (fNormalizedParam - 1) / (5.0 / 12.0));
	}

	// fPluginValue = 0->1 log scaled value
	// returns normal 0->1 value
	inline float calcLogPluginValue(float fPluginValue) const
	{
		// --- MMA Convex Transform
		if (fPluginValue <= 0) return 0.0;
		return 1.0 + (5.0 / 12.0)*log10(fPluginValue);
	}

private:
	T	mMinValue;
	T	mMaxValue;
};

template <typename T, int32_t RealPrecision>
T	AAX_CRafxLogTaperDelegate<T, RealPrecision>::Round(double iValue) const
{
	double precision = RealPrecision;
	if (precision > 0)
		return static_cast<T>(floor(iValue * precision + 0.5) / precision);
    return static_cast<T>(iValue);
}

template <typename T, int32_t RealPrecision>
AAX_CRafxLogTaperDelegate<T, RealPrecision>::AAX_CRafxLogTaperDelegate(T minValue, T maxValue)  :  AAX_ITaperDelegate<T>(),
	mMinValue(minValue),
	mMaxValue(maxValue)
{

}

template <typename T, int32_t RealPrecision>
AAX_CRafxLogTaperDelegate<T, RealPrecision>*		AAX_CRafxLogTaperDelegate<T, RealPrecision>::Clone() const
{
	return new AAX_CRafxLogTaperDelegate(*this);
}

template <typename T, int32_t RealPrecision>
T		AAX_CRafxLogTaperDelegate<T, RealPrecision>::ConstrainRealValue(T value)	const
{
	if (RealPrecision)
		value = Round(value);		//reduce the precision to get proper rounding behavior with integers.

	if (value > mMaxValue)
		return mMaxValue;
	if (value < mMinValue)
		return mMinValue;
	return value;
}

template <typename T, int32_t RealPrecision>
T		AAX_CRafxLogTaperDelegate<T, RealPrecision>::NormalizedToReal(double normalizedValue) const
{
	normalizedValue = calcLogPluginValue(normalizedValue);
	double doubleRealValue = normalizedValue*(mMaxValue - mMinValue) + mMinValue;

	T realValue = (T) doubleRealValue;

	return ConstrainRealValue(realValue);
}

template <typename T, int32_t RealPrecision>
double	AAX_CRafxLogTaperDelegate<T, RealPrecision>::RealToNormalized(T realValue) const
{
	double normValue = (realValue - mMinValue)/(mMaxValue - mMinValue);
	return calcLogParameter(normValue);
}

#endif // AAX_CLOGTAPERDELEGATE_H
