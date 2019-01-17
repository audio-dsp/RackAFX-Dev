ndc Plugs MyFirstStepSequencer VST plugin Source Code
-----------------------------------------------------

This zipfile should contain all the source code for the MyFirstStepSequencer
plugin, including project files for MSVC v6, and XCode v1.5.  To compile it,
you will need some extra libraries/header files:

Both platforms:
VSTSDK: (get it from: 
http://www.steinberg.de/DocSupportDisplay_sbf7ce.html?templ=&doclink=/webvideo/Steinberg/support/doc/VST%20Audio%20Plug-Ins%20SDK_1.htm )
VSTGUI v3 (get it from: http://vstgui.sourceforge.net/ )

Windows Only:
ZLib (get it from: http://www.zlib.net/ )
libPNG (get it from: http://www.libpng.org/pub/png/libpng.html )

I should probably have included vstgui, zlib and libpng, since their licenses
allow it, but my hosting space/bandwidth's limited, so you'll have to get them
for yourselves.

Most of the code's documented using doxygen comments (I think I left out the
AudioEffectX class), so you should be able to run doxygen on it to get some
nicely-formatted html documentation, but it's not particularly complicated code
anyway.

The software's licensed using the MIT/expat license (below) - I would have
preferred to use the GPL, but as far as I can tell (I'm no lawyer), neither of
the GPLs are compatible with Steinberg's license for the VST SDK (because they
don't let you re-distribute their source code).



Copyright (c) 2005 Niall Moody

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
