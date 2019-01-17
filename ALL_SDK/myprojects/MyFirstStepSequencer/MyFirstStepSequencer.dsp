# Microsoft Developer Studio Project File - Name="MyFirstStepSequencer" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=MyFirstStepSequencer - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MyFirstStepSequencer.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MyFirstStepSequencer.mak" CFG="MyFirstStepSequencer - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MyFirstStepSequencer - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "MyFirstStepSequencer - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MyFirstStepSequencer - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /Gi /GX /O2 /I "../../vstgui_3_0_beta4" /I "../common" /I "../../libpng-1.2.8" /I "../../zlib-1.2.3" /I "widgets" /I "." /D "NDEBUG" /D "STRICT" /D "WIN32" /D "_WINDOWS" /D "USE_LIBPNG" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 msvcrt.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libpng.lib zlib.lib oldnames.lib /nologo /subsystem:windows /dll /incremental:yes /machine:I386 /nodefaultlib:"LIBCMT.lib libc.lib" /nodefaultlib /def:".\MyFirstStepSequencer.def" /libpath:"../Common" /libpath:"../../libpng-1.2.8" /libpath:"../../zlib-1.2.3"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "MyFirstStepSequencer - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /ZI /Od /I "../../vstgui_3_0_beta4" /I "../common" /I "../../libpng-1.2.8" /I "../../zlib-1.2.3" /I "widgets" /I "." /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "USE_LIBPNG" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib ole32.lib oleaut32.lib odbc32.lib odbccp32.lib oldnames.lib msvcrt.lib libpng.lib zlib.lib /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"LIBCMT.lib libc.lib" /nodefaultlib /def:".\MyFirstStepSequencer.def" /pdbtype:sept /libpath:"../../libpng-1.2.8" /libpath:"../../zlib-1.2.3"
# SUBTRACT LINK32 /pdb:none /incremental:no

!ENDIF 

# Begin Target

# Name "MyFirstStepSequencer - Win32 Release"
# Name "MyFirstStepSequencer - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Group "Extra Widgets"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\widgets\CSimpleKeyboard.cpp
# End Source File
# Begin Source File

SOURCE=.\widgets\CSimpleKeyboard.h
# End Source File
# Begin Source File

SOURCE=.\widgets\CSimpleKeyboardSwitch.cpp
# End Source File
# Begin Source File

SOURCE=.\widgets\CSimpleKeyboardSwitch.h
# End Source File
# Begin Source File

SOURCE=.\widgets\NiallsAbout.cpp
# End Source File
# Begin Source File

SOURCE=.\widgets\NiallsAbout.h
# End Source File
# Begin Source File

SOURCE=.\widgets\StateButton.cpp
# End Source File
# Begin Source File

SOURCE=.\widgets\StateButton.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\MFSSGui.cpp
# End Source File
# Begin Source File

SOURCE=.\MFSSGui.h
# End Source File
# Begin Source File

SOURCE=.\MyFirstStepSequencer.cpp
# PROP Exclude_From_Scan -1
# PROP BASE Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\MyFirstStepSequencer.def
# PROP Exclude_From_Scan -1
# PROP BASE Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\MyFirstStepSequencer.h
# End Source File
# Begin Source File

SOURCE=.\MyFirstStepSequencerMain.cpp
# PROP Exclude_From_Scan -1
# PROP BASE Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Round.h
# End Source File
# Begin Source File

SOURCE=.\Step.cpp
# End Source File
# Begin Source File

SOURCE=.\Step.h
# End Source File
# End Group
# Begin Group "VST Stuff"

# PROP Default_Filter ""
# Begin Group "VSTGUI beta4"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\vstgui_3_0_beta4\aeffguieditor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\vstgui_3_0_beta4\aeffguieditor.h
# End Source File
# Begin Source File

SOURCE=..\..\vstgui_3_0_beta4\vstcontrols.cpp
# End Source File
# Begin Source File

SOURCE=..\..\vstgui_3_0_beta4\vstcontrols.h
# End Source File
# Begin Source File

SOURCE=..\..\vstgui_3_0_beta4\vstgui.cpp
# End Source File
# Begin Source File

SOURCE=..\..\vstgui_3_0_beta4\vstgui.h
# End Source File
# Begin Source File

SOURCE=..\..\vstgui_3_0_beta4\vstkeycode.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\Common\AEffect.h
# End Source File
# Begin Source File

SOURCE=..\Common\aeffectx.h
# End Source File
# Begin Source File

SOURCE=..\Common\AEffEditor.hpp
# End Source File
# Begin Source File

SOURCE=..\Common\AudioEffect.cpp
# End Source File
# Begin Source File

SOURCE=..\Common\AudioEffect.hpp
# End Source File
# Begin Source File

SOURCE=..\Common\audioeffectx.cpp
# End Source File
# Begin Source File

SOURCE=..\Common\audioeffectx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\images\about_box.png
# End Source File
# Begin Source File

SOURCE=.\images\advanced_hidden2.png
# End Source File
# Begin Source File

SOURCE=.\images\advanced_shown2.png
# End Source File
# Begin Source File

SOURCE=.\images\arrow.png
# End Source File
# Begin Source File

SOURCE=.\images\background.png
# End Source File
# Begin Source File

SOURCE=.\images\flower.png
# End Source File
# Begin Source File

SOURCE=.\MyFirstStepSequencer.rc
# End Source File
# Begin Source File

SOURCE=.\images\note.png
# End Source File
# Begin Source File

SOURCE=.\images\notes_heads.png
# End Source File
# Begin Source File

SOURCE=.\images\notes_heads3.png
# End Source File
# Begin Source File

SOURCE=.\images\num_notes.png
# End Source File
# Begin Source File

SOURCE=.\images\octave.png
# End Source File
# Begin Source File

SOURCE=.\images\question_mark.png
# End Source File
# Begin Source File

SOURCE=.\images\random.png
# End Source File
# Begin Source File

SOURCE=.\images\shuffle_track.png
# End Source File
# Begin Source File

SOURCE=.\images\signal.png
# End Source File
# End Group
# End Target
# End Project
