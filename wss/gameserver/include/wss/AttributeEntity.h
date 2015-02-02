#pragma once

#include <memory>

#include "wss/wss.h"
#include "wss/Attributes.h"
#include "wss/AdvertCommand.h"
#include "wss/AttributesTransform.h"

namespace wss
{

class Advertisement;

using ADVERT_SCORE = std::tuple<const Advertisement*, float>;
using ATTRIBUTES_AND_FLOW = std::tuple<wss::Attributes, wss::AttributeFlow>;

struct AttributeUpdate {
	ATTRIBUTE_VALUE attrValue;
	std::shared_ptr<wss::AttributeFlow> transform;
	AttributeUpdate(const ATTRIBUTE_VALUE &attrValue, const AttributeFlow &transform) : attrValue(attrValue), transform(new AttributeFlow(transform)) {
	}
};

class AttributeEntity
{
public:
	/// @param attributes initial set of ATTRIBUTE_VALUE
	/// @param flows initial set of AttributeFlows which defines initial flows on attributes.
	AttributeEntity(size_t id, const std::vector<ATTRIBUTE_VALUE> &attributes, const std::vector<ATTRIBUTES_AND_FLOW> &flows = std::vector<ATTRIBUTES_AND_FLOW>());

	virtual ~AttributeEntity();

	float score(const std::vector<wss::ATTRIBUTE_VALUE> &deltas);
	int pickAdvertisement(std::vector<ADVERT_SCORE> &scores);

	void update(float dt);

	AdvertCommand getCommand();
	void setCommands(const AdvertCommand &command);

	size_t id;

	void getCurrentAttributes(std::vector<ATTRIBUTE_VALUE> &values);

protected:
private:
	std::vector<AttributeUpdate> _attributes;
	AdvertCommand _command;
};
}
