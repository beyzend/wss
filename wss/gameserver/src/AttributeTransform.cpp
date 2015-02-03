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

AttributeTransform* LinearTransform::clone() const {
	return new LinearTransform(*this);
}

AttributeFlow::AttributeFlow() {

}

AttributeFlow::~AttributeFlow() {

}

void AttributeFlow::addInflow(const AttributeTransform& transform) {
	_inflow.push_back(shared_ptr<AttributeTransform>(transform.clone()));
}

void AttributeFlow::addOutflow(const AttributeTransform& transform) {
	_outflow.push_back(shared_ptr<AttributeTransform>(transform.clone()));
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

	//cout << "totalOutflow: " << totalOutflow << endl;
	std::tie(attribute, currentVal) = value;
	//cout << "attribute: " << attribute << " value: " << currentVal << endl;

	currentVal += totalInflow;
	currentVal -= totalOutflow;

	return std::tie(attribute, currentVal);
}


