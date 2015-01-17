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
	bool updatePositions(std::vector<cocos2d::Sprite*> &positions);
private:
	zmqpp::context _context;
	zmqpp::socket _regionDataSocket;
};
}
