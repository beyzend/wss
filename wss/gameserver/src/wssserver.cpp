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
#include "wss/Map.h"
#include "wss/AttributesTransform.h"


#define within(num) (int) ((float) num * random () / (RAND_MAX + 1.0))

#define GLM_FORCE_SSSE3

const size_t MAP_W = 100;
const size_t MAP_H = 100;

//glm::vec2 start(12,1), end(72,54);

size_t ZONE_SIZE = 5;
size_t NUM_OF_ZONES = (MAP_W) / ZONE_SIZE;

size_t ENT_TO_CREATE = 1000;

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
using WAIT_NODE = tbb::flow::function_node<int>;
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

void processWaitCallbacks(PROCESS_ATTRIBUTE_NODE *nextNode, tbb::concurrent_queue<std::tuple<std::tuple<size_t, glm::vec2>,
		std::tuple<double, std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<double>>>>> &waitQueue) {
	using namespace std;
	using namespace wss;

	chrono::steady_clock clock;

	//while (1) {
		bool more = false;
		if (nextNode == nullptr)
			return;
		do {

			tuple<tuple<size_t, glm::vec2>, tuple<double, chrono::time_point<chrono::steady_clock, chrono::duration<double>>>> waitThing;
			more = waitQueue.try_pop(waitThing);

			if (more) {

				auto idPosition = get<0>(waitThing);
				auto timeStuff = get<1>(waitThing);
				auto timeToWait = get<0>(timeStuff);
				auto timeStart = get<1>(timeStuff);

				if (timeStart == chrono::time_point<chrono::steady_clock, chrono::duration<double>>::min())
					timeStart = clock.now();

				chrono::duration<double> elapsed = clock.now() - timeStart;

				//cout << "elapsed duration for id: " << get<0>(idPosition) << " elapsed: "<< elapsed.count() << " wait time: " << timeToWait << endl;

				if (elapsed.count() > timeToWait) {
					bool mustPut = nextNode->try_put(idPosition);
					while (!mustPut) {
						mustPut = nextNode->try_put(idPosition);
					}
				}
				else {
					timeStuff = make_pair(timeToWait, timeStart);
					waitThing = make_pair(idPosition, timeStuff);
					waitQueue.push(waitThing);
				}
			}
		}while(more);
		//std::this_thread::sleep()
	//}

}

void regionDataPublisher(zmqpp::socket &publisher, PROCESS_ATTRIBUTE_NODE &pathFindNode, tbb::concurrent_queue<PathEntity*> &solvedPathQueue, tbb::concurrent_vector<Entity*> entities) {
	using namespace std;

	std::chrono::steady_clock clock;

	// Initialize path.
	for (auto entity : entities) {
		pathFindNode.try_put(make_pair(entity->id, entity->position));//(std::tuple<size_t, glm::vec2>(entity->id, entity->position));
	}

	size_t lastI = 0;

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
//			rapidjson::Document document;
//			document.SetObject();
//			serializeEntities(document, 0, entities.size(), entities);
//
//			rapidjson::StringBuffer sb;
//			rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
//
//			document.Accept(writer);

			//publisher.send(sb.GetString(), true);

			zmqpp::message_t message;

			bool sendOut = false;

			for (auto entity : entities) {
				if (entity->pathNodes) {
					sendOut = true;
					message << entity->id << entity->position.x << entity->position.y;
				}
			}

			if (sendOut) {
				publisher.send(message, false);
			}

		}
		std::chrono::duration<double> elapsed = clock.now() - start;
		if (elapsed.count() < 1.0/20.0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1000/5 - (size_t)(elapsed.count() * 1000.0)));
		}

	}
}

WAIT_NODE* waitNode = nullptr;

PROCESS_ATTRIBUTE_NODE* processNext = nullptr;

PROCESS_ATTRIBUTE_NODE* getProcessNext() {
	return processNext;
}

WAIT_NODE* getWaitNode() {
	return waitNode;
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

	// Parse map
	Map jsonMap("test4.json");

	// SETUP ENTITIES

	// Use a memory allocator here.
	tbb::concurrent_vector<Entity*> entities;
	std::vector<PathEntity*> pathEntities;
	std::vector<std::shared_ptr<AttributeEntity>> attributeEntities;
	tbb::concurrent_queue< std::tuple<tuple<size_t, glm::vec2>, tuple<double, chrono::time_point<chrono::steady_clock, chrono::duration<double> > > > > waitQueue;

	glm::vec2 start(18,4), end(67,8);
	for (size_t i = 0; i < ENT_TO_CREATE; ++i) {
		glm::vec2 position = randomPosition(start, end);
		pathEntities.push_back(new PathEntity(i, position));
		entities.push_back(new Entity(i, position));
		// create attribute entities with health and happiness
		vector<ATTRIBUTE_VALUE> attributes = {std::make_pair(Attributes::Health, 100), std::make_pair(Attributes::Happiness, 50)};
		attributeEntities.push_back(shared_ptr<AttributeEntity>(new AttributeEntity(i, attributes )));
	}

	std::vector<size_t> map;

	for (size_t y = 0; y < MAP_H; ++y) {
		for (size_t x = 0; x < MAP_W; ++x) {
			map.push_back(1);
		}
	}

	using ADVERT_POS = std::tuple<std::shared_ptr<wss::Advertisement>, glm::vec2>;
	//using vSECTOR_ADVERTS = std::vector<ADVERT_POS>;
	//std::unordered_map<size_t, vSECTOR_ADVERTS> advertZones;

	using vADVERTS = vector<ADVERT_POS>;
	vADVERTS adverts;
	// Create advertisements
	// We will load json Advertisement layer and create FOOD and WORK type advertisements.
	// For now WORK advertisement will scale happiness reward by distance from Y = 0
	// For now just testing code. Will need to model all this!
	for (auto object : jsonMap.getAdvertLayer().objects) {
		if ("food" == object.type) {
			vector<ATTRIBUTE_VALUE> awards = {
					std::make_pair(Attributes::Health, 20.0f)
			};

			AdvertCommand healthCommands("health", queue<AdvertBehaviorTest>({AdvertBehaviorTest::WAIT}),
					queue<glm::vec2>({glm::vec2(1.0f, 5.0f)})); // Should randomly wait when executing this command and not use wait time here.

			shared_ptr<Advertisement> foodAdvert = shared_ptr<Advertisement>(new Advertisement(awards, healthCommands));
			glm::vec2 position(object.x / 16.0f, object.y / 16.0f);
			adverts.push_back(tie(foodAdvert, position));
		}
		else if("work" == object.type) {
			Attributes attr = Attributes::Happiness;
			glm::vec2 pos(object.x / 16.0f, object.y / 16.0f);
			float scale = pos.y / jsonMap.getCollisionLayer().height;
			vector<ATTRIBUTE_VALUE> awards = {
					make_pair(attr, 10 + scale * 10)
			};
			AdvertCommand workCommands("work",
					queue<AdvertBehaviorTest>({AdvertBehaviorTest::WAIT}),
					queue<glm::vec2>({glm::vec2(1.0f, 5.0f)}));

			shared_ptr<Advertisement> workAdvert = shared_ptr<Advertisement>(new Advertisement(awards, workCommands));
			adverts.push_back(tie(workAdvert, pos));
		}
	}

	// Path generator node. This node will input ID_PATH and generate an path then output it.
	PATH_FIND_NODE pathGenerator(g, tbb::flow::unlimited, [&pathEntities](std::tuple<size_t, glm::vec2, glm::vec2> pathRequest)->PathEntity* {
		size_t id = std::get<0>(pathRequest);
		pathEntities[id]->position = std::get<1>(pathRequest);
		pathEntities[id]->moveTo = std::get<2>(pathRequest);
		return pathEntities[id];
	});

	//PROCESS_ATTRIBUTE_NODE* processNext = nullptr;
	chrono::steady_clock clock;
	// Traverse done node. This node will take input and process an entity for the TRAVERSE_DONE_STATE--it will find nearest advertisement.
	PROCESS_ATTRIBUTE_NODE traverseDone(g, tbb::flow::unlimited, [&](std::tuple<size_t, glm::vec2> somethingDoneEvent)->void {
		size_t id = get<0>(somethingDoneEvent);
		auto attributeEntity = attributeEntities[id];
		vec2 position = get<1>(somethingDoneEvent);

		AdvertCommand command = attributeEntity->getCommand();
		AdvertBehaviorTest behavior = command.getBehaviorTree();
		glm::vec2 data = command.popData();

		auto processCommand = [&id, &position, &pathGenerator, &waitQueue, &jsonMap](AdvertBehaviorTest behavior, glm::vec2 data) {
			PROCESS_ATTRIBUTE_NODE* processNext = getProcessNext();
			switch(behavior) {
			case AdvertBehaviorTest::MOVE_TO:
			{
				const Layer& layer = jsonMap.getAdvertLayer();
				size_t randomIdx = within(layer.objects.size());
				const LayerObject& object = layer.objects[randomIdx];
				data = vec2((int)(object.x / 16.0f), (int)(object.y / 16.0));

				// random offset to
				glm::vec2 randomVec = glm::circularRand(ZONE_SIZE);

				pathGenerator.try_put(make_tuple(id, position, data));// + randomVec * glm::linearRand(0.1f, 1.0f)));
				break;
			}
			case AdvertBehaviorTest::WAIT:
			{
				double randomWaitTime = data.x + glm::linearRand(0.0f, 6.0f);
				// Do nothing for WAIT. Future use WAIT generator to generate wait.
				waitQueue.push(make_pair(make_pair(id, position), make_pair(randomWaitTime, chrono::time_point<chrono::steady_clock, chrono::duration<double> >::min())));
				break;
			}
			case AdvertBehaviorTest::NONE:
				//if (processNext)
					//processNext->try_put(make_pair(id, position));
			break;
			}
		};

		if (behavior == AdvertBehaviorTest::NONE) { // No commands. Pick new advertisement.

			AdvertBehaviorTest moveTo = AdvertBehaviorTest::MOVE_TO;
			processCommand(moveTo, data);

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
	PATH_SOLVE_NODE solvePathNode(g, 25, [&map, &solvedPathQueue, &jsonMap](PathEntity* pathEntity)->PathEntity*{
		wss::Path path(jsonMap.getCollisionLayer());
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

	//std::thread waitProcessThread([&]() {
		//processWaitCallbacks(processNext, waitQueue);
	//});

	while(1) {
		processWaitCallbacks(processNext, waitQueue);
	}


	g.wait_for_all();
	updateRegionThread.join();

}



