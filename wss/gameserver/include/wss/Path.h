#pragma once

#include "micropather/micropather.h"

namespace wss {

struct Layer;

class Path : public micropather::Graph
{
public:
	Path(const Layer& layer);

	void setMap(const Layer& layer);

	/**
	 * Return the least possible cost between 2 states.
	 * \param stateStart
	 * \param stateEnd
	 */
	virtual float LeastCostEstimate(void* stateStart, void* stateEnd);

	virtual void AdjacentCost(void* state, MP_VECTOR<micropather::StateCost> *adjacent);

	virtual void PrintStateInfo(void* state);

private:
	const Layer& _layer;
};
}
