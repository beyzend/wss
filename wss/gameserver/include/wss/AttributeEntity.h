#pragma once

#include "wss/wss.gch"

#include "wss/Advertisement.h"

namespace wss
{

class AttributeEntity
{
public:
	AttributeEntity(const std::vector<AttributeValue> &attributes);
	virtual ~AttributeEntity();

	float score(const Advertisement &advert);

protected:
private:
	std::vector<AttributeValue> _attributes;
};
}
