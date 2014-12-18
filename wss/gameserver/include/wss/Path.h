#pragma once

#include "micropather/micropather.h"

namespace wss {

class Path : micropather::Graph
{
public:
	Path(unsigned int mapWidth, unsigned int mapHeight, const std::vector<unsigned int>& map);

	void setMap(unsigned int mapWidth, unsigned int mapHeight, const std::vector<unsigned int>& map);

	/**
	 * Return the least possible cost between 2 states.
	 * \param stateStart
	 * \param stateEnd
	 */
	virtual float LeastCostEstimate(void* stateStart, void* stateEnd);

	virtual void AdjacentCost(void* state, MP_VECTOR<micropather::StateCost> *adjacent);

	virtual void PrintStateInfo(void* state);

private:
	const std::vector<unsigned int>& map;
	unsigned int mapWidth, mapHeight;

};
}
