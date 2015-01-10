/*
 * wssserver.cpp
 *
 *  Created on: Dec 20, 2014
 *      Author: beyzend
 */

#include <iostream>
#include <chrono>
#include <thread>
#include <tuple>

#include <zmqpp/zmqpp.hpp>

#include <tbb/flow_graph.h>
#include <tbb/concurrent_vector.h>
#include <tbb/concurrent_queue.h>
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

using PATH_FIND_NODE = tbb::flow::function_node<std::tuple<size_t, glm::vec2>, PathEntity*>;
using PATH_SOLVE_NODE = tbb::flow::function_node<PathEntity*, PathEntity*>;


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

		if (entity->pathNodes) { // Only write out those with a path

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
	}
	root.AddMember("positions", positions, root.GetAllocator());

}


void regionDataPublisher(zmqpp::socket &publisher, PATH_FIND_NODE &pathFindNode, tbb::concurrent_queue<PathEntity*> &solvedPathQueue, tbb::concurrent_vector<Entity*> entities) {
	using namespace std;

	std::chrono::steady_clock clock;

	// Initialize path.
	for (auto entity : entities) {
		pathFindNode.try_put(std::tuple<size_t, glm::vec2>(entity->id, entity->position));
	}

	while (1) {
		auto start = clock.now();

		// Grab a bunch fo path
		{
			//size_t size = entities.size();
			for (size_t i = 0; i < 200; ++i) {
				PathEntity* pathEntity;
				if (solvedPathQueue.try_pop(pathEntity)) {

					entities[pathEntity->id]->pathNodes = pathEntity->pathNodes;
					entities[pathEntity->id]->currentNode = 0;
				}
			}
		}

		// Traverse nodes
		{
			for (auto entity : entities) {
				if (entity->pathNodes != 0) {
					if (entity->currentNode < entity->pathNodes->size()) {
						size_t currentIndex = (size_t)(*entity->pathNodes)[entity->currentNode++];
						//wss::Utils::indexToXY(currentIndex, MAP_W, entity->position);
						wss::Utils::indexToXY(currentIndex, MAP_W, entity->position);
					}
					else {
						entity->pathNodes = 0;
						pathFindNode.try_put(std::tuple<size_t, glm::vec2>(entity->id, entity->position));
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
	for (size_t i = 0; i < 1000; ++i) {
		glm::vec2 position = randomPosition(start, end);
		pathEntities.push_back(new PathEntity(i, position));
		entities.push_back(new Entity(i, position));
	}

	std::vector<size_t> map;

	for (size_t y = 0; y < MAP_H; ++y) {
		for (size_t x = 0; x < MAP_W; ++x) {
			map.push_back(1);
		}
	}

	// Path generator node. This node will input ID_PATH and generate an path then output it.
	PATH_FIND_NODE pathGenerator(g, 25, [&pathEntities](std::tuple<size_t, glm::vec2> pathRequest)->PathEntity* {
		size_t id = std::get<0>(pathRequest);
		pathEntities[id]->position = std::get<1>(pathRequest);
		return pathEntities[id];
	});

	tbb::concurrent_queue<PathEntity*> solvedPathQueue;

	// Path return node. This node will take input ID_PATH and return it to it's corresponding path entity.
	PATH_SOLVE_NODE solvePathNode(g, 25, [&map, &solvedPathQueue](PathEntity* pathEntity)->PathEntity*{
		wss::Path path(MAP_W, MAP_H, map);
		micropather::MicroPather pather(&path);
		std::chrono::steady_clock clock;

		auto start = clock.now();
		size_t stateStart, stateEnd;
		stateStart = wss::Utils::XYToIndex(pathEntity->position.x, pathEntity->position.y, MAP_W);
		stateEnd = wss::Utils::XYToIndex(within(200), within(200), MAP_W );

		int startX,startY,endX,endY;

		wss::Utils::indexToXY(stateStart, MAP_W, startX, startY);
		wss::Utils::indexToXY(stateEnd, MAP_W, endX, endY);

		float cost;
		pathEntity->pathNodes->clear();
		pather.Solve((void*)stateStart, (void*)stateEnd, pathEntity->pathNodes, &cost);
		pathEntity->traversing.store(true);
		solvedPathQueue.push(pathEntity);
		//cout << "solvedPathQueue pushed! " << pathEntity->id << endl;
		std::chrono::duration<double> elapsed = clock.now() - start;
		//cout << "Path solver elasped time: " << elapsed.count() * 1000.0 << endl;

		return pathEntity;
	});

	zmqpp::context context;
	zmqpp::socket regionDataSocket(context, zmqpp::socket_type::publish);
	regionDataSocket.bind("tcp://127.0.0.1:4200");

	std::thread updateRegionThread([&regionDataSocket, &pathGenerator, &solvedPathQueue, &entities]() {
		regionDataPublisher(regionDataSocket, pathGenerator, solvedPathQueue, entities);
	});

	tbb::flow::make_edge(pathGenerator, solvePathNode);

	g.wait_for_all();

	updateRegionThread.join();

}



