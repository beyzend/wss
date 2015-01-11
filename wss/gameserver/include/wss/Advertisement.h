#pragma once

//check winddows
//#include "wss/wss.gch"
#include "wss.gch"
#include <iostream>
#include <vector>
#include <string>

namespace wss
{

class Advertisement
{
public:
	Advertisement(const std::vector<ATTRIBUTE_VALUE> &deltas) : _advertDeltas(deltas) {

	}
	~Advertisement() {

	}
	/// For now assume that Attributes of attribute entities are normalized in the range of 0...100.
	/// TODO: figure out how to map advertisement costs to arbitrary distribution of values (may need to normalize cost for advertisement).
	const std::vector<ATTRIBUTE_VALUE>& getDeltas() const {
		return _advertDeltas;
	}

private:
	std::vector<ATTRIBUTE_VALUE> _advertDeltas;
};

using ADVERT_SCORE = std::tuple<const Advertisement*, float>;

}

