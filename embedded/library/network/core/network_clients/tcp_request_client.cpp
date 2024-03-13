#include "wifi.hpp"


///////////////////////////////////////////////////
///////// TCPRequestClient ////////////////////////
///////////////////////////////////////////////////

TCPRequestClient::TCPRequestClient(WiFiClient &client, Layer4Encryption encryption, ConnectionInformation &server_information) : NetworkClient(encryption), client(client), server_information(server_information) {}

Response TCPRequestClient::receive_response(WiFiClient &client) {
  Response *response_pointer;
  Response response;

  response_pointer = dynamic_cast<Response*>(this->receive_and_parse(this->client, 10000));

  this->client.stop();

  response = Response(response_pointer);
  return response;
}

Response TCPRequestClient::request(Request *message) {
  Response response;

  switch(this->get_encryption()) {
    case ENCRYPTION_SSL:
      this->client.connectSSL(IPAddressExtended((char*)this->server_information.ip.c_str()), this->server_information.port);
      break;
    case ENCRYPTION_NONE:
      this->client.connect(IPAddressExtended((char*)this->server_information.ip.c_str()), this->server_information.port);
      break;
  }

  NetworkException send_exception = this->client_send_message(this->client, message);
  response = this->receive_response(this->client);

  return response;
}