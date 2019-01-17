# Microsoft Developer Studio Project File - Name="SimpleColourOrgan" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=SimpleColourOrgan - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SimpleColourOrgan.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SimpleColourOrgan.mak" CFG="SimpleColourOrgan - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SimpleColourOrgan - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "SimpleColourOrgan - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "SimpleColourOrgan - Win32 VSync" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SimpleColourOrgan - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /Gi /GX /O2 /I "VSTGL" /I "tinyxml" /I "../../vstsdk2.4" /I "../../vstsdk2.4/public.sdk/source/vst2.x" /D "NDEBUG" /D "STRICT" /D "WIN32" /D "_WINDOWS" /D "TIXML_USE_STL" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 msvcrt.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib opengl32.lib glu32.lib libcpmt.lib /nologo /subsystem:windows /dll /incremental:yes /machine:I386 /libpath:"C:\Program Files\DevStudio\MyProjects\VST Plugins\Common"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "SimpleColourOrgan - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "VSTGL" /I "tinyxml" /I "../../vstsdk2.4" /I "../../vstsdk2.4/public.sdk/source/vst2.x" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "TIXML_USE_STL" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 msvcrt.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib opengl32.lib glu32.lib libcpmtd.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "SimpleColourOrgan - Win32 VSync"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "SimpleColourOrgan___Win32_VSync"
# PROP BASE Intermediate_Dir "SimpleColourOrgan___Win32_VSync"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "SimpleColourOrgan___Win32_VSync"
# PROP Intermediate_Dir "SimpleColourOrgan___Win32_VSync"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /Gi /GX /O2 /I "VSTGL" /I "tinyxml" /I "../../vstsdk2.4" /I "../../vstsdk2.4/public.sdk/source/vst2.x" /D "NDEBUG" /D "STRICT" /D "WIN32" /D "_WINDOWS" /D "TIXML_USE_STL" /YX /FD /c
# ADD CPP /nologo /MD /W3 /Gi /GX /O2 /I "VSTGL" /I "tinyxml" /I "../../vstsdk2.4" /I "../../vstsdk2.4/public.sdk/source/vst2.x" /D "NDEBUG" /D "STRICT" /D "WIN32" /D "_WINDOWS" /D "TIXML_USE_STL" /D "VSYNC" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 msvcrt.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib opengl32.lib glu32.lib libcpmt.lib /nologo /subsystem:windows /dll /incremental:yes /machine:I386 /libpath:"C:\Program Files\DevStudio\MyProjects\VST Plugins\Common"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 msvcrt.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib opengl32.lib glu32.lib libcpmt.lib /nologo /subsystem:windows /dll /incremental:yes /machine:I386 /out:"SimpleColourOrgan___Win32_VSync/SimpleColourOrgan-VSync.dll" /libpath:"C:\Program Files\DevStudio\MyProjects\VST Plugins\Common"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "SimpleColourOrgan - Win32 Release"
# Name "SimpleColourOrgan - Win32 Debug"
# Name "SimpleColourOrgan - Win32 VSync"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Colour.cpp
# End Source File
# Begin Source File

SOURCE=.\Colour.h
# End Source File
# Begin Source File

SOURCE=.\ColourNote.cpp
# End Source File
# Begin Source File

SOURCE=.\ColourNote.h
# End Source File
# Begin Source File

SOURCE=.\ColourOrganGui.cpp
# End Source File
# Begin Source File

SOURCE=.\ColourOrganGui.h
# End Source File
# Begin Source File

SOURCE=.\CriticalSection.cpp
# End Source File
# Begin Source File

SOURCE=.\CriticalSection.h
# End Source File
# Begin Source File

SOURCE=.\PaletteFile.h
# End Source File
# Begin Source File

SOURCE=.\vstplug.def
# End Source File
# Begin Source File

SOURCE=.\VstPlugin.cpp
# End Source File
# Begin Source File

SOURCE=.\VstPlugin.h
# End Source File
# End Group
# Begin Group "VST Stuff"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\vstsdk2.4\public.sdk\source\vst2.x\aeffeditor.h
# End Source File
# Begin Source File

SOURCE=..\..\vstsdk2.4\public.sdk\source\vst2.x\audioeffect.cpp
# End Source File
# Begin Source File

SOURCE=..\..\vstsdk2.4\public.sdk\source\vst2.x\audioeffect.h
# End Source File
# Begin Source File

SOURCE=..\..\vstsdk2.4\public.sdk\source\vst2.x\audioeffectx.cpp
# End Source File
# Begin Source File

SOURCE=..\..\vstsdk2.4\public.sdk\source\vst2.x\audioeffectx.h
# End Source File
# Begin Source File

SOURCE=..\..\vstsdk2.4\public.sdk\source\vst2.x\vstplugmain.cpp
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter ""
# End Group
# Begin Group "VSTGL"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\VSTGL\VSTGLEditor.cpp
# End Source File
# Begin Source File

SOURCE=.\VSTGL\VSTGLEditor.h
# End Source File
# Begin Source File

SOURCE=.\VSTGL\VSTGLTimer.cpp
# End Source File
# Begin Source File

SOURCE=.\VSTGL\VSTGLTimer.h
# End Source File
# End Group
# Begin Group "tinyxml"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\tinyxml\tinystr.cpp
# End Source File
# Begin Source File

SOURCE=.\tinyxml\tinystr.h
# End Source File
# Begin Source File

SOURCE=.\tinyxml\tinyxml.cpp
# End Source File
# Begin Source File

SOURCE=.\tinyxml\tinyxml.h
# End Source File
# Begin Source File

SOURCE=.\tinyxml\tinyxmlerror.cpp
# End Source File
# Begin Source File

SOURCE=.\tinyxml\tinyxmlparser.cpp
# End Source File
# End Group
# End Target
# End Project
