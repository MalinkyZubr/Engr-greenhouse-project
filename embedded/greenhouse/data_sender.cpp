#include "data_sender.hpp"


/// @brief utility class for periodically sending data to the server, or if disconnected, writing data to the flash memory unit
/// @param common_data global common data struct for retrieving data from to send out
/// @param machine_state global machine state to determine state driven operations
/// @param storage config manager object to write to storage in case of disconnect from server
/// @param connection_manager connection manager to send data with to the server
DataSender::DataSender(StorageManager *global_storage, ConnectionManager *connection_manager) : global_storage(global_storage), connection_manager(connection_manager) {}


/// @brief when the device reconnects to the server, it must upload all of the stored data obtained during the disconnect period. This function achieves this
void DataSender::flush_data_storage_to_server() {
  Request request;
  DynamicJsonDocument data(CONFIG_JSON_SIZE);

  bool done = false;

  while(!done) {
    done = this->global_storage->get_data_manager().read_data(data);
    request = Request(POST, "/interface/olddata/", // create server route for olddata that processes time
                                        this->connection_manager->get_server_information().ip, 
                                        this->global_storage->get_identifiers().get_device_id(), this->global_storage->get_machine_state().get_state(), data);
    this->connection_manager->send_request(request);
    data.clear();
  }
  
  this->global_storage->get_data_manager().erase_data();
}


/// @brief DataSender callback for the task manager to execute. If the device is disconnected, it will write data to the flash memory. Otherwise it will write it to the server over wifi
void DataSender::callback() {
  if(this->global_storage->get_machine_state().get_state() == MACHINE_PAUSED) {
    return;
  }

  Request request;
  DynamicJsonDocument data(CONFIG_JSON_SIZE);
  data = this->global_storage->get_common_data().to_json();

  data["project_id"] = this->global_storage->get_identifiers().get_project_id();
  data["device_id"] = this->global_storage->get_identifiers().get_device_id();

  switch(this->global_storage->get_network_state()) {
    case MACHINE_CONNECTED:
      if(this->global_storage->get_data_manager().check_is_storing()) {
        this->flush_data_storage_to_server();
      }
      request = Request(POST, "/interface/data/", 
                                        this->connection_manager->get_server_information().ip, 
                                        this->global_storage->get_identifiers().get_device_id(), this->global_storage->get_machine_state().get_state(), data);
      this->connection_manager->send_request(request);
      break;
    case MACHINE_DISCONNECTED:
      this->global_storage->get_data_manager().write_data(data);
      break;
  }
}