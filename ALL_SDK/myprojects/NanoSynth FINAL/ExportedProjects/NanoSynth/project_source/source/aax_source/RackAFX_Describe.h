
#include "AAX.h"

extern AAX_CEffectID kRackAFX_EffectID;

class AAX_ICollection;
class AAX_IEffectDescriptor;
class AAX_IComponentDescriptor;

// --- description functions
void DescribeAlgComponent(AAX_EStemFormat inStemFormat, AAX_IComponentDescriptor &	outDesc);
AAX_Result GetRackAFXPlugInDescription(AAX_IEffectDescriptor & outDescriptor);
AAX_Result GetEffectDescriptions(AAX_ICollection * outDescriptions);
