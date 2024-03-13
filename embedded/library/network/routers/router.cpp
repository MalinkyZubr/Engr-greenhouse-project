#include "router.hpp"


/// @brief base class for an individual route in the device side router. should include some function to execute
/// @param route string for the route that will be listened for by the device during operations
Route::Route(const char* route, const Method method) : route(route), method(method) {}

/// @brief check to see if some arbitrary route is equivalent to the route of this object
/// @param requested_route route value to check for equivalence
/// @return bool to say if the route matches
bool Route::requested(const Request &request) {
  return (strcmp((char*)request.get_route().c_str(), this->route) && request.get_method() == this->method);
}

/// @brief add a route to the device router. Once added it will be processed if ever called by a device bound request. If there is no more space left in teh routes array, it will return false @see MAX_ROUTES
/// @param route pointer to heap allocated route object
/// @return bool to say if the route could be added
Router& Router::add_route(Route *route) {
  if(this->num_routes < MAX_ROUTES) {
    this->routes[this->num_routes] = route;
    this->num_routes++;
  }
  return *this;
}

/// @brief when a request is received from the server, it will be forwarded to this function to be processed. If the route exists, it will execute that route
/// @param request the request object to be processed
/// @param response string reference the repsonse should be put into
/// @return NetworkReturnErrors error or okay code upon completion of route execution. if the route isnt found, returns NOT_FOUND enum value
Response Router::execute_route(Request &request) {
  Response response;
  bool found_route = false;

  for(int route_num = 0; route_num < MAX_ROUTES; route_num++) {
    if(this->routes[route_num]->requested(request)) {
      response = this->routes[route_num]->execute(request);
      found_route = true;
      break;
    }
  }

  if(!found_route) {
    response = Response(404);
  }

  return response;
}


/// @brief custom destructor for heap allocated routes in the route array
Router::~Router() {
  for(int x = 0; x < MAX_ROUTES; x++) {
    delete this->routes[x];
  }
  this->num_routes = 0;
}