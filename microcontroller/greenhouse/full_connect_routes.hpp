#ifndef FULL_CONNECT_ROUTES_HPP
#define FULL_CONNECT_ROUTES_HPP

#include "wifi.hpp"
#include "http.hpp"
#include "router.hpp"
#include "storage.hpp"
#include "async.hpp"


class NetworkReset : public Route {
  private:
  ConfigManager *storage;
  MachineState *machine_state;
  ConnectionManager *connection_manager;
  
  public:
  NetworkReset(String route, ConfigManager *storage, MachineState *machine_state, ConnectionManager *connection_manager);
  NetworkReturnErrors execute(ParsedRequest &request, String *response) override;
};


class DeviceReset : public Route {
  private: 
  ConfigManager *storage;

  public:
  DeviceReset(String route, ConfigManager *storage);
  NetworkReturnErrors execute(ParsedRequest &request, String *response) override;
};


class SetTime : public Route {
  private:
  CommonData *common_data;

  public:
  SetTime(String route, CommonData *common_data);
  NetworkReturnErrors execute(ParsedRequest &request, String *response) override;
};


class ConfigureDeviceIds : public Route {
  private:
  ConfigManager *storage;
  
  public:
  ConfigureDeviceIds(String route, ConfigManager *storage);
  NetworkReturnErrors execute(ParsedRequest &request, String *response) override;
};


class ConfigureDevicePreset : public Route {
  private:
  ConfigManager *storage;

  public:
  ConfigureDevicePreset(String route, ConfigManager *storage);
  NetworkReturnErrors execute(ParsedRequest &request, String *response) override;
};

class PauseDevice : public Route {
  private:
  MachineState *machine_state;

  public:
  PauseDevice(String route, MachineState *machine_state);
  NetworkReturnErrors execute(ParsedRequest &request, String *response) override;
};

#endif