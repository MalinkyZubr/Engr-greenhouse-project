#include "full_connect_routes.hpp"


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

NetworkReset::NetworkReset(const char* route, const Method method) : Route(route, method) {}

Response NetworkReset::execute(Request &request) {
  Response response(200);
  response.set_directive(DEVICE_NETWORK_RESET);

  return response;
}


DeviceReset::DeviceReset(const char* route, const Method method) : Route(route, method) {}

Response DeviceReset::execute(Request &request) {
  Response response(200);
  response.set_directive(DEVICE_HARD_RESET);
  
  return response;
}


SetTime::SetTime(const char* route, const Method method, StorageManager *global_storage) : Route(route, method), global_storage(global_storage) {}

Response SetTime::execute(Request &request) {
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


ConfigureDeviceIds::ConfigureDeviceIds(const char* route, const Method method, StorageManager *global_storage) : Route(route, method), global_storage(global_storage) {}

Response ConfigureDeviceIds::execute(Request &request) {
  Response response;
  StorageException exception = this->global_storage->get_identifiers().write(request.get_body());
  
  response = storage_route_error_handler(exception);

  return response;
}


ConfigureDevicePreset::ConfigureDevicePreset(const char* route, const Method method, StorageManager *global_storage) : Route(route, method), global_storage(global_storage) {}

//error handling should also go here
Response ConfigureDevicePreset::execute(Request &request) {
  Response response;
  StorageException exception = this->global_storage->get_preset().write(request.get_body());

  response = storage_route_error_handler(exception);

  return response;
}


PauseDevice::PauseDevice(const char* route, const Method method, StorageManager *global_storage) : Route(route, method), global_storage(global_storage) {}

Response PauseDevice::execute(Request &request) {
  Response response;
  bool paused = request.get_body()["paused"];
  StorageException exception;

  MachineOperationalState state = this->global_storage->get_machine_state().get_state();

  if(paused && state != MACHINE_PAUSED) {
    exception = this->global_storage->get_machine_state().from_json(request.get_body());
    response = storage_route_error_handler(exception, 202);
  }
  else if(state == MACHINE_PAUSED) {
    response = Response(429);
  }
  else if(!paused && state != MACHINE_ACTIVE) {
    exception = this->global_storage->get_machine_state().from_json(request.get_body());
    response = storage_route_error_handler(exception);
  }
  else {
    response = Response(429);
  }

  return response;
}