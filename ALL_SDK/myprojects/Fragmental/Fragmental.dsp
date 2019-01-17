# Microsoft Developer Studio Project File - Name="Fragmental" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Fragmental - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Fragmental.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Fragmental.mak" CFG="Fragmental - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Fragmental - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Fragmental - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Fragmental - Win32 PresetSaver" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Fragmental - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /Gi /GR /GX /O2 /Ob2 /I "../" /I "modSources" /I "effects" /I "effects/NiallsPVOC/pv_common/include" /I "effects/NiallsPVOC" /I "../../vstsdk2.4" /I "../../vstsdk2.4/public.sdk/source/vst2.x" /I "../../libpng" /I "../../zlib" /I "../../vstsdk2.4/vstgui.sf/vstgui" /I "MultiGUI" /I "VSTGL" /D "NDEBUG" /D "STRICT" /D "WIN32" /D "_WINDOWS" /D "USE_LIBPNG" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 libcpmt.lib msvcrt.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libpng.lib zlib.lib opengl32.lib glu32.lib /nologo /subsystem:windows /dll /machine:I386 /libpath:"../../libpng" /libpath:"../../zlib"
# SUBTRACT LINK32 /profile /map /debug /nodefaultlib

!ELSEIF  "$(CFG)" == "Fragmental - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "modSources" /I "effects" /I "effects/NiallsPVOC/pv_common/include" /I "effects/NiallsPVOC" /I "../../vstsdk2.4" /I "../../vstsdk2.4/public.sdk/source/vst2.x" /I "../../libpng" /I "../../zlib" /I "../../vstsdk2.4/vstgui.sf/vstgui" /I "MultiGUI" /I "VSTGL" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "USE_LIBPNG" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 libcpmtd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libpng.lib zlib.lib opengl32.lib glu32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /libpath:"../../libpng" /libpath:"../../zlib"
# SUBTRACT LINK32 /profile /map /nodefaultlib

!ELSEIF  "$(CFG)" == "Fragmental - Win32 PresetSaver"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Fragmental___Win32_PresetSaver"
# PROP BASE Intermediate_Dir "Fragmental___Win32_PresetSaver"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Fragmental___Win32_PresetSaver"
# PROP Intermediate_Dir "Fragmental___Win32_PresetSaver"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /Gi /GR /GX /O2 /Ob2 /I "../" /I "modSources" /I "effects" /I "effects/pv_common/include" /I "effects/NiallsPVOC" /I "../../vstsdk2.4" /I "../../vstsdk2.4/public.sdk/source/vst2.x" /I "../../libpng" /I "../../zlib" /I "../../vstsdk2.4/vstgui.sf/vstgui" /I "MultiGUI" /I "VSTGL" /D "NDEBUG" /D "STRICT" /D "WIN32" /D "_WINDOWS" /D "USE_LIBPNG" /YX /FD /c
# ADD CPP /nologo /MD /W3 /Gi /GR /GX /O2 /Ob2 /I "../" /I "modSources" /I "effects" /I "effects/NiallsPVOC/pv_common/include" /I "effects/NiallsPVOC" /I "../../vstsdk2.4" /I "../../vstsdk2.4/public.sdk/source/vst2.x" /I "../../libpng" /I "../../zlib" /I "../../vstsdk2.4/vstgui.sf/vstgui" /I "MultiGUI" /I "VSTGL" /D "NDEBUG" /D "STRICT" /D "WIN32" /D "_WINDOWS" /D "USE_LIBPNG" /D "PRESETSAVER" /D "VST" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 libcpmt.lib msvcrt.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libpng.lib zlib.lib opengl32.lib glu32.lib /nologo /subsystem:windows /dll /machine:I386 /libpath:"../../libpng" /libpath:"../../zlib"
# SUBTRACT BASE LINK32 /profile /map /debug /nodefaultlib
# ADD LINK32 libcpmt.lib msvcrt.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libpng.lib zlib.lib opengl32.lib glu32.lib /nologo /subsystem:windows /dll /machine:I386 /libpath:"../../libpng" /libpath:"../../zlib"
# SUBTRACT LINK32 /profile /map /debug /nodefaultlib

!ENDIF 

# Begin Target

# Name "Fragmental - Win32 Release"
# Name "Fragmental - Win32 Debug"
# Name "Fragmental - Win32 PresetSaver"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Group "effects"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\effects\Accum.cpp
# End Source File
# Begin Source File

SOURCE=.\effects\Accum.h
# End Source File
# Begin Source File

SOURCE=.\effects\Delay.cpp
# End Source File
# Begin Source File

SOURCE=.\effects\Delay.h
# End Source File
# Begin Source File

SOURCE=.\effects\Exag.cpp
# End Source File
# Begin Source File

SOURCE=.\effects\Exag.h
# End Source File
# Begin Source File

SOURCE=.\effects\Grain.cpp
# End Source File
# Begin Source File

SOURCE=.\effects\Grain.h
# End Source File
# Begin Source File

SOURCE=.\effects\Granulator.cpp
# End Source File
# Begin Source File

SOURCE=.\effects\Granulator.h
# End Source File
# Begin Source File

SOURCE=.\effects\Reverse.cpp
# End Source File
# Begin Source File

SOURCE=.\effects\Reverse.h
# End Source File
# Begin Source File

SOURCE=.\effects\Transpose.cpp
# End Source File
# Begin Source File

SOURCE=.\effects\Transpose.h
# End Source File
# End Group
# Begin Group "GUI"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CButtonMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\CButtonMenu.h
# End Source File
# Begin Source File

SOURCE=.\EffectsEditor.cpp
# End Source File
# Begin Source File

SOURCE=.\EffectsEditor.h
# End Source File
# Begin Source File

SOURCE=.\EnvGui.cpp
# End Source File
# Begin Source File

SOURCE=.\EnvGui.h
# End Source File
# Begin Source File

SOURCE=.\KnobGui.cpp
# End Source File
# Begin Source File

SOURCE=.\KnobGui.h
# End Source File
# Begin Source File

SOURCE=.\LFOBarEditor.cpp
# End Source File
# Begin Source File

SOURCE=.\LFOBarEditor.h
# End Source File
# Begin Source File

SOURCE=.\LfoGui.cpp
# End Source File
# Begin Source File

SOURCE=.\LfoGui.h
# End Source File
# Begin Source File

SOURCE=.\MainEditor.cpp
# End Source File
# Begin Source File

SOURCE=.\MainEditor.h
# End Source File
# Begin Source File

SOURCE=.\ModContainer.cpp
# End Source File
# Begin Source File

SOURCE=.\ModContainer.h
# End Source File
# Begin Source File

SOURCE=.\ModEditor.cpp
# End Source File
# Begin Source File

SOURCE=.\ModEditor.h
# End Source File
# Begin Source File

SOURCE=.\ModSourceGui.h
# End Source File
# Begin Source File

SOURCE=.\ModWheel.cpp
# End Source File
# Begin Source File

SOURCE=.\ModWheel.h
# End Source File
# Begin Source File

SOURCE=.\MultiGUI\MultiGUIEditor.cpp
# End Source File
# Begin Source File

SOURCE=.\MultiGUI\MultiGUIEditor.h
# End Source File
# Begin Source File

SOURCE=.\RoutingsEditor.cpp
# End Source File
# Begin Source File

SOURCE=.\RoutingsEditor.h
# End Source File
# Begin Source File

SOURCE=.\SixPointLineEditor.cpp
# End Source File
# Begin Source File

SOURCE=.\SixPointLineEditor.h
# End Source File
# Begin Source File

SOURCE=.\TwoTickKnob.cpp
# End Source File
# Begin Source File

SOURCE=.\TwoTickKnob.h
# End Source File
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
# Begin Group "modSources"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\modSources\EnvelopeFollower.cpp
# End Source File
# Begin Source File

SOURCE=.\modSources\EnvelopeFollower.h
# End Source File
# Begin Source File

SOURCE=.\modSources\LFO.cpp
# End Source File
# Begin Source File

SOURCE=.\modSources\LFO.h
# End Source File
# Begin Source File

SOURCE=.\modSources\ModType.h
# End Source File
# Begin Source File

SOURCE=.\modSources\MultiModSource.cpp
# End Source File
# Begin Source File

SOURCE=.\modSources\MultiModSource.h
# End Source File
# Begin Source File

SOURCE=.\modSources\PhysModKnob.cpp
# End Source File
# Begin Source File

SOURCE=.\modSources\PhysModKnob.h
# End Source File
# Begin Source File

SOURCE=.\modSources\SawTest.cpp
# End Source File
# Begin Source File

SOURCE=.\modSources\SawTest.h
# End Source File
# Begin Source File

SOURCE=.\modSources\SineTest.cpp
# End Source File
# Begin Source File

SOURCE=.\modSources\SineTest.h
# End Source File
# Begin Source File

SOURCE=.\modSources\SquareTest.cpp
# End Source File
# Begin Source File

SOURCE=.\modSources\SquareTest.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\CriticalSection.cpp
# End Source File
# Begin Source File

SOURCE=.\CriticalSection.h
# End Source File
# Begin Source File

SOURCE=.\EffectBase.cpp
# End Source File
# Begin Source File

SOURCE=.\EffectBase.h
# End Source File
# Begin Source File

SOURCE=.\HelperStuff.h
# End Source File
# Begin Source File

SOURCE=.\LibraryLoader.cpp
# End Source File
# Begin Source File

SOURCE=.\LibraryLoader.h
# End Source File
# Begin Source File

SOURCE=.\ModulationManager.cpp
# End Source File
# Begin Source File

SOURCE=.\ModulationManager.h
# End Source File
# Begin Source File

SOURCE=.\ModulationSource.cpp
# End Source File
# Begin Source File

SOURCE=.\ModulationSource.h
# End Source File
# Begin Source File

SOURCE=.\ParameterManager.cpp
# End Source File
# Begin Source File

SOURCE=.\ParameterManager.h
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
# Begin Source File

SOURCE=.\VstPluginPresets.cpp
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

SOURCE=.\images\bmp00134.png
# End Source File
# Begin Source File

SOURCE=.\images\bmp00227.png
# End Source File
# Begin Source File

SOURCE=.\images\bmp00228.png
# End Source File
# Begin Source File

SOURCE=.\images\bmp00229.png
# End Source File
# Begin Source File

SOURCE=.\images\bmp00230.png
# End Source File
# Begin Source File

SOURCE=.\images\bmp00231.png
# End Source File
# Begin Source File

SOURCE=.\images\bmp00232.png
# End Source File
# Begin Source File

SOURCE=.\images\bmp00233.png
# End Source File
# Begin Source File

SOURCE=.\images\bmp00234.png
# End Source File
# Begin Source File

SOURCE=.\images\bmp00235.png
# End Source File
# Begin Source File

SOURCE=.\images\bmp00236.png
# End Source File
# Begin Source File

SOURCE=.\FragmentalResources.rc
# End Source File
# End Group
# End Target
# End Project
