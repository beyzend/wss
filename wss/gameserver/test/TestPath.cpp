/*
 * TestPath.cpp
 *
 *  Created on: Dec 17, 2014
 *      Author: beyzend
 */

#include <vector>
#include <memory>
#include <algorithm>

#include <iostream>

#include "glm/vec2.hpp"
#include "gtest.h"
#include "wss/Path.h"

class PathTest : public ::testing::Test {
protected:
	PathTest() {

	}

	virtual ~PathTest() {

	}

	virtual void SetUp() {


		for (int y = 0; y < mapHeight; ++y)
		{
			for (int x = 0; x < mapWidth; ++x)
			{
				map.push_back(1);
			}
		}


		path0.reset(new wss::Path(mapWidth, mapHeight, map));


	}

	virtual void TearDown() {

	}

	void indexToXY(size_t index, unsigned int &x, unsigned int &y, unsigned int width) {
		x = index % width; y = index / width;
	}

	void xyToIndex(unsigned x, unsigned y, size_t &index, unsigned int width) {
		index = y * width + x;
	}

	std::vector<unsigned int> map;

	std::unique_ptr<wss::Path> path0;

	size_t mapWidth = 10;
	size_t mapHeight = 10;

};

TEST_F(PathTest, TestLowCost) {
	size_t stateStart, stateEnd;

	xyToIndex(0, 0, stateStart, 10);
	xyToIndex(1, 1, stateEnd, 10);

	float cost = path0->LeastCostEstimate((void*)stateStart, (void*)stateEnd);
	ASSERT_EQ(cost, (glm::vec2(1,1) - glm::vec2(0,0)).length());

	xyToIndex(5,5, stateStart, 100);
	xyToIndex(4,5, stateEnd, 100);

	cost = path0->LeastCostEstimate((void*)stateStart, (void*)stateEnd);
	ASSERT_EQ(cost, (glm::vec2(4,5) - glm::vec2(5,5)).length());


	xyToIndex(230, 323, stateStart, 5000);
	xyToIndex(3, 2344, stateEnd, 5000);

	cost = path0->LeastCostEstimate((void*)stateStart, (void*)stateEnd);
	ASSERT_EQ(cost, (glm::vec2(3, 2344) - glm::vec2(230, 323)).length());

}

TEST_F(PathTest, TestAdjacentCost) {

	using namespace glm;

	// Assuming cells: [1, 0], [0, 1], [1, 1]

	auto checkCells = [this](vec2 stateVec, std::vector<vec2> &cells){

		MP_VECTOR<micropather::StateCost> adj;

		size_t state;
		xyToIndex(stateVec.x, stateVec.y, state, mapWidth);
		path0->AdjacentCost((void*)state, &adj);

		ASSERT_EQ(adj.size(), cells.size());

		for (auto stateCost : adj) {
			// Find this cell.
			size_t x, y;
			indexToXY((size_t)stateCost.state, x, y, mapWidth);
			vec2 stateCell(x, y);

			auto iterator = std::find_if(cells.begin(), cells.end(), [stateCell](vec2 cell) {
				return (stateCell == cell);
			});

			ASSERT_NE(cells.end(), iterator);
			ASSERT_EQ(stateCost.cost, (stateVec - stateCell).length());
		}
	};
	std::vector<vec2> cells = {vec2(1, 0), vec2(0, 1), vec2(1, 1)};
	checkCells(vec2(0, 0), cells);

	cells = {vec2(0,0), vec2(1,0), vec2(2,0),
			vec2(0,1), vec2(2,1),
			vec2(0,2), vec2(1,2), vec2(2,2)
	};

	checkCells(vec2(1, 1), cells);

	cells = {vec2(mapWidth - 2, 0),
			vec2(mapWidth - 2, 1), vec2(mapWidth - 1, 1)
	};

	checkCells(vec2(mapWidth - 1, 0), cells);

	cells = {vec2(mapWidth - 2, 0), vec2(mapWidth - 1, 0),
		vec2(mapWidth - 2, 1), //state cell
		vec2(mapWidth - 2, 2), vec2(mapWidth - 1, 2)
	};

	checkCells(vec2(mapWidth - 1, 1), cells);

}




int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}



