//	HarmonicalEditor3D.h - Simple OpenGL editor window.
//	--------------------------------------------------------------------------
//	Copyright (c) 2005 Niall Moody
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

#ifndef HARMONICALEDITOR3D_H_
#define HARMONICALEDITOR3D_H_

#include "VSTGLEditor.h"
#include "Harmonical.h"

///	The OpenGL (3D) panel of the gui.
class HarmonicalEditor3D : public VSTGLEditor
{
  public:
	///	Constructor.
	HarmonicalEditor3D(AudioEffect *effect);
	///	Desctructor.
	~HarmonicalEditor3D();

	///	Called when the editor is opened.
	long open(void *ptr);
	///	Called when the editor is closed.
	void close();

	///	Called when a mouse down event occurs.
	void onMouseDown(int button, int x, int y);
	///	Called when a mouse move event occurs.
	void onMouseMove(int x, int y);
	///	Called when a mouse up event occurs.
	void onMouseUp(int button, int x, int y);

	///	Called every frame (i.e. every idle() call) to update the scene.
	void draw();

	///	Called just to set the background colour.
	void setParameter(long index, float value);

	///	Informs the editor of the current state of voice's vertices.
	void setVertices(Vertex *v, int voice, float env);
	///	Informs the editor which voices are active.
	void noteOn(int voice);
	///	Informs the editor when voices go inactive.
	void noteOff(int voice);
  private:
	///	Calculate the camera's current position.
	void calcCameraPosition();

	float cameraXPan;
	float cameraYPan;
	float cameraXRot;
	float cameraYRot;
	float cameraDepth;
	bool leftDown;
	bool middleDown;
	bool rightDown;
	int lastX;
	int lastY;

	///	The vertices for all the voices.
	Vertex vertices[NUM_VOICES][NUM_VERTICES];
	///	The offset from the centre of the scene for each voice.
	Vertex offsets[NUM_VOICES];
	///	Which voices are currently active.
	bool voiceIsActive[NUM_VOICES];
	///	How many voices are currently active.
	int numActiveVoices;
	///	Envelope value used to move the object in and out of the screen.
	float envVal[NUM_VOICES];

	///	Where the camera is currently at (the camera will move to focus on the currently active voices).
	Vertex currentCameraPosition;

	///	Used to store the colour of the background as 3 floats.
	Vertex backgroundColour;
};

#endif
