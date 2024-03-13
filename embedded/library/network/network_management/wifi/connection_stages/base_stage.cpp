#include "wifi.hpp"


///////////////////////////////////////////////////
///////// ConnectionStage /////////////////////////
///////////////////////////////////////////////////

// template <typename L, typename H, typename R>
// void ConnectionStage<L, H, R>::configure_message_handler() {
//  // need another attribute for this
// }

template <typename L, typename H, typename R>
L& ConnectionStage<L, H, R>::get_listener() {
  return this->listener;
}

template <typename L, typename H, typename R>
H* ConnectionStage<L, H, R>::get_handler() {
  return this->message_handler;
}

template <typename L, typename H, typename R>
void ConnectionStage<L, H, R>::set_handler(H* message_handler) {
  this->message_handler = message_handler;
}

template <typename L, typename H, typename R>
ConnectionStage<L, H, R>::~ConnectionStage() {
  delete this->message_handler;
}