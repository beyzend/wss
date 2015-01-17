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

bool NetworkClient::updatePositions(vector<Sprite*> &entities) {
	bool updated = false;

	zmqpp::message_t message;
	_regionDataSocket.receive(message,true);
	if (message.parts() > 0) {

		for (size_t i = 0; i < message.parts() / 3; ++i) {
			size_t id;
			float x,y;
			message >> id >> x >> y;
			//positions[id] = Vec2(x * 16.0, y * 16.0);//->setPosition(Vec2(x * 16.0, -y * 16.0) + worldToCamera);
			entities[id]->setPosition(x * 16.0, -y * 16.0);
		}
	}
	return updated;
}
