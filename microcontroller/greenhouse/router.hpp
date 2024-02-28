#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <Arduino.h>
#include <ArduinoJson.h>
#include "http.hpp"
#include "exceptions.hpp"

#define MAX_ROUTES 10


class Route {
  public:
  const char* route;
  const Method method;

  Route(const char* route, const Method method);
  bool requested(const Request &request);

  virtual Response execute(const Request &request) {};
};


class Router {
  private:
  Route *routes[MAX_ROUTES];
  int num_routes;

  public:
  Router& add_route(Route *route);

  Response execute_route(Request &request);
  ~Router();
};


#endif