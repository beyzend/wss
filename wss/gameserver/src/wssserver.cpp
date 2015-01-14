/*
 * wssserver.cpp
 *
 *  Created on: Dec 20, 2014
 *      Author: beyzend
 */

#include "wss/wss.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <tuple>
#include <unordered_map>

#include <zmqpp/zmqpp.hpp>

#include <tbb/flow_graph.h>
#include <tbb/concurrent_vector.h>
#include <tbb/concurrent_queue.h>
#include <glm/vec2.hpp>
#include <glm/gtc/random.hpp>

#include <thread>
#include <mutex>

#include "micropather/micropather.h"

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filestream.h"

#include "wss/Utils.h"
#include "wss/Path.h"
#include "wss/Attributes.h"
#include "wss/AttributeEntity.h"
#include "wss/Advertisement.h"
#include "wss/AdvertCommand.h"

#define within(num) (int) ((float) num * random () / (RAND_MAX + 1.0))

#define GLM_FORCE_SSSE3

const size_t MAP_W = 200;
const size_t MAP_H = 200;

size_t ZONE_SIZE = 20;
size_t NUM_OF_ZONES = MAP_W / ZONE_SIZE;

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
	glm::vec2 moveTo;

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

using PROCESS_ATTRIBUTE_NODE = tbb::flow::function_node<std::tuple<size_t, glm::vec2>>;
using PATH_FIND_NODE = tbb::flow::function_node<std::tuple<size_t, glm::vec2, glm::vec2>, PathEntity*>;
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


void regionDataPublisher(zmqpp::socket &publisher, PROCESS_ATTRIBUTE_NODE &pathFindNode, tbb::concurrent_queue<PathEntity*> &solvedPathQueue, tbb::concurrent_vector<Entity*> entities) {
	using namespace std;

	std::chrono::steady_clock clock;

	// Initialize path.
	for (auto entity : entities) {
		pathFindNode.try_put(make_pair(entity->id, entity->position));//(std::tuple<size_t, glm::vec2>(entity->id, entity->position));
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

PROCESS_ATTRIBUTE_NODE* processNext = nullptr;

PROCESS_ATTRIBUTE_NODE* getProcessNext() {
	return processNext;
}


int main(int argc, char** argv) {
	using namespace std;
	using namespace glm;
	using namespace tbb;
	using namespace wss;

	using PATH_VEC = vector<void*>;
	cout << "Starting Intel TBB and ZMQ server!" << endl;
	cout << "Constructing TBB flow graph" << endl;

	size_t i;
	tbb::flow::graph g;

	// SETUP ENTITIES

	// Use a memory allocator here.
	tbb::concurrent_vector<Entity*> entities;
	std::vector<PathEntity*> pathEntities;
	std::vector<std::shared_ptr<AttributeEntity>> attributeEntities;

	glm::vec2 start(30,30), end(80,50);
	for (size_t i = 0; i < 1000; ++i) {
		glm::vec2 position = randomPosition(start, end);
		pathEntities.push_back(new PathEntity(i, position));
		entities.push_back(new Entity(i, position));
		// create attribute entities with health and happiness
		vector<ATTRIBUTE_VALUE> attributes = {std::make_pair(Attributes::Health, 100), std::make_pair(Attributes::Happiness, 50)};
		attributeEntities.push_back(shared_ptr<AttributeEntity>(new AttributeEntity(i, attributes)));
	}

	std::vector<size_t> map;

	for (size_t y = 0; y < MAP_H; ++y) {
		for (size_t x = 0; x < MAP_W; ++x) {
			map.push_back(1);
		}
	}

	using ADVERT_POS = std::tuple<std::shared_ptr<wss::Advertisement>, glm::vec2>;
	using vSECTOR_ADVERTS = std::vector<ADVERT_POS>;
	std::unordered_map<size_t, vSECTOR_ADVERTS> advertZones;

	size_t advertsPerZone = 5;
	// Setup ADVERTISEMENTS: randomly create random advertisement within each zone.
	for (size_t y = 0; y < NUM_OF_ZONES; ++y) {
		for (size_t x = 0; x < NUM_OF_ZONES; ++x) {
			glm::vec2 center(x*ZONE_SIZE, y*ZONE_SIZE);
			std::vector<ADVERT_POS> advertPos;
			size_t zoneIndex = wss::Utils::XYToIndex((int)x, (int)y, NUM_OF_ZONES);
			for (size_t i = 0; i < advertsPerZone; ++i) {
				//glm::vec2 randCircle = glm::diskRand((float)ZONE_SIZE-1);
				//glm::vec2 advertPosition = center + randCircle;
				glm::vec2 advertPosition = glm::linearRand(center, center + glm::vec2(ZONE_SIZE, ZONE_SIZE));
				cout << "Zone x,y: " << x << "," << y << endl;
				cout << "advertPosition: " << advertPosition.x << ", " << advertPosition.y << endl;

				//Make commands
				std::queue<AdvertBehaviorTest> behaviors;
				behaviors.push(AdvertBehaviorTest::MOVE_TO);
				behaviors.push(AdvertBehaviorTest::WAIT);

				std::queue<glm::vec2> data;

				glm::vec2 randomZone(glm::linearRand(0.0f, (float)NUM_OF_ZONES), glm::linearRand(0.0f, (float)NUM_OF_ZONES));

				while((int)randomZone.x == x && (int)randomZone.y == y) {
					randomZone = glm::vec2(glm::linearRand(0.0f, (float)NUM_OF_ZONES), glm::linearRand(0.0f, (float)NUM_OF_ZONES));
				}

				randomZone *= ZONE_SIZE;

				data.push(randomZone);
				data.push(glm::vec2(glm::linearRand(1.0f, 6.0f), 0.0f));

				wss::AdvertCommand command("testcommand", behaviors, data);

				std::vector<wss::ATTRIBUTE_VALUE> deltas = { std::make_pair(wss::Attributes::Health, within(20)), std::make_pair(wss::Attributes::Happiness,
						within(15)), std::make_pair(wss::Attributes::Health, -5 - within(5)) };
				std::shared_ptr<Advertisement> advert(new Advertisement(deltas, command));

				//advertPos[i] = std::make_pair(advert, std::get<1>(advertPos[i]));


				advertPos.push_back(std::make_tuple(advert, advertPosition));
			}
			advertZones[zoneIndex] = advertPos;
		}
	}

	/*
	// Move randomly to another advert.
	for (size_t y = 0; y < NUM_OF_ZONES; ++y) {
		for (size_t x = 0; x < NUM_OF_ZONES; ++x) {
			size_t zoneIndex = wss::Utils::XYToIndex((int)x, (int)y, NUM_OF_ZONES);
			auto advertPos = advertZones[zoneIndex];
			for (size_t i = 0; i < advertsPerZone; ++i) {

			}
			advertZones[zoneIndex] = advertPos;
		}
	}
	*/

	// Path generator node. This node will input ID_PATH and generate an path then output it.
	PATH_FIND_NODE pathGenerator(g, tbb::flow::unlimited, [&pathEntities](std::tuple<size_t, glm::vec2, glm::vec2> pathRequest)->PathEntity* {
		size_t id = std::get<0>(pathRequest);
		pathEntities[id]->position = std::get<1>(pathRequest);
		pathEntities[id]->moveTo = std::get<2>(pathRequest);
		return pathEntities[id];
	});

	//PROCESS_ATTRIBUTE_NODE* processNext = nullptr;

	// Traverse done node. This node will take input and process an entity for the TRAVERSE_DONE_STATE--it will find nearest advertisement.
	PROCESS_ATTRIBUTE_NODE traverseDone(g, tbb::flow::unlimited, [&](std::tuple<size_t, glm::vec2> somethingDoneEvent)->void {
		size_t id = get<0>(somethingDoneEvent);
		auto attributeEntity = attributeEntities[id];
		vec2 position = get<1>(somethingDoneEvent);

		AdvertCommand command = attributeEntity->getCommand();
		AdvertBehaviorTest behavior = command.getBehaviorTree();
		glm::vec2 data = command.popData();

		auto processCommand = [&id, &position, &pathGenerator](AdvertBehaviorTest behavior, glm::vec2 data) {
			PROCESS_ATTRIBUTE_NODE* processNext = getProcessNext();
			switch(behavior) {
			case AdvertBehaviorTest::MOVE_TO:
			{
				//cout << "MOVE_TO COMMAND SELECTED! " << data.x << " , " << data.y << endl;
				//data = glm::vec2(glm::linearRand(0.0f, (float)MAP_W), glm::linearRand(0.0f, (float)MAP_H));
				pathGenerator.try_put(make_tuple(id, position, data));
				break;
			}
			case AdvertBehaviorTest::WAIT:
			{
				// Do nothing for WAIT. Future use WAIT generator to generate wait.

				if (processNext)
					processNext->try_put(make_pair(id, position));
				break;
			}
			case AdvertBehaviorTest::NONE:
				//if (processNext)
					//processNext->try_put(make_pair(id, position));
			break;
			}
		};

		if (behavior == AdvertBehaviorTest::NONE) { // No commands. Pick new advertisement.

			size_t zoneId = Utils::XYToIndex(position.x / ZONE_SIZE, position.y / ZONE_SIZE, NUM_OF_ZONES);
			auto adverts = advertZones[zoneId];

			vector<ADVERT_SCORE> scores;
			for (auto advert_pos : adverts) {
				auto advert = get<0>(advert_pos);
				auto position = get<1>(advert_pos);
				scores.push_back(make_pair(advert.get(), attributeEntity->score(*advert)));
			}
			int whichOne = attributeEntity->pickAdvertisement(scores);
			if (whichOne > -1) {
				auto pickedAdvert = get<0>(scores[whichOne]);
				glm::vec2 advertPosition;
				// Stupid... Find position for this advert
				for (auto advert_pos : adverts) {
					auto advert = get<0>(advert_pos);
					if (advert.get() == pickedAdvert)
						advertPosition = get<1>(advert_pos);
				}

				// Get behaviors
				AdvertCommand command = pickedAdvert->getCommand();
				//cout << "Scored advertisement!" << endl;
				// Process the current commands
				//AdvertBehaviorTest behavior = command.getBehaviorTree();
				//glm::vec2 data = command.popData();
				//if (behavior != AdvertBehaviorTest::NONE) {
					//processCommand(behavior, data);
				//}
				//else {
					//cout << "logical error... no behavior for advert!" << endl;
				//}
				// Walk to the selected advertisement
				AdvertBehaviorTest moveTo = AdvertBehaviorTest::MOVE_TO;
				processCommand(moveTo, advertPosition);


				attributeEntity->setCommands(command); //Current test implementation is copy based so need to reattach command after operation.
			}
			else {
				cout << "NO SCORE GIRL!" << endl;
			}
		}
		else { // There are still commands. Process next command.
			//AdvertCommand command = attributeEntity->getCommand();
			//AdvertBehaviorTest behavior = command.getBehaviorTree();
			//cout << "command behavior size: " << command.getBehaviorTreeSize() << endl;
			if (behavior != AdvertBehaviorTest::NONE) {
				processCommand(behavior, data);
			}
			else {
				cout << "logical error... no behavior in advertisement!" << endl;
			}
			//cout << "Processed commands!" << endl;
			attributeEntity->setCommands(command);
		}
		return;
	});

	processNext = new PROCESS_ATTRIBUTE_NODE(g, tbb::flow::unlimited, [&](std::tuple<size_t, glm::vec2> somethingDoneEvent)->void {
		traverseDone.try_put(somethingDoneEvent);
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
		//stateEnd = wss::Utils::XYToIndex(within(200), within(200), MAP_W );
		stateEnd = wss::Utils::XYToIndex(pathEntity->moveTo.x, pathEntity->moveTo.y, MAP_W);
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

	std::thread updateRegionThread([&]() {
		regionDataPublisher(regionDataSocket, traverseDone, solvedPathQueue, entities);
	});

	tbb::flow::make_edge(pathGenerator, solvePathNode);

	g.wait_for_all();

	updateRegionThread.join();

}



