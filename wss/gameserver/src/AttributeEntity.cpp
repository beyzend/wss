#include <algorithm>

#include "wss/AttributeEntity.h"
#include "wss/Utils.h"

using namespace wss;

AttributeEntity::AttributeEntity(const std::vector<ATTRIBUTE_VALUE> &attributes) : _attributes(attributes) {

}

AttributeEntity::~AttributeEntity() {

}

float AttributeEntity::score(const Advertisement &advert) {

	float returnScore = 0.0f;
	for (auto advertAttributeDelta : advert.getDeltas()) {
		Attributes advertAttribute = std::get<0>(advertAttributeDelta);
		float delta = std::get<1>(advertAttributeDelta);
		// Loop thru each attribute in entity to find a match. Must assert no duplicate attributes.
		for (auto entityAttribute : _attributes) {
			if (std::get<0>(entityAttribute) == advertAttribute) {
				// Add this score
				std::int32_t futureValue = std::get<1>(entityAttribute) + delta;
				returnScore += Score::ComputeScore(std::get<1>(entityAttribute), futureValue);
			}
		}
	}
	return returnScore;
}

/***
 * This method will pick an advertisment based on passed in ADVERT_SCORE tuple.
 * This method will alter the passed in vector.
 *
 * \return ADVERT_SCORE tuple containing advert and associated score.
 */
int AttributeEntity::pickAdvertisement(std::vector<ADVERT_SCORE> &scores) {

		if (scores.size() < 1)
		return -1;

	// For now use the top one.
	std::sort(scores.begin(), scores.end(), [](ADVERT_SCORE score1, ADVERT_SCORE score2){
		return std::get<1>(score1) < std::get<1>(score2);
	});

	// Just return the first one.
	return 0;
}

