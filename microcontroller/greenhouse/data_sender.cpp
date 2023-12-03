#include "data_sender.hpp"


DataSender::DataSender(CommonData *common_data, MachineState *machine_state, ConfigManager *storage, ConnectionManager *connection_manager) : common_data(common_data), machine_state(machine_state), storage(storage), connection_manager(connection_manager) {}

void DataSender::callback() {
  if(this->machine_state->operational_state == MACHINE_PAUSED) {
    return;
  }

  String request;
  DynamicJsonDocument data(CONFIG_JSON_SIZE);

  data["project_name"] = this->storage->config.identifying_information.project_name;
  data["temperature"] = this->common_data->temperature;
  data["humidity"] = this->common_data->humidity;
  data["moisture"] = this->common_data->moisture;
  data["light_exposure"] = this->common_data->light_level;

  switch(this->machine_state->connection_state) {
    case MACHINE_CONNECTED:
      request = Requests::request(POST, String("/interface/data/" + this->storage->config.identifying_information.device_name), 
                                        this->connection_manager->server_information.ip, 
                                        this->storage->config.identifying_information.device_id, data);
      this->connection_manager->connected_send(request);
      break;
    case MACHINE_DISCONNECTED:
      break;
  }
}