#include "full_connect_routes.hpp"


/// @brief route telling the device to disconnect from the server and return to broadcasting mode (WIP to convert from device reset)
/// @param route string for the route to set this object to listen for
/// @param storage for interfacing with storage to make reset of network persistent
/// @param machine_state global machine state to modify device behavior after network reset
/// @param connection_manager connection manager, needed to change network state
NetworkReset::NetworkReset(String route, ConfigManager *storage, MachineState *machine_state, ConnectionManager *connection_manager) : Route(route), storage(storage), machine_state(machine_state), connection_manager(connection_manager) {}

NetworkReturnErrors NetworkReset::execute(ParsedRequest &request, String *response) { // find something else to do here
  this->storage->reset();
  this->machine_state->operational_state = MACHINE_PAUSED;
  this->machine_state->connection_state = MACHINE_DISCONNECTED;
  this->connection_manager->network_state = INITIALIZING;

  return CONNECTION_FAILURE; // to break out of the conneciton loop
}


/// @brief request to entirely reset the device (using the reset pin)
/// @param route route for the device reset
/// @param storage storage manager for executing the device reset
DeviceReset::DeviceReset(String route, ConfigManager *storage) : Route(route), storage(storage) {}

NetworkReturnErrors DeviceReset::execute(ParsedRequest &request, String *response) {
  // server must not check for response
  this->storage->reset();
}


/// @brief time sync request from the server to set the time on the device, and let the device know when its nighttime. This should be send twice every 24 hours, once at sunset, once at sunrise
/// @param route route to listen on for time request
/// @param common_data global common data struct used for storing whether or not it is day or nighttime
SetTime::SetTime(String route, CommonData *common_data) : Route(route), common_data(common_data) {}

NetworkReturnErrors SetTime::execute(ParsedRequest &request, String *response) {
  bool nighttime = request.body["is_night"];
  this->common_data->nighttime = nighttime;

  return OKAY;
}


/// @brief route for configuring device identification fields
/// @param route the route to listen on for device identification modification requests
/// @param storage storage object to write configurations to
ConfigureDeviceIds::ConfigureDeviceIds(String route, ConfigManager *storage) : Route(route), storage(storage) {}

NetworkReturnErrors ConfigureDeviceIds::execute(ParsedRequest &request, String *response) {
  Identifiers new_identifiers;
  this->storage->deserialize_device_identifiers(new_identifiers, request.body);
  if(this->storage->set_device_identifiers(new_identifiers)) {
   *response = Responses::response(200, false);
    return OKAY;
  }
  else {
    *response = Responses::response(500, false);
    return STORAGE_WRITE_FAILURE;
  }
}


/// @brief route to listen on for preset configurations from the server
/// @param route route on which device listens for preset config requests
/// @param storage storage interface to write configurations to flash memory
ConfigureDevicePreset::ConfigureDevicePreset(String route, ConfigManager *storage) : Route(route), storage(storage) {}

//error handling should also go here
NetworkReturnErrors ConfigureDevicePreset::execute(ParsedRequest &request, String *response) {
  Preset new_preset;
  this->storage->deserialize_preset(new_preset, request.body);
  if(this->storage->set_preset(new_preset)) {
    *response = Responses::response(200, false);
    return OKAY;
  }
  else {
    *response = Responses::response(500, false);
    return STORAGE_WRITE_FAILURE;
  }
}


/// @brief request from server to change the machine operational state, either to or from paused
/// @param route route to listen for state change requests on
/// @param machine_state global machine state to write requested status to
PauseDevice::PauseDevice(String route, MachineState *machine_state) : Route(route), machine_state(machine_state) {}

NetworkReturnErrors PauseDevice::execute(ParsedRequest &request, String *response) {
  bool paused = request.body["paused"];
  if(paused && this->machine_state->operational_state != MACHINE_PAUSED) {
    this->machine_state->operational_state = MACHINE_PAUSED;
    *response = Responses::response(202, false);
  }
  else if(this->machine_state->operational_state == MACHINE_PAUSED) {
    *response = Responses::response(429, false);
  }
  else if(!paused && this->machine_state->operational_state != MACHINE_ACTIVE) {
    this->machine_state->operational_state = MACHINE_ACTIVE;
    *response = Responses::response(200, false);
  }
  else {
    *response = Responses::response(429, false);
  }

  return OKAY;
}