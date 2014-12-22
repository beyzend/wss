/*
 * wssserver.cpp
 *
 *  Created on: Dec 20, 2014
 *      Author: beyzend
 */

#include <iostream>
#include <chrono>
#include <thread>


#include <zmqpp/zmqpp.hpp>

#include <tbb/flow_graph.h>
#include <tbb/concurrent_vector.h>
#include <glm/vec2.hpp>

#include <thread>
#include <mutex>

#include "micropather/micropather.h"

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filestream.h"

#include "wss/Utils.h"
#include "wss/Path.h"

#define within(num) (int) ((float) num * random () / (RAND_MAX + 1.0))

#define GLM_FORCE_SSSE3

const size_t MAP_W = 200;
const size_t MAP_H = 200;

struct Entity {

	size_t id;
	glm::vec2 position;
	size_t currentNode;
	std::vector<void*>* pathNodes;
	tbb::atomic<bool> traversing;
	Entity(size_t id, const glm::vec2 position) :
			id(id), position(position), currentNode(0), pathNodes(0), traversing(false) {
	}
};

struct PathEntity {
	size_t id;
	tbb::atomic<bool> traversing;
	std::vector<void*>* pathNodes;
	glm::vec2 position;

	PathEntity(size_t id, glm::vec2 position) : id(id), traversing(false), pathNodes(new std::vector<void*>()),
			position(position)
	{
	}

	PathEntity(const PathEntity &entity)
	{
		id = entity.id;
		traversing.store(entity.traversing);
		pathNodes = entity.pathNodes;
		position = entity.position;
	}

};

using PATH_FIND_NODE = tbb::flow::function_node<PathEntity*, PathEntity*>;



glm::vec2 randomPosition(glm::vec2 start, glm::vec2 end) {
	glm::vec2 range = end - start;
	return glm::vec2(start.x + within( range.x ), start.y + within( range.y ));
}

void serializeEntities(rapidjson::Document &root, size_t start, size_t end, tbb::concurrent_vector<Entity*> &entities) {
	using namespace rapidjson;

	// Create the positions.
	//Value positions;
	//positions.parse

	rapidjson::Value positions;
	positions.SetArray();

	rapidjson::Document::AllocatorType& allocator = root.GetAllocator();

	for(size_t i = start; i < end; ++i)
	{
		Entity* entity = entities[i];
		rapidjson::Value entityValue;
		entityValue.SetObject();

		//std::cout << "serializing entity id: " << entity->id << std::endl;
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
	root.AddMember("positions", positions, root.GetAllocator());

}


void regionDataPublisher(zmqpp::socket &publisher, tbb::concurrent_vector<Entity*> entities) {
	using namespace std;

	std::chrono::steady_clock clock;

	while (1) {
		auto start = clock.now();
		// Traverse nodes
		{
			for (auto entity : entities) {

				if (entity->traversing.load() == true ) {
					if (entity->currentNode >= entity->pathNodes->size()) {
						entity->pathNodes = 0;
					}
					else {
						size_t currentIndex = (size_t)(*entity->pathNodes)[entity->currentNode++];
						//wss::Utils::indexToXY(currentIndex, MAP_W, entity->position);
						size_t x, y;
						wss::Utils::indexToXY(currentIndex, MAP_W, entity->position);
						//cout << "current position: " << x << ", " << y << endl;
					}
				}

			}
		}

		{
			rapidjson::Document document;
			document.SetObject();
			serializeEntities(document, 0, entities.size(), entities);

			rapidjson::StringBuffer sb;
			rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

			document.Accept(writer);

			publisher.send(sb.GetString());
		}
		std::chrono::duration<double> elapsed = clock.now() - start;
		if (elapsed.count() < 1.0/5.0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1000/5 - (size_t)(elapsed.count() * 1000.0)));
		//cout << "elpased region publisher" << endl;
		}

	}
}

void traversePath(PATH_FIND_NODE &pathFindNode, std::vector<PathEntity*> &pathEntities, tbb::concurrent_vector<Entity*> entities) {
	using namespace std;

	std::chrono::steady_clock clock;

	while (1) {
		auto start = clock.now();
		// Loop through pathEntities.
		{
		for (auto pathEntity : pathEntities) {
			if (pathEntity->traversing.load() == true) {

				if (entities[pathEntity->id]->traversing.load() == false) { // Begin entity path traversal. Later on this can be a message.

					entities[pathEntity->id]->currentNode = 0;

					entities[pathEntity->id]->pathNodes = pathEntity->pathNodes;
					entities[pathEntity->id]->traversing.store(true);

				}
				else {
					if (entities[pathEntity->id]->pathNodes == 0) {
						entities[pathEntity->id]->traversing.store(false);
						pathEntity->traversing.store(false);
						// load the last path. need to change this behavior later.
						size_t currentIndex = (size_t)(*pathEntity->pathNodes)[pathEntity->pathNodes->size()-1];
						//wss::Utils::indexToXY(currentIndex, MAP_W, pathEntity->position);
						pathEntity->position = entities[pathEntity->id]->position;
						//pathEntity->pathNodes->clear();


					}
				}
			}
			else {
			// If path is no longer traversing
				// Generate a path find event
				//cout << "Putting pathEntity into it!" << endl;
				pathFindNode.try_put(pathEntity);
			}
		}
		}
		std::chrono::duration<double> elapsed = clock.now() - start;
		if (elapsed.count() < 1.0/5.0)
			std::this_thread::sleep_for(std::chrono::milliseconds(1000/5 - (size_t)(elapsed.count() * 1000.0)));
		elapsed = clock.now() - start;
		//cout << "AI traversed total time including sleep is: " << elapsed.count() * 1000.0 << endl;
	}
}

int main(int argc, char** argv) {
	using namespace std;
	using namespace glm;
	using namespace tbb;

	using PATH_VEC = vector<void*>;
	cout << "Starting Intel TBB and ZMQ server!" << endl;
	cout << "Constructing TBB flow graph" << endl;

	size_t i;
	tbb::flow::graph g;

	// Use a memory allocator here.
	tbb::concurrent_vector<Entity*> entities;
	std::vector<PathEntity*> pathEntities;

	glm::vec2 start(30,30), end(80,50);
	for (size_t i = 0; i < 950; ++i) {
		glm::vec2 position = randomPosition(start, end);
		pathEntities.push_back(new PathEntity(i, position));
		entities.push_back(new Entity(i, position));
	}

	// Path generator node. This node will input ID_PATH and generate an path then output it.
	PATH_FIND_NODE pathGenerator(g, 100, [=](PathEntity* pathEntity)->PathEntity* {
		return pathEntity;
	});

	std::vector<size_t> map;

	for (size_t y = 0; y < MAP_H; ++y) {
		for (size_t x = 0; x < MAP_W; ++x) {
			map.push_back(1);
		}
	}



	// Path return node. This node will take input ID_PATH and return it to it's corresponding path entity.
	PATH_FIND_NODE returnPathNode(g, 20, [&map](PathEntity* pathEntity)->PathEntity*{
		wss::Path path(MAP_W, MAP_H, map);
		micropather::MicroPather pather(&path);
		//cout << "solving path for id: " << pathEntity->id << endl;
		std::chrono::steady_clock clock;
		auto start = clock.now();
		size_t stateStart, stateEnd;
		//cout << "path start: " << pathEntity->position.x << ", " << pathEntity->position.y << endl;
		stateStart = wss::Utils::XYToIndex(pathEntity->position.x, pathEntity->position.y, MAP_W);
		stateEnd = wss::Utils::XYToIndex(30 + within(50), 30 + within(40), MAP_W);

		size_t startX,startY,endX,endY;

		wss::Utils::indexToXY(stateStart, MAP_W, startX, startY);
		wss::Utils::indexToXY(stateEnd, MAP_W, endX, endY);

		//cout << "startI: " << startX << ", " << startY << endl;
		//cout << "endI: " << endX << ", " << endY << endl;


		float cost;
		pathEntity->pathNodes->clear();
		pather.Solve((void*)stateStart, (void*)stateEnd, pathEntity->pathNodes, &cost);
		pathEntity->traversing.store(true);
		std::chrono::duration<double> elapsed = clock.now() - start;
		//cout << "Path solver elasped time: " << elapsed.count() * 1000.0 << endl;
		return pathEntity;
	});

	std::thread traversePathThread([&pathGenerator, &pathEntities, &entities]() {
		traversePath(pathGenerator, pathEntities, entities);
	});

	zmqpp::context context;
	zmqpp::socket regionDataSocket(context, zmqpp::socket_type::publish);
	regionDataSocket.bind("tcp://127.0.0.1:4200");

	std::thread updateRegionThread([&regionDataSocket, &entities]() {
		regionDataPublisher(regionDataSocket, entities);
	});

	tbb::flow::make_edge(pathGenerator, returnPathNode);

	g.wait_for_all();

	traversePathThread.join();
	updateRegionThread.join();

}



