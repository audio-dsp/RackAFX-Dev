# Microsoft Developer Studio Project File - Name="TempoSyncReverser" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=TempoSyncReverser - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TempoSyncReverser.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TempoSyncReverser.mak" CFG="TempoSyncReverser - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TempoSyncReverser - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TempoSyncReverser - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TempoSyncReverser - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /Gi /GX /O2 /I "../../vstsdk2.3/source/common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "STRICT" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib msvcrt.lib oldnames.lib /nologo /subsystem:windows /dll /incremental:yes /machine:I386 /nodefaultlib:"LIBCMT.lib libc.lib" /nodefaultlib /def:".\TempoSyncReverser.def" /libpath:"C:\Program Files\DevStudio\MyProjects\VST Plugins\Common"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "TempoSyncReverser - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../../vstsdk2.3/source/common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib msvcrt.lib oldnames.lib /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"LIBCMT.lib libc.lib" /nodefaultlib /def:".\TempoSyncReverser.def" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "TempoSyncReverser - Win32 Release"
# Name "TempoSyncReverser - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\TempoSyncReverser.cpp
# PROP Exclude_From_Scan -1
# PROP BASE Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\TempoSyncReverser.def
# PROP Exclude_From_Scan -1
# PROP BASE Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\TempoSyncReverser.h
# End Source File
# Begin Source File

SOURCE=.\TempoSyncReverserEditor.cpp
# PROP Exclude_From_Scan -1
# PROP BASE Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\TempoSyncReverserEditor.h
# End Source File
# Begin Source File

SOURCE=.\TempoSyncReverserMain.cpp
# PROP Exclude_From_Scan -1
# PROP BASE Exclude_From_Build 1
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

SOURCE=..\..\vstsdk2.3\source\common\aeffguieditor.cpp
# End Source File
# Begin Source File

SOURCE=..\Common\aeffguieditor.h
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

SOURCE=..\Common\vstcontrols.cpp
# End Source File
# Begin Source File

SOURCE=..\Common\vstcontrols.h
# End Source File
# Begin Source File

SOURCE=..\Common\vstgui.cpp
# End Source File
# Begin Source File

SOURCE=..\Common\vstgui.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\images\1.bmp
# End Source File
# Begin Source File

SOURCE=.\images\1_12.bmp
# End Source File
# Begin Source File

SOURCE=.\images\1_16.bmp
# End Source File
# Begin Source File

SOURCE=.\images\1_2.bmp
# End Source File
# Begin Source File

SOURCE=.\images\1_3.bmp
# End Source File
# Begin Source File

SOURCE=.\images\1_32.bmp
# End Source File
# Begin Source File

SOURCE=.\images\1_4.bmp
# End Source File
# Begin Source File

SOURCE=.\images\1_6.bmp
# End Source File
# Begin Source File

SOURCE=.\images\1_8.bmp
# End Source File
# Begin Source File

SOURCE=.\images\2.bmp
# End Source File
# Begin Source File

SOURCE=.\images\3.bmp
# End Source File
# Begin Source File

SOURCE=.\images\4.bmp
# End Source File
# Begin Source File

SOURCE=.\images\Back.bmp
# End Source File
# Begin Source File

SOURCE=.\images\Reverse.bmp
# End Source File
# Begin Source File

SOURCE=.\TempoSyncReverser.rc
# End Source File
# Begin Source File

SOURCE=.\images\TickBox.bmp
# End Source File
# End Group
# End Target
# End Project
