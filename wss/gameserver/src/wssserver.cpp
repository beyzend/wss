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

#define within(num) (int) ((float) num * random () / (RAND_MAX + 1.0))

void regionDataPublisher(zmqpp::socket &publisher, std::vector<int> &entities) {
	using namespace std;

	while (1) {

		stringstream entityStream;

		for (auto entity : entities) {
			entityStream << entity << " ";
		}
		std::string outString = entityStream.str();
		//std::cout << outString << std::endl;
		publisher.send(outString);


		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

}

int main(int argc, char** argv) {
	using namespace std;

	cout << "Starting ZeroMQ server..." << endl;

	std::vector<int> entities;

	for (size_t i = 0; i < 100; ++i) {
		entities.push_back(within(100));
	}

	zmqpp::context context;

	zmqpp::socket regionDataSocket(context, zmqpp::socket_type::publish);
	regionDataSocket.bind("tcp://127.0.0.1:4200");

	std::thread t1([&regionDataSocket, &entities]() {
		regionDataPublisher(regionDataSocket, entities);
	});
	t1.join();

	return 0;
}
