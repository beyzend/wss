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

#include <rapidjson/document.h>
#include "rapidjson/prettywriter.h"
#include "rapidjson/filestream.h"

#include "wss/Path.h"
#include "wss/Utils.h"

#define within(num) (int) ((float) num * random () / (RAND_MAX + 1.0))

#define GLM_FORCE_SSSE3

struct Entity {

	size_t id;
	glm::vec2 position;
	std::vector<void*>* paths;
	std::size_t currentPathNode;


	Entity(size_t id, const glm::vec2 position) :
			id(id), position(position), paths(0), currentPathNode(0) {
	}

};

struct PathEntity {
	size_t id;
	bool traversingPath;
	size_t currentNode;

	PathEntity(size_t id, bool traversingPath = false) :
		id(id), traversingPath(traversingPath), currentNode(0) {

	}
};

const size_t MAP_W = 250;
const size_t MAP_H = 250;

std::recursive_mutex mutex;
std::atomic_size_t atomicId;

//const char EntityPosition[] = "{ \"id\": -1, \"pos\":[0, 0] }";
//const char EntityTemplate[] = "{ \"positions\": [ ]";

void serializeEntities(rapidjson::Document &root, std::vector<Entity*> &entities) {
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
		entityId.SetInt(entity->id);

		rapidjson::Value xPosition;
		rapidjson::Value yPosition;
		xPosition.SetDouble(entity->position.x);
		yPosition.SetDouble(entity->position.y);

		entityValue.AddMember("id", entityId, allocator);
		entityValue.AddMember("x", xPosition, allocator);
		entityValue.AddMember("y", yPosition, allocator);


		positions.PushBack(entityValue, allocator);

	}
	lock.unlock();
	root.AddMember("positions", positions, root.GetAllocator());

}

void regionDataPublisher(zmqpp::socket &publisher, std::vector<Entity*> &entities) {
	using namespace std;

	std::chrono::steady_clock clock;

	std::vector< std::chrono::time_point< std::chrono::steady_clock > > startTimes;

	for (auto entity : entities)
	{
		startTimes.push_back(clock.now());
	}

	const double TIME_PER_NODE = 1.0/5.0;
	std::chrono::duration<double, std::ratio<1,5> > constantTime;
	size_t processed = 0;

	while (1) {

		//std::chrono::time_point< std::chrono::steady_clock > start = clock.now();
		auto start = clock.now();
		// Update path
		{
			std::lock_guard<std::recursive_mutex> lock(::mutex);
			for (auto entity : entities) {
				if (entity->paths) {
					glm::vec2 position;
					if (entity->currentPathNode + 1 >= entity->paths->size()) { // At last node.
						size_t x,y;
						wss::Utils::indexToXY((size_t)(*entity->paths)[entity->currentPathNode], MAP_W, x, y);
						position = glm::vec2(x, y);
						entity->currentPathNode = 0;
						entity->paths = 0;
					}
					else { // still has node to traverse
						std::chrono::time_point<std::chrono::steady_clock> now = clock.now();

						std::chrono::duration<double> elapsed = now - startTimes[entity->id];
						if (elapsed.count() > TIME_PER_NODE) {
							size_t x, y;
							size_t currentIndex = (size_t)(*entity->paths)[entity->currentPathNode++];
							wss::Utils::indexToXY(currentIndex, MAP_W, x, y);
							position = glm::vec2(x, y);
							startTimes[entity->id] = now;
						}
					}
					entity->position = position;
				}
			}
		}


		// Test document
		rapidjson::Document document;
		document.SetObject();
		serializeEntities(document, entities);

		rapidjson::StringBuffer sb;

		rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

		document.Accept(writer);

		publisher.send(sb.GetString());

		std::chrono::duration<double> elapsed = clock.now() - start;
		if (elapsed.count() < 1.0/2.0)
			std::this_thread::sleep_for(std::chrono::milliseconds(1000/2 - (size_t)(elapsed.count() * 1000.0)));
	}

}

size_t uniqueId()
{
	return atomicId++;
}

void addPlayer(size_t id, std::vector<size_t> playerIds)
{
	std::unique_lock<std::recursive_mutex> lock(mutex);
	playerIds.push_back(id);
	lock.unlock();
}

size_t addEntity(rapidjson::StringBuffer &sb, const rapidjson::Value &value, std::vector<Entity*> &entities)
{
	size_t id = uniqueId();
	std::cout << "Add entity with id: " << id << std::endl;
	std::unique_lock<std::recursive_mutex> lock(mutex);
	entities.push_back(new Entity(id, glm::vec2(50, 50)));
	lock.unlock();

	// Send reply
	rapidjson::Document replyRoot;
	replyRoot.SetObject();
	rapidjson::Document::AllocatorType& allocator = replyRoot.GetAllocator();

	replyRoot.AddMember("type", 1, allocator);
	replyRoot.AddMember("id", id, allocator);

	rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

	replyRoot.Accept(writer);

	std::cout << "replying add entity: " << sb.GetString() << std::endl;

	return id;

}

void removeEntityWithReply(zmqpp::socket &requestSocket, const rapidjson::Value &data, std::vector<Entity*> &entities)
{
	//{ "id":int }
	const rapidjson::Value &idValue =  data["id"];

	if (idValue.IsUint()) {
		size_t id = idValue.GetUint();
		std::cout << "Remove entity with id: " << id << std::endl;

		std::lock_guard<std::recursive_mutex> lock(mutex);
		entities.erase(std::remove_if(entities.begin(), entities.end(), [id](Entity *entity){
			return entity->id == id;
		}), entities.end());
	}
}

/*
 * This method handles asynchronous region requests.
 */
void regionRequestHandler(zmqpp::socket &requestSocket, std::vector<Entity*> &entities, std::vector<size_t> &playerIds) {
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
				{
					rapidjson::StringBuffer sb;
					size_t id = addEntity(sb, data, entities);
					addPlayer(id, playerIds);
					requestSocket.send(sb.GetString());
				}
					break;
				default:
					break;
				}
			}
			// Throw error here

		}

	}

}

/**
 * NOTE: This is just temp. for testing AI path finder.
 */
void regionAIUpdater(std::vector<Entity*> &entities, std::vector<PathEntity*> &pathEntities, std::vector<size_t> &map) {

	std::cout << "regionAIUpdater: " << std::endl;
	wss::Path path(MAP_W, MAP_H, map);
	micropather::MicroPather pather(&path);
	std::vector<std::vector<void*>*> pathNodes;
	for (auto pathEntity : pathEntities) {
		pathNodes.push_back(new std::vector<void*>());
	}

	size_t processed = 0;

	while(1) {
		std::vector<glm::vec2> positions;
		// Hackish. I'm just prototyping. I will think about thread performance later. I want to use a tasked based threading system here.
		for (auto pathEntity : pathEntities) {
			if (pathEntity->traversingPath) {
				{
					std::unique_lock<std::recursive_mutex> lock(mutex, std::try_to_lock);

					if (lock.owns_lock()) {
						if (entities[pathEntity->id]->paths == 0) {
							pathEntity->traversingPath = false;
						}
					}
				}

			}
			else {
				pathEntity->traversingPath = true;
				glm::vec2 entityPosition;
				{
					std::lock_guard<std::recursive_mutex> lock(mutex);
					entityPosition = entities[pathEntity->id]->position;
				}
				size_t stateStart, stateEnd;
				stateStart = wss::Utils::XYToIndex(entityPosition.x, entityPosition.y, MAP_W);
				stateEnd = wss::Utils::XYToIndex(within(100), within(100), MAP_W);
				float totalCost = 0;
				pathNodes[pathEntity->id]->clear();
				pather.Solve((void*) stateStart, (void*) stateEnd, pathNodes[pathEntity->id], &totalCost);
				{
					std::unique_lock<std::recursive_mutex> lock(mutex, std::try_to_lock);
					if (lock.owns_lock()) {
						entities[pathEntity->id]->paths = pathNodes[pathEntity->id];
					}
				}

			}
			if (processed++ % 20 == 0) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1000/20));
			}
		}
		//std::this_thread::sleep_for(std::chrono::milliseconds(1000/30));

	}
}

void generateRandomEntities(size_t numOfEntities, glm::vec2 start, glm::vec2 end, std::vector<Entity*> &entities, std::vector<PathEntity*> &pathEntities) {

	float lenX = end.x - start.x;
	float lenY = end.y - start.y;

	for (size_t i = 0; i < numOfEntities; ++i) {
		size_t id = uniqueId();
		float x = within( lenX ) + start.x;
		float y = within( lenY ) + start.y;

		entities.push_back(new Entity(id, glm::vec2(x, y)));
		pathEntities.push_back(new PathEntity(id));
	}

}

int main(int argc, char** argv) {
	using namespace std;
	using namespace glm;

	cout << "Starting ZeroMQ server..." << endl;

	cout << "Constructing map!" << endl;

	std::vector<Entity*> entities;
	std::vector<size_t> playerIds;
	std::vector<PathEntity*> pathEntities;

	generateRandomEntities(50, glm::vec2(30, 30), glm::vec2(70, 70), entities, pathEntities);

	zmqpp::context context;

	zmqpp::socket regionDataSocket(context, zmqpp::socket_type::publish);
	regionDataSocket.bind("tcp://127.0.0.1:4200");

	zmqpp::socket regionRequestSocket(context, zmqpp::socket_type::reply);
	regionRequestSocket.bind("tcp://127.0.0.1:4201");

	std::thread t1([&regionDataSocket, &entities]()
	{
		regionDataPublisher(regionDataSocket, entities);
	});

	std::thread t2([&regionRequestSocket, &entities, &playerIds]()
	{
		regionRequestHandler(regionRequestSocket, entities, playerIds);
	});


	std::vector<size_t> map;

	for (size_t y = 0; y < MAP_H; ++y) {
		for (size_t x = 0; x < MAP_W; ++x) {
			map.push_back(1);
		}
	}

	std::thread t3([&entities, &pathEntities, &map]() {
		regionAIUpdater(entities, pathEntities, map);
	});


	t1.join();
	t2.join();
	return 0;
}
