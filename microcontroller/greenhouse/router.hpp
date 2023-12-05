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

  /// @brief abstract function needed in all routes to allow execution of code
  /// @param request request object to process
  /// @param response response string to write response to
  /// @return NetworkReturnErrors contains the result code for the execution
  virtual NetworkReturnErrors execute(ParsedRequest &request, String *response) {};
};

class Router {
  private:
  Route *routes[MAX_ROUTES];
  int num_routes;

  public:
  bool add_route(Route *route);
  NetworkReturnErrors execute_route(ParsedRequest &request, String *response);
  ~Router();
};

#endif