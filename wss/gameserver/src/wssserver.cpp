#include <iostream>
#include <string>
#include <sstream>

#include <zmqpp/zmqpp.hpp>

#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <chrono>
#include <thread>
#include <vector>

#include <glm/vec2.hpp>

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filestream.h"

#define within(num) (int) ((float) num * random () / (RAND_MAX + 1.0))

#define GLM_FORCE_SSSE3

struct Entity {

	size_t id;
	glm::vec2 position;

	Entity(size_t id, const glm::vec2 position) :
			id(id), position(position) {
	}

};

//const char EntityPosition[] = "{ \"id\": -1, \"pos\":[0, 0] }";
//const char EntityTemplate[] = "{ \"positions\": [ ]";

void serializeEntities(rapidjson::Document &root, std::vector<Entity> &entities) {
	using namespace rapidjson;

	// Create the positions.
	//Value positions;
	//positions.parse

	rapidjson::Value positions;
	positions.SetArray();

	rapidjson::Document::AllocatorType& allocator = root.GetAllocator();
	for (auto entity : entities) {
		rapidjson::Value entityValue;
		entityValue.SetObject();

		rapidjson::Value entityId;
		entityId.SetInt(entity.id);

		rapidjson::Value xPosition;
		rapidjson::Value yPosition;
		xPosition.SetDouble(entity.position.x);
		yPosition.SetDouble(entity.position.y);

		entityValue.AddMember("id", entityId, allocator);
		entityValue.AddMember("x", xPosition, allocator);
		entityValue.AddMember("y", yPosition, allocator);

		positions.PushBack(entityValue, allocator);

	}

	root.AddMember("positions", positions, root.GetAllocator());

}

void regionDataPublisher(zmqpp::socket &publisher, std::vector<Entity> &entities) {
	using namespace std;

	while (1) {

		stringstream entityStream;

		for (auto entity : entities) {
			entityStream << entity.id << ":" << entity.position.x << ":" << entity.position.y << " ";
		}
		std::string outString = entityStream.str();
		//std::cout << outString << std::endl;
		publisher.send(outString);



		// Test document
		rapidjson::Document document;
		document.SetObject();

		serializeEntities(document, entities);

		rapidjson::StringBuffer sb;

		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);

		document.Accept(writer);

		cout << "entity json: " << sb.GetString() << endl;

		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

}

void generateRandomEntities(size_t numOfEntities, glm::vec2 start, glm::vec2 end, std::vector<Entity> &entities) {

	float lenX = end.x - start.x;
	float lenY = end.y - start.y;

	for (size_t i = 0; i < numOfEntities; ++i) {
		size_t id = within( 100 );
		float x = within( lenX ) + start.x;
		float y = within( lenY ) + start.y;

		entities.push_back(Entity(id, glm::vec2(x, y)));
	}

}

int main(int argc, char** argv) {
	using namespace std;

	cout << "Starting ZeroMQ server..." << endl;

	std::vector<Entity> entities;

	generateRandomEntities(100, glm::vec2(0, 0), glm::vec2(240, 240), entities);

	zmqpp::context context;

	zmqpp::socket regionDataSocket(context, zmqpp::socket_type::publish);
	regionDataSocket.bind("tcp://127.0.0.1:4200");

	std::thread t1([&regionDataSocket, &entities]()
	{
		regionDataPublisher(regionDataSocket, entities);
	});
	t1.join();

	return 0;
}
