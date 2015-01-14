#pragma once

//check winddows
//#include "wss/wss.gch"
#include "wss/wss.h"
#include <iostream>
#include <vector>
#include <queue>
#include <string>

#include "wss/AdvertCommand.h"

namespace wss
{

class Advertisement
{
public:
	Advertisement(const std::vector<ATTRIBUTE_VALUE> &deltas, const AdvertCommand &commands) : _advertDeltas(deltas), _commands(commands) {

	}
	~Advertisement() {

	}
	/// For now assume that Attributes of attribute entities are normalized in the range of 0...100.
	/// TODO: figure out how to map advertisement costs to arbitrary distribution of values (may need to normalize cost for advertisement).
	const std::vector<ATTRIBUTE_VALUE>& getDeltas() const {
		return _advertDeltas;
	}

	AdvertCommand getCommand() const { return _commands; }

private:
	std::vector<ATTRIBUTE_VALUE> _advertDeltas;
	AdvertCommand _commands;
};
}

