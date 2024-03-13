#include "udp_messages.hpp"


///////////////////////////////////////////
//////////// UDPRequest ///////////////////
///////////////////////////////////////////

void UDPMessage::set_exception(NetworkException exception) {
  this->exception = exception;
}

NetworkException UDPMessage::get_exception() {
  return this->exception;
}

///////////////////////////////////////////
//////////// UDPRequest ///////////////////
///////////////////////////////////////////

UDPRequest::UDPRequest(String ip_address, const Identifiers &identifiers) : ip_address(ip_address), device_name(identifiers.get_device_name()), device_id(identifiers.get_device_id()) {
  if(!this->device_name.equals("") && !this->device_id.equals("")) {
    this->expidited = true;
  }
  else {
    this->expidited = false;
  }
}

String UDPRequest::to_string() {
  DynamicJsonDocument udp_data(CONFIG_JSON_SIZE);
  String serialized;

  udp_data["ip_address"] = this->ip_address;
  udp_data["device_name"] = this->device_name;
  udp_data["device_id"] = this->device_id;
  udp_data["expidited"] = this->expidited;

  serializeJson(udp_data, serialized);
  return serialized;
}

///////////////////////////////////////////
//////////// UDPResponse //////////////////
///////////////////////////////////////////

UDPResponse::UDPResponse(String &response_string) {
  DynamicJsonDocument response_data(CONFIG_JSON_SIZE);

  deserializeJson(response_data, response_string);
  
  this->server_ip = (char*)&response_data["server_ip"];
  this->server_port = response_data["server_port"];
}

String UDPResponse::get_server_ip() {
  return server_ip;
}

int UDPResponse::get_server_port() {
  return this->server_port;
}