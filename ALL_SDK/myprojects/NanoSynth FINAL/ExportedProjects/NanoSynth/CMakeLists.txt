# ---------------------------------------------------------------------------------
#
# --- UNIVERSAL CMakeLists.txt
# --- Created by RackAFX(TM)
# --- www.willpirkle.com
#
# ---------------------------------------------------------------------------------
cmake_minimum_required (VERSION 3.4.3)

# ---------------------------------------------------------------------------------
#
# --- UNIVERSAL SDK FOLDER NAMES
#
# ---------------------------------------------------------------------------------
#
# --- Here you can set the names of your UNIVERSAL SDK sub-USE_FOLDERS
#     NOTE: this is not needed when you export individual API projects
set(UNI_AAX_SDK_FOLDER AAX_SDK)
set(UNI_AU_SDK_FOLDER AU_SDK)
set(UNI_VST3_SDK_FOLDER VST_SDK/VST3_SDK)
# ---------------------------------------------------------------------------------

#--- BUILD FLAGS
#
# --- Universal Build Flag - when using combined SDKs; can still build independently
#                            with individual flags below
set(UNIVERSAL_SDK_BUILD TRUE)

# --- Individual project builds
set(AAX_SDK_BUILD TRUE)
set(AU_SDK_BUILD TRUE)
set(VST_SDK_BUILD TRUE)

# ---------------------------------------------------------------------------------
#
# --- PROJECT NAMES
#
# ---------------------------------------------------------------------------------
# --- the name of the project as exported from RackAFX
set(RAFX_EXP_PROJECT_NAME NanoSynth)

if(UNIVERSAL_SDK_BUILD)
	# --- names of sub-projects for universal builds; you can change them here:
	set(EXP_PROJECT_NAME ${RAFX_EXP_PROJECT_NAME}_UNIVERSAL)
	set(EXP_SUBPROJECT_NAME_AAX ${RAFX_EXP_PROJECT_NAME}_AAX)
	set(EXP_SUBPROJECT_NAME_AU ${RAFX_EXP_PROJECT_NAME}_AU)
	set(EXP_SUBPROJECT_NAME_VST ${RAFX_EXP_PROJECT_NAME}_VST)
else()
	# --- names of sub-projects for non-universal builds are same as normal builds; you can change them here:
	set(EXP_PROJECT_NAME ${RAFX_EXP_PROJECT_NAME})
	set(EXP_SUBPROJECT_NAME_AAX ${RAFX_EXP_PROJECT_NAME})
	set(EXP_SUBPROJECT_NAME_AU ${RAFX_EXP_PROJECT_NAME})
	set(EXP_SUBPROJECT_NAME_VST ${RAFX_EXP_PROJECT_NAME})
endif()

# ---------------------------------------------------------------------------------
#
# --- SDK ROOTS
#
# ---------------------------------------------------------------------------------
# --- PARENT: this is the UNIVERSAL project folder
get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)

# --- GRANDPARENT: this is the outer "myprojects" folder
get_filename_component(GRANDPARENT_DIR ${PARENT_DIR} DIRECTORY)

# --- GR_GRANDPARENT: this is the outer "ALL_SDK" folder, or whatever the user calls it
get_filename_component(GR_GRANDPARENT_DIR ${GRANDPARENT_DIR} DIRECTORY)

# ---------------------------------------------------------------------------------
#
#	  NOTE: the SDK path defaults to the location included in the "readme.txt" file
#			in the Ported Project folder. However, you can customize it here
#			by either replacing the code with your own CMake script, or by
#			setting the ROOT variable to an absolute path, e.g.
#			MacOS:
#				set(ROOT "/Users/<your user name>/Documents/AU_SDK")
#
#			Windows:
#				set(ROOT "C:/AU_SDK")
#
# ---------------------------------------------------------------------------------
if(UNIVERSAL_SDK_BUILD)
	# --- AAX:
	set(AAX_SDK_ROOT ${GRANDPARENT_DIR}/${UNI_AAX_SDK_FOLDER})

	# --- AU:
	set(AU_SDK_ROOT ${GRANDPARENT_DIR}/${UNI_AU_SDK_FOLDER})

	# --- VST3:
	set(SDK_ROOT ${GRANDPARENT_DIR}/${UNI_VST3_SDK_FOLDER})
else()
	# --- AAX:
	set(AAX_SDK_ROOT ${GRANDPARENT_DIR})

	# --- AU:
	set(AU_SDK_ROOT ${GRANDPARENT_DIR})

	# --- VST3:
	set(SDK_ROOT ${GRANDPARENT_DIR})

endif()

# --- VSTGUI4: NOTE: we use our own VSTGUI4 folder - portable to new SDKs where you
#              may not want to update VSTGUI just yet and won't interfere
#		       		 with VST3 SDK whatsoever. You do NOT need to customize this path!
set(VSTGUI_ROOT ${PARENT_DIR})

# ---------------------------------------------------------------------------------
#
# ---  Basic Setup
#
# ---------------------------------------------------------------------------------
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR OR EXISTS "${CMAKE_BINARY_DIR}/CMakeLists.txt")
	message(SEND_ERROR "In-source builds are not allowed. Please create a separate build directory and run 'cmake /path/to/folder [options]' there.")
	message(FATAL_ERROR "You can remove the file \"CMakeCache.txt\" and directory \"CMakeFiles\" in ${CMAKE_SOURCE_DIR}.")
endif()

if(POLICY CMP0054)
	cmake_policy(SET CMP0054 NEW)
endif()

if(APPLE)
  set(MAC TRUE)
elseif(WIN32)
	set(WIN TRUE)
elseif(UNIX)
	set(LINUX TRUE)
endif()

# ---------------------------------------------------------------------------------
#
# --- RAFX UNIVERSAL Ported Project
#
# ---------------------------------------------------------------------------------
project(${EXP_PROJECT_NAME})

if(UNIX)
	if(XCODE)
		set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++14") # --- NOTE: VST3 requires c++14!
		set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
	elseif(APPLE)
		set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
		link_libraries(c++)
	else()
		set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-multichar")
		link_libraries(stdc++fs pthread dl)
	endif()
elseif(WIN)
	# add_definitions(-D_UNICODE)
	add_compile_options(/fp:fast)
	add_compile_options($<$<CONFIG:Release>:/Oi>)	# Enable Intrinsic Functions (Yes)
	add_compile_options($<$<CONFIG:Release>:/Ot>)	# Favor Size Or Speed (Favor fast code)
	add_compile_options($<$<CONFIG:Release>:/GF>)	# Enable String Pooling
	add_compile_options($<$<CONFIG:Release>:/EHa>)	# Enable C++ Exceptions
	add_compile_options($<$<CONFIG:Release>:/Oy>)	# Omit Frame Pointers
	#add_compile_options($<$<CONFIG:Release>:/Ox>)	# Optimization (/O2: Maximise Speed /0x: Full Optimization)
endif()


# --- add various API projects
set(AAX_CMAKE_FOLDER cmake/aax_cmake)
set(AU_CMAKE_FOLDER cmake/au_cmake)
set(VST_CMAKE_FOLDER cmake/vst_cmake)

# --- you can rename the plugin output folders; VST is set to VST3 (see below)
set(AAX_PLUGIN_FOLDER AAX)
set(AU_PLUGIN_FOLDER AU)
# --- VST plugin folder is set to "VST3" inside of one of the vst cmake modules, which we don't want to mess with
#     set(VST_PLUGIN_FOLDER VST3)

# --- add each project:
if(AAX_SDK_BUILD)
	add_subdirectory(${AAX_CMAKE_FOLDER})
endif()

if(MAC AND AU_SDK_BUILD)
	add_subdirectory(${AU_CMAKE_FOLDER})
endif()

if(VST_SDK_BUILD)
	add_subdirectory(${VST_CMAKE_FOLDER})
endif()

