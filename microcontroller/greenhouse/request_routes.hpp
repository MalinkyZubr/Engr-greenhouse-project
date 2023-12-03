#ifndef REQUEST_ROUTES_HPP
#define REQUEST_ROUTES_HPP

#include "wifi.hpp"
#include "http.hpp"
#include "router.hpp"
#include "storage.hpp"
#include "async.hpp"


class ResetDevice : public Route {
  private:
  ConfigManager *storage;
  MachineState *machine_state;
  ConnectionManager *connection_manager;
  
  public:
  ResetDevice(String route, ConfigManager *storage, MachineState *machine_state, ConnectionManager *connection_manager);
  NetworkReturnErrors execute(WiFiClient &client, ParsedRequest &request, String *response) override;
};


class SetTime : public Route {
  private:
  CommonData *common_data;

  public:
  SetTime(String route, CommonData *common_data);
  NetworkReturnErrors execute(WiFiClient &client, ParsedRequest &request, String *response) override;
};


class ConfigureDevice : public Route {
  private:
  ConfigManager *storage;
  
  public:
  ConfigureDevice(String route, ConfigManager *storage);
  NetworkReturnErrors execute(WiFiClient &client, ParsedRequest &request, String *response) override;
};

#endif