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

class PeakParameter : public Parameter
{
public:
	PeakParameter (int32 flags, int32 id, const TChar* title);

	virtual void toString (ParamValue normValue, String128 string) const;
	virtual bool fromString (const TChar* string, ParamValue& normValue) const;
};
}}}