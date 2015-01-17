#include "Client.h"
#include <iostream>

using namespace std;
using namespace WssClient;
using namespace cocos2d;

NetworkClient::NetworkClient() : _regionDataSocket(_context, zmqpp::socket_type::subscribe){

}

NetworkClient::~NetworkClient() {
}

void NetworkClient::connect() {

	_regionDataSocket = zmqpp::socket(_context, zmqpp::socket_type::subscribe);
	_regionDataSocket.subscribe("");

	_regionDataSocket.connect("tcp://127.0.0.1:4200");
	cout << "Connecting to zmq server at 4200" << endl;

}

bool NetworkClient::updatePositions(size_t numToRead, vector<Vec2> &positions) {
	bool updated = false;

	zmqpp::message_t message;
	_regionDataSocket.receive(message,true);
	if (message.parts() > 0) {

		for (size_t i = 0; i < message.parts() / 3; ++i) {
			size_t id;
			float x,y;
			message >> id >> x >> y;
			positions[id] = Vec2(x * 16.0, y * 16.0);//->setPosition(Vec2(x * 16.0, -y * 16.0) + worldToCamera);

			if (i%100 == 0) {
				//cout << "id,x,y: " << id << ", " << x << ", " << y << endl;
			}
		}
	}
	return updated;
}
