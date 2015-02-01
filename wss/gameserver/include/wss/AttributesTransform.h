#pragma once

#include <functional>
#include <tuple>
#include <cmath>
#include <queue>
#include <memory>

#include "Attributes.h"

namespace wss {

class AttributeTransform {
public:
	virtual ~AttributeTransform() {}

	virtual float getCurrentValue(float dt) = 0;
	virtual bool expired() = 0;
protected:
	AttributeTransform(){}
private:
};

enum class FlowType {
	FINITE, INFINITE
};

class LinearTransform : public AttributeTransform {
public:
	LinearTransform(float amount, float perUnit, FlowType type);
	virtual ~LinearTransform();

	virtual float getCurrentValue(float dt);
	virtual bool expired();

	float getPerUnit() const { return _perUnit; }
	float getAmount() const { return _amount; }
	FlowType getType() const { return _type; }

private:
	float _perUnit, _amount;
	FlowType _type;
};

class AttributeFlow {
public:
	AttributeFlow();
	virtual ~AttributeFlow();

	void addInflow(std::shared_ptr<AttributeTransform> transform);
	void addOutflow(std::shared_ptr<AttributeTransform> transform);

	ATTRIBUTE_VALUE getCurrentValue(ATTRIBUTE_VALUE value, float dt);
	//ATTRIBUTE_VALUE getCurrentValueLazy(ATTRIBUTE_VALUE value, float duration, float step);

private:
	std::vector<std::shared_ptr<AttributeTransform>> _inflow;
	std::vector<std::shared_ptr<AttributeTransform>> _outflow;

};

} //end namespace wss
