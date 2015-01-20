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

#include <cmath>

#include "glm/glm.hpp"
#include "glm/vec2.hpp"
#include "gtest.h"
#include "wss/Path.h"

#include "wss/Map.h"

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

	}

	virtual void TearDown() {

	}

	void indexToXY(size_t index, int &x, int &y, unsigned int width) {
		x = index % width; y = index / width;
	}

	void xyToIndex(int x, int y, size_t &index, unsigned int width) {
		index = y * width + x;
	}

	std::vector<unsigned int> map;


	size_t mapWidth = 10;
	size_t mapHeight = 10;



};

TEST_F(PathTest, TestLowCost) {
	using namespace wss;

	size_t stateStart, stateEnd;

	xyToIndex(0, 0, stateStart, 10);
	xyToIndex(1, 1, stateEnd, 10);

	Layer layer;
	layer.width = 10;
	layer.height = 10;

	layer.data = std::vector<int32_t>({
				  1,1,0,0,0,0,0,0,0,0,
				  1,1,0,0,0,0,0,0,0,0,
				  0,0,1,0,0,0,0,0,0,0,
				  1,0,0,1,0,0,0,0,0,0,
				  0,0,0,1,0,0,0,0,0,0,
				  0,0,0,1,0,0,0,0,0,0,
				  0,0,0,1,0,0,0,0,0,0,
				  0,0,0,1,0,0,0,0,0,0,
				  0,0,0,1,0,0,0,0,0,0,
				  0,0,0,1,0,0,0,0,0,0
			});

	wss::Path* path0 = new wss::Path(layer);

	// Test basic function
	float cost = path0->LeastCostEstimate((void*)stateStart, (void*)stateEnd);

	float constantCost = 1.41421f;

	ASSERT_NEAR(constantCost, cost, 0.0001f);

	xyToIndex(1, 0, stateStart, 10);
	xyToIndex(2, 1, stateEnd, 10);

	float scale = layer.width * 2.0f;

	constantCost *= scale;

	path0 = new wss::Path(layer);

	cost = path0->LeastCostEstimate((void*)stateStart, (void*)stateEnd);
	ASSERT_NEAR(constantCost, cost, 0.0001f);
}

TEST_F(PathTest, TestAdjacentCost) {

	using namespace glm;
	using namespace wss;
	// Assuming cells: [1, 0], [0, 1], [1, 1]



	auto checkCells = [this](vec2 stateVec, std::vector<vec2> &cells){

		Layer layer;
		layer.width = 10;
		layer.height = 10;

		layer.data = std::vector<int32_t>( {
					1,1,0,0,0,0,0,0,0,0,
					1,1,1,0,0,0,0,0,0,0,
					0,0,1,0,0,0,0,0,0,0,
					1,0,0,1,0,0,0,0,0,0,
					0,0,0,1,0,0,0,0,0,0,
					0,0,0,1,0,0,0,0,0,0,
					0,0,0,1,0,0,0,0,0,0,
					0,0,0,1,0,0,0,0,0,0,
					0,0,0,1,0,0,0,0,0,0,
					0,0,0,1,0,0,0,0,0,0
				});

		MP_VECTOR<micropather::StateCost> adj;

		size_t state;
		xyToIndex(stateVec.x, stateVec.y, state, mapWidth);

		wss::Path *path0 = new wss::Path(layer);

		path0->AdjacentCost((void*)state, &adj);

		ASSERT_EQ(adj.size(), cells.size());

		for (auto stateCost : adj) {
			// Find this cell.
			int x, y;
			indexToXY((size_t)stateCost.state, x, y, mapWidth);
			vec2 stateCell(x, y);

			auto iterator = std::find_if(cells.begin(), cells.end(), [stateCell](vec2 cell) {
				return (stateCell == cell);
			});

			ASSERT_NE(cells.end(), iterator);
			size_t cellIndex;
			xyToIndex(stateCell.x, stateCell.y, cellIndex, layer.width);
			std::int32_t data = layer.data[cellIndex];
			float scale = (data != 0) ? 1.0f : layer.width * 2.0f;
			ASSERT_NEAR(glm::length(stateVec - stateCell) * scale, stateCost.cost, 0.0001);
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



