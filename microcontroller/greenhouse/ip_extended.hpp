#ifndef IP_EXTENDED_HPP
#define IP_EXTENDED_HPP

#include <Arduino.h>
#include <WiFi101.h>


class IPAddressExtended : public IPAddress {
  private:
  int char_array_to_int(char* int_as_char);
  int get_octet(char* ip_address, int octet);

  public:
  IPAddressExtended(char* ip_address);
};


#endif