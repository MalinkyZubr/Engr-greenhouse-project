#ifndef WIFI_INFO_HPP
#define WIFI_INFO_HPP

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

enum NetworkTypes {
  HOME = 'h',
  ENTERPRISE = 'e',
  OPEN = 'o'
};

typedef struct {
  NetworkTypes type;
  String ssid;
  String username;
  String password;
  int channel;
  NetworkReturnErrors error;
} WifiInfo;

#endif