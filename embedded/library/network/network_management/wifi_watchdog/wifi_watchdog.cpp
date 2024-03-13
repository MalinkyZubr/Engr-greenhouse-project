#include "wifi.hpp"


///////////////////////////////////////////////////
///////// WifiWatchdog ////////////////////////////
///////////////////////////////////////////////////

/// @brief the wifi watchdog watches to see if the connection to the wireless AP is stable. if connection is lost, it sets the network state to down. This is meant to be a Task manager task @see TaskManager
/// @param connected_manager the connection manager for which the watchdog is checking wifi status
/// @param machine_state machine state with which the watchdog tracks and sets machine connection state on wifi failure
WiFiWatchdog::WiFiWatchdog(ConnectionManager *connected_manager, StorageManager *global_storage) : connected_manager(connected_manager), global_storage(global_storage) {}


/// @brief checks the wifi status of the connection manager. if the connection to AP is down, machine network state is set to MACHINE_DISCONNETED and the network state is set to down
void WiFiWatchdog::check_wifi_status() {
  int Wifi_status = WiFi.status();

  // check if the wifi is disconnected, first and foremost
  if(Wifi_status == WL_CONNECTION_LOST || Wifi_status == WL_DISCONNECTED) {
    this->global_storage->set_network_state(MACHINE_DISCONNECTED);
    this->connected_manager->set_stage_identifier(STAGE_STARTUP);
  }
}


/// @brief callback for the wifi watchdog to be run by the task manager
void WiFiWatchdog::callback() {
  this->check_wifi_status();
}