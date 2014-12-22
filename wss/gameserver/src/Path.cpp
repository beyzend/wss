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

using namespace wss;

Path::Path(unsigned int mapWidth, unsigned int mapHeight, const std::vector<unsigned int>& map) : mapWidth(mapWidth), mapHeight(mapHeight), map(map){
	using namespace std;
}


float Path::LeastCostEstimate(void* stateStart, void* stateEnd)
{
	//right now use distance to test.
	size_t startIndex = (size_t)static_cast<size_t*>(stateStart);
	size_t endIndex = (size_t)static_cast<size_t*>(stateEnd);

	//assert

	return glm::length((glm::vec2(startIndex % mapWidth, startIndex / mapWidth) - glm::vec2(endIndex % mapWidth, endIndex / mapWidth)));

}

void Path::AdjacentCost(void* state, MP_VECTOR<micropather::StateCost> *adjacent)
{

	size_t currentIndex = (size_t)(state);

	// Clip
	size_t x, y;
	wss::Utils::indexToXY(currentIndex, mapWidth, x, y);

	assert(x < mapWidth && y < mapHeight);

	// top left corner.
	size_t sx, sy, ex, ey;

	sx = (x == 0) ? x : x - 1;
	ex = (x >= mapWidth - 1) ? mapWidth - 1 : x + 1;
	sy = (y == 0) ? y : y - 1;
	ey = (y >= mapWidth - 1) ? mapHeight - 1 : y + 1;

	// Generate the adjacent cells
	for (size_t cy = sy; cy <= ey; ++cy) {
		for (size_t cx = sx; cx <= ex; ++cx) {
			if (cx == x && cy == y) // X marks the spot
				continue;
			micropather::StateCost stateCost;

			float cost = glm::length((glm::vec2(x, y) - glm::vec2(cx, cy)));
			stateCost.state = (void*)(wss::Utils::XYToIndex(cx, cy, mapWidth));
			stateCost.cost = cost;
			adjacent->push_back(stateCost);
		}
	}
}

void Path::PrintStateInfo(void* state)
{
}



