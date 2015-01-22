#pragma once

#include <tuple>

namespace wss {

enum class Attributes {
	Health = 0, Attack = 1, Defense = 2, Power = 4, Happiness = 8, Fear = 16, Hunger = 32
};

using ATTRIBUTE_VALUE = std::tuple<Attributes, float>;
}



