#ifndef FULL_CONNECT_ROUTES_HPP
#define FULL_CONNECT_ROUTES_HPP


#include "http.hpp"
#include "router.hpp"
#include "storage.hpp"


Router full_connection_router(StorageManager *global_storage);

Response storage_route_error_handler(StorageException exception, int okay_code);

class NetworkReset : public Route {
  public:
  NetworkReset(const String route, const Method method);
  Response execute(ParsedRequest &request) override;
};


class DeviceReset : public Route {
  public:
  DeviceReset(const String route, const Method method);
  Response execute(ParsedRequest &request) override;
};


class SetTime : public Route {
  private:
  StorageManager *global_storage

  public:
  SetTime(const String route, const Method method, StorageManager *global_storage);
  Response execute(ParsedRequest &request) override;
};


class ConfigureDeviceIds : public Route {
  private:
  StorageManager *global_storage
  
  public:
  ConfigureDeviceIds(const String route, const Method method, StorageManager *global_storage);
  Response execute(ParsedRequest &request) override;
};


class ConfigureDevicePreset : public Route {
  private:
  StorageManager *global_storage

  public:
  ConfigureDevicePreset(const String route, const Method method, StorageManager *global_storage);
  Response execute(ParsedRequest &request) override;
};

class PauseDevice : public Route {
  private:
  StorageManager *global_storage

  public:
  PauseDevice(const String route, const Method method, StorageManager *global_storage);
  Response execute(ParsedRequest &request) override;
};

#endif