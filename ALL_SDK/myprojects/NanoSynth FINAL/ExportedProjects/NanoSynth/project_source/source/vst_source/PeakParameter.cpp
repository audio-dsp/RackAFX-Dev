#include "PeakParameter.h"

namespace Steinberg {
namespace Vst {
namespace Sock2VST3 {
//------------------------------------------------------------------------
PeakParameter::PeakParameter (int32 flags, int32 id, const TChar* title)
{
	UString (info.title, USTRINGSIZE (info.title)).assign (title);
	
	info.flags = flags;
	info.id = id;
	info.stepCount = 0;
	info.defaultNormalizedValue = 0.5f;
	info.unitId = kRootUnitId;
	
	setNormalized (1.f);
}

//------------------------------------------------------------------------
void PeakParameter::toString (ParamValue normValue, String128 string) const
{
	String str;
	if (normValue > 0.0001)
	{
		str.printf ("%.3f", 20 * log10f ((float)normValue));
	}
	else
	{
		str.assign ("-");
		str.append (kInfiniteSymbol);
	}
	str.toWideString (kCP_Utf8);
	str.copyTo16 (string, 0, 128);
}

//------------------------------------------------------------------------
bool PeakParameter::fromString (const TChar* string, ParamValue& normValue) const
{
	return false;
}

}}}