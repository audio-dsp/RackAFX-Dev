# ---------------------------------------------------------------------------------
#
# --- AU Project CMakeLists.txt
# --- Created by RackAFX(TM)
# --- www.willpirkle.com
#
# ---------------------------------------------------------------------------------
set(SDK_ROOT ${AU_SDK_ROOT})

set(MAC_ROOT "../../mac/au")
set(SOURCE_ROOT "../../source")
set(RESOURCE_ROOT "../../resources")

# --- local roots
set(RAFX_SOURCE_ROOT "${SOURCE_ROOT}/rafx_source")
set(VSTGUI_SOURCE_ROOT "${SOURCE_ROOT}/vstgui_source")
set(AU_SOURCE_ROOT "${SOURCE_ROOT}/au_source")
set(COCOA_SOURCE_ROOT "${SOURCE_ROOT}/au_source/cocoa_ui")

# ---------------------------------------------------------------------------------
#
# ---  RAFX Ported plugin files
#
# ---------------------------------------------------------------------------------
set(rafx_au_sources
	${RAFX_SOURCE_ROOT}/NanoSynth.h
	${RAFX_SOURCE_ROOT}/plugin.h
	${RAFX_SOURCE_ROOT}/pluginconstants.h
	${RAFX_SOURCE_ROOT}/resource.h
	${RAFX_SOURCE_ROOT}/trace.h
	${RAFX_SOURCE_ROOT}/DCA.h
	${RAFX_SOURCE_ROOT}/EnvelopeGenerator.h
	${RAFX_SOURCE_ROOT}/Filter.h
	${RAFX_SOURCE_ROOT}/LFO.h
	${RAFX_SOURCE_ROOT}/ModulationMatrix.h
	${RAFX_SOURCE_ROOT}/MoogLadderFilter.h
	${RAFX_SOURCE_ROOT}/Oscillator.h
	${RAFX_SOURCE_ROOT}/QBLimitedOscillator.h
	${RAFX_SOURCE_ROOT}/synthfunctions.h
	${RAFX_SOURCE_ROOT}/VAOnePoleFilter.h
	${RAFX_SOURCE_ROOT}/NanoSynth.cpp
	${RAFX_SOURCE_ROOT}/plugin.cpp
	${RAFX_SOURCE_ROOT}/pluginobjects.cpp
	${RAFX_SOURCE_ROOT}/DCA.cpp
	${RAFX_SOURCE_ROOT}/EnvelopeGenerator.cpp
	${RAFX_SOURCE_ROOT}/Filter.cpp
	${RAFX_SOURCE_ROOT}/LFO.cpp
	${RAFX_SOURCE_ROOT}/ModulationMatrix.cpp
	${RAFX_SOURCE_ROOT}/MoogLadderFilter.cpp
	${RAFX_SOURCE_ROOT}/Oscillator.cpp
	${RAFX_SOURCE_ROOT}/QBLimitedOscillator.cpp
	${RAFX_SOURCE_ROOT}/VAOnePoleFilter.cpp 
)

set(au_core_sources
	${AU_SOURCE_ROOT}/FilterVersion.h
	${AU_SOURCE_ROOT}/RackAFX2AUFilter.cpp
	${AU_SOURCE_ROOT}/RackAFX2AUFilter.exp
	${AU_SOURCE_ROOT}/RackAFX2AUFilter.h
)

# ---------------------------------------------------------------------------------
#
# ---  AU SDK files
#
# ---------------------------------------------------------------------------------
set(aubase_sources
	${SDK_ROOT}/AUPublic/AUBase/AUBase.cpp
	${SDK_ROOT}/AUPublic/AUBase/AUBase.h
	${SDK_ROOT}/AUPublic/AUBase/AUDispatch.cpp
	${SDK_ROOT}/AUPublic/AUBase/AUDispatch.h
	${SDK_ROOT}/AUPublic/AUBase/AUInputElement.cpp
	${SDK_ROOT}/AUPublic/AUBase/AUInputElement.h
	${SDK_ROOT}/AUPublic/AUBase/AUOutputElement.cpp
	${SDK_ROOT}/AUPublic/AUBase/AUOutputElement.h
	${SDK_ROOT}/AUPublic/AUBase/AUPlugInDispatch.cpp
	${SDK_ROOT}/AUPublic/AUBase/AUPlugInDispatch.h
	${SDK_ROOT}/AUPublic/AUBase/AUScopeElement.cpp
	${SDK_ROOT}/AUPublic/AUBase/AUScopeElement.h
	${SDK_ROOT}/AUPublic/AUBase/ComponentBase.cpp
	${SDK_ROOT}/AUPublic/AUBase/ComponentBase.h
)

set(auinstbase_sources
	${SDK_ROOT}/AUPublic/AUInstrumentBase/AUInstrumentBase.cpp
	${SDK_ROOT}/AUPublic/AUInstrumentBase/AUInstrumentBase.h
	${SDK_ROOT}/AUPublic/AUInstrumentBase/LockFreeFIFO.h
	${SDK_ROOT}/AUPublic/AUInstrumentBase/MIDIControlHandler.h
	${SDK_ROOT}/AUPublic/AUInstrumentBase/SynthElement.cpp
	${SDK_ROOT}/AUPublic/AUInstrumentBase/SynthElement.h
	${SDK_ROOT}/AUPublic/AUInstrumentBase/SynthEvent.h
	${SDK_ROOT}/AUPublic/AUInstrumentBase/SynthNote.cpp
	${SDK_ROOT}/AUPublic/AUInstrumentBase/SynthNote.h
	${SDK_ROOT}/AUPublic/AUInstrumentBase/SynthNoteList.cpp
	${SDK_ROOT}/AUPublic/AUInstrumentBase/SynthNoteList.h
)

set(auotherbase_sources
	${SDK_ROOT}/AUPublic/OtherBases/AUEffectBase.cpp
	${SDK_ROOT}/AUPublic/OtherBases/AUEffectBase.h
	${SDK_ROOT}/AUPublic/OtherBases/AUMIDIBase.cpp
	${SDK_ROOT}/AUPublic/OtherBases/AUMIDIBase.h
	${SDK_ROOT}/AUPublic/OtherBases/AUMIDIEffectBase.cpp
	${SDK_ROOT}/AUPublic/OtherBases/AUMIDIEffectBase.h
	${SDK_ROOT}/AUPublic/OtherBases/MusicDeviceBase.cpp
	${SDK_ROOT}/AUPublic/OtherBases/MusicDeviceBase.h
)

set(auutility_sources
	${SDK_ROOT}/AUPublic/Utility/AUBaseHelper.cpp
	${SDK_ROOT}/AUPublic/Utility/AUBaseHelper.h
	${SDK_ROOT}/AUPublic/Utility/AUBuffer.cpp
	${SDK_ROOT}/AUPublic/Utility/AUBuffer.h
	${SDK_ROOT}/AUPublic/Utility/AUMIDIDefs.h
	${SDK_ROOT}/AUPublic/Utility/AUSilentTimeout.h
)

set(aupublicutility_sources
	${SDK_ROOT}/PublicUtility/CAAtomic.h
	${SDK_ROOT}/PublicUtility/CAAtomicStack.h
	${SDK_ROOT}/PublicUtility/CAAudioChannelLayout.cpp
	${SDK_ROOT}/PublicUtility/CAAudioChannelLayout.h
	${SDK_ROOT}/PublicUtility/CAAutoDisposer.h
	${SDK_ROOT}/PublicUtility/CABufferList.cpp
	${SDK_ROOT}/PublicUtility/CABufferList.h
	${SDK_ROOT}/PublicUtility/CAByteOrder.h
	${SDK_ROOT}/PublicUtility/CADebugger.cpp
	${SDK_ROOT}/PublicUtility/CADebugger.h
	${SDK_ROOT}/PublicUtility/CADebugMacros.cpp
	${SDK_ROOT}/PublicUtility/CADebugMacros.h
	${SDK_ROOT}/PublicUtility/CADebugPrintf.cpp
	${SDK_ROOT}/PublicUtility/CADebugPrintf.h
	${SDK_ROOT}/PublicUtility/CAException.h
	${SDK_ROOT}/PublicUtility/CAGuard.cpp
	${SDK_ROOT}/PublicUtility/CAGuard.h
	${SDK_ROOT}/PublicUtility/CAHostTimeBase.cpp
	${SDK_ROOT}/PublicUtility/CAHostTimeBase.h
	${SDK_ROOT}/PublicUtility/CALogMacros.h
	${SDK_ROOT}/PublicUtility/CAMath.h
	${SDK_ROOT}/PublicUtility/CAMutex.cpp
	${SDK_ROOT}/PublicUtility/CAMutex.h
	${SDK_ROOT}/PublicUtility/CAReferenceCounted.h
	${SDK_ROOT}/PublicUtility/CAStreamBasicDescription.cpp
	${SDK_ROOT}/PublicUtility/CAStreamBasicDescription.h
	${SDK_ROOT}/PublicUtility/CAThreadSafeList.h
	${SDK_ROOT}/PublicUtility/CAVectorUnit.cpp
	${SDK_ROOT}/PublicUtility/CAVectorUnit.h
	${SDK_ROOT}/PublicUtility/CAVectorUnitTypes.h
	${SDK_ROOT}/PublicUtility/CAXException.cpp
	${SDK_ROOT}/PublicUtility/CAXException.h
)

# ---------------------------------------------------------------------------------
#
# ---  COCOA UI files
#
# ---------------------------------------------------------------------------------
set(cocoa_sources
	${COCOA_SOURCE_ROOT}/NanoSynth284027328_UIView.h
	${COCOA_SOURCE_ROOT}/NanoSynth284027328_UIView.m
	${COCOA_SOURCE_ROOT}/NanoSynth284027328_ViewFactory.h
	${COCOA_SOURCE_ROOT}/NanoSynth284027328_ViewFactory.m
)

# ---------------------------------------------------------------------------------
#
# ---  VSTGUI files
#
# ---------------------------------------------------------------------------------
set(custom_vstgui_sources
	${VSTGUI_SOURCE_ROOT}/KickButtonWP.cpp
	${VSTGUI_SOURCE_ROOT}/KnobWP.cpp
	${VSTGUI_SOURCE_ROOT}/RackAFXMeterDetector.cpp
	${VSTGUI_SOURCE_ROOT}/SliderWP.cpp
	${VSTGUI_SOURCE_ROOT}/VuMeterWP.cpp
	${VSTGUI_SOURCE_ROOT}/XYPadWP.cpp
	${VSTGUI_SOURCE_ROOT}/vstgui4constants.h
	${VSTGUI_SOURCE_ROOT}/KickButtonWP.h
	${VSTGUI_SOURCE_ROOT}/KnobWP.h
	${VSTGUI_SOURCE_ROOT}/RackAFXMeterDetector.h
	${VSTGUI_SOURCE_ROOT}/SliderWP.h
	${VSTGUI_SOURCE_ROOT}/VuMeterWP.h
	${VSTGUI_SOURCE_ROOT}/XYPadWP.h
	${VSTGUI_SOURCE_ROOT}/fplatform.h
	${VSTGUI_SOURCE_ROOT}/ftypes.h
	${VSTGUI_SOURCE_ROOT}/rafx2vstguimessages.h
	${VSTGUI_SOURCE_ROOT}/RackAFXGUIFactory.cpp
	${VSTGUI_SOURCE_ROOT}/RAFXSwitchView.cpp
	${VSTGUI_SOURCE_ROOT}/RAFXSwitchView.h
	${VSTGUI_SOURCE_ROOT}/RafxVSTEditor.cpp
	${VSTGUI_SOURCE_ROOT}/RafxVSTEditor.h
	${VSTGUI_SOURCE_ROOT}/vstguidefaults.h
	${VSTGUI_SOURCE_ROOT}/WPXMLParser.cpp
	${VSTGUI_SOURCE_ROOT}/WPXMLParser.h
	${VSTGUI_SOURCE_ROOT}/pugiconfig.hpp
	${VSTGUI_SOURCE_ROOT}/pugixml.cpp
	${VSTGUI_SOURCE_ROOT}/pugixml.hpp
)

set(vstgui_sources
	${VSTGUI_ROOT}/vstgui4/vstgui/vstgui_mac.mm
	${VSTGUI_ROOT}/vstgui4/vstgui/vstgui_uidescription_mac.mm
)

# ---------------------------------------------------------------------------------
#
# ---  setup IDE folders/filters
#
# ---------------------------------------------------------------------------------
source_group(RackAFXFiles FILES ${rafx_au_sources})
source_group(AUSource FILES ${au_core_sources})
source_group(CustomVSTGUI FILES ${custom_vstgui_sources})
source_group(vstgui4 FILES ${vstgui_sources})
source_group(AUCore\\AUBase FILES ${aubase_sources})
source_group(AUCore\\AUInstrumentBase FILES ${auinstbase_sources})
source_group(AUCore\\OtherBases FILES ${auotherbase_sources})
source_group(AUCore\\Utility FILES ${auutility_sources})
source_group(AUCore\\PublicUtility FILES ${aupublicutility_sources})

# ---------------------------------------------------------------------------------
#
# ---  COCOA UI target:
#
# ---------------------------------------------------------------------------------
set(cocoatarget ${EXP_SUBPROJECT_NAME_AU}_CocoaUI)

# ---  add the plugin source and link to lib, etc…
add_library(${cocoatarget} MODULE ${cocoa_sources})

# source_group(CocoaUI FILES ${cocoa_sources})

# --- output folder for the plugin
set_target_properties(${cocoatarget} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${AU_PLUGIN_FOLDER})

# --- set installation directory (must have this for cocoa)
set_target_properties(${cocoatarget} PROPERTIES XCODE_ATTRIBUTE_INSTALL_PATH "$(USER_LIBRARY_DIR)/Bundles" XCODE_ATTRIBUTE_SKIP_INSTALL "Yes")

# --- product name matches AU plugin target name
set_target_properties(${cocoatarget} PROPERTIES XCODE_ATTRIBUTE_PRODUCT_NAME ${EXP_SUBPROJECT_NAME_AU})

# --- save product name for later
get_target_property(COCOAPRODUCT_NAME ${cocoatarget} XCODE_ATTRIBUTE_PRODUCT_NAME)

# --- setup project
if(XCODE)
	set_target_properties(${cocoatarget} PROPERTIES BUNDLE TRUE)
	set_target_properties(${cocoatarget} PROPERTIES CXX_VISIBILITY_PRESET default)
	set_target_properties(${cocoatarget} PROPERTIES CMAKE_C_VISIBILITY_PRESET default)
  set_target_properties(${cocoatarget} PROPERTIES XCODE_ATTRIBUTE_GENERATE_PKGINFO_FILE "YES")
	set_target_properties(${cocoatarget} PROPERTIES XCODE_ATTRIBUTE_GENERATE_MASTER_OBJECT_FILE "YES")
	set_target_properties(${cocoatarget} PROPERTIES XCODE_ATTRIBUTE_OTHER_LDFLAGS "-all_load")

	# --- note: " " prior to AUPLUGIN etc... must be there to separate vars
	set_property(TARGET ${cocoatarget} APPEND_STRING PROPERTY XCODE_ATTRIBUTE_GCC_PREPROCESSOR_DEFINITIONS "$<$<CONFIG:Debug>: AUPLUGIN=1>$<$<CONFIG:Release>: AUPLUGIN=1>")
	set_property(TARGET ${cocoatarget} APPEND_STRING PROPERTY XCODE_ATTRIBUTE_GCC_PREPROCESSOR_DEFINITIONS "$<$<CONFIG:Debug>: DEVELOPMENT=1>$<$<CONFIG:Release>: RELEASE=1>")

	target_link_libraries(${cocoatarget} PRIVATE "-framework Cocoa")
	target_link_libraries(${cocoatarget} PRIVATE "-framework AudioToolbox")
	target_link_libraries(${cocoatarget} PRIVATE "-framework AudioUnit")

	# ---  setup header search paths
	target_include_directories(${cocoatarget} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_ROOT})
	target_include_directories(${cocoatarget} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${AU_SOURCE_ROOT})
	target_include_directories(${cocoatarget} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${RAFX_SOURCE_ROOT})
	target_include_directories(${cocoatarget} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${VSTGUI_SOURCE_ROOT})
endif()

# ---------------------------------------------------------------------------------
#
# ---  AU target:
#
# ---------------------------------------------------------------------------------
set(target ${EXP_SUBPROJECT_NAME_AU})

# ---  add the plugin source and link to lib, etc…
add_library(${target} MODULE ${rafx_au_sources} ${au_core_sources} ${vstgui_sources} ${custom_vstgui_sources} ${aubase_sources} ${auinstbase_sources} ${auotherbase_sources} ${auutility_sources} ${aupublicutility_sources})

# --- output folder for the plugin
set_target_properties(${target} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${AU_PLUGIN_FOLDER})

# --- add the exported symbol file
set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_EXPORTED_SYMBOLS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${AU_SOURCE_ROOT}/RackAFX2AUFilter.exp")

# --- setup the project
if(XCODE)
	set_target_properties(${target} PROPERTIES BUNDLE TRUE)
	set_target_properties(${target} PROPERTIES CXX_VISIBILITY_PRESET default)
	set_target_properties(${target} PROPERTIES CMAKE_C_VISIBILITY_PRESET default)
  set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_GENERATE_PKGINFO_FILE "YES")
  set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_WRAPPER_EXTENSION "component")
	set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_GENERATE_MASTER_OBJECT_FILE "YES")
	set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_OTHER_LDFLAGS "-all_load")
	set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_GENERATE_PKGINFO_FILE "YES")

	# --- note: " " prior to AUPLUGIN etc... must be there to separate vars
	set_property(TARGET ${target} APPEND_STRING PROPERTY XCODE_ATTRIBUTE_GCC_PREPROCESSOR_DEFINITIONS "$<$<CONFIG:Debug>: AUPLUGIN=1>$<$<CONFIG:Release>: AUPLUGIN=1>")
	set_property(TARGET ${target} APPEND_STRING PROPERTY XCODE_ATTRIBUTE_GCC_PREPROCESSOR_DEFINITIONS "$<$<CONFIG:Debug>: DEVELOPMENT=1>$<$<CONFIG:Release>: RELEASE=1>")

	target_link_libraries(${target} PRIVATE "-framework OpenGL")
	target_link_libraries(${target} PRIVATE "-framework CoreMIDI")
	target_link_libraries(${target} PRIVATE "-framework Accelerate")
	target_link_libraries(${target} PRIVATE "-framework QuartzCore")
	target_link_libraries(${target} PRIVATE "-framework Cocoa")
	target_link_libraries(${target} PRIVATE "-framework AudioToolbox")
	target_link_libraries(${target} PRIVATE "-framework AudioUnit")
	target_link_libraries(${target} PRIVATE "-framework CoreAudio")
	target_link_libraries(${target} PRIVATE "-framework CoreFoundation")
endif()

# ---  setup header search paths
target_include_directories(${target} PUBLIC ${VSTGUI_ROOT}/)
target_include_directories(${target} PUBLIC ${VSTGUI_ROOT}/vstgui4)
target_include_directories(${target} PUBLIC ${SDK_ROOT}/PublicUtility)
target_include_directories(${target} PUBLIC ${SDK_ROOT}/AUPublic/Utility)
target_include_directories(${target} PUBLIC ${SDK_ROOT}/AUPublic/AUBase)
target_include_directories(${target} PUBLIC ${SDK_ROOT}/AUPublic/AUInstrumentBase)
target_include_directories(${target} PUBLIC ${SDK_ROOT}/AUPublic/OtherBases)
target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_ROOT})
target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${AU_SOURCE_ROOT})
target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${RAFX_SOURCE_ROOT})
target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${VSTGUI_SOURCE_ROOT})

# ---------------------------------------------------------------------------------
#
# ---  Resources:
#
# ---------------------------------------------------------------------------------
set(resource_sources
	${RESOURCE_ROOT}/RackAFX.uidesc
	${RESOURCE_ROOT}/darkgreyvinyl.png
	${RESOURCE_ROOT}/bluemetal.png
	${RESOURCE_ROOT}/greenmetal.png
	${RESOURCE_ROOT}/greymetal.png
	${RESOURCE_ROOT}/lightgreymetal.png
	${RESOURCE_ROOT}/yellowmetal.png
	${RESOURCE_ROOT}/background.png
	${RESOURCE_ROOT}/groupframe.png
	${RESOURCE_ROOT}/knob.png
	${RESOURCE_ROOT}/blackgearknob.png
	${RESOURCE_ROOT}/blueableton.png
	${RESOURCE_ROOT}/chickenhead.png
	${RESOURCE_ROOT}/fxknob.png
	${RESOURCE_ROOT}/greenableton.png
	${RESOURCE_ROOT}/greyableton.png
	${RESOURCE_ROOT}/knob303.png
	${RESOURCE_ROOT}/knobm400.png
	${RESOURCE_ROOT}/minibrute.png
	${RESOURCE_ROOT}/sslblue.png
	${RESOURCE_ROOT}/sslgreen.png
	${RESOURCE_ROOT}/sslred.png
	${RESOURCE_ROOT}/alphablackgearknob1.png
	${RESOURCE_ROOT}/alphablackgearknob2.png
	${RESOURCE_ROOT}/alphachickenhead.png
	${RESOURCE_ROOT}/alphafxknob.png
	${RESOURCE_ROOT}/alphagreenableton.png
	${RESOURCE_ROOT}/alphaknob303.png
	${RESOURCE_ROOT}/alpham400.png
	${RESOURCE_ROOT}/alphaminibrute.png
	${RESOURCE_ROOT}/alphasslblue.png
	${RESOURCE_ROOT}/alphasslgreen.png
	${RESOURCE_ROOT}/alphasslred.png
	${RESOURCE_ROOT}/amberonoff.png
	${RESOURCE_ROOT}/twossamber.png
	${RESOURCE_ROOT}/twossgreen.png
	${RESOURCE_ROOT}/assignbamber.png
	${RESOURCE_ROOT}/rbstripamber2.png
	${RESOURCE_ROOT}/rbstripamber3.png
	${RESOURCE_ROOT}/rbstripamber4.png
	${RESOURCE_ROOT}/rbstripamber5.png
	${RESOURCE_ROOT}/rbstripamber6.png
	${RESOURCE_ROOT}/rbstripamber7.png
	${RESOURCE_ROOT}/rbstripamber8.png
	${RESOURCE_ROOT}/rbstripgreen2.png
	${RESOURCE_ROOT}/rbstripgreen3.png
	${RESOURCE_ROOT}/rbstripgreen4.png
	${RESOURCE_ROOT}/rbstripgreen5.png
	${RESOURCE_ROOT}/rbstripgreen6.png
	${RESOURCE_ROOT}/rbstripgreen7.png
	${RESOURCE_ROOT}/rbstripgreen8.png
	${RESOURCE_ROOT}/rbstripsilverbutton2.png
	${RESOURCE_ROOT}/rbstripsilverbutton3.png
	${RESOURCE_ROOT}/rbstripsilverbutton4.png
	${RESOURCE_ROOT}/rbstripsilverbutton5.png
	${RESOURCE_ROOT}/rbstripsilverbutton6.png
	${RESOURCE_ROOT}/rbstripsilverbutton7.png
	${RESOURCE_ROOT}/rbstripsilverbutton8.png
	${RESOURCE_ROOT}/vsbackground.png
	${RESOURCE_ROOT}/hsbackground.png
	${RESOURCE_ROOT}/hsbackS700.png
	${RESOURCE_ROOT}/vsbackgroundtwo.png
	${RESOURCE_ROOT}/vshandle.png
	${RESOURCE_ROOT}/hshandS700.png
	${RESOURCE_ROOT}/hshandle.png
	${RESOURCE_ROOT}/vuledoff.png
	${RESOURCE_ROOT}/vuledon.png
	${RESOURCE_ROOT}/vuledoffinv.png
	${RESOURCE_ROOT}/vuledoninv.png
	${RESOURCE_ROOT}/vsbackshort56.png
	${RESOURCE_ROOT}/vshandsmall56.png
	${RESOURCE_ROOT}/hsbackshort56.png
	${RESOURCE_ROOT}/hshandsmall56.png
	${RESOURCE_ROOT}/alphaampknob.png
	${RESOURCE_ROOT}/alphachickenhead2.png
	${RESOURCE_ROOT}/alphachickenhead3.png
	${RESOURCE_ROOT}/alphamaschine.png
	${RESOURCE_ROOT}/alphaorangeamp.png
	${RESOURCE_ROOT}/alphasslblue2.png
	${RESOURCE_ROOT}/alphasslgreen2.png
	${RESOURCE_ROOT}/alphasslred2.png
	${RESOURCE_ROOT}/alphasslyellow.png
	${RESOURCE_ROOT}/alphawhitedust.png
	${RESOURCE_ROOT}/ampknob.png
	${RESOURCE_ROOT}/bigampknob.png
	${RESOURCE_ROOT}/bigoberheimbutton.png
	${RESOURCE_ROOT}/bigpowerswitch.png
	${RESOURCE_ROOT}/bigprohphetbutton.png
	${RESOURCE_ROOT}/bigrubberbutton.png
	${RESOURCE_ROOT}/bigtoggleswitch.png
	${RESOURCE_ROOT}/chickenhead2.png
	${RESOURCE_ROOT}/chickenhead3.png
	${RESOURCE_ROOT}/giantampknob.png
	${RESOURCE_ROOT}/maschine.png
	${RESOURCE_ROOT}/medoberheimbutton.png
	${RESOURCE_ROOT}/medpowerswitch.png
	${RESOURCE_ROOT}/medprophetbutton.png
	${RESOURCE_ROOT}/medrubberbutton.png
	${RESOURCE_ROOT}/medtoggleswitch.png
	${RESOURCE_ROOT}/microoberheimbutton.png
	${RESOURCE_ROOT}/microtoggleswitch.png
	${RESOURCE_ROOT}/orangeamp.png
	${RESOURCE_ROOT}/smalloberheimbutton.png
	${RESOURCE_ROOT}/smallpowerswitch.png
	${RESOURCE_ROOT}/smallprohetbutton.png
	${RESOURCE_ROOT}/smallrubberbutton.png
	${RESOURCE_ROOT}/smalltoggleswitch.png
	${RESOURCE_ROOT}/tinytoggleswitch.png
	${RESOURCE_ROOT}/sslblue2.png
	${RESOURCE_ROOT}/sslgreen2.png
	${RESOURCE_ROOT}/sslyellow.png
	${RESOURCE_ROOT}/sslyellow2.png
	${RESOURCE_ROOT}/whitedust.png
	${RESOURCE_ROOT}/whitedust.png
	${RESOURCE_ROOT}/prophet.png
	${RESOURCE_ROOT}/alphaprophet.png
	${RESOURCE_ROOT}/giantprophet.png
	${RESOURCE_ROOT}/smallprophet.png
	${RESOURCE_ROOT}/tinyprophet.png
	${RESOURCE_ROOT}/blackplastic.png
	${RESOURCE_ROOT}/alphablackplastic.png
	${RESOURCE_ROOT}/giantblackplastic.png
	${RESOURCE_ROOT}/smallblackplastic.png
	${RESOURCE_ROOT}/analogvumeter.png
	${RESOURCE_ROOT}/smallanalogvumeter.png
	${RESOURCE_ROOT}/biganalogvumeter.png
	${RESOURCE_ROOT}/medanalogvumeter.png
	${RESOURCE_ROOT}/horizvuledoff.png
	${RESOURCE_ROOT}/horizvuledoffinv.png
	${RESOURCE_ROOT}/horizvuledon.png
	${RESOURCE_ROOT}/horizvuledoninv.png
	${RESOURCE_ROOT}/vertswitchfilters.png
	${RESOURCE_ROOT}/horizswitchsynth.png
	${RESOURCE_ROOT}/knobvst.png
	${RESOURCE_ROOT}/knobvst.png
)
# --- attach to target
target_sources(${target} PRIVATE ${resource_sources})
set_source_files_properties(${resource_sources} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

# --- set folder/filter
source_group(Resources FILES ${resource_sources})

# --- info.pList files
set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_INFOPLIST_FILE "${CMAKE_CURRENT_LIST_DIR}/${MAC_ROOT}/Info.plist")
set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_INFOPLIST_PREPROCESS "YES")

set_target_properties(${cocoatarget} PROPERTIES XCODE_ATTRIBUTE_INFOPLIST_FILE "${CMAKE_CURRENT_LIST_DIR}/${MAC_ROOT}/cocoaInfo.plist")
set_target_properties(${cocoatarget} PROPERTIES XCODE_ATTRIBUTE_INFOPLIST_PREPROCESS "YES")

# --- copy folders (.bundle and .component are directories, so use remove/copy directory)
set(TARGET_PATH "${CMAKE_BINARY_DIR}/${AU_PLUGIN_FOLDER}/${CMAKE_BUILD_TYPE}/${target}.component") # --- AU/Debug or /Release
set(COCOATARGET_PATH "${CMAKE_BINARY_DIR}/${AU_PLUGIN_FOLDER}/${CMAKE_BUILD_TYPE}/${COCOAPRODUCT_NAME}.bundle") # --- AU/Debug or /Release

if(XCODE)
	# --- copy cocoa bundle into resources folder
	set(COCOA_IN_RSRSC_PATH ${TARGET_PATH}/Contents/Resources/${COCOAPRODUCT_NAME}.bundle)
	add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E remove_directory ${COCOA_IN_RSRSC_PATH})
	add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${COCOATARGET_PATH} ${COCOA_IN_RSRSC_PATH})

	# --- copy plugin into library folder
	set(PLUGIN_FOLDER ~/Library/Audio/Plug-Ins/Components/${target}.component)
	add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E remove_directory ${PLUGIN_FOLDER})
	add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${TARGET_PATH} ${PLUGIN_FOLDER})
endif()

