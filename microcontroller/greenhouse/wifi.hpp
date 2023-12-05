#ifndef WIFI_HPP
#define WIFI_HPP

#include <Arduino.h>
#include <WiFi101.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include "async.hpp"
#include "storage.hpp"
#include "machine_state.hpp"
#include "http.hpp"
#include "webpage.hpp"
#include "router.hpp"
#include "wifi_info.hpp"


#define RECEPTION_PID 6
#define LOCAL_PORT 2211
#define UDPMulticastAddress "224.0.2.4"
#define UDPReceiveBuffSize 30
#define EXTERNAL_PORT 1337
#define AP_CONFIG_PASSWORD "GREENHOUSE_CONFIG"


/// @brief connection info for server and client connections
/// @param ip: string representing target IP
/// @param mac: string representing target mac address
/// @param error: error for when returning connection info from methods
typedef struct {
    String ip;
    String mac;
    int port;
    NetworkReturnErrors error;
} ConnectionInfo;


/// @brief enum containing all network manager states. Each of these represents a different step in the network management process
/// @param INITIALIZING the device is acting in AP Mode waiting to serve a webpage to configure wifi information, and connect to a network
/// @param BROADCASTING the device is broadcasting UDP packets to the mutlicast server address waiting for acknowledgement to proceed with association
/// @param ASSOCIATING the device is authenticating to the server and syncing its local data (preset, identifiers, etc) with the server. Prelude to full connection
/// @param CONNECTED the device is fully authenticated and connected to the server, and can commence regular operations
/// @param STARTUP this is the wifi network connection sequence if the device has already connected to the wifi previously
/// @param DOWN state when the device loses connection and must recover
enum States {
    INITIALIZING, 
    BROADCASTING,
    ASSOCIATING,
    CONNECTED, 
    STARTUP,
    DOWN
};


/// @brief struct containing all the necessary objects for serving different connection types during association and operations
/// @param UDPserver: udp server object for sending and receiving UDP requests during broadcast phase
/// @param Startupserver: insecure wifiserver for communicating with client during wifi provisioning process (webpage for wifi setup)
/// @param SSLclient: sending secure requests to server during regular operations
/// @param SSLlistener: listen for and respond to requests from the server
class Connection {
  public:
  WiFiUDP UDPserver;
  WiFiServer Startupserver; 
  WiFiSSLClient SSLclient; // for sending requests to the server
  WiFiServer SSLlistener; // you should use beginSSL on this for receiving https connections

  Connection() : Startupserver(LOCAL_PORT), SSLlistener(LOCAL_PORT) {};
};

class ConnectionManager {
  private:
  bool check_ssid_existence(String &ssid);

  ParsedMessage rest_receive(WiFiClient &client, int timeout);

  // initialization
  bool set_ssid_config();

  int get_ap_channel(String &ssid);

  bool enterprise_connect(WifiInfo &enterprise_wifi_info);
  
  bool home_connect(WifiInfo &home_wifi_info);

  NetworkReturnErrors connect_wifi(WifiInfo &wifi_info);

  bool initialization();

  WifiInfo receive_credentials(WiFiClient &client);
  
  NetworkReturnErrors send_broadcast(String &json_data, IPAddress &address, char *receive_buffer, int buff_size, DynamicJsonDocument &received, int timeout); 

  bool broadcast(bool expidited);

  bool connect_to_server();

  bool association(); // to be run inside broadcast when receive confirmation

  void listener_error_handler(NetworkReturnErrors error);

  public:
  MachineState *machine_state;
  States network_state = INITIALIZING;

  WifiInfo wifi_information;
  ConnectionInfo own_information;
  ConnectionInfo server_information;
  TaskManager *task_manager;
  Connection state_connection;

  Router *router;

  ConfigManager *storage;

  ConnectionManager(TaskManager *task_manager, Router *routes, ConfigManager *storage, MachineState *machine_state);
  ConnectionManager(TaskManager *task_manager, Router *routes, ConfigManager *storage, MachineState *machine_state, WifiInfo wifi_information);
  ConnectionManager(TaskManager *task_manager, Router *routes, ConfigManager *storage, MachineState *machine_state, WifiInfo wifi_information, ConnectionInfo server_information);

  NetworkReturnErrors listener();
  ParsedResponse connected_send(String &request);

  void run();
};

class WiFiWatchdog : public Callable {
  private:
  int wifi_fail_counter = 0;
  void check_wifi_status();
  MachineState *machine_state;

  public:
  ConnectionManager *connected_manager;

  WiFiWatchdog(ConnectionManager *connected_manager, MachineState *machine_state);
  void callback();
};

#endif