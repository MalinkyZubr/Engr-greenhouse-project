#include "router.hpp"


Route::Route(String route) : route(route) {}

bool Route::requested(String &requested_route) {
  return(this->route.equals(requested_route));
}

bool Router::add_route(String route) {
  if(this->num_routes < MAX_ROUTES) {
    this->routes[this->num_routes] = new Route(route);
    this->num_routes++;
    return true;
  }
  return false;
}

ReturnErrors Router::execute_route(String &route, ParsedRequest &request, String *response) {
  ReturnErrors return_code;
  for(int route_num = 0; route_num < MAX_ROUTES; route_num++) {
    if(this->routes[route_num]->requested(route)) {
        return this->routes[route_num]->execute(request, response);
    }
  }
  return NOT_FOUND;
}

Router::~Router() {
  for(int x = 0; x < MAX_ROUTES; x++) {
    delete this->routes[x];
  }
}