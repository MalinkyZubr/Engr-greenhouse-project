#include "ip_extended.hpp"


int IPAddressExtended::char_array_to_int(char* int_as_char) {
  int octet;
  int factor = pow(10, strlen(int_as_char) - 1);

  for(int index = 0; index < strlen(int_as_char); index++) {
    octet += static_cast<int>(int_as_char[index]) * factor;
    factor /= 10;
  }
}

int IPAddressExtended::get_octet(char* ip_address, int octet_num) {
  int current_octet = 1;
  char* octet_char;
  int octet;

  for(int x = 0; x < strlen(ip_address); x++) {
    if(ip_address[x] == '.') {
      if(current_octet == octet_num) {
        break;
       }
       current_octet++;
    }
    else {
      octet_char += ip_address[x];
    }
  }

  octet = this->char_array_to_int(octet_char);
  return octet;
}

IPAddressExtended::IPAddressExtended(char* ip_address) : IPAddress() {
  
}