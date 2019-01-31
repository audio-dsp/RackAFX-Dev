
#pragma pack(1)

#include "basic_blocks.h"
#include "globals.h"
#include <cmath>
#include <cstdio>

Filter::Filter() :
	BasicBlock(3) {
	x_1[0] = 0.0;
	x_1[1] = 0.0;
	x_1[2] = 0.0;
	x_1[3] = 0.0;
	y_1[0] = 0.0;
	y_1[1] = 0.0;
	y_1[2] = 0.0;
	y_1[3] = 0.0;

}

Filter::~Filter() {
}

void Filter::setInputSignal(BasicBlock * block) {
	(this->inputs)[0] = block;
}

void Filter::setFrequencyInput(BasicBlock * block) {
	(this->inputs)[1] = block;
}

void Filter::setQualityInput(BasicBlock * block) {
	(this->inputs)[2] = block;
}

float Filter::getNextValue() {
	if (!this->ON){
		return inputs[0]->getNextValue();
	}
	float input;
	float freq;
	float k;
	float output;
	float temp[3];
	float b0;
	float a1;
	float omegaTilC;

	input = inputs[0]->getNextValue();
	freq = inputs[1]->getNextValue();
	k = inputs[2]->getNextValue();

	omegaTilC = 2 * sample_rate * tan(freq * 2 * 3.1415926536 / (sample_rate * 2));
	b0 = (omegaTilC / sample_rate) / (omegaTilC / sample_rate + 2);
	a1 = (omegaTilC / sample_rate - 2) / (omegaTilC / sample_rate + 2);

	input = input - k * y_1[3];
	temp[0] = b0 * (input + x_1[0]) - a1 * y_1[0];
	x_1[0] = input;
	y_1[0] = temp[0];
	temp[1] = b0 * (temp[0] + x_1[1]) - a1 * y_1[1];
	x_1[1] = temp[0];
	y_1[1] = temp[1];
	temp[2] = b0 * (temp[1] + x_1[2]) - a1 * y_1[2];
	x_1[2] = temp[1];
	y_1[2] = temp[2];
	output = b0 * (temp[2] + x_1[3]) - a1 * y_1[3];
	x_1[3] = temp[2];
	y_1[3] = output;
	return output;
}
;
