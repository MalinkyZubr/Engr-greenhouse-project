#ifndef REQUEST_ROUTES_HPP
#define REQUEST_ROUTES_HPP

#include "network/wifi.hpp"
#include "network/http.hpp"
#include "network/router.hpp"
#include "storage/storage.hpp"
#include "asynchronous/async.hpp"


class ResetDevice : public Route {
  private:
  ConfigManager *storage;
  MachineState *machine_state;
  ConnectionManager *connection_manager;
  
  public:
  ResetDevice(String route, ConfigManager *storage, MachineState *machine_state, ConnectionManager *connection_manager);
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