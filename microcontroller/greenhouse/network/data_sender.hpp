#ifndef DATA_SENDER_HPP
#define DATA_SENDER_HPP


#include "wifi.hpp"
#include "../storage/storage.hpp"
#include "../asynchronous/machine_state.hpp"
#include "../asynchronous/async.hpp"
#include "http.hpp"


class DataSender : Callable {
  private:
  CommonData *common_data;
  MachineState *machine_state;
  ConfigManager *storage;
  ConnectionManager *connection_manager;

  public:
  void flush_data_storage_to_server();
  DataSender(CommonData *common_data, MachineState *machine_state, ConfigManager *config_manager, ConnectionManager *connection_manager);
  void callback();
};

#endif