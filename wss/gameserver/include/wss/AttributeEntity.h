#pragma once

#include "wss/wss.h"
#include "wss/Attributes.h"
#include "wss/AdvertCommand.h"

namespace wss
{

class Advertisement;

using ADVERT_SCORE = std::tuple<const Advertisement*, float>;

class AttributeEntity
{
public:
	AttributeEntity(size_t id, const std::vector<ATTRIBUTE_VALUE> &attributes);
	virtual ~AttributeEntity();

	float score(const Advertisement &advert);
	int pickAdvertisement(std::vector<ADVERT_SCORE> &scores);

	AdvertCommand getCommand();
	void setCommands(const AdvertCommand &command);

	size_t id;

protected:
private:
	std::vector<ATTRIBUTE_VALUE> _attributes;
	AdvertCommand _command;
};
}
