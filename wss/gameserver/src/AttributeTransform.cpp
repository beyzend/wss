#include "wss/AttributesTransform.h"

#include <algorithm>
#include <iostream>
using namespace std;
using namespace wss;

AttributeFlow::AttributeFlow() {

}

AttributeFlow::~AttributeFlow() {

}

void AttributeFlow::addInflow(ATTRIBUTE_DELTA delta) {
	_inflow.push_back(delta);
}

void AttributeFlow::addOutflow(ATTRIBUTE_DELTA delta) {
	_outflow.push_back(delta);
}

ATTRIBUTE_VALUE AttributeFlow::getCurrentValue(ATTRIBUTE_VALUE value, float dt) {

	Attributes attribute;
	float currentVal;

	auto getTotal = [&](vector<ATTRIBUTE_DELTA> &flow)->float {
		float total = 0.0f;

		for (auto iter = flow.begin(); iter < flow.end(); ++iter) {
			float remaining, unit;
			tie(remaining, unit) = *iter;
			if (remaining >= 0.0001) {
				remaining -= unit * dt;
				total += unit * dt;
				*iter = tie(remaining, unit);
			}
			else {
				iter = flow.erase(iter, iter + 1);
			}
		}
		return total;
	};

	std::tie(attribute, currentVal) = value;

	auto totalInflow = getTotal(_inflow);
	auto totalOutflow = getTotal(_outflow);

	currentVal += totalInflow;
	currentVal -= totalOutflow;

	return std::tie(attribute, currentVal);
}


