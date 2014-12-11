#include <iostream>
#include <string>
#include <iostream>

#include <zmqpp/zmqpp.hpp>


int main ( int argc, char** argv )
{
  using namespace std;
  cout << "FUCK YOU! TO DO: Use ZerMQPP binding" << endl;
  
  zmqpp::context context;
  
  zmqpp::socket server ( context, zmqpp::socket_type::push );
  server.bind ( "tcp://127.0.0.1:4200" );
  
  zmqpp::socket client ( context, zmqpp::socket_type::pull );
  client.connect( "tcp://127.0.0.1:4200" );
  
  zmqpp::message request;
  request << "Hello";
  server.send( request );

  zmqpp::message response;
  client.receive( response );

  assert( "Hello" == response.get( 0 ) );
  cout << "Grasslands test OK" << endl;
  
  return 0;
}
