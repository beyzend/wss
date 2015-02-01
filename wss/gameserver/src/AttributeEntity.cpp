#include "wss/wss.h"

#include <algorithm>
#include <tuple>

#include "wss/AttributeEntity.h"
#include "wss/Advertisement.h"
#include "wss/Utils.h"

using namespace std;
using namespace wss;



AttributeEntity::AttributeEntity(size_t id, const std::vector<ATTRIBUTE_VALUE> &attributes, const vector<ATTRIBUTES_AND_FLOW> &flows) :
		id(id) {

	if (flows.size() == 0){
		for (auto attribute : attributes) {
			_attributes.push_back(AttributeUpdate(attribute, AttributeFlow()));
		}
	}
	else {
		for (auto attribute : attributes) {
			Attributes entAttr; float value;
			tie(entAttr, value) = attribute;
			auto found = find_if(flows.cbegin(), flows.cend(), [=](const ATTRIBUTES_AND_FLOW flow)->bool {
				Attributes attr; AttributeFlow trans;
				tie(attr, trans);
				return (entAttr == attr);
			});
			if (found != flows.cend()) {
				_attributes.push_back(AttributeUpdate(attribute, get<1>(*found)));
			}
			else {
				_attributes.push_back(AttributeUpdate(attribute, AttributeFlow()));
			}
		}
	}
}

AttributeEntity::~AttributeEntity() {

}

float AttributeEntity::score(const std::vector<wss::ATTRIBUTE_VALUE> &deltas) {

	float returnScore = 0.0f;
	for (auto advertAttributeDelta : deltas) {
		Attributes advertAttribute; float delta;
		tie(advertAttribute, delta) = advertAttributeDelta;

		// Loop thru each attribute in entity to find a match. Must assert no duplicate attributes.
		for (auto entityAttribute : _attributes) {
			Attributes attribute; float currentValue;
			tie(attribute, currentValue) = entityAttribute.attrValue;
			if (attribute == advertAttribute) {
				// Add this score
				std::int32_t futureValue = currentValue + delta;
				returnScore += Score::ComputeScore(currentValue, futureValue);
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
	std::sort(scores.begin(), scores.end(), [](ADVERT_SCORE score1, ADVERT_SCORE score2) {
		return std::get<1>(score1) < std::get<1>(score2);
	});

	// Just return the first one.
	return 0;
}

AdvertCommand AttributeEntity::getCommand() {
	return _command;
}

void AttributeEntity::setCommands(const AdvertCommand& command) {
	_command = command;
}
