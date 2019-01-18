# ---------------------------------------------------------------------------------
#
# --- AAX Project CMakeLists.txt
# --- Created by RackAFX(TM)
# --- www.willpirkle.com
#
# ---------------------------------------------------------------------------------
set(SDK_ROOT ${AAX_SDK_ROOT})

set(WIN_ROOT "../../win/aax")
set(MAC_ROOT "../../mac/aax")
set(SOURCE_ROOT "../../source")
set(RESOURCE_ROOT "../../resources")

# --- local roots
set(RAFX_SOURCE_ROOT "${SOURCE_ROOT}/rafx_source")
set(VSTGUI_SOURCE_ROOT "${SOURCE_ROOT}/vstgui_source")
set(AAX_SOURCE_ROOT "${SOURCE_ROOT}/aax_source")

# ---------------------------------------------------------------------------------
#
# ---  RAFX Ported plugin files
#
# ---------------------------------------------------------------------------------
set(rafx_aax_sources
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

set(aax_core_sources
	${AAX_SOURCE_ROOT}/AAX_CRafxLogTaperDelegate.h
	${AAX_SOURCE_ROOT}/AAX_CVoltOctaveTaperDelegate.h
	${AAX_SOURCE_ROOT}/AAXtoVSTGUIButtonState.h
	${AAX_SOURCE_ROOT}/RackAFX_Describe.cpp
	${AAX_SOURCE_ROOT}/RackAFX_Describe.h
	${AAX_SOURCE_ROOT}/RackAFX_GUI.cpp
	${AAX_SOURCE_ROOT}/RackAFX_GUI.h
	${AAX_SOURCE_ROOT}/RackAFX_Parameters.cpp
	${AAX_SOURCE_ROOT}/RackAFX_Parameters.h
)

# ---------------------------------------------------------------------------------
#
# ---  AAX SDK files
#
# ---------------------------------------------------------------------------------
set(aax_lib_sources_mac
	${AAX_SDK_ROOT}/Interfaces/AAX_Exports.cpp
	${AAX_SDK_ROOT}/Libs/Debug/libAAXLibrary.a
)

set(aax_lib_sources_win
	${AAX_SDK_ROOT}/Interfaces/AAX_Exports.cpp
	${AAX_SDK_ROOT}/Libs/Debug/AAXLibrary_x64_D.lib
	${AAX_SDK_ROOT}/Libs/Release/AAXLibrary_x64.lib
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

set(vstgui_sources_mac
	${VSTGUI_ROOT}/vstgui4/vstgui/vstgui_mac.mm
	${VSTGUI_ROOT}/vstgui4/vstgui/vstgui_uidescription_mac.mm
)

set(vstgui_sources_win
	${VSTGUI_ROOT}/vstgui4/vstgui/vstgui_win32.cpp
	${VSTGUI_ROOT}/vstgui4/vstgui/vstgui_uidescription_win32.cpp
)

# ---------------------------------------------------------------------------------
#
# ---  setup IDE folders/filters
#
# ---------------------------------------------------------------------------------
source_group(RackAFXFiles FILES ${rafx_aax_sources})
source_group(AAXCore FILES ${aax_core_sources})
source_group(CustomVSTGUI FILES ${custom_vstgui_sources})
if(MAC)
	source_group(AvidLibraries FILES ${aax_lib_sources_mac})
	source_group(vstgui4 FILES ${vstgui_sources_mac})
elseif(WIN)
	source_group(AvidLibraries FILES ${aax_lib_sources_win})
	source_group(vstgui4 FILES ${vstgui_sources_win})
endif()

# ---------------------------------------------------------------------------------
#
# ---  AAX target:
#
# ---------------------------------------------------------------------------------
set(target ${EXP_SUBPROJECT_NAME_AAX})

# ---  add the plugin source
if(MAC)
	add_library(${target} MODULE ${rafx_aax_sources} ${aax_core_sources} ${aax_lib_sources_mac} ${vstgui_sources_mac} ${custom_vstgui_sources})
elseif(WIN)
	add_library(${target} MODULE ${rafx_aax_sources} ${aax_core_sources} ${aax_lib_sources_win} ${vstgui_sources_win} ${custom_vstgui_sources})
endif()

# --- link to the AAX Library: NOTE: user must compile this first - there are no CMake files with AASX SDK!
if(MAC)
	add_library(aaxLibraryD STATIC IMPORTED)
	set_property(TARGET aaxLibraryD PROPERTY IMPORTED_LOCATION "${AAX_SDK_ROOT}/Libs/Debug/libAAXLibrary.a")
	target_link_libraries(${target} PRIVATE debug aaxLibraryD)

	add_library(aaxLibraryR STATIC IMPORTED)
	set_property(TARGET aaxLibraryR PROPERTY IMPORTED_LOCATION ${AAX_SDK_ROOT}/Libs/Release/libAAXLibrary.a)
	target_link_libraries(${target} PRIVATE optimized aaxLibraryR)
elseif(WIN)
	add_library(aaxLibraryD STATIC IMPORTED)
	set_property(TARGET aaxLibraryD PROPERTY IMPORTED_LOCATION "${AAX_SDK_ROOT}/Libs/Debug/AAXLibrary_x64_D.lib")
	target_link_libraries(${target} PRIVATE debug aaxLibraryD)

	add_library(aaxLibraryR STATIC IMPORTED)
	set_property(TARGET aaxLibraryR PROPERTY IMPORTED_LOCATION ${AAX_SDK_ROOT}/Libs/Release/AAXLibrary_x64.lib )
	target_link_libraries(${target} PRIVATE optimized aaxLibraryR)
endif()

# --- output folder for the plugin
if(MAC)
	set_target_properties(${target} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${AAX_PLUGIN_FOLDER})
endif()

# --- setup the project
if(MAC)
	set_target_properties(${target} PROPERTIES BUNDLE TRUE)
	set_target_properties(${target} PROPERTIES CXX_VISIBILITY_PRESET default)
	set_target_properties(${target} PROPERTIES CMAKE_C_VISIBILITY_PRESET default)
  set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_GENERATE_PKGINFO_FILE "YES")
  set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_WRAPPER_EXTENSION "aaxplugin")
	set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_GENERATE_MASTER_OBJECT_FILE "YES")
	set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_OTHER_LDFLAGS "-all_load")
	set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_GENERATE_PKGINFO_FILE "YES")

	target_link_libraries(${target} PRIVATE "-framework Accelerate")
	target_link_libraries(${target} PRIVATE "-framework OpenGL")
	target_link_libraries(${target} PRIVATE "-framework CoreMIDI")
	target_link_libraries(${target} PRIVATE "-framework CoreServices")
	target_link_libraries(${target} PRIVATE "-framework Carbon")
	target_link_libraries(${target} PRIVATE "-framework Cocoa")
	target_link_libraries(${target} PRIVATE "-framework Quartz")
	target_link_libraries(${target} PRIVATE "-framework CoreFoundation")

	# --- remove -all_load from other flags
	set_property(TARGET ${target} PROPERTY XCODE_ATTRIBUTE_OTHER_LDFLAGS "")

	# --- eureka
	set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_DYLIB_CURRENT_VERSION "1.0")
	set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_DYLIB_COMPATIBILITY_VERSION "1.0")

	# --- NOTE: you can change this to suit your company, year, etc...
	set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_CURRENT_YEAR "2018")
	set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_AVID_COMPANY_STRING "Avid Technology, Inc.")
	set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_AVID_COPYRIGHT "$Copyright 1991-$(CURRENT_YEAR) $(AVID_COMPANY_STRING)")

	# --- set individual properties!
	set(CMAKE_XCODE_ATTRIBUTE_GCC_OPTIMIZATION_LEVEL "$<$<CONFIG:Debug>:0>$<$<CONFIG:Release>:3>")
	set(CMAKE_XCODE_ATTRIBUTE_COPY_PHASE_STRIP "$<$<CONFIG:Debug>:NO>$<$<CONFIG:Release>:YES>")

	# --- APPEND preprocessor strings PER CONFIGURATION
	set_property(TARGET ${target} APPEND PROPERTY XCODE_ATTRIBUTE_GCC_PREPROCESSOR_DEFINITIONS "$<$<CONFIG:Debug>:_DEBUG=1>$<$<CONFIG:Release>:_NDEBUG=1>")

	# --- note: " " prior to AAXPLUGIN etc... must be there to separate vars
	set_property(TARGET ${target} APPEND_STRING PROPERTY XCODE_ATTRIBUTE_GCC_PREPROCESSOR_DEFINITIONS "$<$<CONFIG:Debug>: AAXPLUGIN=1>$<$<CONFIG:Release>: AAXPLUGIN=1>")
	set_property(TARGET ${target} APPEND_STRING PROPERTY XCODE_ATTRIBUTE_GCC_PREPROCESSOR_DEFINITIONS "$<$<CONFIG:Debug>: DEVELOPMENT=1>$<$<CONFIG:Release>: RELEASE=1>")
	# --- add_compile_options($<$<CONFIG:Debug>:-DVSTGUI_LIVE_EDITING=1>)

	# --- other linker flags here:
	set_property(TARGET ${target} APPEND_STRING PROPERTY XCODE_ATTRIBUTE_OTHER_LDFLAGS "$<$<CONFIG:Debug>: >$<$<CONFIG:Release>:-Xlinker -headerpad -Xlinker 1500>")
elseif(WIN)
	add_definitions(-DAAXPLUGIN)
	add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
	add_definitions(-D_SCL_SECURE_NO_DEPRECATE)
	add_definitions(-DWINDOWS_VERSION)
	set_target_properties(${target} PROPERTIES SUFFIX ".aaxplugin")

	# --- turn off incremental linking; this reduces plugin size by removing the .ilk file from the package
	set_target_properties(${target} PROPERTIES LINK_FLAGS "/INCREMENTAL:NO")

	# --- add linked libs, as per Avid examples
	target_link_libraries(${target} PRIVATE "odbc32.lib")
	target_link_libraries(${target} PRIVATE "odbccp32.lib")
	target_link_libraries(${target} PRIVATE "psapi.lib")

	# --- set the output directories - note that an AAX plugin is actually a directory, not unlike a MacOS plugin (AU, VST, AAX)
	set_target_properties(${target}
    		PROPERTIES
    		ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/archive"
			  LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/${AAX_PLUGIN_FOLDER}/$(Configuration)/$(ProjectName).aaxplugin/Contents/$(PlatformName)/"
   			LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/${AAX_PLUGIN_FOLDER}/$(Configuration)/$(ProjectName).aaxplugin/Contents/$(PlatformName)/"
	)

	# --- RUN THE PRE-BUILD BAT FILE
	add_custom_command(TARGET ${target}
	                   PRE_BUILD
	                   COMMAND cmd /c "${AAX_SDK_ROOT}/Utilities/CreatePackage.bat" "$(OutDir)" "${AAX_SDK_ROOT}/Utilities/PlugIn.ico")
endif()


# ---  setup header search paths
target_include_directories(${target} PUBLIC ${VSTGUI_ROOT}/)
target_include_directories(${target} PUBLIC ${VSTGUI_ROOT}/vstgui4)
target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_ROOT})
target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${RAFX_SOURCE_ROOT})
target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${VSTGUI_SOURCE_ROOT})
target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${AAX_SOURCE_ROOT})

# --- from original xcconfig files: same for debug/release
target_include_directories(${target} PUBLIC ${AAX_SDK_ROOT}/Interfaces)
target_include_directories(${target} PUBLIC ${AAX_SDK_ROOT}/Interfaces/ACF)
target_include_directories(${target} PUBLIC ${AAX_SDK_ROOT}/Interfaces/ACF)
target_include_directories(${target} PUBLIC ${AAX_SDK_ROOT}/Extensions/MIDI)
target_include_directories(${target} PUBLIC ${AAX_SDK_ROOT}/Extensions/Platform/include)
target_include_directories(${target} PUBLIC ${AAX_SDK_ROOT}/Extensions/Topology)
target_include_directories(${target} PUBLIC ${AAX_SDK_ROOT}/TI/SignalProcessing)


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

if(XCODE)
	set_source_files_properties(${resource_sources} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
endif()

# --- set folder/filter
source_group(Resources FILES ${resource_sources})

if(XCODE)
	# --- info.pList files
	set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_INFOPLIST_FILE "${CMAKE_CURRENT_LIST_DIR}/${MAC_ROOT}/Info.plist")
	set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_INFOPLIST_PREPROCESSOR_DEFINITIONS "PRODUCT_NAME=$(PRODUCT_NAME) EXECUTABLE_NAME=$(EXECUTABLE_NAME) DYLIB_CURRENT_VERSION=$(DYLIB_CURRENT_VERSION) VERSION_NUM=$(DYLIB_CURRENT_VERSION) VERSION_STR=$(DYLIB_CURRENT_VERSION) CURRENT_YEAR=$(CURRENT_YEAR) AVID_COPYRIGHT=$(AVID_COPYRIGHT) AVID_COMPANY_STRING=$(AVID_COMPANY_STRING)")
	set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_INFOPLIST_PREPROCESS "YES")

	# --- copy folders (.bundle and .component are directories, so use remove/copy directory)
	#     NOTE: /Library/Application Support/Avid/Audio/Plug-Ins is the AAX location BUT this folder is protected so can't copy programmatically
	#
	#           User must manually copy from the /mac_build/AAX folder to the /Library/Application Support/Avid/Audio/Plug-Ins folder
elseif(WIN)
	# --- WIN resource files
	target_sources(${target} PRIVATE ${WIN_ROOT}/RAFX2AAXWrap.rc)
	target_sources(${target} PRIVATE ${WIN_ROOT}/RAFX2AAXWrapResource.h)
	target_sources(${target} PRIVATE ${WIN_ROOT}/resource.h)
endif()

