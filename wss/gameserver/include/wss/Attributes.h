#pragma once

#include <tuple>
#include <iostream>

namespace wss {

enum class Attributes {
	Health = 0, Attack = 1, Defense = 2, Power = 4, Happiness = 8, Fear = 16, Hunger = 32

};

static std::ostream& operator<<(std::ostream& os, const Attributes& attr) {

	using namespace std;
	using namespace wss;

	string output;
	switch(attr) {
	case Attributes::Health:
	{
		output = "Health";
	}
	break;
	case Attributes::Attack:
	{
		output = "Attack";
	}
	break;
	case Attributes::Defense:
	{
		output = "Defense";
	}
	break;
	case Attributes::Power:
	{
		output = "Power";
	}
	break;
	case Attributes::Happiness:
	{
		output = "Happiness";

	}
	break;
	case Attributes::Fear:
	{
		output = "Fear";
	}
	break;
	case Attributes::Hunger:
	{
		output = "Hunger";
	}
	break;
	default:
	{
		output = "Unknown";
	}
	}
	os << output << " : " << (int)attr;
	return os;
}

using ATTRIBUTE_VALUE = std::tuple<Attributes, float>;
}



