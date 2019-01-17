//	PhysModKnob.h - A knob which acts as a damped mass-spring.
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

#ifndef PHYSMODKNOB_H_
#define PHYSMODKNOB_H_

#include "ModType.h"
#include "../ParameterManager.h"
#include "../HelperStuff.h"

///	A knob which acts as a damped mass-spring.
class PhysModKnob : public ModType,
					public ParameterCallback
{
  public:
	///	Constructor.
	PhysModKnob(VstPlugin *plugin);
	///	Destructor.
	~PhysModKnob();

	///	Updates the knob's value.
	void getBlock(float *block,
				  VstInt32 blockSize,
				  VstInt32 barStart,
				  float ppq);
	///	Returns the current position of the knob.
	float getKnobPos() const {return ((knobPos.x*amplitude)+intendedPos);};

	///	To set the 3 parameters.
	void parameterChanged(VstInt32 index, float val);
	///	Informs us of the current samplerate.
	void setSamplerate(float rate);

	///	Returns the current value of the indexed parameter.
	float getValue(VstInt32 index);
	///	Returns a textual version of the indexed parameter's value.
	std::string getTextValue(VstInt32 index);
  private:
	///	Simple struct used for the RK4 calculations.
	struct TwoFloats
	{
		///	Constructor.
		TwoFloats() {x = 0; v = 0;};

		///	Position/Velocity.
		union
		{
			float x;
			float dx;
		};
		///	Velocity/Acceleration.
		union
		{
			float v;
			float dv;
		};
	};

	///	Calculates the acceleration of the knob position.
	/*strictinline float acceleration(const TwoFloats& state)
	{
		return -(stiffness/mass)*state.x - damping*state.v;
	};*/

	///	A single stage of the RK4 algo.
	strictinline TwoFloats evaluate(float dt,
									const TwoFloats &d)
	{
		TwoFloats state;
		state.x = knobPos.x + d.dx*dt;
		state.v = knobPos.v + d.dv*dt;

		TwoFloats output;
		output.dx = state.v;
		output.dv = -stiffnessAndMass*state.x - damping*state.v;
		return output;
	};

	TwoFloats evaluate()
	{
		TwoFloats output;
		output.dx = knobPos.v;
		output.dv = -stiffnessAndMass*knobPos.x - damping*knobPos.v;
		return output;
	}

	///	Calculates the knob's next position.
	strictinline void integrate(float dt)
	{
		TwoFloats a = evaluate();
		TwoFloats b = evaluate(dt*0.5f, a);
		TwoFloats c = evaluate(dt*0.5f, b);
		TwoFloats d = evaluate(dt, c);

		const float dxdt = 1.0f/6.0f * (a.dx + 2.0f*(b.dx + c.dx) + d.dx);
		const float dvdt = 1.0f/6.0f * (a.dv + 2.0f*(b.dv + c.dv) + d.dv);

		knobPos.x = knobPos.x + dxdt * dt;
		knobPos.v = knobPos.v + dvdt * dt;
	};

	///	IDs for our parameters.
	enum
	{
		Mass,
		Damping,
		Position,

		NumParameters
	};

	///	The ParameterManager for this plugin.
	ParameterManager *paramManager;
	///	The ParameterManager IDs for all our parameters.
	VstInt32 paramIds[NumParameters];

	///	The knob's current position and velocity.
	TwoFloats knobPos;

	///	The knob's intended position.
	float intendedPos;
	///	The knob's mass.
	float mass;
	///	The knob's damping.
	float damping;

	///	The knob's spring stiffness.
	float stiffness;
	///	stiffness/mass.
	float stiffnessAndMass;

	///	The current sample period.
	float samplePeriod;
	///	The current amplitude of the knob's motion.
	float amplitude;
};

#endif
