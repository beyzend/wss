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
#include <mutex>
#include <atomic>

#include <algorithm>

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

std::recursive_mutex mutex;
std::atomic_size_t atomicId;

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

	std::unique_lock<std::recursive_mutex> lock(mutex);
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
	lock.unlock();
	root.AddMember("positions", positions, root.GetAllocator());

}

void regionDataPublisher(zmqpp::socket &publisher, std::vector<Entity> &entities) {
	using namespace std;

	while (1) {

		// Test document
		rapidjson::Document document;
		document.SetObject();

		serializeEntities(document, entities);

		rapidjson::StringBuffer sb;

		rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

		document.Accept(writer);

		publisher.send(sb.GetString());

		std::this_thread::sleep_for(std::chrono::milliseconds(1000/5));
	}

}

size_t uniqueId()
{
	return atomicId++;
}

void addEntityWithReply(zmqpp::socket &requestSocket, const rapidjson::Value &value, std::vector<Entity> &entities)
{
	size_t id = uniqueId();
	std::cout << "Add entity with id: " << id << std::endl;
	std::unique_lock<std::recursive_mutex> lock(mutex);
	entities.push_back(Entity(id, glm::vec2(within(250), within(250))));
	lock.unlock();

	// Send reply
	rapidjson::Document replyRoot;
	replyRoot.SetObject();
	rapidjson::Document::AllocatorType& allocator = replyRoot.GetAllocator();

	replyRoot.AddMember("type", 1, allocator);
	replyRoot.AddMember("id", id, allocator);

	rapidjson::StringBuffer sb;
	rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

	replyRoot.Accept(writer);

	std::cout << "replying add entity: " << sb.GetString() << std::endl;

	requestSocket.send(sb.GetString());

}

void removeEntityWithReply(zmqpp::socket &requestSocket, const rapidjson::Value &data, std::vector<Entity> &entities)
{
	//{ "id":int }
	const rapidjson::Value &idValue =  data["id"];

	if (idValue.IsUint()) {
		size_t id = idValue.GetUint();
		std::cout << "Remove entity with id: " << id << std::endl;

		std::lock_guard<std::recursive_mutex> lock(mutex);
		entities.erase(std::remove_if(entities.begin(), entities.end(), [id](Entity entity){
			return entity.id == id;
		}), entities.end());
	}
}

/*
 * This method handles asynchronous region requests.
 */
void regionRequestHandler(zmqpp::socket &requestSocket, std::vector<Entity> &entities) {
	using namespace std;

	const size_t ADD_ENTITY = 1;
	const size_t REMOVE_ENTITY = 2;

	while (1) {
		std::string request;
		requestSocket.receive(request);
		// parse out JSON request
		rapidjson::Document requestRoot;

		requestRoot.Parse(request.c_str());

		// { "type":int, "data: {}" }
		if (requestRoot.HasMember("type"))
		{
			rapidjson::Value& type = requestRoot["type"];
			if (type.IsUint())
			{
				rapidjson::Value& data = requestRoot["data"];
				assert(data.IsObject()); //Except over here

				switch(type.GetUint())
				{
				case ADD_ENTITY:
					addEntityWithReply(requestSocket, data, entities);
					break;
				default:
					break;
				}
			}
			// Throw error here

		}

	}

}

void generateRandomEntities(size_t numOfEntities, glm::vec2 start, glm::vec2 end, std::vector<Entity> &entities) {

	float lenX = end.x - start.x;
	float lenY = end.y - start.y;

	for (size_t i = 0; i < numOfEntities; ++i) {
		size_t id = uniqueId();
		float x = within( lenX ) + start.x;
		float y = within( lenY ) + start.y;

		entities.push_back(Entity(id, glm::vec2(x, y)));
	}

}

int main(int argc, char** argv) {
	using namespace std;

	cout << "Starting ZeroMQ server..." << endl;

	std::vector<Entity> entities;

	generateRandomEntities(100, glm::vec2(0, 0), glm::vec2(100, 100), entities);

	zmqpp::context context;

	zmqpp::socket regionDataSocket(context, zmqpp::socket_type::publish);
	regionDataSocket.bind("tcp://127.0.0.1:4200");

	zmqpp::socket regionRequestSocket(context, zmqpp::socket_type::reply);
	regionRequestSocket.bind("tcp://127.0.0.1:4201");

	std::thread t1([&regionDataSocket, &entities]()
	{
		regionDataPublisher(regionDataSocket, entities);
	});

	std::thread t2([&regionRequestSocket, &entities]()
	{
		regionRequestHandler(regionRequestSocket, entities);
	});


	t1.join();
	t2.join();
	return 0;
}
