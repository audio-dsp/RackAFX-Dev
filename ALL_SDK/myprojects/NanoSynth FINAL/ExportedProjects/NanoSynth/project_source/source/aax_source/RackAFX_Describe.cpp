#include "RackAFX_Describe.h"
#include "RackAFX_Parameters.h"
#include "RackAFX_GUI.h"

#include "AAX_ICollection.h"
#include "AAX_IComponentDescriptor.h"
#include "AAX_IEffectDescriptor.h"

#include "AAX_IPropertyMap.h"
#include "AAX_Errors.h"
#include "AAX_Assert.h"

// --- set by RAFX
AAX_CEffectID kRackAFX_EffectID = "rackafx.NanoSynth";

// --- component description
void DescribeAlgComponent(AAX_EStemFormat inStemFormat, AAX_IComponentDescriptor& outDesc)
{
    AAX_Result err = AAX_SUCCESS;
    AAX_ASSERT(inStemFormat == AAX_eStemFormat_Mono || inStemFormat == AAX_eStemFormat_Stereo);

    // --- setup properties
    AAX_IPropertyMap* properties = outDesc.NewPropertyMap();
    AAX_ASSERT (properties);
    if ( !properties ) return;

    properties->AddProperty(AAX_eProperty_ManufacturerID, cRAFX_ManufactureID);
    properties->AddProperty(AAX_eProperty_ProductID, cRAFX_ProductID);
    properties->AddProperty(AAX_eProperty_InputStemFormat, inStemFormat);
    properties->AddProperty(AAX_eProperty_OutputStemFormat, inStemFormat);
    properties->AddProperty(AAX_eProperty_CanBypass, true);

#ifndef USE_CUSTOM_GUI
    properties->AddProperty(AAX_eProperty_UsesClientGUI, true); 	// Register for auto-GUI
#endif

    switch( inStemFormat )
    {
        case AAX_eStemFormat_Mono:
        {
            err = properties->AddProperty ( AAX_eProperty_PlugInID_Native, cRAFX_TypeID_MonoNative );
            err = outDesc.AddProcessProc_Native(RackAFX_Parameters::StaticRenderAudio, properties);
            err = RackAFX_Parameters::StaticDescribe(outDesc);
            AAX_ASSERT(err == AAX_SUCCESS);
        } break;

        case AAX_eStemFormat_Stereo:
        {
            err = properties->AddProperty ( AAX_eProperty_PlugInID_Native, cRAFX_TypeID_StereoNative );
            err = outDesc.AddProcessProc_Native(RackAFX_Parameters::StaticRenderAudio, properties);
            err = RackAFX_Parameters::StaticDescribe(outDesc);
            AAX_ASSERT (err == AAX_SUCCESS);
        } break;

        default:
            break;
    }
}

// --- plugin description
AAX_Result GetRackAFXPlugInDescription(AAX_IEffectDescriptor&	outDescriptor)
{
	int	err = AAX_SUCCESS;
	AAX_IComponentDescriptor* compDesc = outDescriptor.NewComponentDescriptor ();
	if (!compDesc)
		return AAX_ERROR_NULL_OBJECT;

	// --- Effect identifiers
	outDescriptor.AddName("NanoSynth");
	outDescriptor.AddName("NanoSynth"); // short version is placed in mixer channel's insert box

	// --- setup category
#ifdef IS_SYNTH
	outDescriptor.AddCategory(AAX_ePlugInCategory_SWGenerators);
#else
	outDescriptor.AddCategory(RAFX_AAX_Category);
#endif

	// --- Mono algorithm component
	compDesc->Clear ();
	DescribeAlgComponent ( AAX_eStemFormat_Mono, *compDesc );
	err = outDescriptor.AddComponent ( compDesc );
	AAX_ASSERT (err == AAX_SUCCESS);

	// --- Stereo algorithm component
	compDesc->Clear ();
	DescribeAlgComponent ( AAX_eStemFormat_Stereo, *compDesc );
	err = outDescriptor.AddComponent ( compDesc );
	AAX_ASSERT (err == AAX_SUCCESS);

	// --- Data model
	err = outDescriptor.AddProcPtr( (void *) RackAFX_Parameters::Create, kAAX_ProcPtrID_Create_EffectParameters );
	AAX_ASSERT (err == AAX_SUCCESS);

	// outDescriptor.AddResourceInfo ( AAX_eResourceType_PageTable, "DemoDist.xml" );
#ifdef USE_CUSTOM_GUI
		outDescriptor.AddProcPtr((void*)RackAFX_GUI::Create, kAAX_ProcPtrID_Create_EffectGUI);
		AAX_ASSERT (err == AAX_SUCCESS);
#endif

    // --- meter display properties // JAN
    AAX_IPropertyMap* meterProperties = outDescriptor.NewPropertyMap();
    AAX_ASSERT (meterProperties);
    //
    meterProperties->AddProperty ( AAX_eProperty_Meter_Type, AAX_eMeterType_CLGain );
    meterProperties->AddProperty ( AAX_eProperty_Meter_Orientation, AAX_eMeterOrientation_Default );
    outDescriptor.AddMeterDescription( cRAFX_MeterID[0], "GRLeft", meterProperties );

    meterProperties = outDescriptor.NewPropertyMap();
    AAX_ASSERT (meterProperties);
    //
    meterProperties->AddProperty ( AAX_eProperty_Meter_Type, AAX_eMeterType_CLGain );
    meterProperties->AddProperty ( AAX_eProperty_Meter_Orientation, AAX_eMeterOrientation_Default );
    outDescriptor.AddMeterDescription( cRAFX_MeterID[1], "GRRight", meterProperties );

	return AAX_SUCCESS;
}

// --- effect description
AAX_Result GetEffectDescriptions ( AAX_ICollection * outCollection )
{
	AAX_Result result = AAX_SUCCESS;

	AAX_IEffectDescriptor* plugInDescriptor = outCollection->NewDescriptor();
	if ( plugInDescriptor )
	{
		result = GetRackAFXPlugInDescription(*plugInDescriptor);
		if ( result == AAX_SUCCESS )
			outCollection->AddEffect ( kRackAFX_EffectID, plugInDescriptor );
	}
	else
	{
		result = AAX_ERROR_NULL_OBJECT;
	}

	outCollection->SetManufacturerName("Jonathan Moore");
	outCollection->AddPackageName("RAFX AAX Plug-In");
	outCollection->AddPackageName("RAFX Plug-In");
	outCollection->SetPackageVersion(1);

	return result;
}


