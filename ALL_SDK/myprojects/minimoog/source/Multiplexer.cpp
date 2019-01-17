/*
 * Multiplexer.cpp
 *
 */

#include "../include/basic_blocks.h"

Multiplexer::Multiplexer(int numberOfInputs, int selectedInput):
	BasicBlock(numberOfInputs),
	selectedInput(selectedInput){

}

Multiplexer::~Multiplexer() {

}

void Multiplexer::setSelectedInput(int i) {
	selectedInput = i;
}

int Multiplexer::getSelectedInput(){
	return selectedInput;
}

float Multiplexer::getNextValue() {
	return inputs[selectedInput]->getNextValue();
}
