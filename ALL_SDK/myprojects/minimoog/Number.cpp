/*
 * Number.cpp
 *
 */

#include "basic_blocks.h"

Number::Number(float number) : BasicBlock(0),number(number) {


}

Number::~Number() {
	// TODO Auto-generated destructor stub
}


float Number::getNextValue(){
	return number;
}

void Number::setNumber(float number){
	this->number = number;
}

float Number::getNumber() {
	return number;
}
