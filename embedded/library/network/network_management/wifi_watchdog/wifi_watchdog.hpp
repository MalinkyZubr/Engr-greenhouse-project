class WiFiWatchdog : public Callable {
  private:
  int wifi_fail_counter = 0;
  StorageManager *global_storage;

  void check_wifi_status();

  public:
  ConnectionManager *connected_manager;

  WiFiWatchdog(ConnectionManager *connected_manager, StorageManager *global_storage);
  void callback();
};