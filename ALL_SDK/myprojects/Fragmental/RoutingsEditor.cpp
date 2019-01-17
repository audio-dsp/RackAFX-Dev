//	RoutingsEditor.cpp - The VSTGL editor handling the effects routings.
//	---------------------------------------------------------------------------
//	Copyright (c) 2006-2007 Niall Moody
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
//	---------------------------------------------------------------------------

#include "RoutingsEditor.h"
#include "VstPlugin.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
RoutingsEditor::RoutingsEditor(AudioEffect *effect):
//VSTGLEditor(effect, Antialias4x),
VSTGLEditor(effect),
lineTrans(1.0f),
sliderSelected(0),
mainPlugin(effect)
{
	int i;

	//Set the opengl context's size - This must be called here!
	setRect(0, 0, 263, 150);

	//Set the sliders' y positions.
	for(i=0;i<6;++i)
	{
		sliders[i].x = 4;
		//sliders[i].y = (i * 22)+11;
		sliders[i].y = (i * 23)+9;
	}

	for(i=0;i<9;++i)
		lineVisible[i] = false;
}

//-----------------------------------------------------------------------------
RoutingsEditor::~RoutingsEditor()
{
	
}

//-----------------------------------------------------------------------------
void RoutingsEditor::guiOpen()
{
	//Setup OpenGL stuff.
	glShadeModel(GL_SMOOTH);
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

	glViewport(0, 0, getWidth(), getHeight());

    //Set viewing projection.
    glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0.0f, getWidth(), (getHeight()-1), 0.0f, -10.0f, 10.0f);

    //Position viewer.
    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(2.0f);

	setParameter(VstPlugin::Route1,
				 mainPlugin->getParameter(VstPlugin::Route1));
	setParameter(VstPlugin::Route2,
				 mainPlugin->getParameter(VstPlugin::Route2));
	setParameter(VstPlugin::Route3,
				 mainPlugin->getParameter(VstPlugin::Route3));
	setParameter(VstPlugin::Route4,
				 mainPlugin->getParameter(VstPlugin::Route4));
	setParameter(VstPlugin::Route5,
				 mainPlugin->getParameter(VstPlugin::Route5));
	setParameter(VstPlugin::Route6,
				 mainPlugin->getParameter(VstPlugin::Route6));
}

//-----------------------------------------------------------------------------
void RoutingsEditor::guiClose()
{
	
}

//-----------------------------------------------------------------------------
void RoutingsEditor::onMouseDown(int button, int x, int y)
{
	int i, j;

	if(button == 1)
	{
		for(i=0;i<6;++i)
		{
			if(isInside(x, y, sliders[i]))
			{
				sliderSelected = i+1;
				for(j=0;j<9;++j)
					lineVisible[j] = false;

				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void RoutingsEditor::onMouseMove(int x, int y)
{
	if(sliderSelected)
	{
		//Update slider's position.
		sliders[sliderSelected-1].x = x-10;
		if(sliders[sliderSelected-1].x < 4)
			sliders[sliderSelected-1].x = 4;
		else if(sliders[sliderSelected-1].x > (getWidth()-25))
			sliders[sliderSelected-1].x = (getWidth()-25);

		//Update slider's routing value.
		if(x < 25)
			sliders[sliderSelected-1].routing = 0;
		else if(x < 56)
			sliders[sliderSelected-1].routing = 1;
		else if(x < 98)
			sliders[sliderSelected-1].routing = 2;
		else if(x < 140)
			sliders[sliderSelected-1].routing = 3;
		else if(x < 182)
			sliders[sliderSelected-1].routing = 4;
		else if(x < 226)
			sliders[sliderSelected-1].routing = 5;
		else
			sliders[sliderSelected-1].routing = 6;
	}
}

//-----------------------------------------------------------------------------
void RoutingsEditor::onMouseUp(int button, int x, int y)
{
	if((button == 1) && (sliderSelected))
	{
		effect->setParameterAutomated((VstPlugin::Route1+(sliderSelected-1)),
									  ((float)sliders[sliderSelected-1].routing)/6.0f);

		sliderSelected = 0;
		lineTrans = 0.0f;
	}
}

//-----------------------------------------------------------------------------
void RoutingsEditor::draw()
{
	int i;
	int tempX, tempY;
	int width = getWidth();
	int height = getHeight();

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	//Shade in 'sticky points'.
	glColor3f(0.75f, 0.75f, 0.75f);
	glBegin(GL_QUADS);
	{
		//Bypass sticky point.
		glColor3f(0.675f, 0.675f, 0.675f);
		glVertex2d(4, 4);
		glVertex2d(25, 4);
		glVertex2d(25, height-4);
		glVertex2d(4, height-4);

		//Point 1.
		glColor3f(0.8f, 0.8f, 0.8f);
		glVertex2d(25, 4);
		glVertex2d(46, 4);
		glVertex2d(46, height-4);
		glVertex2d(25, height-4);

		//Point 2.
		glVertex2d(67, 4);
		glVertex2d(88, 4);
		glVertex2d(88, height-4);
		glVertex2d(67, height-4);

		//Point 3.
		glVertex2d(109, 4);
		glVertex2d(130, 4);
		glVertex2d(130, height-4);
		glVertex2d(109, height-4);

		//Point 4.
		glVertex2d(151, 4);
		glVertex2d(172, 4);
		glVertex2d(172, height-4);
		glVertex2d(151, height-4);

		//Point 5.
		glVertex2d(194, 4);
		glVertex2d(215, 4);
		glVertex2d(215, height-4);
		glVertex2d(194, height-4);

		//Point 6.
		glVertex2d(237, 4);
		glVertex2d(258, 4);
		glVertex2d(258, height-4);
		glVertex2d(237, height-4);
	}
	glEnd();

	//Draw 'sticky points'.
	glColor3f(0.5f, 0.5f, 0.5f);

	glBegin(GL_LINES);
	{
		//Point 1 left line.
		glVertex2d(25, 4);
		glVertex2d(25, height-4);
	}
	glEnd();

	glEnable(GL_LINE_STIPPLE);
	glLineStipple(1, 0x0F0F);

	glBegin(GL_LINES);
	{
		//Point 1 right line.
		glVertex2d(46, 4);
		glVertex2d(46, height-4);

		//Point 2 left line.
		glVertex2d(67, 4);
		glVertex2d(67, height-4);
		//Point 2 right line.
		glVertex2d(88, 4);
		glVertex2d(88, height-4);

		//Point 3 left line.
		glVertex2d(109, 4);
		glVertex2d(109, height-4);
		//Point 3 right line.
		glVertex2d(130, 4);
		glVertex2d(130, height-4);

		//Point 4 left line.
		glVertex2d(151, 4);
		glVertex2d(151, height-4);
		//Point 4 right line.
		glVertex2d(172, 4);
		glVertex2d(172, height-4);

		//Point 5 left line.
		glVertex2d(194, 4);
		glVertex2d(194, height-4);
		//Point 5 right line.
		glVertex2d(215, 4);
		glVertex2d(215, height-4);

		//Point 6 left line.
		glVertex2d(237, 4);
		glVertex2d(237, height-4);
	}
	glEnd();
	glDisable(GL_LINE_STIPPLE);

	//Draw bezier curves.
	glColor4f(0.5f, 0.5f, 0.5f, lineTrans);
	//glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
	if(lineTrans < 1.0f)
		lineTrans += 0.05f;
	for(i=0;i<9;++i)
	{
		if(lineVisible[i])
			lines[i].draw();
	}

	//Draw sliders.
	for(i=0;i<6;++i)
	{
		//glColor4f(0.0f, 0.0f, 0.0f, 0.25f);
		glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
		glBegin(GL_QUADS);
		{
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			glVertex2d(sliders[i].x, sliders[i].y);
			glVertex2d((sliders[i].x+21), sliders[i].y);
			glColor4f(0.85f, 0.85f, 0.85f, 0.5f);
			glVertex2d((sliders[i].x+21), (sliders[i].y+18));
			glVertex2d(sliders[i].x, (sliders[i].y+18));
		}
		glEnd();

		glColor4f(0.4f, 0.4f, 0.4f, 1.0f);
		glBegin(GL_LINES);
		{
			glVertex2d(sliders[i].x, sliders[i].y);
			glVertex2d((sliders[i].x+21), sliders[i].y);

			glVertex2d((sliders[i].x+21), (sliders[i].y-1));
			glVertex2d((sliders[i].x+21), (sliders[i].y+19));

			glVertex2d((sliders[i].x+21), (sliders[i].y+18));
			glVertex2d(sliders[i].x, (sliders[i].y+18));

			glVertex2d(sliders[i].x, (sliders[i].y+19));
			glVertex2d(sliders[i].x, (sliders[i].y-1));
		}
		glEnd();

		//Draw domino patterns.
		//glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
		switch(i)
		{
			case 0:
				tempX = sliders[i].x + 9;
				tempY = sliders[i].y + 6;
				glLineWidth(4.0f);
				glBegin(GL_LINES);
				{
					glVertex2d(tempX, tempY+2);
					glVertex2d(tempX+4, tempY+2);
				}
				glEnd();
				glLineWidth(2.0f);
				break;
			case 1:
				tempX = sliders[i].x + 4;
				tempY = sliders[i].y + 2;
				glLineWidth(4.0f);
				glBegin(GL_LINES);
				{
					glVertex2d(tempX, tempY+2);
					glVertex2d(tempX+4, tempY+2);

					tempX += 10;
					tempY += 10;
					glVertex2d(tempX, tempY+2);
					glVertex2d(tempX+4, tempY+2);
				}
				glEnd();
				glLineWidth(2.0f);
				break;
			case 2:
				tempX = sliders[i].x + 4;
				tempY = sliders[i].y + 2;
				glLineWidth(4.0f);
				glBegin(GL_LINES);
				{
					glVertex2d(tempX, tempY+2);
					glVertex2d(tempX+4, tempY+2);

					tempX += 5;
					tempY += 5;
					glVertex2d(tempX, tempY+2);
					glVertex2d(tempX+4, tempY+2);

					tempX += 5;
					tempY += 5;
					glVertex2d(tempX, tempY+2);
					glVertex2d(tempX+4, tempY+2);
				}
				glEnd();
				glLineWidth(2.0f);
				break;
			case 3:
				tempX = sliders[i].x + 4;
				tempY = sliders[i].y + 2;
				glLineWidth(4.0f);
				glBegin(GL_LINES);
				{
					glVertex2d(tempX, tempY+2);
					glVertex2d(tempX+4, tempY+2);

					tempX += 10;
					glVertex2d(tempX, tempY+2);
					glVertex2d(tempX+4, tempY+2);

					tempY += 10;
					glVertex2d(tempX, tempY+2);
					glVertex2d(tempX+4, tempY+2);

					tempX -= 10;
					glVertex2d(tempX, tempY+2);
					glVertex2d(tempX+4, tempY+2);
				}
				glEnd();
				glLineWidth(2.0f);
				break;
			case 4:
				tempX = sliders[i].x + 4;
				tempY = sliders[i].y + 2;
				glLineWidth(4.0f);
				glBegin(GL_LINES);
				{
					glVertex2d(tempX, tempY+2);
					glVertex2d(tempX+4, tempY+2);

					tempX += 10;
					glVertex2d(tempX, tempY+2);
					glVertex2d(tempX+4, tempY+2);

					tempY += 10;
					glVertex2d(tempX, tempY+2);
					glVertex2d(tempX+4, tempY+2);

					tempX -= 10;
					glVertex2d(tempX, tempY+2);
					glVertex2d(tempX+4, tempY+2);

					tempX += 5;
					tempY -= 5;
					glVertex2d(tempX, tempY+2);
					glVertex2d(tempX+4, tempY+2);
				}
				glEnd();
				glLineWidth(2.0f);
				break;
			case 5:
				tempX = sliders[i].x + 4;
				tempY = sliders[i].y + 2;
				glLineWidth(4.0f);
				glBegin(GL_LINES);
				{
					glVertex2d(tempX, tempY+2);
					glVertex2d(tempX+4, tempY+2);

					tempY += 5;
					glVertex2d(tempX, tempY+2);
					glVertex2d(tempX+4, tempY+2);

					tempY += 5;
					glVertex2d(tempX, tempY+2);
					glVertex2d(tempX+4, tempY+2);

					tempX += 10;
					tempY -= 10;
					glVertex2d(tempX, tempY+2);
					glVertex2d(tempX+4, tempY+2);

					tempY += 5;
					glVertex2d(tempX, tempY+2);
					glVertex2d(tempX+4, tempY+2);

					tempY += 5;
					glVertex2d(tempX, tempY+2);
					glVertex2d(tempX+4, tempY+2);
				}
				glEnd();
				glLineWidth(2.0f);
				break;
		}
	}

	//Draw outline.
	glColor3f(0.5f, 0.5f, 0.5f);
	glBegin(GL_LINES);
	{
		glVertex2d(4, 4);
		glVertex2d(width-4, 4);

		glVertex2d(width-4, 3);
		glVertex2d(width-4, height-3);

		glVertex2d(width-4, height-4);
		glVertex2d(4, height-4);

		glVertex2d(4, height-3);
		glVertex2d(4, 3);
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void RoutingsEditor::timerCallback()
{
	int i;
	int tempint;
	int notMoving = 0;

	if(!sliderSelected)
	{
		//Update sliders' positions if necessary.
		for(i=0;i<6;++i)
		{
			switch(sliders[i].routing)
			{
				case 0:
					if(sliders[i].x > 4)
					{
						tempint = (int)((float)(sliders[i].x-4)/2);
						if(!tempint)
							++tempint;
						sliders[i].x -= tempint;
					}
					else
						++notMoving;
					break;
				case 1:
					if(sliders[i].x > 25)
					{
						tempint = (int)((float)(sliders[i].x-25)/2);
						if(!tempint)
							++tempint;
						sliders[i].x -= tempint;
					}
					else if(sliders[i].x < 25)
					{
						tempint = (int)((float)(25-sliders[i].x)/2);
						if(!tempint)
							++tempint;
						sliders[i].x += tempint;
					}
					else
						++notMoving;
					break;
				case 2:
					if(sliders[i].x > 67)
					{
						tempint = (int)((float)(sliders[i].x-67)/2);
						if(!tempint)
							++tempint;
						sliders[i].x -= tempint;
					}
					else if(sliders[i].x < 67)
					{
						tempint = (int)((float)(67-sliders[i].x)/2);
						if(!tempint)
							++tempint;
						sliders[i].x += tempint;
					}
					else
						++notMoving;
					break;
				case 3:
					if(sliders[i].x > 109)
					{
						tempint = (int)((float)(sliders[i].x-109)/2);
						if(!tempint)
							++tempint;
						sliders[i].x -= tempint;
					}
					else if(sliders[i].x < 109)
					{
						tempint = (int)((float)(109-sliders[i].x)/2);
						if(!tempint)
							++tempint;
						sliders[i].x += tempint;
					}
					else
						++notMoving;
					break;
				case 4:
					if(sliders[i].x > 151)
					{
						tempint = (int)((float)(sliders[i].x-151)/2);
						if(!tempint)
							++tempint;
						sliders[i].x -= tempint;
					}
					else if(sliders[i].x < 151)
					{
						tempint = (int)((float)(151-sliders[i].x)/2);
						if(!tempint)
							++tempint;
						sliders[i].x += tempint;
					}
					else
						++notMoving;
					break;
				case 5:
					if(sliders[i].x > 194)
					{
						tempint = (int)((float)(sliders[i].x-194)/2);
						if(!tempint)
							++tempint;
						sliders[i].x -= tempint;
					}
					else if(sliders[i].x < 194)
					{
						tempint = (int)((float)(194-sliders[i].x)/2);
						if(!tempint)
							++tempint;
						sliders[i].x += tempint;
					}
					else
						++notMoving;
					break;
				case 6:
					if(sliders[i].x > 237)
					{
						tempint = (int)((float)(sliders[i].x-237)/2);
						if(!tempint)
							++tempint;
						sliders[i].x -= tempint;
					}
					else if(sliders[i].x < 237)
					{
						tempint = (int)((float)(237-sliders[i].x)/2);
						if(!tempint)
							++tempint;
						sliders[i].x += tempint;
					}
					else
						++notMoving;
					break;
			}
		}
	}

	if(notMoving == 6)
		updateBezierCurves();

	refreshGraphics();
}

//-----------------------------------------------------------------------------
void RoutingsEditor::setParameter(int index, float value)
{
	if((index >= VstPlugin::Route1) && (index <= VstPlugin::Route6))
		sliders[index-VstPlugin::Route1].routing = round(value * 6.0f);
}

//-----------------------------------------------------------------------------
void RoutingsEditor::updateBezierCurves()
{
	int midX, midY;
	int currentLine;
	int i, j, k, l, m;
	int routingTo = 0;

	for(i=0;i<9;++i)
		lineVisible[i] = false;

	//Loop going through the sticky points.
	for(i=1;i<6;++i)
	{
		//Loop going through the sliders.
		for(j=0;j<6;++j)
		{
			if(sliders[j].routing == i)
			{
				//Loop going through the other sticky points.
				for(k=(i+1);k<7;++k)
				{
					//Loop going through the other sliders.
					for(l=0;l<6;++l)
					{
						if(l == j)
							continue;
						else if(sliders[l].routing == k)
						{
							routingTo = k;

							//Find 1st unused line.
							for(m=0;m<9;++m)
							{
								if(!lineVisible[m])
								{
									lineVisible[m] = true;
									currentLine = m;
									break;
								}
							}

							//Update bezier curve points.
							midX = ((sliders[l].x+1) - (sliders[j].x+22))/2;
							midX += (sliders[j].x+22);
							midY = ((sliders[l].y+9) - (sliders[j].y+9))/2;
							midY += (sliders[j].y+9);
							lines[m].setPoints(BezierCurve::BezierPoint(sliders[j].x+22, sliders[j].y+9),
											   BezierCurve::BezierPoint(sliders[l].x+1, sliders[l].y+9),
											   BezierCurve::BezierPoint(midX, sliders[j].y+9),
											   BezierCurve::BezierPoint(midX, sliders[l].y+9));
											   
						}
					}
					if(routingTo != 0)
						break;
				}
				routingTo = 0;
			}
		}
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
RoutingsEditor::BezierCurve::BezierCurve()
{
	points[0].x = 0.0f;
	points[0].y = 0.0f;
	points[1].x = 1.0f;
	points[1].y = 0.0f;
	controls[0].x = 0.5f;
	controls[0].y = 1.0f;
	controls[1].x = 0.5f;
	controls[1].y = 1.0f;

	updateCoefficients();
}

//-----------------------------------------------------------------------------
RoutingsEditor::BezierCurve::~BezierCurve()
{
	
}

//-----------------------------------------------------------------------------
void RoutingsEditor::BezierCurve::setPoints(BezierPoint p1,
										    BezierPoint p2,
										    BezierPoint c1,
										    BezierPoint c2)
{
	points[0].x = p1.x;
	points[0].y = p1.y;
	points[1].x = p2.x;
	points[1].y = p2.y;
	controls[0].x = c1.x;
	controls[0].y = c1.y;
	controls[1].x = c2.x;
	controls[1].y = c2.y;

	if(p2.x > p1.x)
		numIterations = static_cast<int>(p2.x - p1.x);
	else
		numIterations = static_cast<int>(p1.x - p2.x);

	updateCoefficients();
}

//-----------------------------------------------------------------------------
void RoutingsEditor::BezierCurve::draw()
{
	int i;
	float tempX, tempY, tempT;
	float invIterations = 1.0f/(float)numIterations;

	glBegin(GL_LINE_STRIP);
	{
		for(i=0;i<numIterations;++i)
		{
			tempT = static_cast<float>(i) * invIterations;

			tempX = a.x * (tempT*tempT*tempT);
			tempX += b.x * (tempT*tempT);
			tempX += c.x * tempT;
			tempX += points[0].x;

			tempY = a.y * (tempT*tempT*tempT);
			tempY += b.y * (tempT*tempT);
			tempY += c.y * tempT;
			tempY += points[0].y;

			glVertex2f(tempX, tempY);
		}
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void RoutingsEditor::BezierCurve::updateCoefficients()
{
	c.x = 3.0f * (controls[0].x - points[0].x);
	b.x = 3.0f * (controls[1].x - controls[0].x);
	b.x -= c.x;
	a.x = points[1].x - points[0].x - c.x;
	a.x -= b.x;

	c.y = 3.0f * (controls[0].y - points[0].y);
	b.y = 3.0f * (controls[1].y - controls[0].y);
	b.y -= c.y;
	a.y = points[1].y - points[0].y - c.y;
	a.y -= b.y;
}
