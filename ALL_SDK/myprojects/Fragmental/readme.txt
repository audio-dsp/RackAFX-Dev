Fragmental Source Code
----------------------

Fragmental is a VST 2.4 plugin, and uses the version of VSTGUI included with the
VST 2.4 SDK (is that 3.0?).  You'll also need libpng and zlib to compile it, but
I think that's all there is as far as external libraries go.   Because of
Steinberg's stupid license, the phase vocoder code has to be built as a
separate library (NiallsPVOCLib), but the MSVC v6 workspace file has this
setup for you.  The phase vocoder code is not my own, but the work of various
people (see the file headings or the manual), and is licensed under the LGPL,
unlike my code, which is licensed under the MIT license.

I should point out that the routings editor is done in OpenGL, using my VSTGL
framework.  I've tried to keep my code as cross-platform as possible (well,
at least Windows and OSX), but the library-loading code is only written for
Windows, and would need porting to OSX if you're interested in building the
plugin on a Mac.

- Niall Moody (22/06/07).
