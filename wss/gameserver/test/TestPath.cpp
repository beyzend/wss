/*
 * TestPath.cpp
 *
 *  Created on: Dec 17, 2014
 *      Author: beyzend
 */

#include <vector>
#include <memory>

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

		const size_t width = 10;
		const size_t height = 10;

		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				map.push_back(1);
			}
		}


		path0.reset(new wss::Path(10, 10, map));


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

	MP_VECTOR<micropather::StateCost> adj;

	size_t state;
	xyToIndex(0, 0, state, 100);
	glm::vec2 stateVec(0, 0);

	path0->AdjacentCost((void*)state, &adj);

	// Assuming cells: [1, 0], [0, 1], [1, 1]
	ASSERT_EQ(adj.size(), 3);

	ASSERT_EQ(adj[0].cost, (stateVec - glm::vec2(1, 0)).length());
	ASSERT_EQ(adj[1].cost, (stateVec - glm::vec2(0, 1)).length());
	ASSERT_EQ(adj[2].cost, (stateVec - glm::vec2(1, 1)).length());
}




int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}



