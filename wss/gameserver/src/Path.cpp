/*
 * Path.cpp
 *
 *  Created on: Dec 16, 2014
 *      Author: beyzend
 */
#include <iostream>
#include <cmath>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/vec2.hpp>

#include "wss/Path.h"
#include "wss/Utils.h"
#include "wss/Map.h"

using namespace wss;
using namespace std;

Path::Path(const Layer& layer) : _layer(layer){
}

float Path::LeastCostEstimate(void* stateStart, void* stateEnd)
{
	using namespace std;
	using namespace wss;
	//right now use distance to test.
	int startIndex = (int)(stateStart);
	int endIndex = (int)(stateEnd);

	int data = _layer.data[endIndex];

	glm::vec2 start;
	glm::vec2 end;
	wss::Utils::indexToXY(startIndex, _layer.width, start);
	wss::Utils::indexToXY(endIndex, _layer.width, end);

	using namespace wss;
	using namespace glm;
	// For now if end is not walkable then weight it such that it's length is scaled outside of map.

	float scale = (data != 0) ? 1.0 : _layer.width*2.0;

	float length = glm::length(start - end);

	return length*scale;
}

void Path::AdjacentCost(void* state, MP_VECTOR<micropather::StateCost> *adjacent)
{
	using namespace std;

	size_t currentIndex = (size_t)(state);

	// Clip
	int x, y;
	wss::Utils::indexToXY(currentIndex, _layer.width, x, y);

	assert(x < _layer.width && y < _layer.height);

	// top left corner.
	size_t sx, sy, ex, ey;

	sx = (x == 0) ? x : x - 1;
	ex = (x >= _layer.width - 1) ? _layer.width - 1 : x + 1;
	sy = (y == 0) ? y : y - 1;
	ey = (y >= _layer.height - 1) ? _layer.height - 1 : y + 1;

	// Generate the adjacent cells
	for (size_t cy = sy; cy <= ey; ++cy) {
		for (size_t cx = sx; cx <= ex; ++cx) {
			if (cx == x && cy == y) // X marks the spot
				continue;
			micropather::StateCost stateCost;

			//float cost = glm::length((glm::vec2(x, y) - glm::vec2(cx, cy)));
			size_t cellIndex = Utils::XYToIndex(glm::vec2(cx, cy), _layer.width);
			float cost = LeastCostEstimate((void*)currentIndex, (void*)cellIndex);

			stateCost.state = (void*)(cellIndex);
			stateCost.cost = cost;

			glm::vec2 position;
			Utils::indexToXY((int)stateCost.state, _layer.width, position);
			adjacent->push_back(stateCost);
		}
	}
}

void Path::PrintStateInfo(void* state)
{
}



