#ifndef WIFI_INFO_HPP
#define WIFI_INFO_HPP


/// @brief enum of errors that can be returned by functions in the syste,
enum NetworkReturnErrors {
  OKAY,
  CONNECTION_FAILURE,
  WIFI_FAILURE,
  TIMEOUT,
  NOT_FOUND,
  AUTHENTICATION_FAILURE,
  ERROR,
  STORAGE_WRITE_FAILURE
};


/// @brief enum of possible wifi connection authentication types supported by the network management software
enum NetworkTypes {
  HOME = 'h',
  ENTERPRISE = 'e',
  OPEN = 'o'
};


/// @brief struct containing critical wifi information for maintaining device connectivity
typedef struct {
  NetworkTypes type;
  String ssid;
  String username;
  String password;
  int channel;
  NetworkReturnErrors error;
} WifiInfo;

#endif