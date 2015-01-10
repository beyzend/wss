#include <iostream>

#include "gtest.h"
#include "wss/Utils.h"

class ScoreUtilsTest : public ::testing::Test {
protected:
	ScoreUtilsTest() {

	}

	virtual ~ScoreUtilsTest() {

	}

	virtual void SetUp() {

	}

	virtual void TearDown() {

	}

};

TEST_F(ScoreUtilsTest, TestScores) {
	using namespace wss;

	// Less than 0, 0 to 100,
	size_t valueOne = -1;
	size_t valueTwo = 0;
	size_t valueThree = 100;

	float constOne = 10.0 / size_t(-1);
	float constTwo = 10.0; // 10.0 / 0.0 is defined to return 10.0
	float constThree = 10.0 / 100;

	ASSERT_FLOAT_EQ(wss::Score::Attenuate(valueOne), constOne);
	ASSERT_FLOAT_EQ(wss::Score::Attenuate(valueTwo), constTwo);
	ASSERT_FLOAT_EQ(wss::Score::Attenuate(valueThree), constThree);

	float scoreOne = constOne - 10.0 / 20;
	float scoreTwo = constTwo - 10.0 / 50;
	float scoreThree = constThree - constThree;
	float scoreFour = 10.0 / 20.0 - 10.0 / 45;

	ASSERT_FLOAT_EQ(wss::Score::ComputeScore(-1, 20), scoreOne);
	ASSERT_FLOAT_EQ(wss::Score::ComputeScore(0, 50), scoreTwo);
	ASSERT_FLOAT_EQ(wss::Score::ComputeScore(100, 100), scoreThree);
	ASSERT_FLOAT_EQ(wss::Score::ComputeScore(20, 45), scoreFour);


}

int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
