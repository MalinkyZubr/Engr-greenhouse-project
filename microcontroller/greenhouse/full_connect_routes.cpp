#include "full_connect_routes.hpp"


Router full_connection_router(StorageManager *global_storage) {
  Router router;

  return router
    .add_route(new NetworkReset("/reset/network", POST))
    .add_route(new DeviceReset("/reset/hard", DELETE))
    .add_route(new SetTime("/time", POST, global_storage))
    .add_route(new ConfigureDeviceIds("/configure/id", POST, global_storage))
    .add_route(new ConfigureDevicePreset("/configure/preset", POST, global_storage))
    .add_route(new PauseDevice("/configure/status", PUT, global_storage));
}

Response storage_route_error_handler(StorageException exception, int okay_code = 200) {
  Response response;

  switch(exception) {
    case STORAGE_IDENTIFIER_FIELD_MISSING:
      response = Response(415);
      break;
    case STORAGE_WRITE_FAILURE:
      response = Response(500);
      break;
    case STORAGE_OKAY:
      response = Response(okay_code);
      break;
  }
  
  return response;
}

///////////////////////////////////////////////////
////////////////// NetworkReset ///////////////////
///////////////////////////////////////////////////

NetworkReset::NetworkReset(const String route, const Method method) : Route(route, method) {}

Response NetworkReset::execute(ParsedRequest &request) {
  Response response(200);
  response.set_directive(DEVICE_NETWORK_RESET);

  return response;
}


DeviceReset::DeviceReset(const String route, const Method method) : Route(route, method) {}

Response DeviceReset::execute(ParsedRequest &request) {
  Response response(200);
  response.set_directive(DEVICE_HARD_RESET);
  
  return response;
}


SetTime::SetTime(const String route, const Method method, StorageManager *global_storage) : Route(route, method), global_storage(global_storage) {}

Response SetTime::execute(ParsedRequest &request) {
  Response response;

  if(!request.get_body().containsKey("datetime")) {
    response = Response(415);
  }
  else {
    DataManager data_manager = this->global_storage->get_data_manager();
    data_manager.set_reference_datetime(request.get_body()["datetime"]);

    response = Response(200);
  }

  return response;
}


ConfigureDeviceIds::ConfigureDeviceIds(const String route, const Method method, StorageManager *global_storage) : Route(route, method), global_storage(global_storage) {}

Response ConfigureDeviceIds::execute(ParsedRequest &request) {
  Response response;
  StorageException exception = this->global_storage->get_identifiers().write(request.get_body());
  
  response = storage_route_error_handler(exception);

  return response;
}


ConfigureDevicePreset::ConfigureDevicePreset(const String route, const Method method, StorageManager *global_storage) : Route(route, method), global_storage(global_storage) {}

//error handling should also go here
Response ConfigureDevicePreset::execute(ParsedRequest &request) {
  Response response;
  StorageException exception = this->global_storage->get_preset().write(request.get_body());

  response = storage_route_error_handler(exception);

  return response;
}


PauseDevice::PauseDevice(const String route, const Method method, StorageManager *global_storage) : Route(route, method), global_storage(global_storage) {}

Response PauseDevice::execute(ParsedRequest &request) {
  Response response;
  bool paused = request.body["paused"];
  StorageException exception

  MachineState state_manager = this->global_storage->get_machine_state();
  MachineOperationalState state = state_manager.get_state();

  if(paused && state != MACHINE_PAUSED) {
    exception = state_manager.from_json(request.get_body());
    response = storage_route_error_handler(exception, 202);
  }
  else if(this->machine_state->operational_state == MACHINE_PAUSED) {
    response = Response(429);
  }
  else if(!paused && this->machine_state->operational_state != MACHINE_ACTIVE) {
    exception = state_manager.from_json(request.get_body());
    response = storage_route_error_handler(exception);
  }
  else {
    response = Response(429);
  }

  return response;
}