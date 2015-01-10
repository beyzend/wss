#include <vector>
#include <memory>
#include <algorithm>
#include <tuple>

#include <iostream>

#include "gtest.h"
#include "wss/Attributes.h"
#include "wss/AttributeEntity.h"
#include "wss/Advertisement.h"


class AttributeTest : public ::testing::Test {
protected:
	AttributeTest() {

	}

	virtual ~AttributeTest() {

	}

	virtual void SetUp() {

	}

	virtual void TearDown() {

	}

};

TEST_F(AttributeTest, TestScore) {
	// Not using a mock object for now.
	using namespace wss;
	std::vector<AttributeValue> attributesOne = {std::make_tuple(Attributes::Health, 100), std::make_tuple(Attributes::Attack, 10)};
	Advertisement advertOne(attributesOne);

	AttributeEntity oneEntity;

	float cost = oneEntity.score(advertOne);
	ASSERT_FLOAT_EQ(cost, 1.0f);

}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

