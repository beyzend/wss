#include <iostream>
#include <string>
#include <iostream>

#include <zmqpp/zmqpp.hpp>

#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <chrono>
#include <thread>


#define within(num) (int) ((float) num * random () / (RAND_MAX + 1.0))

int main(int argc, char** argv) {
	using namespace std;

	cout << "Starting ZeroMQ server..." << endl;

	zmqpp::context context;

	zmqpp::socket server(context, zmqpp::socket_type::publish);
	server.bind("tcp://127.0.0.1:4200");
	server.bind("ipc://weather.ipc");

	//zmqpp::socket client(context, zmqpp::socket_type::pull);
	//client.connect("tcp://127.0.0.1:4200");

	while( 1 ) {
		int zipcode, temperature, relhumidity;

		zipcode = within( 100000 );
		temperature = within( 215 ) - 80;
		relhumidity = within( 50 ) + 10;

		zmqpp::message message;
		message << "zipcode: " << zipcode << " temp: " << temperature << " relhumidity: " << relhumidity;

		cout << "Sending message: zip,temp,hum: " << zipcode << "," << temperature << "," << relhumidity << endl;


		server.send( message );


		std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
	}

	return 0;
}
