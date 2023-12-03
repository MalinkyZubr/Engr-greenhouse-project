#include "request_routes.hpp"


ResetDevice::ResetDevice(String route, ConfigManager *storage, MachineState *machine_state, ConnectionManager *connection_manager) : Route(route), storage(storage), machine_state(machine_state), connection_manager(connection_manager) {}

NetworkReturnErrors ResetDevice::execute(ParsedRequest &request, String *response) {
  this->storage->reset();
  this->machine_state->operational_state = MACHINE_PAUSED;
  this->machine_state->connection_state = MACHINE_DISCONNECTED;
  this->connection_manager->network_state = INITIALIZING;

  return CONNECTION_FAILURE; // to break out of the conneciton loop
}

SetTime::SetTime(String route, CommonData *common_data) : Route(route), common_data(common_data) {}

NetworkReturnErrors SetTime::execute(ParsedRequest &request, String *response) {
  bool nighttime = request.body["is_night"];
  this->common_data->nighttime = nighttime;

  return OKAY;
}


ConfigureDeviceIds::ConfigureDeviceIds(String route, ConfigManager *storage) : Route(route), storage(storage) {}

NetworkReturnErrors ConfigureDeviceIds::execute(ParsedRequest &request, String *response) {
  Identifiers new_identifiers;
  this->storage->deserialize_device_identifiers(new_identifiers, request.body);
  if(this->storage->set_device_identifiers(new_identifiers)) {
   *response = Responses::response(200);
    return OKAY;
  }
  else {
    *response = Responses::response(500);
    return STORAGE_WRITE_FAILURE;
  }
}


ConfigureDevicePreset::ConfigureDevicePreset(String route, ConfigManager *storage) : Route(route), storage(storage) {}

//error handling should also go here
NetworkReturnErrors ConfigureDevicePreset::execute(ParsedRequest &request, String *response) {
  Preset new_preset;
  this->storage->deserialize_preset(new_preset, request.body);
  if(this->storage->set_preset(new_preset)) {
    *response = Responses::response(200);
    return OKAY;
  }
  else {
    *response = Responses::response(500);
    return STORAGE_WRITE_FAILURE;
  }
}

PauseDevice::PauseDevice(String route, MachineState *machine_state) : Route(route), machine_state(machine_state) {}

NetworkReturnErrors PauseDevice::execute(ParsedRequest &request, String *response) {
  bool paused = request.body["paused"];
  if(paused && this->machine_state->operational_state != MACHINE_PAUSED) {
    this->machine_state->operational_state = MACHINE_PAUSED;
    *response = Responses::response(202);
  }
  else if(this->machine_state->operational_state == MACHINE_PAUSED) {
    *response = Responses::response(429);
  }
  else if(!paused && this->machine_state->operational_state != MACHINE_ACTIVE) {
    this->machine_state->operational_state = MACHINE_ACTIVE;
    *response = Responses::response(200);
  }
  else {
    *response = Responses::response(429);
  }

  return OKAY;
}