#include "wss/AttributeTransform.h"

using namespace std;
using namespace wss;

AttributeFlow::AttributeFlow() {

}

AttributeFlow::~AttributeFlow() {

}

void AttributeFlow::addInflow(ATTRIBUTE_DELTA delta) {
	_inflow.push(delta);
}

void AttributeFlow::addOutflow(ATTRIBUTE_DELTA delta) {
	_outflow.push(delta);
}

ATTRIBUTE_VALUE AttributeFlow::getCurrentValue(ATTRIBUTE_VALUE value, float dt) {
	Attributes attribute;
	float currentVal;

	std::tie(attribute, currentVal) = value;
	return std::tie(attribute, currentVal);
}


