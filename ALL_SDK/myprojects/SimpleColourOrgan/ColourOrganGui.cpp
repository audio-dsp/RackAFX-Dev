//	ColourOrganGui.cpp - The GUI of the plugin (it's basically all gui anyway).
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

#include "ColourOrganGui.h"
#include "VstPlugin.h"
#include "PaletteFile.h"
#include "tinyxml.h"
#include <fstream>
#include <cmath>

#ifdef WIN32
#include "shlobj.h"
#endif

using namespace std;

//----------------------------------------------------------------------------
float round(float inval)
{
	float tempf, retval;

	retval = floor(inval);
	tempf = fmod(inval, retval);

	if(tempf > 0.5f)
		retval += 1.0f;

	return retval;
}

//----------------------------------------------------------------------------
ColourOrganGui::ColourOrganGui(AudioEffect *effect):
#ifndef VSYNC
VSTGLEditor(effect),
#else
VSTGLEditor(effect, WaitForVerticalSync),
#endif
Timer(30), //30ms == ~30fps?
paletteIndex(0),
attackTime(1.0f),
releaseTime(1.0f),
velocity(0.0f),
env(0.0f),
numNotes(0)
{
	//Set window size.
#ifdef MACX
	//I have no idea why, but if the width == the height, our HIView doesn't
	//receive kEventBoundsChanged (in Bidule, anyway), so we have to offset
	//one dimension.
	setRect(0, 0, 201, 200);
#else
	setRect(0, 0, 200, 200);
#endif

	//Create palette file if it does not exist.
	if(!doesPaletteFileExist())
		createPaletteFile();
}

//----------------------------------------------------------------------------
ColourOrganGui::~ColourOrganGui()
{
	
}

//----------------------------------------------------------------------------
void ColourOrganGui::guiOpen()
{
	int i;

	//Set parameters.
	for(i=0;i<VstPlugin::numParameters;++i)
		setParameter(i, effect->getParameter(i));

	//Load colour palette according to paletteIndex.
	loadPalette();

	//Setup OpenGL stuff.
	glShadeModel(GL_SMOOTH);
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glViewport(0, 0, getWidth(), getHeight());

    // set viewing projection
    glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f,
				   (GLfloat)getWidth()/(GLfloat)getHeight(),
				   0.1f,
				   100.0f);

    // position viewer
    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

    glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(2.0f);

	//Start timer to constantly update gui.
	start();
}

//----------------------------------------------------------------------------
void ColourOrganGui::guiClose()
{
	stop();
}

//----------------------------------------------------------------------------
void ColourOrganGui::onMouseDown(int button, int x, int y)
{
	if(button == 1)
		paletteIndex = (paletteIndex+1) % getNumPalettes();
	else if(button == 2)
	{
		--paletteIndex;

		if(paletteIndex < 0)
			paletteIndex = getNumPalettes() - 1;
	}

	effect->setParameterAutomated(VstPlugin::ColourPalette,
								  (static_cast<float>(paletteIndex)/100.0f));
}

//----------------------------------------------------------------------------
void ColourOrganGui::draw()
{
	int i;
	int j = 0;
	float invNum;
	Colour tempColour(silentColour);

	/*//Read currentColour.
	colourSection.enter();
	tempColour = currentColour;
	colourSection.leave();*/

	//Update numNotes.
	numNotes = 0;
	for(i=0;i<128;++i)
	{
		if(notes[i].getIsActive())
			++numNotes;
	}
	invNum = 1.0f/static_cast<float>(numNotes);

	if(numNotes)
	{
		//Calculate currentColour.
		for(i=0;i<128;++i)
		{
			if(notes[i].getIsActive())
			{
				//Set silent colour, so chords work right.
				notes[i].setSilentColour(tempColour);
				//Get note's colour.
				tempColour += notes[i].tick() * invNum;
				++j;
			}
			if(j >= numNotes)
				break;
		}
		currentColour = tempColour;
	}
	else
		currentColour = silentColour;

	//Draw colour.
	glClearColor(static_cast<float>(currentColour.red) * (1.0f/255.0f),
				 static_cast<float>(currentColour.green) * (1.0f/255.0f),
				 static_cast<float>(currentColour.blue) * (1.0f/255.0f),
				 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
}

//----------------------------------------------------------------------------
void ColourOrganGui::setParameter(int index, float value)
{
	int i;

	switch(index)
	{
		case VstPlugin::ColourPalette:
			//Update paletteIndex.
			paletteIndex = static_cast<int>(round(100.0f * value));
			if(paletteIndex < 0)
				paletteIndex = getNumPalettes() - 1;
			paletteIndex = paletteIndex % getNumPalettes();

			//Load new palette.
			loadPalette();
			break;
		case VstPlugin::Attack:
			for(i=0;i<128;++i)
				notes[i].setAttack(value);
			break;
		case VstPlugin::Release:
			for(i=0;i<128;++i)
				notes[i].setRelease(value);
			break;
	}
}

//----------------------------------------------------------------------------
void ColourOrganGui::timerCallback()
{
	refreshGraphics();
}

//----------------------------------------------------------------------------
void ColourOrganGui::noteOn(int note, float vel)
{
	int i;

	if(numNotes < 128)
	{
		for(i=0;i<128;++i)
		{
			if(!notes[i].getIsActive())
			{
				notes[i].noteOn(colourPalette[note%12], vel);
				notes[i].setMidiNote(note);
				break;
			}
		}
	}
}

//----------------------------------------------------------------------------
void ColourOrganGui::noteOff(int note)
{
	int i;

	for(i=0;i<128;++i)
	{
		if(notes[i].getIsActive())
		{
			if(notes[i].getMidiNote() == note)
				notes[i].noteOff();
		}
	}
}

//----------------------------------------------------------------------------
void ColourOrganGui::loadPalette()
{
	int i;

	//Make sure the file exists first...
	if(!doesPaletteFileExist())
		createPaletteFile();

	//Load file.
	string tempstr = getHomeDir();
	tempstr += "SimpleColourOrganPalettes.xml";
	TiXmlDocument doc(tempstr);
	doc.LoadFile();

	TiXmlElement* root = doc.FirstChild("SimpleColourOrgan")->ToElement();
	TiXmlNode *palette;
	TiXmlElement *child;
	if(root)
	{
		//Loop through root's children, looking for paletteIndex.
		i = 0;
		for(palette=root->FirstChild("Palette");
			palette;
			palette=palette->NextSibling("Palette"))
		{
			if(i == paletteIndex)
				break;
			++i;
		}
		//If we've found it...
		if(palette)
		{
			//Get silent value.
			child = palette->FirstChild("silent")->ToElement();
			if(child)
				silentColour = child->Attribute("col");
			//Get C value.
			child = palette->FirstChild("C")->ToElement();
			if(child)
				colourPalette[0] = child->Attribute("col");
			//Get C# value.
			child = palette->FirstChild("Cs")->ToElement();
			if(child)
				colourPalette[1] = child->Attribute("col");
			//Get D value.
			child = palette->FirstChild("D")->ToElement();
			if(child)
				colourPalette[2] = child->Attribute("col");
			//Get D# value.
			child = palette->FirstChild("Ds")->ToElement();
			if(child)
				colourPalette[3] = child->Attribute("col");
			//Get E value.
			child = palette->FirstChild("E")->ToElement();
			if(child)
				colourPalette[4] = child->Attribute("col");
			//Get F value.
			child = palette->FirstChild("F")->ToElement();
			if(child)
				colourPalette[5] = child->Attribute("col");
			//Get F# value.
			child = palette->FirstChild("Fs")->ToElement();
			if(child)
				colourPalette[6] = child->Attribute("col");
			//Get G value.
			child = palette->FirstChild("G")->ToElement();
			if(child)
				colourPalette[7] = child->Attribute("col");
			//Get G# value.
			child = palette->FirstChild("Gs")->ToElement();
			if(child)
				colourPalette[8] = child->Attribute("col");
			//Get A value.
			child = palette->FirstChild("A")->ToElement();
			if(child)
				colourPalette[9] = child->Attribute("col");
			//Get A# value.
			child = palette->FirstChild("As")->ToElement();
			if(child)
				colourPalette[10] = child->Attribute("col");
			//Get B value.
			child = palette->FirstChild("B")->ToElement();
			if(child)
				colourPalette[11] = child->Attribute("col");

			//Update the colours for notes.
			for(i=0;i<128;++i)
			{
				notes[i].setSilentColour(silentColour);
				if(notes[i].getIsActive())
					notes[i].setColour(colourPalette);
			}
		}
	}
}

//----------------------------------------------------------------------------
bool ColourOrganGui::doesPaletteFileExist() const
{
	string tempstr = getHomeDir();
	tempstr += "SimpleColourOrganPalettes.xml";

	ifstream in(tempstr.c_str());

	return (in != 0);
}

//----------------------------------------------------------------------------
void ColourOrganGui::createPaletteFile() const
{
	string tempstr = getHomeDir();
	tempstr += "SimpleColourOrganPalettes.xml";

	ofstream out(tempstr.c_str());

	out << paletteFilePart1 << paletteFilePart2;
}

//----------------------------------------------------------------------------
int ColourOrganGui::getNumPalettes() const
{
	int retval = 0;

	//Make sure the file exists first...
	if(!doesPaletteFileExist())
		createPaletteFile();

	//Load file.
	string tempstr = getHomeDir();
	tempstr += "SimpleColourOrganPalettes.xml";
	TiXmlDocument doc(tempstr);
	doc.LoadFile();

	TiXmlNode *root = doc.FirstChild("SimpleColourOrgan");
	if(root)
	{
		TiXmlNode *child = 0;
		while(child = root->IterateChildren("Palette", child))
			++retval;
	}

	return retval;
}

//----------------------------------------------------------------------------
string ColourOrganGui::getHomeDir() const
{
	string retval;

#ifdef WIN32
	char tempstr[2048];

	//Note: Creates the directory if it doesn't exist..
	SHGetSpecialFolderPath(0, tempstr, CSIDL_APPDATA, true);
	retval = tempstr;
	retval += "\\";
#elif MACX
	OSStatus err;
	FSRef folderRef;

	err = FSFindFolder(kUserDomain,
					   kPreferencesFolderType,
					   kDontCreateFolder,
					   &folderRef);
	if(err == noErr)
	{
		unsigned char tempstr[2048];
		err = FSRefMakePath(&folderRef,
							static_cast<UInt8 *>(tempstr),
							2048);
		if(err == noErr)
		{
			retval = reinterpret_cast<char *>(tempstr);
			retval += "/";
		}
	}
#endif

	return retval;
}
