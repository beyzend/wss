#pragma once

#include <functional>
#include <tuple>
#include <cmath>
#include <queue>

#include "Attributes.h"

namespace wss {

using ATTRIBUTE_DELTA = std::tuple<float,float>;

class AttributeFlow {
public:
	explicit AttributeFlow();
	virtual ~AttributeFlow();

	void addInflow(ATTRIBUTE_DELTA delta);
	void addOutflow(ATTRIBUTE_DELTA delta);

	ATTRIBUTE_VALUE getCurrentValue(ATTRIBUTE_VALUE value, float dt);
	//ATTRIBUTE_VALUE getCurrentValueLazy(ATTRIBUTE_VALUE value, float duration, float step);

private:

	std::queue<ATTRIBUTE_DELTA> _inflow;
	std::queue<ATTRIBUTE_DELTA> _outflow;

};

} //end namespace wss
