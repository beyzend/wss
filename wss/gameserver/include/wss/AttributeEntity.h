#pragma once

#include "wss/Attributes.h"

namespace wss
{

class Advertisement;

using ADVERT_SCORE = std::tuple<const Advertisement*, float>;


class AttributeEntity
{
public:
	AttributeEntity(const std::vector<ATTRIBUTE_VALUE> &attributes);
	virtual ~AttributeEntity();

	float score(const Advertisement &advert);
	int pickAdvertisement(std::vector<ADVERT_SCORE> &scores);

protected:
private:
	std::vector<ATTRIBUTE_VALUE> _attributes;
};
}
