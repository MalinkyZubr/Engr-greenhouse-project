#ifndef DATA_SENDER_HPP
#define DATA_SENDER_HPP


#include "wifi.hpp"
#include "storage.hpp"
#include "async.hpp"
#include "http.hpp"


class DataSender : Callable {
  private:
  StorageManager *global_storage;
  ConnectionManager *connection_manager;

  public:
  void flush_data_storage_to_server();
  DataSender(StorageManager *global_storage, ConnectionManager *connection_manager);
  void callback();
};

#endif