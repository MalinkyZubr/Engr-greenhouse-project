#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <Arduino.h>
#include <WiFi101.h>
#include "async.hpp"
#include "http.hpp"
#include "machine_state.hpp"

#define MAX_ROUTES 10

class Route {
  public:
  const String route;

  Route(String route);
  bool requested(String &requested_route);
  virtual NetworkReturnErrors execute(WiFiClient &client, ParsedRequest &request, String *response);
};

class Router {
  private:
  Route *routes[MAX_ROUTES];
  int num_routes;

  public:
  bool add_route(String route);
  NetworkReturnErrors execute_route(WiFiClient &client, ParsedRequest &request, String *response);
  ~Router();
};

#endif