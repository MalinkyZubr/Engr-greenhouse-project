#include "router.hpp"


/// @brief base class for an individual route in the device side router. should include some function to execute
/// @param route string for the route that will be listened for by the device during operations
template <typename R>
Router<R>::Route::Route(const String route) : route(route) {}

/// @brief check to see if some arbitrary route is equivalent to the route of this object
/// @param requested_route route value to check for equivalence
/// @return bool to say if the route matches
template <typename R>
bool Router<R>::Route::requested(String &requested_route) {
  return(this->route.equals(requested_route));
}

/// @brief add a route to the device router. Once added it will be processed if ever called by a device bound request. If there is no more space left in teh routes array, it will return false @see MAX_ROUTES
/// @param route pointer to heap allocated route object
/// @return bool to say if the route could be added
template <typename R>
static bool Router<R>::add_route(Route *route) {
  if(this->num_routes < MAX_ROUTES) {
    this->routes[this->num_routes] = route;
    this->num_routes++;
    return true;
  }
  return false;
}

/// @brief when a request is received from the server, it will be forwarded to this function to be processed. If the route exists, it will execute that route
/// @param request the request object to be processed
/// @param response string reference the repsonse should be put into
/// @return NetworkReturnErrors error or okay code upon completion of route execution. if the route isnt found, returns NOT_FOUND enum value
template <typename R>
RouteReturn<R> Router<R>::execute_route(Request &request) {
  RouteReturn<R> return_value;
  return_value.exception = NOT_FOUND;
  for(int route_num = 0; route_num < MAX_ROUTES; route_num++) {
    if(this->routes[route_num]->requested(request.route)) {
      return_value = this->routes[route_num]->execute(request);
      break;
    }
  }
  return return_value;
}


/// @brief custom destructor for heap allocated routes in the route array
template <typename R>
Router<R>::~Router() {
  for(int x = 0; x < MAX_ROUTES; x++) {
    delete this->routes[x];
  }
  this->num_routes = 0;
}