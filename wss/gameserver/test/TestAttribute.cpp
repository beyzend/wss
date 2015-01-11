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

	// Test those that match none match one, match multiple, matches all
	std::vector<AttributeValue> attributesMatchNone = {std::make_tuple(Attributes::Hunger, 2)};
	std::vector<AttributeValue> attributesOne = {std::make_tuple(Attributes::Health, 5)};
	std::vector<AttributeValue> attributesMulti = {std::make_tuple(Attributes::Health, 20), std::make_tuple(Attributes::Attack, 50)};
	std::vector<AttributeValue> attributesAll = {std::make_tuple(Attributes::Health, 50), std::make_tuple(Attributes::Attack, 20), std::make_tuple(Attributes::Defense, 40), std::make_tuple(Attributes::Power, 5)};

	std::vector<AttributeValue> oneEntityAttributes = {std::make_tuple(Attributes::Health, 80), std::make_tuple(Attributes::Attack, 50), std::make_tuple(Attributes::Defense, 5), std::make_tuple(Attributes::Power, 66)};

	AttributeEntity oneEntity(oneEntityAttributes);

	Advertisement advertOne(attributesMatchNone);

	float constScore = 0.0f;
	float score = oneEntity.score(advertOne);
	ASSERT_FLOAT_EQ(score, constScore);

}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

