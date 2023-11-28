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
#define PORT 2211


enum NetworkTypes {
    HOME,
    ENTERPRISE
};

typedef struct {
    String ssid;
    String username;
    String password;
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

  Connection() : Startupserver(PORT){};
};

class ConnectionManager {
  public:
  States state = INITIALIZING;
  NetworkTypes type = HOME;

  WifiInfo wifi_information;
  ConnectionInfo own_information;
  ConnectionInfo server_information;
  TaskManager task_manager;
  Connection state_connection;

  ConfigManager storage;

  ConnectionManager(NetworkTypes type, TaskManager task_manager, ConfigManager storage);
  ConnectionManager(NetworkTypes type, TaskManager task_manager, ConfigManager storage, WifiInfo wifi_information);
  ConnectionManager(NetworkTypes type, TaskManager task_manager, ConfigManager storage, WifiInfo wifi_information, ConnectionInfo server_information);
  
  ParsedMessage rest_receive(WiFiClient client);
  bool rest_send(String message);

  bool set_ssid_config();
  bool initialization();
  WifiInfo receive_credentials(WiFiClient &client);
  bool check_credentials(WifiInfo &wifi_information);
  bool configuration(); // soft ap mode operations

  ConnectionInfo association(); // to be run inside broadcast when receive confirmation
  ConnectionInfo broadcast();

  void connected();
  bool send();
  DynamicJsonDocument receive();
  void run();
};

#endif