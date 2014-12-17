/*
 * Path.cpp
 *
 *  Created on: Dec 16, 2014
 *      Author: beyzend
 */

#include <iostream>
#include <cmath>
#include <algorithm>

#include <glm/vec2.hpp>

#include "wss/Path.h"

using namespace wss;

Path::Path(unsigned int mapWidth, unsigned int mapHeight, const std::vector<unsigned int>& map) : mapWidth(mapWidth), mapHeight(mapHeight), map(map){
	using namespace std;
}


float Path::LeastCostEstimate(void* stateStart, void* stateEnd)
{
	//right now use distance to test.
	size_t startIndex = (size_t)static_cast<size_t*>(stateStart);
	size_t endIndex = (size_t)static_cast<size_t*>(stateEnd);

	return (glm::vec2(startIndex % mapWidth, startIndex / mapWidth) - glm::vec2(endIndex % mapWidth, endIndex / mapWidth)).length();

}

void Path::AdjacentCost(void* state, MP_VECTOR<micropather::StateCost> *adjacent)
{
	size_t currentIndex = (size_t)(state);
	size_t topLeftCornerIndex = (currentIndex / mapWidth - 1) * mapWidth;

	// get adjacent squares.
	std::vector<std::vector<size_t>> rows = {{0,1,2},{0,2},{0,1,2}};
	size_t rowCount = 0;


	std::for_each(rows.begin(), rows.end(), [&rowCount, topLeftCornerIndex](std::vector<size_t> row){
		std::for_each(row.begin(), row.end(), [&rowCount, topLeftCornerIndex](size_t col){
			micropather::StateCost stateCost;
			float cost = (glm::vec2(1, 1) - glm::vec2(col, rowCount)).length();
			size_t state = rowCount * 3 + col;
			stateCost.state = (void*)(topLeftCornerIndex + state);
			stateCost.cost = cost;
		});
		rowCount++;
	});
}

void Path::PrintStateInfo(void* state)
{
	//blah;
}



