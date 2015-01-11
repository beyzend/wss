#include "wss/AttributeEntity.h"
#include "wss/Utils.h"
using namespace wss;

AttributeEntity::AttributeEntity(const std::vector<AttributeValue> &attributes) : _attributes(attributes) {

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

