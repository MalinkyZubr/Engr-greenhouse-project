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
#include "router.hpp"
#include "machine_state.hpp"
#include "wifi_info.hpp"


#define RECEPTION_PID 6
#define LOCAL_PORT 2211
#define UDPMulticastAddress "224.0.2.4"
#define UDPReceiveBuffSize 30
#define EXTERNAL_PORT 1337
#define AP_CONFIG_PASSWORD "GREENHOUSE_CONFIG"


typedef struct {
    String ip;
    String mac;
    int port;
    NetworkReturnErrors error;
} ConnectionInfo;

enum States {
    INITIALIZING, 
    WIFI_RECOVERY,
    BROADCASTING,
    ASSOCIATING,
    CONNECTED, 
    DOWN
};

class Connection {
  public:
  WiFiUDP UDPserver;
  WiFiServer Startupserver; 
  WiFiSSLClient SSLclient; // for sending requests to the server
  WiFiServer SSLlistener; // you should use beginSSL on this for receiving https connections

  Connection() : Startupserver(LOCAL_PORT), SSLlistener(LOCAL_PORT) {};
};

class ConnectionManager {
  public:
  MachineState *machine_state;
  States network_state = INITIALIZING;

  WifiInfo wifi_information;
  ConnectionInfo own_information;
  ConnectionInfo server_information;
  TaskManager *task_manager;
  Connection state_connection;
  MessageQueue *message_queue;

  Router *router;

  ConfigManager *storage;

  ConnectionManager(TaskManager *task_manager, Router *routes, ConfigManager *storage, MessageQueue *message_queue, MachineState *machine_state);
  ConnectionManager(TaskManager *task_manager, Router *routes, ConfigManager *storage, MessageQueue *message_queue, MachineState *machine_state, WifiInfo wifi_information);
  ConnectionManager(TaskManager *task_manager, Router *routes, ConfigManager *storage, MessageQueue *message_queue, MachineState *machine_state, WifiInfo wifi_information, ConnectionInfo server_information);

  ParsedMessage rest_receive(WiFiClient &client, int timeout);

  // initialization
  bool set_ssid_config();

  int get_ap_channel(String &ssid);

  bool enterprise_connect(WifiInfo &enterprise_wifi_info);
  bool home_connect(WifiInfo &home_wifi_info);

  bool connect_wifi(WifiInfo &wifi_info);

  bool initialization();
  WifiInfo receive_credentials(WiFiClient &client);
  
  NetworkReturnErrors send_broadcast(String &json_data, IPAddress &address, char *receive_buffer, int buff_size, DynamicJsonDocument &received, int timeout); 
  bool broadcast(bool expidited);

  bool connect_to_server();
  void package_identifying_info(DynamicJsonDocument &to_package);
  const char* prepare_identifier_field(String &field_value);
  int* prepare_identifier_field(int &field_value);
  void write_identifying_info(ParsedResponse &response);
  bool association(); // to be run inside broadcast when receive confirmation

  NetworkReturnErrors listener();
  void listener_error_handler(NetworkReturnErrors error);
  ParsedResponse connected_send(String &request);

  void run();
};

class WiFiWatchdog : public Callable {
  private:
  int wifi_fail_counter = 0;
  void check_wifi_status();
  bool handle_wifi_down();
  MachineState *machine_state;

  public:
  ConnectionManager *connected_manager;
  CommonData *common_data;

  WiFiWatchdog(CommonData *common_data, ConnectionManager *connected_manager, MachineState *machine_state);
  void callback();
};

#endif