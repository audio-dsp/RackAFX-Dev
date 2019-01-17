# ---------------------------------------------------------------------------------
#
# --- VST3 Project CMakeLists.txt
# --- Created by RackAFX(TM)
# --- www.willpirkle.com
#
# ---------------------------------------------------------------------------------
set(WIN_ROOT "../../win/vst")
set(MAC_ROOT "../../mac/vst")
set(SOURCE_ROOT "../../source")
set(RESOURCE_ROOT "../../resources")

# --- local roots
set(RAFX_SOURCE_ROOT "${SOURCE_ROOT}/rafx_source")
set(VSTGUI_SOURCE_ROOT "${SOURCE_ROOT}/vstgui_source")
set(VST_SOURCE_ROOT "${SOURCE_ROOT}/vst_source")

# ---------------------------------------------------------------------------------
#
# ---  RAFX Ported plugin files
#
# ---------------------------------------------------------------------------------
set(rafx_vst_sources
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

set(vst_core_sources
	${VST_SOURCE_ROOT}/factory.cpp
	${VST_SOURCE_ROOT}/VSTProcessor.cpp
	${VST_SOURCE_ROOT}/PeakParameter.cpp
	${VST_SOURCE_ROOT}/LogParameter.cpp
	${VST_SOURCE_ROOT}/VoltOctaveParameter.cpp
	${VST_SOURCE_ROOT}/version.h
	${VST_SOURCE_ROOT}/VSTProcessor.h
	${VST_SOURCE_ROOT}/PeakParameter.h
	${VST_SOURCE_ROOT}/LogParameter.h
	${VST_SOURCE_ROOT}/VoltOctaveParameter.h
	${VST_SOURCE_ROOT}/RafxPluginFactory.cpp
	${VST_SOURCE_ROOT}/RafxPluginFactory.h
)

# ---------------------------------------------------------------------------------
#
# ---  VST SDK files
#
# ---------------------------------------------------------------------------------
set(vstsdk_sources
	${SDK_ROOT}/public.sdk/source/vst/vstsinglecomponenteffect.cpp
	${SDK_ROOT}/public.sdk/source/vst/vstsinglecomponenteffect.h
	${SDK_ROOT}/public.sdk/source/vst/vst2wrapper/vst2wrapper.sdk.cpp
)

# ---------------------------------------------------------------------------------
#
# ---  VSTGUI files
#
# ---------------------------------------------------------------------------------
set(custom_vstgui_sources
	${VSTGUI_SOURCE_ROOT}/RackAFXGUIFactory.cpp
	${VSTGUI_SOURCE_ROOT}/TextLabelEditWP.cpp
	${VSTGUI_SOURCE_ROOT}/KickButtonWP.cpp
	${VSTGUI_SOURCE_ROOT}/KnobWP.cpp
	${VSTGUI_SOURCE_ROOT}/PadControllerWP.cpp
	${VSTGUI_SOURCE_ROOT}/RackAFXMeterDetector.cpp
	${VSTGUI_SOURCE_ROOT}/SliderWP.cpp
	${VSTGUI_SOURCE_ROOT}/VST3EditorWP.cpp
	${VSTGUI_SOURCE_ROOT}/VuMeterWP.cpp
	${VSTGUI_SOURCE_ROOT}/XYPadWP.cpp
	${VSTGUI_SOURCE_ROOT}/TextLabelEditWP.h
	${VSTGUI_SOURCE_ROOT}/vstgui4constants.h
	${VSTGUI_SOURCE_ROOT}/KickButtonWP.h
	${VSTGUI_SOURCE_ROOT}/KnobWP.h
	${VSTGUI_SOURCE_ROOT}/PadControllerWP.h
	${VSTGUI_SOURCE_ROOT}/RackAFXMeterDetector.h
	${VSTGUI_SOURCE_ROOT}/SliderWP.h
	${VSTGUI_SOURCE_ROOT}/VST3EditorWP.h
	${VSTGUI_SOURCE_ROOT}/VuMeterWP.h
	${VSTGUI_SOURCE_ROOT}/XYPadWP.h
)

# ---------------------------------------------------------------------------------
#
# ---  VST3 target:
#
# ---------------------------------------------------------------------------------
set(target ${EXP_SUBPROJECT_NAME_VST})

# --- add the plugin source and link to lib, etc... smtg_add_vst3plugin does all the setup
smtg_add_vst3plugin(${target} ${SDK_ROOT} ${rafx_vst_sources} ${vst_core_sources} ${custom_vstgui_sources} ${vstsdk_sources})

# ---  setup header search paths
target_include_directories(${target} PUBLIC ${VSTGUI_ROOT}/)
target_include_directories(${target} PUBLIC ${VSTGUI_ROOT}/vstgui4)
target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_ROOT})
target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${RAFX_SOURCE_ROOT})
target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${VSTGUI_SOURCE_ROOT})
target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${VST_SOURCE_ROOT})

# --- setup link lib
target_link_libraries(${target} PRIVATE base sdk vstgui_support)

# --- preprocessor for D2D for windows
if(WIN)
	add_definitions(-DVSTGUI_DIRECT2D_SUPPORT=1)
endif()

# ---------------------------------------------------------------------------------
#
# ---  Resources:
#
# ---------------------------------------------------------------------------------
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/RackAFX.uidesc")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/darkgreyvinyl.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/bluemetal.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/greenmetal.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/greymetal.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/lightgreymetal.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/yellowmetal.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/background.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/groupframe.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/knob.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/blackgearknob.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/blueableton.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/chickenhead.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/fxknob.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/greenableton.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/greyableton.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/knob303.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/knobm400.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/minibrute.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/sslblue.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/sslgreen.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/sslred.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/alphablackgearknob1.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/alphablackgearknob2.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/alphachickenhead.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/alphafxknob.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/alphagreenableton.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/alphaknob303.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/alpham400.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/alphaminibrute.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/alphasslblue.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/alphasslgreen.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/alphasslred.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/amberonoff.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/twossamber.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/twossgreen.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/assignbamber.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/rbstripamber2.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/rbstripamber3.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/rbstripamber4.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/rbstripamber5.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/rbstripamber6.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/rbstripamber7.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/rbstripamber8.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/rbstripgreen2.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/rbstripgreen3.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/rbstripgreen4.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/rbstripgreen5.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/rbstripgreen6.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/rbstripgreen7.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/rbstripgreen8.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/rbstripsilverbutton2.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/rbstripsilverbutton3.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/rbstripsilverbutton4.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/rbstripsilverbutton5.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/rbstripsilverbutton6.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/rbstripsilverbutton7.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/rbstripsilverbutton8.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/vsbackground.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/hsbackground.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/hsbackS700.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/vsbackgroundtwo.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/vshandle.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/hshandS700.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/hshandle.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/vuledoff.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/vuledon.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/vuledoffinv.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/vuledoninv.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/vsbackshort56.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/vshandsmall56.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/hsbackshort56.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/hshandsmall56.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/alphaampknob.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/alphachickenhead2.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/alphachickenhead3.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/alphamaschine.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/alphaorangeamp.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/alphasslblue2.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/alphasslgreen2.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/alphasslred2.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/alphasslyellow.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/alphawhitedust.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/ampknob.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/bigampknob.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/bigoberheimbutton.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/bigpowerswitch.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/bigprohphetbutton.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/bigrubberbutton.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/bigtoggleswitch.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/chickenhead2.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/chickenhead3.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/giantampknob.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/maschine.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/medoberheimbutton.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/medpowerswitch.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/medprophetbutton.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/medrubberbutton.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/medtoggleswitch.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/microoberheimbutton.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/microtoggleswitch.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/orangeamp.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/smalloberheimbutton.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/smallpowerswitch.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/smallprohetbutton.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/smallrubberbutton.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/smalltoggleswitch.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/tinytoggleswitch.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/sslblue2.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/sslgreen2.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/sslyellow.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/sslyellow2.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/whitedust.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/whitedust.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/prophet.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/alphaprophet.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/giantprophet.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/smallprophet.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/tinyprophet.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/blackplastic.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/alphablackplastic.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/giantblackplastic.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/smallblackplastic.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/analogvumeter.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/smallanalogvumeter.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/biganalogvumeter.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/medanalogvumeter.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/horizvuledoff.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/horizvuledoffinv.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/horizvuledon.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/horizvuledoninv.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/vertswitchfilters.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/horizswitchsynth.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/knobvst.png")
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/knobvst.png")

if(MAC)
	smtg_set_bundle(${target} INFOPLIST "${CMAKE_CURRENT_LIST_DIR}/${MAC_ROOT}/Info.plist" PREPROCESS)
elseif(WIN)
	target_sources(${target} PRIVATE ${WIN_ROOT}/RAFX2VSTWrap.rc)
endif()


# fix missing VSTPluginMain symbol when also building VST 2 version
set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_EXPORTED_SYMBOLS_FILE "")

if (WIN)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if(WIN)
	set(TARGET_PATH $<TARGET_FILE:${target}>)
elseif(XCODE)
	set(TARGET_PATH "${VST3_OUTPUT_DIR}/${CMAKE_BUILD_TYPE}/${target}.vst3")
else()
	set(TARGET_PATH "${VST3_OUTPUT_DIR}/${target}.vst3")
endif()

# for Xcode, add the build-phase to copy the VST2 and VST3 plugins to the library for the user
if(XCODE)
# --- the VST3 Cmake Modules now copy a symlink (shortcut) to the MacOS VST3 director so this has been removed
# 	set(PLUGIN_FOLDER ~/Library/Audio/Plug-Ins/VST3/${target}.vst3)
# 	ADD_CUSTOM_COMMAND(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E remove_directory ${PLUGIN_FOLDER})
# 	ADD_CUSTOM_COMMAND(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${TARGET_PATH} ${PLUGIN_FOLDER})

# --- the VST2 plugins were not changed, so I will still copy the VST plugin here (renamning from vst3 to vst in the process)
	set(PLUGIN_FOLDER ~/Library/Audio/Plug-Ins/VST/${target}.vst)
	ADD_CUSTOM_COMMAND(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E remove_directory ${PLUGIN_FOLDER})
	ADD_CUSTOM_COMMAND(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${TARGET_PATH} ${PLUGIN_FOLDER})
endif()

source_group(RackAFXFiles FILES ${rafx_vst_sources})
source_group(CustomVSTGUI FILES ${custom_vstgui_sources})
source_group(VST3Core FILES ${vst_core_sources})
source_group(VST3SDK FILES ${vstsdk_sources})

