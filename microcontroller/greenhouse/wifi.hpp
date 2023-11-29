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


enum NetworkTypes {
  HOME,
  ENTERPRISE,
  OPEN
};

typedef struct {
  NetworkTypes type;
  String ssid;
  String username;
  String password;
  int channel;
} WifiInfo;

typedef struct {
    String ip;
    String mac;
    int port;
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

  ParsedMessage rest_receive(WiFiClient &client);

  // initialization
  bool set_ssid_config();

  int get_ap_channel(String &ssid);

  bool enterprise_connect(WifiInfo &enterprise_wifi_info);
  bool home_connect(WifiInfo &home_wifi_info);

  bool connect_wifi(WifiInfo &wifi_info);

  bool initialization();
  WifiInfo receive_credentials(WiFiClient &client);
  bool configuration(); // soft ap mode operations

  bool association(); // to be run inside broadcast when receive confirmation
  
  bool send_broadcast(String &json_data, IPAddress &address, char *receive_buffer, int buff_size, DynamicJsonDocument &received); 
  bool broadcast();

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