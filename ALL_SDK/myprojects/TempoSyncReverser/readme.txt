ndc Plugs Tempo Sync Reverser VST plugin Source Code
----------------------------------------------------

Ok, so everything you need to compile the plugin should be here - source files, images, project
files.  The project files are for MSVC v6 on Windows, and Project Builder on OS X.  You'll need
to have the VST SDK as well - I'm not allowed to re-distribute that, but you can get it from: 
http://www.steinberg.de/DocSupportDisplay_sb.asp?templ=&doclink=/webvideo/steinberg/support/doc/VST%20Audio%20Plug-Ins%20SDK_1.htm

(by the way, I just used the version of VSTGUI that comes with the SDK, since I didn't need
fancy png graphics etc.)

The software's licensed using the MIT/expat license (below) - I would have preferred to use the
GPL, but as far as I can tell (I'm no lawyer), neither of the GPL's are compatible with
Steinberg's license for the VST SDK (because they don't let you re-distribute their source
code).



Copyright (c) 2005-2006 Niall Moody

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the
following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
