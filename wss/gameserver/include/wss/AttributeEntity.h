#pragma once

#include "wss/wss.gch"

#include "wss/Advertisement.h"

namespace wss
{

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
