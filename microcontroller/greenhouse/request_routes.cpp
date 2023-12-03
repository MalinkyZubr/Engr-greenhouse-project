#include "request_routes.hpp"


ResetDevice::ResetDevice(String route, ConfigManager *storage, MachineState *machine_state, ConnectionManager *connection_manager) : Route(route), storage(storage), machine_state(machine_state), connection_manager(connection_manager) {}

NetworkReturnErrors ResetDevice::execute(WiFiClient &client, ParsedRequest &request, String *response) {
  this->storage->reset();
  this->machine_state->operational_state = MACHINE_PAUSED;
  this->machine_state->connection_state = MACHINE_DISCONNECTED;
  this->connection_manager->network_state = INITIALIZING;

  return CONNECTION_FAILURE; // to break out of the conneciton loop
}

SetTime::SetTime(String route, CommonData *common_data) : Route(route), common_data(common_data) {}

NetworkReturnErrors SetTime::execute(WiFiClient &client, ParsedRequest &request, String *response) {
  bool nighttime = request.body["is_night"];
  this->common_data->nighttime = nighttime;

  return OKAY;
}


ConfigureDevice::ConfigureDevice(String route, ConfigManager *storage) : Route(route), storage(storage) {
    // implement here
}