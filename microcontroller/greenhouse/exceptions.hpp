#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

/// @brief enum of errors that can be returned by functions in the syste,
enum NetworkExceptions {
  NETWORK_OKAY,
  NETWORK_SERVER_CONNECTION_FAILURE,
  NETWORK_WIFI_FAILURE,
  NETWORK_TIMEOUT,
  NETWORK_RESOURCE_NOT_FOUND,
  NETWORK_AUTHENTICATION_FAILURE,
};

enum StorageExceptions {
  STORAGE_OKAY,
  STORAGE_WRITE_FAILURE,
};

#endif