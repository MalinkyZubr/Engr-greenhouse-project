#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <Arduino.h>
#include <ArduinoJson.h>
#include "http.hpp"
#include "exceptions.hpp"

#define MAX_ROUTES 10


class Route {
    public:
    const String route;
    const Method method;

    Route(const String route, const Method method);
    bool requested(Request &request);

    virtual Response execute(Request &request) {};
};


class Router {
  private:
  Route *routes[MAX_ROUTES];
  int num_routes;

  public:
  Router& add_route(Route *route);

  NetworkException execute_route(Request &request);
  ~Router();
};


#endif