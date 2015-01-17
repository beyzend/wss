#pragma once

#include <vector>

#include <cocos2d.h>
#include <zmqpp/zmqpp.hpp>

namespace WssClient
{
class NetworkClient {
public:
	NetworkClient();
	virtual ~NetworkClient();

	void connect();
	bool updatePositions(size_t numToRead, std::vector<cocos2d::Vec2> &positions);
private:
	zmqpp::context _context;
	zmqpp::socket _regionDataSocket;
};
}
