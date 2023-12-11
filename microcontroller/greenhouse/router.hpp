#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <Arduino.h>
#include <WiFi101.h>
#include "async.hpp"
#include "http.hpp"
#include "exceptions.hpp"

#define MAX_ROUTES 10


template <typename R>
typedef struct {
  NetworkExceptions exception;
  R return_value;
}RouteReturn;


template <typename R = void*>
class Router {
  class Route;

  public:
  Route *routes[MAX_ROUTES];
  int num_routes;

  class Route {
    public:
    const String route;

    Route(const String route);
    bool requested(String &requested_route);

    virtual RouteReturn<R> execute(Request &request) {};
  };

  bool add_route(Route *route);

  RouteReturn<R> execute_route(Request &request);
  ~Router();
};


#endif