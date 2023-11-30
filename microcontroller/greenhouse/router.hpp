#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "async.hpp"
#include "http.hpp"

#define MAX_ROUTES 10

class Route {
  private:
  const String route;
  

  public:
  Route(String route);
  bool requested(String &requested_route);
  virtual ReturnErrors execute(ParsedRequest &request, String *response);
};

class Router {
  private:
  Route *routes[MAX_ROUTES];
  int num_routes;

  public:
  bool add_route(String route);
  ReturnErrors execute_route(String &route, ParsedRequest &request, String *response);
  ~Router();
};

#endif