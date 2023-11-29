#ifndef WIFI_HPP
#define WIFI_HPP

#include <Arduino.h>
#include <WiFi101.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include "async.hpp"
#include "http.hpp"
#include "storage.hpp"
#include "webpage.hpp"


#define RECEPTION_PID 6
#define LOCAL_PORT 2211
#define UDPMulticastAddress "224.0.2.4"
#define UDPReceiveBuffSize 30
#define EXTERNAL_PORT 1337
#define AP_CONFIG_PASSWORD "GREENHOUSE_CONFIG"


enum NetworkTypes {
  HOME,
  ENTERPRISE,
  OPEN,
};

typedef struct {
  NetworkTypes type;
  String ssid;
  String username;
  String password;
  int channel;
  ReturnErrors error;
} WifiInfo;

typedef struct {
    String ip;
    String mac;
    int port;
    ReturnErrors error;
} ConnectionInfo;

enum States {
    INITIALIZING, 
    BROADCASTING,
    ASSOCIATING,
    CONNECTED, 
    DOWN
};

class Connection {
  public:
  WiFiUDP UDPserver;
  WiFiSSLClient SSLclient;
  WiFiServer Startupserver;

  Connection() : Startupserver(LOCAL_PORT){};
};

class ConnectionManager {
  public:
  States state = INITIALIZING;

  WifiInfo wifi_information;
  ConnectionInfo own_information;
  ConnectionInfo server_information;
  TaskManager *task_manager;
  Connection state_connection;

  ConfigManager *storage;

  ConnectionManager(TaskManager *task_manager, ConfigManager *storage);
  ConnectionManager(TaskManager *task_manager, ConfigManager *storage, WifiInfo wifi_information);
  ConnectionManager(TaskManager *task_manager, ConfigManager *storage, WifiInfo wifi_information, ConnectionInfo server_information);

  ParsedMessage rest_receive(WiFiClient &client, int timeout);

  // initialization
  bool set_ssid_config();

  int get_ap_channel(String &ssid);

  bool enterprise_connect(WifiInfo &enterprise_wifi_info);
  bool home_connect(WifiInfo &home_wifi_info);

  bool connect_wifi(WifiInfo &wifi_info);

  bool initialization();
  WifiInfo receive_credentials(WiFiClient &client);
  
  ReturnErrors send_broadcast(String &json_data, IPAddress &address, char *receive_buffer, int buff_size, DynamicJsonDocument &received, int timeout); 
  bool broadcast();

  bool connect_to_server();
  void package_identifying_info(DynamicJsonDocument &to_package);
  const char* prepare_identifier_field(String &field_value);
  int* prepare_identifier_field(int &field_value);
  void write_identifying_info(ParsedResponse &response);
  bool association(); // to be run inside broadcast when receive confirmation

  void connected();
  bool send();
  DynamicJsonDocument receive();
  void run();
};

class WiFiWatchdog : public Callable {
  private:
  int wifi_fail_counter = 0;
  void check_wifi_status();
  bool handle_wifi_down();

  public:
  ConnectionManager *connected_manager;
  CommonData *common_data;

  WiFiWatchdog(CommonData *common_data, ConnectionManager *connected_manager);
  void callback();
};

#endif