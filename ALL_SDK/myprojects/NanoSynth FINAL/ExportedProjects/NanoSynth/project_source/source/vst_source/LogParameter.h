#pragma once
#include "public.sdk/source/vst/vstparameters.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "public.sdk/source/vst/vstaudioeffect.h"
#include "vstgui/plugin-bindings/vst3editor.h"
#include "pluginterfaces/base/ustring.h"

namespace Steinberg {
namespace Vst {
namespace Sock2VST3 {

class LogParameter : public Parameter
{
public:
	LogParameter(const TChar* title, ParamID tag, const TChar* units = 0,
					ParamValue minPlain = 0., ParamValue maxPlain = 1., ParamValue defaultValuePlain = 0.,
					int32 stepCount = 0, int32 flags = ParameterInfo::kCanAutomate, UnitID unitID = kRootUnitId);

	virtual void toString (ParamValue normValue, String128 string) const;
	virtual bool fromString (const TChar* string, ParamValue& normValue) const;
	virtual ParamValue toPlain(ParamValue _valueNormalized) const;	
	virtual ParamValue toNormalized(ParamValue plainValue) const;
	virtual ParamValue getMin () const {return minPlain;}
	virtual void setMin (ParamValue value) {minPlain = value;}
	virtual ParamValue getMax () const {return maxPlain;}
	virtual void setMax (ParamValue value) {maxPlain = value;}

protected:
	ParamValue minPlain;
	ParamValue maxPlain;

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
};
}}}