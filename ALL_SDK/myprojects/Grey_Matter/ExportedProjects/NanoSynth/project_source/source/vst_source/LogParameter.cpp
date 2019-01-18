#include "LogParameter.h"

namespace Steinberg {
namespace Vst {
namespace Sock2VST3 {
//------------------------------------------------------------------------
LogParameter::LogParameter(const TChar* title, ParamID tag, const TChar* units,
								ParamValue minPlain, ParamValue maxPlain, ParamValue defaultValuePlain,
								int32 stepCount, int32 flags, UnitID unitID)
: minPlain (minPlain)
, maxPlain (maxPlain)
{
	UString (info.title, tStrBufferSize (String128)).assign (title);

	UString uUnits (info.units, tStrBufferSize (String128));
	if (units)
	{
		uUnits.assign (units);
	}

	info.stepCount = stepCount;
	info.defaultNormalizedValue = valueNormalized = toNormalized(defaultValuePlain);
	info.flags = flags;
	info.id = tag;
	info.unitId = unitID;
}

//------------------------------------------------------------------------
void LogParameter::toString(ParamValue normValue, String128 string) const
{
	UString128 wrapper;
	wrapper.printFloat(toPlain(normValue), precision);
	wrapper.copyTo(string, 128);
}

//------------------------------------------------------------------------
bool LogParameter::fromString (const TChar* string, ParamValue& normValue) const
{
	UString wrapper ((TChar*)string, strlen16 (string));
	if (wrapper.scanFloat (normValue))
	{
		normValue = toNormalized(normValue);
		return true;
	}
	return false;
}

// --- convert 0->1 to cooked value
ParamValue LogParameter::toPlain(ParamValue _valueNormalized) const	
{
	_valueNormalized = calcLogPluginValue(_valueNormalized);
	return _valueNormalized*(getMax() - getMin()) + getMin();
}

// --- convert cooked value to 0->1 value
ParamValue LogParameter::toNormalized(ParamValue plainValue) const
{
	ParamValue normValue = (plainValue - getMin())/(getMax() - getMin());
	return calcLogParameter(normValue);
}

}}}