=============================================

AAX SDK 2.3.1

ReadMe.txt
Copyright 2014-2018 by Avid Technology, Inc.
All rights reserved.
	
CONFIDENTIAL: This document contains confidential information.  Do not 
read or examine this document unless you are an Avid Technology employee
or have signed a non-disclosure agreement with Avid Technology which protects
the	confidentiality of this document.  DO NOT DISCLOSE ANY INFORMATION 
CONTAINED IN THIS DOCUMENT TO ANY THIRD-PARTY WITHOUT THE PRIOR WRITTEN	CONSENT 
OF Avid Technology, INC.

=============================================

Welcome to AAX! Open the SDK's HTML documentation for complete documentation.

---
Quick Start

  1) Open the AAX Library project for your platform and IDE from Libs/AAXLibrary
  
     This is a static library containing base implementations of the AAX API. AAX plug-ins
     typically link to this library and override its classes.
  
  2) Compile the AAX Library using your desired build configuration
     
     On Mac, be sure to choose which version of the standard library you would like to
     link against:
     - for libstdc++ use the "AAXLibrary" target
     - for libc++ use the "AAXLibrary_libcpp" target (the default for example plug-ins)
     
  3) Open and compile the DemoGain example plug-in at ExamplePlugIns/DemoGain
  
  4) Install the resulting DemoGain_example.aaxplugin into the system AAX plug-ins
     directory
     
     Mac: /Library/Application Support/Avid/Audio/Plug-Ins
     Windows (32-bit plug-ins): C:\Program Files (x86)\Common Files\Avid\Audio\Plug-Ins
     Windows (64-bit plug-ins): C:\Program Files\Common Files\Avid\Audio\Plug-Ins
  
  5) Run the plug-in in an AAX host which supports unsigned AAX plug-ins, such as a
     Pro Tools developer build, Media Composer, or the DSH command-line shell

---
Usage Notes

* On Windows, if you place the SDK too deep in your file hierarchy you may encounter build
  failures due to the system's path length limitation. To avoid this issue, locate the SDK
  near the root of your drive.

---
Compatibility

This release of the AAX SDK has been tested with the following IDEs

  Visual Studio
    Visual Studio 2010
    Visual Studio 2013
    Visual Studio 2015

  Xcode
    Xcode 5.1.1
    Xcode 8
  
  TI Code Composer Studio
    CCS 7.1.0

As of this writing, the current version of Visual Studio 2017 (15.2 (26430.6)) is not
compatible due to warnings in STL classes. To use Visual Studio 2017 you may need to
disable "treat warnings as errors". This configuration has not been tested by Avid.

Although some Xcode 3 and Visual Studio 2008 projects are included, these projects and
IDEs have not been updated or tested with this SDK release.

---
Documentation Notes

The SDK documentation is also provided in PDF form. The PDF documentation is not
recommended for browsing, but can be useful for text-based searches.

If you wish to recompile the code-generated documentation:
  1) Download and install the latest version of Doxygen from www.doxygen.org
  2) (Optional for dot) Download and install the latest Graphviz tools from
     www.graphviz.org
  3) (Optional for MSC) Download and install the latest version of the mscgen tool
     from http://www.mcternan.me.uk/mscgen/
  4) Run Doxygen from the AAX_SDK/Documentation/Doxygen directory using the included
     AAX_SDK_Doxyfile config file. On Mac, simply run the generate_aax_sdk_doxygen.sh
     script to re-compile and format the documentation. This script requires that the
     doxygen command-line program is included in your PATH variable.
