#pragma once

//check winddows
//#include "wss/wss.gch"
#include "wss.gch"
#include <iostream>
#include <vector>
#include <string>

namespace wss
{

typedef std::tuple<wss::Attributes, size_t> AttributeValue;

class Advertisement
{
public:
	Advertisement(const std::vector<AttributeValue> &deltas) : _advertDeltas(deltas) {

	}
	~Advertisement() {

	}

	const std::vector<AttributeValue>& getDeltas() {
		return _advertDeltas;
	}

private:
	std::vector<AttributeValue> _advertDeltas;
};
}
