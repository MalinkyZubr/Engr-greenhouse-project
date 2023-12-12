#ifndef UDP_MESSAGES_HPP
#define UDP_MESSAGES_HPP

#include <ArduinoJson.h>
#include "storage.hpp"
#include "exceptions.hpp"


class UDPMessage {
  private:
  NetworkExceptions exception = NETWORK_OKAY;

  public:
  void set_exception(NetworkExceptions exception);
  NetworkExceptions get_exception();
}

class UDPRequest : public UDPMessage {
  private:
  String ip_address;
  String device_name = "";
  String device_id = "";
  bool expidited;

  public:
  UDPRequest(String ip_address, Identifiers &identifiers);
  String to_string();
};

class UDPResponse : public UDPMessage {
  private:
  String server_ip;
  int server_port;

  public:
  UDPResponse(String &response_string);
  String get_server_ip();
  int get_server_port();
};

#endif