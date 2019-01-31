/*
 * Multiplier.cpp
 *
 */
#include "basic_blocks.h"

Multiplier::Multiplier(int numberOfInputs) : BasicBlock(numberOfInputs) {

}


float Multiplier::getNextValue() {
	float acc = 1.0;
	for ( vector<BasicBlock *>::iterator it = inputs.begin(); it < inputs.end(); it++ ){
		acc *= (*it)->getNextValue();
	}
	return acc;
}
