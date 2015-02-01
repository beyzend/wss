#include "wss/AttributesTransform.h"

#include <algorithm>
#include <iostream>
using namespace std;
using namespace wss;

LinearTransform::LinearTransform(float amount, float perUnit, FlowType type) : AttributeTransform(), _perUnit(perUnit), _amount(amount), _type(type) {

}

LinearTransform::~LinearTransform() {
}

float LinearTransform::getCurrentValue(float dt) {
	_amount -= _perUnit * dt;
	return _perUnit * dt;
}

bool LinearTransform::expired() {
	if (_type == FlowType::INFINITE)
		return false;
	if (_amount < 0.0001) {
		return true;
	}
	return false;
}

AttributeFlow::AttributeFlow() {

}

AttributeFlow::~AttributeFlow() {

}

void AttributeFlow::addInflow(shared_ptr<AttributeTransform> transform) {
	_inflow.push_back(transform);
}

void AttributeFlow::addOutflow(shared_ptr<AttributeTransform> transform) {
	_outflow.push_back(transform);
}

ATTRIBUTE_VALUE AttributeFlow::getCurrentValue(ATTRIBUTE_VALUE value, float dt) {

	Attributes attribute;
	float currentVal;

	auto getTotal = [&](vector<shared_ptr<AttributeTransform>> &flow)->float {
		float total = 0.0f;

		for (auto iter = flow.begin(); iter < flow.end(); ++iter) {

			if (!(*iter)->expired()) {
				total += (*iter)->getCurrentValue(dt);
			}
			else
				iter = flow.erase(iter, iter + 1);
		}
		return total;
	};

	auto totalInflow = getTotal(_inflow);
	auto totalOutflow = getTotal(_outflow);

	cout << "totalInflow: " << totalInflow << endl;
	cout << "totalOutflow: " << totalOutflow << endl;
	cout << "currentVal: " << currentVal << endl;

	std::tie(attribute, currentVal) = value;

	currentVal += totalInflow;
	currentVal -= totalOutflow;

	cout << "currentValue is: " << currentVal << endl;

	return std::tie(attribute, currentVal);
}


