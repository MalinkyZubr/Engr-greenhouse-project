#include "wifi.hpp"


///////////////////////////////////////////////////
///////// UDPClient ///////////////////////////////
///////////////////////////////////////////////////

UDPClient::UDPClient(WiFiUDP &udp_server, ConnectionInformation &local_information, ConnectionInformation &multicast_information) : udp_server(udp_server), multicast_information(multicast_information) {
  this->udp_server.begin(local_information.port);
}

NetworkException UDPClient::send_udp(UDPRequest &request) {
  String request_string = request.to_string();

  this->udp_server.beginPacket(IPAddressExtended((char*)this->multicast_information.ip.c_str()), this->multicast_information.port);
  this->udp_server.write(request_string.c_str());
  this->udp_server.endPacket();

  return NETWORK_OKAY;
}

UDPResponse UDPClient::receive_udp(int timeout) {
  UDPResponse response;

  char receive_buffer[32];
  String response_string;

  long long start = millis();

  while(!this->udp_server.parsePacket() && (millis() - start < timeout)) {}
  if(timeout < millis() - start) {
    response.set_exception(NETWORK_TIMEOUT);
  }
  else {
    response_string = receive_buffer;
    this->udp_server.read(receive_buffer, 32);
    response = UDPResponse(response_string);
  }
  return response;
}