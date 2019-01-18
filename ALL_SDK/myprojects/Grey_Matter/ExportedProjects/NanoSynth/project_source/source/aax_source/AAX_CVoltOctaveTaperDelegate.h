#ifndef	AAX_CVATAPERDELEGATE_H
#define AAX_CVATAPERDELEGATE_H

#include "AAX_ITaperDelegate.h"
#include "AAX_UtilsNative.h"
#include "AAX.h"	//for types

#include <cmath>	//for floor(), log()

template <typename T, int32_t RealPrecision=1000>
class AAX_CVoltOctaveTaperDelegate : public AAX_ITaperDelegate<T>
{
public:
		AAX_CVoltOctaveTaperDelegate(T minValue=0, T maxValue=1);

	//Virtual Overrides
	AAX_CVoltOctaveTaperDelegate<T, RealPrecision>*	Clone() const;
	T		GetMinimumValue()	const						{ return mMinValue; }
	T		GetMaximumValue()	const						{ return mMaxValue; }
	T		ConstrainRealValue(T value)	const;
	T		NormalizedToReal(double normalizedValue) const;
	double	RealToNormalized(T realValue) const;

protected:
	T	Round(double iValue) const;

	// --- v6.6 for log/exp controls
	static inline float fastpow2 (float p)
	{
	  float offset = (p < 0) ? 1.0f : 0.0f;
	  float clipp = (p < -126) ? -126.0f : p;
	  int w = clipp;
	  float z = clipp - w + offset;
	  union { unsigned int i; float f; } v = { static_cast<unsigned int> ( (1 << 23) * (clipp + 121.2740575f + 27.7280233f / (4.84252568f - z) - 1.49012907f * z) ) };

	  return v.f;
	}

	static inline float fastlog2 (float x)
	{
	  union { float f; unsigned int i; } vx = { x };
	  union { unsigned int i; float f; } mx = { (vx.i & 0x007FFFFF) | 0x3f000000 };
	  float y = vx.i;
	  y *= 1.1920928955078125e-7f;

	  return y - 124.22551499f
			   - 1.498030302f * mx.f
			   - 1.72587999f / (0.3520887068f + mx.f);
	}

	// cooked to VA Scaled 0->1 param
	inline double calcVoltOctaveParameter(double fCookedParam) const
	{
		double dOctaves = fastlog2(mMaxValue/mMinValue);
		return fastlog2( fCookedParam/mMinValue )/dOctaves;
	}

	// fPluginValue = 0->1
	// returns VA scaled version 0->1
	inline double calcVoltOctavePluginValue(double fPluginValue) const
	{
		double dOctaves = fastlog2(mMaxValue/mMinValue);
		double fDisplay = mMinValue*fastpow2(fPluginValue*dOctaves);
		double fDiff = mMaxValue - mMinValue;
		return (fDisplay - mMinValue)/fDiff;
	}

private:
	T	mMinValue;
	T	mMaxValue;
};

template <typename T, int32_t RealPrecision>
T	AAX_CVoltOctaveTaperDelegate<T, RealPrecision>::Round(double iValue) const
{
	double precision = RealPrecision;
	if (precision > 0)
		return static_cast<T>(floor(iValue * precision + 0.5) / precision);
    return static_cast<T>(iValue);
}

template <typename T, int32_t RealPrecision>
AAX_CVoltOctaveTaperDelegate<T, RealPrecision>::AAX_CVoltOctaveTaperDelegate(T minValue, T maxValue)  :  AAX_ITaperDelegate<T>(),
	mMinValue(minValue),
	mMaxValue(maxValue)
{

}

template <typename T, int32_t RealPrecision>
AAX_CVoltOctaveTaperDelegate<T, RealPrecision>*		AAX_CVoltOctaveTaperDelegate<T, RealPrecision>::Clone() const
{
	return new AAX_CVoltOctaveTaperDelegate(*this);
}

template <typename T, int32_t RealPrecision>
T		AAX_CVoltOctaveTaperDelegate<T, RealPrecision>::ConstrainRealValue(T value)	const
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
T		AAX_CVoltOctaveTaperDelegate<T, RealPrecision>::NormalizedToReal(double normalizedValue) const
{
	normalizedValue = calcVoltOctavePluginValue(normalizedValue);
	double doubleRealValue = normalizedValue*(mMaxValue - mMinValue) + mMinValue;
	T realValue = (T) doubleRealValue;

	return ConstrainRealValue(realValue);
}

template <typename T, int32_t RealPrecision>
double	AAX_CVoltOctaveTaperDelegate<T, RealPrecision>::RealToNormalized(T realValue) const
{
	return calcVoltOctaveParameter(realValue);
}

#endif // AAX_CLOGTAPERDELEGATE_H
