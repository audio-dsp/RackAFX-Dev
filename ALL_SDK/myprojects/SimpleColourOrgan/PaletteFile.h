//	PaletteFile.h - The Contents of the palette file to write to the user's
//					home directory.
//	--------------------------------------------------------------------------
//	Copyright (c) 2006 Niall Moody
//
//	Permission is hereby granted, free of charge, to any person obtaining a
//	copy of this software and associated documentation files (the "Software"),
//	to deal in the Software without restriction, including without limitation
//	the rights to use, copy, modify, merge, publish, distribute, sublicense,
//	and/or sell copies of the Software, and to permit persons to whom the
//	Software is furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//	THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//	DEALINGS IN THE SOFTWARE.
//	--------------------------------------------------------------------------

#ifndef PALETTEFILE_H_
#define PALETTEFILE_H_

///	Have to split it into 2 parts, because MSVC doesn't like strings greater
///	than 2048 characters...
const char *paletteFilePart1 = "<\?xml version=\"1.0\" encoding=\"ISO-8859-1\"\?>\n\
\n\
<SimpleColourOrgan>\n\
	<Palette name=\"A. Wallace Rimington (Black=silent)\">\n\
		<silent col=\"000000\"/>\n\
		<C col=\"FA0B0C\"/>\n\
		<Cs col=\"A00C09\"/>\n\
		<D col=\"F44712\"/>\n\
		<Ds col=\"F88010\"/>\n\
		<E col=\"F5F43C\"/>\n\
		<F col=\"709226\"/>\n\
		<Fs col=\"149033\"/>\n\
		<G col=\"27A481\"/>\n\
		<Gs col=\"1B9081\"/>\n\
		<A col=\"1C0D82\"/>\n\
		<As col=\"7F087C\"/>\n\
		<B col=\"D71386\"/>\n\
	</Palette>\n\
	<Palette name=\"A. Wallace Rimington (White=silent)\">\n\
		<silent col=\"FFFFFF\"/>\n\
		<C col=\"FA0B0C\"/>\n\
		<Cs col=\"A00C09\"/>\n\
		<D col=\"F44712\"/>\n\
		<Ds col=\"F88010\"/>\n\
		<E col=\"F5F43C\"/>\n\
		<F col=\"709226\"/>\n\
		<Fs col=\"149033\"/>\n\
		<G col=\"27A481\"/>\n\
		<Gs col=\"1B9081\"/>\n\
		<A col=\"1C0D82\"/>\n\
		<As col=\"7F087C\"/>\n\
		<B col=\"D71386\"/>\n\
	</Palette>\n\
	<Palette name=\"Alexander Scriabin (Black=silent)\">\n\
		<silent col=\"000000\"/>\n\
		<C col=\"FA0B0C\"/>\n\
		<Cs col=\"D71386\"/>\n\
		<D col=\"F5F43C\"/>\n\
		<Ds col=\"5A5685\"/>\n\
		<E col=\"1C5BA0\"/>\n\
		<F col=\"A00C09\"/>\n\
		<Fs col=\"1C0D82\"/>\n\
		<G col=\"F88010\"/>\n\
		<Gs col=\"7F087C\"/>\n\
		<A col=\"149033\"/>\n\
		<As col=\"5A5685\"/>\n\
		<B col=\"1C5BA0\"/>\n\
	</Palette>\n\
	<Palette name=\"Alexander Scriabin (White=silent)\">\n\
		<silent col=\"FFFFFF\"/>\n\
		<C col=\"FA0B0C\"/>\n\
		<Cs col=\"D71386\"/>\n\
		<D col=\"F5F43C\"/>\n\
		<Ds col=\"5A5685\"/>\n\
		<E col=\"1C5BA0\"/>\n\
		<F col=\"A00C09\"/>\n\
		<Fs col=\"1C0D82\"/>\n\
		<G col=\"F88010\"/>\n\
		<Gs col=\"7F087C\"/>\n\
		<A col=\"149033\"/>\n\
		<As col=\"5A5685\"/>\n\
		<B col=\"1C5BA0\"/>\n\
	</Palette>\n\
";

const char *paletteFilePart2 = "	<Palette name=\"Louis Bertrand Castel (Black=silent)\">\n\
		<silent col=\"000000\"/>\n\
		<C col=\"1C0D82\"/>\n\
		<Cs col=\"1B9081\"/>\n\
		<D col=\"149033\"/>\n\
		<Ds col=\"709226\"/>\n\
		<E col=\"F5F43C\"/>\n\
		<F col=\"F5D23B\"/>\n\
		<Fs col=\"F88010\"/>\n\
		<G col=\"FA0B0C\"/>\n\
		<Gs col=\"A00C09\"/>\n\
		<A col=\"D71386\"/>\n\
		<As col=\"4B0E7D\"/>\n\
		<B col=\"7F087C\"/>\n\
	</Palette>\n\
	<Palette name=\"Louis Bertrand Castel (White=silent)\">\n\
		<silent col=\"FFFFFF\"/>\n\
		<C col=\"1C0D82\"/>\n\
		<Cs col=\"1B9081\"/>\n\
		<D col=\"149033\"/>\n\
		<Ds col=\"709226\"/>\n\
		<E col=\"F5F43C\"/>\n\
		<F col=\"F5D23B\"/>\n\
		<Fs col=\"F88010\"/>\n\
		<G col=\"FA0B0C\"/>\n\
		<Gs col=\"A00C09\"/>\n\
		<A col=\"D71386\"/>\n\
		<As col=\"4B0E7D\"/>\n\
		<B col=\"7F087C\"/>\n\
	</Palette>\n\
	<Palette name=\"Niall Moody Pastel\">\n\
		<silent col=\"FFFFFF\"/>\n\
		<C col=\"FFCBCB\"/>\n\
		<Cs col=\"FFCBE2\"/>\n\
		<D col=\"FFCBFB\"/>\n\
		<Ds col=\"E2CBFF\"/>\n\
		<E col=\"CBD0FF\"/>\n\
		<F col=\"CBE2FF\"/>\n\
		<Fs col=\"CBF7FF\"/>\n\
		<G col=\"CBFFEE\"/>\n\
		<Gs col=\"CDFFCB\"/>\n\
		<A col=\"E6FFCB\"/>\n\
		<As col=\"FFF6CB\"/>\n\
		<B col=\"FFE2CB\"/>\n\
	</Palette>\n\
</SimpleColourOrgan>\n\
";

#endif
