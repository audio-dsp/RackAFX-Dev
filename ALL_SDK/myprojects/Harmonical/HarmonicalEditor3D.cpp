//	HarmonicalEditor3D.cpp - Simple OpenGL editor window.
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

#include "HarmonicalEditor3D.h"

#include <cmath>

//----------------------------------------------------------------------------
HarmonicalEditor3D::HarmonicalEditor3D(AudioEffect *effect):
VSTGLEditor(effect),
cameraXPan(0.0f), cameraYPan(0.0f),
cameraXRot(0.0f), cameraYRot(0.0f),
cameraDepth(-12.0f),
leftDown(false), middleDown(false), rightDown(false)
{
	int i;
	float xy;

	_rect.left = 0;
	_rect.top = 0;
	_rect.right = 600;
	_rect.bottom = 424;

	Harmonical *harm = static_cast<Harmonical *>(effect);
	harm->setEd3D(this);

	xy = 0.0f;
	for(i=0;i<NUM_VOICES;i++)
	{
		voiceIsActive[i] = false;
		envVal[i] = 0.0f;

		offsets[i].x = /*25.0f + */(15.0f*cosf(xy*2.0f*PI)) - 15.0f;
		offsets[i].y = /*25.0f + */(15.0f*sinf(xy*2.0f*PI));
		offsets[i].z = 0.0f;
		xy += 1.0f/(static_cast<float>(NUM_VOICES));
	}
	voiceIsActive[0] = true; //This should always be true?

	numActiveVoices = 1;
	calcCameraPosition();

	setParameter(Harmonical::BackgroundColour,
				 effect->getParameter(Harmonical::BackgroundColour));
}

//----------------------------------------------------------------------------
HarmonicalEditor3D::~HarmonicalEditor3D()
{
	
}

//----------------------------------------------------------------------------
long HarmonicalEditor3D::open(void *ptr)
{
	//****Always call this!****
	VSTGLEditor::open(ptr);

	glShadeModel(GL_SMOOTH);
	//glClearColor(0.75f, 0.75f, 1.0f, 0.0f);
	glClearColor(backgroundColour.x,
				 backgroundColour.y,
				 backgroundColour.z,
				 0.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glViewport(0, 0, (_rect.right-_rect.left), (_rect.bottom-_rect.top));

    // set viewing projection
    glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f,
				   (GLfloat)(_rect.right-_rect.left)/(GLfloat)(_rect.bottom-_rect.top),
				   0.1f,
				   150.0f);

    // position viewer
    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

    glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(2.0f);

	return true;
}

//----------------------------------------------------------------------------
void HarmonicalEditor3D::close()
{
	//****Always call this!****
	VSTGLEditor::close();
}

//----------------------------------------------------------------------------
void HarmonicalEditor3D::onMouseDown(int button, int x, int y)
{
	if(button == 1)
	{
		leftDown = true;
		lastX = x;
		lastY = y;
	}
	else if(button == 2)
	{
		rightDown = true;
		lastX = x;
		lastY = y;
	}
	else if(button == 3)
	{
		middleDown = true;
		lastX = x;
		lastY = y;
	}
	else
	{
		leftDown = true;
		lastX = x;
		lastY = y;
	}
}

//----------------------------------------------------------------------------
void HarmonicalEditor3D::onMouseMove(int x, int y)
{
	float tempf;
	int width, height;

	width = (_rect.right - _rect.left);
	height = (_rect.bottom - _rect.top);

	if(lastX != x)
	{
		if(leftDown && rightDown)
		{
			tempf = effect->getParameter(Harmonical::BackgroundColour);
			tempf = fmod(tempf+(0.015625f*((float)(x-lastX)/(float)width)),
						 1.0f);
			effect->setParameterAutomated(Harmonical::BackgroundColour,
										  tempf);
		}
		else if(leftDown)
		{
			cameraXRot += 128.0f*((float)(x-lastX)/(float)width);
			lastX = x;
		}
		else if(rightDown)
		{
			cameraXPan += 16.0f*((float)(x-lastX)/(float)width);
			lastX = x;
		}
	}

	if(lastY != y)
	{
		if(leftDown && rightDown)
		{
			
		}
		else if(leftDown)
		{
			cameraYRot += 128.0f*((float)(y-lastY)/(float)height);
			lastY = y;
		}
		else if(rightDown)
		{
			cameraYPan -= 16.0f*((float)(y-lastY)/(float)height);
			lastY = y;
		}
		if(middleDown)
		{
			cameraDepth += 32.0f*((float)(y-lastY)/(float)height);
			lastY = y;
		}
	}
}

//----------------------------------------------------------------------------
void HarmonicalEditor3D::onMouseUp(int button, int x, int y)
{
	if(button == 1)
	{
		leftDown = false;
		lastX = x;
		lastY = y;
	}
	else if(button == 2)
	{
		rightDown = false;
		lastX = x;
		lastY = y;
	}
	else if(button == 3)
	{
		middleDown = false;
		lastX = x;
		lastY = y;
	}
	else
	{
		leftDown = false;
		lastX = x;
		lastY = y;
	}
}

//----------------------------------------------------------------------------
void HarmonicalEditor3D::draw()
{
	int i, j, k, l;
	float tempf;

	glClearColor(backgroundColour.x,
				 backgroundColour.y,
				 backgroundColour.z,
				 0.0f);

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	//tempf = -100.0f + envVal;
	glTranslatef(currentCameraPosition.x,
				 currentCameraPosition.y,
				 currentCameraPosition.z);
	glTranslatef(cameraXPan, cameraYPan, (cameraDepth/*+tempf*/));
	glRotatef(cameraXRot, 0.0f, 1.0f, 0.0f);
	glRotatef(cameraYRot, 1.0f, 0.0f, 0.0f);

	for(l=0;l<NUM_VOICES;l++)
	{
		if(!voiceIsActive[l])
			continue;

		glTranslatef(offsets[l].x, offsets[l].y, offsets[l].z);

		tempf = -50.0f + envVal[l];
		glTranslatef(0.0f, 0.0f, tempf);

		glBegin(GL_QUADS);
		k = 0;
		for(j=0;j<(NUM_LOOPS-1);j++)
		{
			for(i=0;i<(NUM_LOOPS-1);i++)
			{
				glColor3f(static_cast<float>(fabs(vertices[l][k].x)),
						  static_cast<float>(fabs(vertices[l][k].y)),
						  static_cast<float>(fabs(vertices[l][k].z)));
				glVertex3f(vertices[l][k].x,
						   vertices[l][k].y,
						   vertices[l][k].z);
				glColor3f(static_cast<float>(fabs(vertices[l][(k+NUM_LOOPS)].x)),
						  static_cast<float>(fabs(vertices[l][(k+NUM_LOOPS)].y)),
						  static_cast<float>(fabs(vertices[l][(k+NUM_LOOPS)].z)));
				glVertex3f(vertices[l][(k+NUM_LOOPS)].x,
						   vertices[l][(k+NUM_LOOPS)].y,
						   vertices[l][(k+NUM_LOOPS)].z);
				glColor3f(static_cast<float>(fabs(vertices[l][(k+NUM_LOOPS)+1].x)),
						  static_cast<float>(fabs(vertices[l][(k+NUM_LOOPS)+1].y)),
						  static_cast<float>(fabs(vertices[l][(k+NUM_LOOPS)+1].z)));
				glVertex3f(vertices[l][(k+NUM_LOOPS)+1].x,
						   vertices[l][(k+NUM_LOOPS)+1].y,
						   vertices[l][(k+NUM_LOOPS)+1].z);
				glColor3f(static_cast<float>(fabs(vertices[l][k+1].x)),
						  static_cast<float>(fabs(vertices[l][k+1].y)),
						  static_cast<float>(fabs(vertices[l][k+1].z)));
				glVertex3f(vertices[l][k+1].x,
						   vertices[l][k+1].y,
						   vertices[l][k+1].z);
				k++;
			}
			glColor3f(static_cast<float>(fabs(vertices[l][k].x)),
					  static_cast<float>(fabs(vertices[l][k].y)),
					  static_cast<float>(fabs(vertices[l][k].z)));
			glVertex3f(vertices[l][k].x,
					   vertices[l][k].y,
					   vertices[l][k].z);
			glColor3f(static_cast<float>(fabs(vertices[l][(k+NUM_LOOPS)].x)),
					  static_cast<float>(fabs(vertices[l][(k+NUM_LOOPS)].y)),
					  static_cast<float>(fabs(vertices[l][(k+NUM_LOOPS)].z)));
			glVertex3f(vertices[l][(k+NUM_LOOPS)].x,
					   vertices[l][(k+NUM_LOOPS)].y,
					   vertices[l][(k+NUM_LOOPS)].z);
			glColor3f(static_cast<float>(fabs(vertices[l][(k+1)].x)),
					  static_cast<float>(fabs(vertices[l][(k+1)].y)),
					  static_cast<float>(fabs(vertices[l][(k+1)].z)));
			glVertex3f(vertices[l][(k+1)].x,
					   vertices[l][(k+1)].y,
					   vertices[l][(k+1)].z);
			glColor3f(static_cast<float>(fabs(vertices[l][(k+1)-NUM_LOOPS].x)),
					  static_cast<float>(fabs(vertices[l][(k+1)-NUM_LOOPS].y)),
					  static_cast<float>(fabs(vertices[l][(k+1)-NUM_LOOPS].z)));
			glVertex3f(vertices[l][(k+1)-NUM_LOOPS].x,
					   vertices[l][(k+1)-NUM_LOOPS].y,
					   vertices[l][(k+1)-NUM_LOOPS].z);
			k++;
		}
		glEnd();

		glBegin(GL_LINE_STRIP);
		glColor3f(1.0f, 1.0f, 1.0f);
		k = 0;
		for(j=0;j<(NUM_LOOPS-1);j++)
		{
			for(i=0;i<(NUM_LOOPS-1);i++)
			{
				glVertex3f(vertices[l][k].x,
						   vertices[l][k].y,
						   vertices[l][k].z);
				glVertex3f(vertices[l][(k+NUM_LOOPS)].x,
						   vertices[l][(k+NUM_LOOPS)].y,
						   vertices[l][(k+NUM_LOOPS)].z);
				glVertex3f(vertices[l][(k+NUM_LOOPS)+1].x,
						   vertices[l][(k+NUM_LOOPS)+1].y,
						   vertices[l][(k+NUM_LOOPS)+1].z);
				glVertex3f(vertices[l][k+1].x,
						   vertices[l][k+1].y,
						   vertices[l][k+1].z);
				k++;
			}
			glVertex3f(vertices[l][k].x,
					   vertices[l][k].y,
					   vertices[l][k].z);
			glVertex3f(vertices[l][(k+NUM_LOOPS)].x,
					   vertices[l][(k+NUM_LOOPS)].y,
					   vertices[l][(k+NUM_LOOPS)].z);
			glVertex3f(vertices[l][(k+1)].x,
					   vertices[l][(k+1)].y,
					   vertices[l][(k+1)].z);
			glVertex3f(vertices[l][(k+1)-NUM_LOOPS].x,
					   vertices[l][(k+1)-NUM_LOOPS].y,
					   vertices[l][(k+1)-NUM_LOOPS].z);
			k++;
		}
		glEnd();

		glTranslatef(-offsets[l].x, -offsets[l].y, -offsets[l].z);

		glTranslatef(0.0f, 0.0f, -tempf);
	}

	glFlush();
}

//----------------------------------------------------------------------------
void HarmonicalEditor3D::setParameter(long index, float value)
{
	float tempf;

	if(index == Harmonical::BackgroundColour)
	{
		if(value < 0.2f)
		{
			tempf = value * 5.0f;
			backgroundColour.x = (tempf*0.75f) + ((1.0f-tempf)*0.0f);
			backgroundColour.y = (tempf*0.75f) + ((1.0f-tempf)*0.0f);
			backgroundColour.z = (tempf*1.0f) + ((1.0f-tempf)*0.0f);
		}
		else if(value < 0.4f)
		{
			tempf = ((value-0.2f) * 5.0f) ;
			backgroundColour.x = (tempf*0.596f) + ((1.0f-tempf)*0.75f);
			backgroundColour.y = (tempf*0.815f) + ((1.0f-tempf)*0.75f);
			backgroundColour.z = (tempf*0.698f) + ((1.0f-tempf)*1.0f);
		}
		else if(value < 0.6f)
		{
			tempf = ((value-0.4f) * 5.0f);
			backgroundColour.x = (tempf*1.0f) + ((1.0f-tempf)*0.596f);
			backgroundColour.y = (tempf*0.843f) + ((1.0f-tempf)*0.815f);
			backgroundColour.z = (tempf*0.419f) + ((1.0f-tempf)*0.698f);
		}
		else if(value < 0.8f)
		{
			tempf = ((value-0.6f) * 5.0f);
			backgroundColour.x = (tempf*0.580f) + ((1.0f-tempf)*1.0f);
			backgroundColour.y = (tempf*0.0f) + ((1.0f-tempf)*0.843f);
			backgroundColour.z = (tempf*0.0f) + ((1.0f-tempf)*0.419f);
		}
		else
		{
			tempf = ((value-0.8f) * 5.0f);
			backgroundColour.x = (tempf*0.0f) + ((1.0f-tempf)*0.580f);
			backgroundColour.y = (tempf*0.0f) + ((1.0f-tempf)*0.0f);
			backgroundColour.z = (tempf*0.0f) + ((1.0f-tempf)*0.0f);
		}
	}
}

//----------------------------------------------------------------------------
void HarmonicalEditor3D::setVertices(Vertex *v, int voice, float env)
{
	int i;

	envVal[voice] = env*50.0f;

	for(i=0;i<NUM_VERTICES;i++)
		vertices[voice][i] = v[i] + offsets[voice];
}

//----------------------------------------------------------------------------
void HarmonicalEditor3D::noteOn(int voice)
{
	voiceIsActive[voice] = true;
	numActiveVoices++;
	if(numActiveVoices > NUM_VOICES)
		numActiveVoices = NUM_VOICES;
	calcCameraPosition();
}

//----------------------------------------------------------------------------
void HarmonicalEditor3D::noteOff(int voice)
{
	if(voice != 0)
	{
		voiceIsActive[voice] = false;
		numActiveVoices--;
		calcCameraPosition();
	}
}

//----------------------------------------------------------------------------
void HarmonicalEditor3D::calcCameraPosition()
{
	int i;
	float tempx, tempy;

	tempx = offsets[0].x;
	tempy = 0.0f;
	for(i=0;i<NUM_VOICES;i++)
	{
		if(voiceIsActive[i])
		{
			if(fabs(offsets[i].x) > (fabs(tempx)))
				tempx = offsets[i].x;
			//tempx += offsets[i].x;
			tempy += offsets[i].y;
		}
	}
	tempx = tempx - offsets[0].x;
	tempy /= numActiveVoices;

	currentCameraPosition.x = -tempx; //minus because the camera sees the objects 'oppositely' (thimk of mirrors)
	currentCameraPosition.y = -tempy;
	currentCameraPosition.z = -(static_cast<float>(numActiveVoices) * (50.0f/static_cast<float>(NUM_VOICES)));
}
