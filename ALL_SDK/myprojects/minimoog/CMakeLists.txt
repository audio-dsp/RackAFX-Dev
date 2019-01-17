
###############################################################################
## target definitions #########################################################
###############################################################################
set(plug_sources
    include/aeffect.h
    include/aeffectx.h
    include/aeffeditor.h
    include/audioeffect.h
    include/basic_blocks.h
    include/globals.h
    include/Moog.h
    include/MoogPlugin.h
    include/vstfxstore.h
	include/plugcontroller.h
	include/plugids.h
	include/plugprocessor.h
	include/version.h

    source/Adder.cpp
    source/ADSR.cpp
    source/audioeffect.cpp
    source/audioeffectx.cpp
    source/BasicBlock.cpp
    source/Filter.cpp
    source/globals.cpp
    source/Moog.cpp
    source/MoogPlugin.cpp
    source/Multiplexer.cpp
    source/Multiplier.cpp
    source/Noise.cpp
    source/Number.cpp
    source/Oscil.cpp
    source/Waveform.cpp
    source/vstplugmain.cpp
    source/plugcontroller.cpp
	source/plugfactory.cpp
	source/plugprocessor.cpp
    )

#--- HERE change the target Name for your plug-in (for ex. set(target myDelay))-------
set(target minimoog)

#--- HERE change the target Name for your plug-in (for ex. set(target myDelay))-------
smtg_add_vst3plugin (${target} ${SDK_ROOT} ${plug_sources})
set_target_properties(${target} PROPERTIES ${SDK_IDE_MYPLUGINS_FOLDER})
target_link_libraries(${target} PRIVATE base sdk)


#if(MAC)
    #smtg_set_bundle(${target} INFOPLIST "${CMAKE_CURRENT_LIST_DIR}/resource/Info.plist" PREPROCESS)
#elseif(WIN)
    #target_sources(${target} PRIVATE resource/plug.rc)
#endif()


###############################################################################
## packaging ##################################################################
###############################################################################

# all install commands get the same destination. this allows us to use paths
# relative to the executable.
#install(TARGETS example DESTINATION example_destination)
# this is basically a repeat of the file copy instruction that copies the
# resources in the build directory, but here we tell cmake that we want it
# in the package
#install(DIRECTORY resources DESTINATION example_destination)

# now comes everything we need, to create a package
# there are a lot more variables you can set, and some
# you need to set for some package types, but we want to
# be minimal here
#set(CPACK_PACKAGE_NAME "MyExample")
#set(CPACK_PACKAGE_VERSION "1.0.0")

# we don't want to split our program up into several things
#set(CPACK_MONOLITHIC_INSTALL 1)

# This must be last
#include(CPack)

