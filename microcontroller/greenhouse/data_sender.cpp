#include "data_sender.hpp"


DataSender::DataSender(CommonData *common_data, MachineState *machine_state, ConfigManager *storage, ConnectionManager *connection_manager) : common_data(common_data), machine_state(machine_state), storage(storage), connection_manager(connection_manager) {}

void DataSender::flush_data_storage_to_server() {
  String request;
  DynamicJsonDocument data(CONFIG_JSON_SIZE);

  bool done = false;

  while(!done) {
    done = this->storage->writer->decrement_read(data);
    request = Requests::request(POST, String("/interface/olddata/"), // create server route for olddata that processes time
                                        this->connection_manager->server_information.ip, 
                                        this->storage->config.identifying_information.device_id, this->machine_state->operational_state, data);
    this->connection_manager->connected_send(request);
  }
  
  this->storage->writer->erase_all_data();
}

void DataSender::callback() {
  if(this->machine_state->operational_state == MACHINE_PAUSED) {
    return;
  }

  String request;
  DynamicJsonDocument data(CONFIG_JSON_SIZE);

  data["project_id"] = this->storage->config.identifying_information.project_id;
  data["device_id"] = this->storage->config.identifying_information.device_id;
  data["temperature"] = this->common_data->temperature;
  data["humidity"] = this->common_data->humidity;
  data["moisture"] = this->common_data->moisture;
  data["light_exposure"] = this->common_data->light_level;

  switch(this->machine_state->connection_state) {
    case MACHINE_CONNECTED:
      if(this->storage->writer->is_storing) {
        this->flush_data_storage_to_server();
      }
      request = Requests::request(POST, String("/interface/data/"), 
                                        this->connection_manager->server_information.ip, 
                                        this->storage->config.identifying_information.device_id, this->machine_state->operational_state, data);
      this->connection_manager->connected_send(request);
      break;
    case MACHINE_DISCONNECTED:
      this->storage->writer->write_data(data);
      break;
  }
}