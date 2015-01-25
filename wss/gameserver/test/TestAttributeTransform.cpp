#include <vector>
#include <memory>
#include <algorithm>
#include <tuple>
#include <queue>

#include <iostream>

#include "gtest.h"
#include "wss/AttributesTransform.h"

using namespace wss;
using namespace std;

class AttributeTransformTest : public ::testing::Test {
protected:
	AttributeTransformTest() {

	}

	virtual ~AttributeTransformTest() {

	}

	virtual void SetUp() {

	}

	virtual void TearDown() {

	}

	// Test those that match none match one, match multiple, matches all
	std::vector<ATTRIBUTE_VALUE> attributesMatchNone = {std::make_tuple(Attributes::Hunger, 2)};
	std::vector<ATTRIBUTE_VALUE> attributesOne = {std::make_tuple(Attributes::Health, 5)};
	std::vector<ATTRIBUTE_VALUE> attributesMulti = {std::make_tuple(Attributes::Health, 20), std::make_tuple(Attributes::Power, 30)};
	std::vector<ATTRIBUTE_VALUE> oneEntityAttributes = {std::make_tuple(Attributes::Health, 80), std::make_tuple(Attributes::Attack, 50), std::make_tuple(Attributes::Defense, 5), std::make_tuple(Attributes::Power, 66)};

};

TEST_F(AttributeTransformTest, TestFlows) {
	Attributes healthAttr = Attributes::Health;
	AttributeFlow healthFlow;
	healthFlow.addInflow(make_tuple(10.0f, 1.0f)); //100.0f health increase at 10.0 health per second.
	healthFlow.addOutflow(make_tuple(5.0f, 1.0f));

	ATTRIBUTE_VALUE health = make_pair(healthAttr, 0.0f);

	float after10Seconds = 5.0f;

	for (size_t i = 0; i < 10; ++i) {
		health = healthFlow.getCurrentValue(health, 1.0f);
	}

	ASSERT_NEAR(after10Seconds, get<1>(health), 0.0001f);

	health = make_pair(healthAttr, 0.0f);

	float after5Seconds = 0.0f;

	for (size_t i = 0; i < 10000; ++i) {
		health = healthFlow.getCurrentValue(health, 10.0f / 1000.0f);
	}
	ASSERT_NEAR(after5Seconds, get<1>(health), 0.0001f);
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

