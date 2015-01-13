#include <vector>
#include <memory>
#include <algorithm>
#include <tuple>
#include <queue>

#include <iostream>

#include "gtest.h"
#include "wss/Attributes.h"
#include "wss/AttributeEntity.h"
#include "wss/Advertisement.h"

using namespace wss;
using namespace std;

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

	// Test those that match none match one, match multiple, matches all
	std::vector<ATTRIBUTE_VALUE> attributesMatchNone = {std::make_tuple(Attributes::Hunger, 2)};
	std::vector<ATTRIBUTE_VALUE> attributesOne = {std::make_tuple(Attributes::Health, 5)};
	std::vector<ATTRIBUTE_VALUE> attributesMulti = {std::make_tuple(Attributes::Health, 20), std::make_tuple(Attributes::Power, 30)};
	std::vector<ATTRIBUTE_VALUE> oneEntityAttributes = {std::make_tuple(Attributes::Health, 80), std::make_tuple(Attributes::Attack, 50), std::make_tuple(Attributes::Defense, 5), std::make_tuple(Attributes::Power, 66)};

	wss::AdvertCommand command = wss::AdvertCommand("test", std::queue<AdvertBehaviorTest>({AdvertBehaviorTest::NONE}), std::queue<glm::vec2>({glm::vec2()}));
	std::vector<AdvertCommand> commands = {wss::AdvertCommand(command)};
};

TEST_F(AttributeTest, TestScore) {
	// Not using a mock object for now.

	AttributeEntity oneEntity(oneEntityAttributes);

	using AttrVector = std::vector<std::vector<ATTRIBUTE_VALUE>>;
	using ScoreVector = std::vector<float>;

	auto testScore = [](int32_t currentVal, int32_t futureVal) {
		return 10.0f / currentVal - 10.0f / futureVal;
	};

	AttrVector attrs = {attributesMatchNone, attributesOne, attributesMulti};
	//attributes One: A(80) - A(80+5)-- attributesMulti: Health, A(80) - A(80+20); Power: A(66) - A(66+30)
	ScoreVector scores = {0.0f, testScore(80, 85), testScore(80, 100) + testScore(66, 96) };

	auto doTest = [&](AttributeEntity &oneEntity, AttrVector &attrs, ScoreVector &scores) {
		for (size_t i = 0; i < attrs.size(); ++i) {
			auto attr = attrs[i];
			float constScore = scores[i];


			Advertisement advert(attr, commands);
			float score = oneEntity.score(advert);

			ASSERT_FLOAT_EQ(constScore, score);
		}
	};

	doTest(oneEntity, attrs, scores);
}

// NOTE: The math behind the models should have been tested already. If I had more time I would throughly test the math model here. Since we don't, just make sure models
//are tested in a math modeler.
TEST_F(AttributeTest, TestPickAdvert) {


	AttributeEntity oneEntity(oneEntityAttributes);

	std::vector<ATTRIBUTE_VALUE> anotherOne = {std::make_tuple(Attributes::Health, 20)};

	Advertisement advertOne(attributesOne, commands);
	Advertisement betterOne(anotherOne, commands);

	float scoreOne = oneEntity.score(advertOne);
	float betterScore = oneEntity.score(betterOne);

	std::vector<ADVERT_SCORE> scoreTuples = {std::make_tuple(&advertOne, scoreOne), std::make_tuple(&betterOne, betterScore)};

	int index = oneEntity.pickAdvertisement(scoreTuples);

	ASSERT_EQ(0, index);

}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

