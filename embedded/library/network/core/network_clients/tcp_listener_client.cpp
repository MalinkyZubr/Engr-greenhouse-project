#include "wifi.hpp"


///////////////////////////////////////////////////
///////// TCPListenerClient ///////////////////////
///////////////////////////////////////////////////

TCPListenerClient::TCPListenerClient(WiFiServer &listener, Layer4Encryption encryption, Router router) : listener(listener), router(router), NetworkClient(encryption) {
  switch(encryption) {
    case ENCRYPTION_NONE:
      this->listener.begin();
      break;
    case ENCRYPTION_SSL:
      this->listener.beginSSL();
      break;
  }
}

NetworkException TCPListenerClient::respond(Response &response, WiFiClient &client) {
  String response_str = response.serialize();
  NetworkException exception = NETWORK_OKAY;
  if(!client.println(response_str)) {
    exception = NETWORK_SERVER_CONNECTION_FAILURE;
  }

  client.stop();
  return exception;
}

NetworkException TCPListenerClient::listen() {
  Request *received_ptr;
  Request received;
  WiFiClient client = this->listener.available();

  received_ptr = dynamic_cast<Request*>(this->receive_and_parse(client, 5000));
  if(received_ptr->get_exception() != NETWORK_OKAY) {
    delete received_ptr;
    return received_ptr->get_exception();
  }

  received = Request(received_ptr); // this frees the pointer memory

  Response response = this->router.execute_route(received);
  
  return this->respond(response, client);
}