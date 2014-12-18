/*
 * TestPath.cpp
 *
 *  Created on: Dec 17, 2014
 *      Author: beyzend
 */

#include "gtest.h"

class PathTest : public ::testing::Test {
protected:
	PathTest() {

	}

	virtual ~PathTest() {

	}

	virtual void SetUp() {

	}

	virtual void TearDown() {

	}

};

TEST_F(PathTest, TestLowCost) {
	ASSERT_EQ(1, 1);
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}



