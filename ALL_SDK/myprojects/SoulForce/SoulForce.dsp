# Microsoft Developer Studio Project File - Name="SoulForce" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=SoulForce - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SoulForce.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SoulForce.mak" CFG="SoulForce - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SoulForce - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "SoulForce - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SoulForce - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /Gi /GX /O2 /I "../../vstsdk2.4" /I "../../vstsdk2.4/public.sdk/source/vst2.x" /I "../../vstsdk2.4/vstgui.sf/vstgui" /I "../../libpng-1.2.8" /I "../../zlib-1.2.3" /D "NDEBUG" /D "STRICT" /D "WIN32" /D "_WINDOWS" /D "USE_LIBPNG" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 libcpmt.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib msvcrt.lib libpng.lib zlib.lib /nologo /subsystem:windows /dll /incremental:yes /machine:I386 /nodefaultlib:"LIBCMT.lib libc.lib" /nodefaultlib /libpath:"C:\Program Files\DevStudio\MyProjects\VST Plugins\Common" /libpath:"../../libpng-1.2.8" /libpath:"../../zlib-1.2.3"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "SoulForce - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../../vstsdk2.4" /I "../../vstsdk2.4/public.sdk/source/vst2.x" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 libcpmtd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib msvcrt.lib /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"LIBCMT.lib libc.lib" /nodefaultlib /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "SoulForce - Win32 Release"
# Name "SoulForce - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Editor.cpp
# End Source File
# Begin Source File

SOURCE=.\Editor.h
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

SOURCE=..\..\vstsdk2.4\vstgui.sf\vstgui\aeffguieditor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\vstsdk2.4\vstgui.sf\vstgui\aeffguieditor.h
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

SOURCE=..\..\vstsdk2.4\vstgui.sf\vstgui\vstcontrols.cpp
# End Source File
# Begin Source File

SOURCE=..\..\vstsdk2.4\vstgui.sf\vstgui\vstcontrols.h
# End Source File
# Begin Source File

SOURCE=..\..\vstsdk2.4\vstgui.sf\vstgui\vstgui.cpp
# End Source File
# Begin Source File

SOURCE=..\..\vstsdk2.4\vstgui.sf\vstgui\vstgui.h
# End Source File
# Begin Source File

SOURCE=..\..\vstsdk2.4\public.sdk\source\vst2.x\vstplugmain.cpp
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\images\bmp00127.png
# End Source File
# Begin Source File

SOURCE=.\images\bmp00128.png
# End Source File
# Begin Source File

SOURCE=.\images\bmp00129.png
# End Source File
# Begin Source File

SOURCE=.\images\bmp00130.png
# End Source File
# Begin Source File

SOURCE=.\images\bmp00131.png
# End Source File
# Begin Source File

SOURCE=.\images\bmp00132.png
# End Source File
# Begin Source File

SOURCE=.\images\bmp00133.png
# End Source File
# Begin Source File

SOURCE=.\SoulForceResources.rc
# End Source File
# End Group
# End Target
# End Project
