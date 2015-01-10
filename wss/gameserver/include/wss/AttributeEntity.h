#pragma once

#include "wss/wss.gch"

#include "wss/Advertisement.h"

namespace wss
{

class AttributeEntity
{
public:
	AttributeEntity();
	~AttributeEntity();

	float score(const Advertisement &advert);

protected:
private:
};
}
