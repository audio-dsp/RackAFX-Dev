# Microsoft Developer Studio Project File - Name="AmplitudeImposer" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=AmplitudeImposer - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "AmplitudeImposer.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "AmplitudeImposer.mak" CFG="AmplitudeImposer - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "AmplitudeImposer - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "AmplitudeImposer - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "AmplitudeImposer - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /Gi /GX /O2 /I "D:\Program Files\Microsoft Visual Studio\MyProjects" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "STRICT" /D "VSTGUI3" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib vstgui.lib oldnames.lib msvcrt.lib /nologo /subsystem:windows /dll /incremental:yes /machine:I386 /nodefaultlib:"LIBCMT.lib libc.lib" /nodefaultlib /def:".\AmplitudeImposer.def" /libpath:"C:\Program Files\DevStudio\MyProjects\VST Plugins\Common"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "AmplitudeImposer - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib vstgui.lib oldnames.lib msvcrt.lib /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"LIBCMT.lib libc.lib" /nodefaultlib /def:".\AmplitudeImposer.def" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "AmplitudeImposer - Win32 Release"
# Name "AmplitudeImposer - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AmplitudeImposer.cpp
# PROP Exclude_From_Scan -1
# PROP BASE Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\AmplitudeImposer.def
# PROP Exclude_From_Scan -1
# PROP BASE Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\AmplitudeImposer.h
# End Source File
# Begin Source File

SOURCE=.\AmplitudeImposerEditor.cpp
# PROP Exclude_From_Scan -1

!IF  "$(CFG)" == "AmplitudeImposer - Win32 Release"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "AmplitudeImposer - Win32 Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\AmplitudeImposerEditor.h

!IF  "$(CFG)" == "AmplitudeImposer - Win32 Release"

!ELSEIF  "$(CFG)" == "AmplitudeImposer - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\AmplitudeImposerMain.cpp
# PROP Exclude_From_Scan -1
# PROP BASE Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE="..\GUI Building Blocks\RightClickControls.cpp"
# End Source File
# Begin Source File

SOURCE="..\GUI Building Blocks\RightClickControls.h"
# End Source File
# End Group
# Begin Group "VST Stuff"

# PROP Default_Filter ""
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

SOURCE=..\..\vstgui_3_0_beta2\aeffguieditor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\vstgui_3_0_beta2\aeffguieditor.h
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
# Begin Source File

SOURCE=..\..\vstgui_3_0_beta2\cscrollview.cpp
# End Source File
# Begin Source File

SOURCE=..\..\vstgui_3_0_beta2\cscrollview.h
# End Source File
# Begin Source File

SOURCE=..\..\vstgui_3_0_beta2\vstcontrols.cpp
# End Source File
# Begin Source File

SOURCE=..\..\vstgui_3_0_beta2\vstcontrols.h
# End Source File
# Begin Source File

SOURCE=..\..\vstgui_3_0_beta2\vstgui.cpp
# End Source File
# Begin Source File

SOURCE=..\..\vstgui_3_0_beta2\vstgui.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter ""
# End Group
# Begin Source File

SOURCE=.\AmplitudeImposer.rc
# End Source File
# Begin Source File

SOURCE=.\images\back.bmp
# End Source File
# Begin Source File

SOURCE=.\images\slider.bmp
# End Source File
# End Target
# End Project
