#include <iostream>

#include <zmq.hpp>

int main ( int argc, char** argv )
{
  using namespace std;
  cout << "FUCK YOU! TO DO: Use ZerMQPP binding" << endl;

  //Initialize zeromq
  zmq::context_t context ( 1 );
  zmq::socket_t publisher ( context, ZMQ_PUB );
  publisher.bind ( "tcp://127.0.0.1:5556" );
  publisher.bind( "ipc://weather.ipc" );
  while ( 1 ) 
    {
      zmq::message_t message ( 20 );
      snprintf ( ( char * ) message.data(), 20,
		 "fuck you!" );
      publisher.send( message );
    }
  
  return 0;
}
