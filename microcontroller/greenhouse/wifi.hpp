#ifndef WIFI_HPP
#define WIFI_HPP

#include <Arduino.h>
#include <WiFi101.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include "async.hpp"
#include "storage.hpp"
#include "http.hpp"
#include "webpage.hpp"
#include "exceptions.hpp"
#include "startup_webpage_routes.hpp"
#include "full_connect_routes.hpp"
#include "network_utils.hpp"
#include "udp_messages.hpp"


#define RECEPTION_PID 6
#define LOCAL_PORT 2211
#define UDPMulticastAddress "224.0.2.4"
#define UDPReceiveBuffSize 30
#define EXTERNAL_PORT 1337
#define AP_CONFIG_PASSWORD "GHCONFIG"


/// @brief global machine connection state for tracking if the device is connected to the server or not
enum MachineConnectionState {
  MACHINE_CONNECTED,
  MACHINE_DISCONNECTED
};

/// @brief connection info for server and client connections
/// @param ip: string representing target IP
/// @param mac: string representing target mac address
/// @param exception: exception for when returning connection info from methods
typedef struct {
    IPAddress ip;
    int port;
} ConnectionInformation;


/// @brief enum containing all network manager states. Each of these represents a different step in the network management process
/// @param INITIALIZING the device is acting in AP Mode waiting to serve a webpage to configure wifi information, and connect to a network
/// @param BROADCASTING the device is broadcasting UDP packets to the mutlicast server address waiting for acknowledgement to proceed with association
/// @param ASSOCIATING the device is authenticating to the server and syncing its local data (preset, identifiers, etc) with the server. Prelude to full connection
/// @param CONNECTED the device is fully authenticated and connected to the server, and can commence regular operations
/// @param STARTUP this is the wifi network connection sequence if the device has already connected to the wifi previously
/// @param DOWN state when the device loses connection and must recover
enum ConnectionStageIdentifier {
  STAGE_INITIALIZING, 
  STAGE_BROADCASTING,
  STAGE_ASSOCIATING,
  STAGE_CONNECTED, 
  STAGE_STARTUP,
  STAGE_DOWN
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

enum Layer4Encryption {
  ENCRYPTION_SSL,
  ENCRYPTION_NONE
};

class NetworkClient {
  private:
  Layer4Encryption layer_4_encryption_type = NONE;

  ParsedMessage parse_message(String &unparsed_message);
  NetworkExceptions client_receive_message(WiFiClient &client, int timeout, String *message);

  public:
  NetworkClient(Layer4Encryption layer_4_encryption)
  Layer4Encryption get_encryption();

  NetworkExceptions client_send_message(WiFiClient &client, HTTPMessage *message); // must free dynamically allocated message
  HTTPMessage* receive_and_parse(WiFiClient &client, int timeout);
};

class TCPRequestClient : private NetworkClient {
  private:
  WiFiClient &client;
  ConnectionInformation server_information;

  Response receive_response(WiFiClient &client);

  public:
  TCPRequestClient(WiFiClient &client, Layer4Encryption layer_4_encryption, ConnectionInformation &server_information);

  Response request(Request *message);
};

class TCPListenerClient : private NetworkClient {
  private:
  WiFiServer &listener;
  Request* request_queue;
  Router router;

  NetworkExceptions respond(Response &response, WiFiClient &client);

  public:
  TCPListenerClient(WiFiServer &listener, Layer4Encryption layer_4_encryption, Router router);
  NetworkExceptions listen();
};

class UDPClient {
  private:
  WiFiUDP &udp_server;
  ConnectionInformation multicast_information;

  public:
  UDPClient(WiFiUDP &udp_server, int local_port, ConnectionInformation multicast_information)
  NetworkExceptions send_udp(UDPRequest &request);
  UDPResponse receive_udp(int timeout);
};

typedef struct {
  Identifiers identifier;
  Preset preset;
} AssociationReturnStruct;

class StageReturnBase {
  NetworkExceptions exception;

  public:
  StageReturnBase() {};
  StageReturnBase(NetworkExceptions exception);

  NetworkExceptions get_exception();
  void set_exception(NetworkExceptions exception);
};

template<typename R>
class StageReturn : public StageReturnBase {
  private:
  R return_value;

  public:
  StageReturn() {};
  StageReturn(NetworkExceptions exception, R return_value);

  R get_return_value();
  NetworkExceptions get_exception();

  void set_return_value(R return_value);
  void set_exception(NetworkExceptions exception);
};

template<typename L, typename H, typename R>
class ConnectionStage {
  private:
  L listener;
  H *message_handler;

  public:
  ConnectionStage() {};
  ConnectionStage(int listener_port) : listener(listener_port) {};

  L get_listener();

  void set_handler(H* message_handler)
  H* get_handler();

  virtual StageReturn<R> *run() = 0;

  ~ConnectionStage();
};

class StageWifiBaseClass : public ConnectionStage<WifiServer, TCPListenerClient, WifiInfo> {
  private:
  bool enterprise_connect(WifiInfo &info);
  bool home_connect(WiFiInfo &info);

  public:
  NetworkExceptions connect_wifi(WiFiInfo &info);
};

// add common templated interface for the message handler
class StageWifiInitialization : public StageWifiBaseClass {
  private:
  WifiInfo temporary_wifi_information;
  bool wifi_configured = false;

  String ssid_config();
  bool start_access_point();
  NetworkExceptions receive_credentials(WiFiClient &client);

  public:
  StageWifiInitialization(int listen_port);
  StageReturn<WifiInfo> *run() override;
};

class StageWifiStartup : public StageWifiBaseClass {
  private:
  WifiInfo &wifi_info;

  public:
  StageWifiStartup(WifiInfo &wifi_info);
  StageReturn<WifiInfo> *run() override;
};

class StageBroadcasting : public ConnectionStage<WiFiUDP, UDPClient, ConnectionInformation> {
  private:
  ConnectionInformation multicast_information;
  ConnectionInformation local_information;
  Identifiers &device_identifiers;

  public:
  StageBroadcasting(ConnectionInformation local_information, ConnectionInformation multicast_information, const Identifiers &device_identifiers);
  StageReturn<ConnectionInformation> *run() override;
};

class StageAssociation : public ConnectionStage<WiFiSSLClient, TCPRequestClient, AssociationReturnStruct> {
  private:
  ConnectionInformation server_information;
  Identifiers &identifiers;
  MachineState &machine_state;

  NetworkExceptions test_server_connection();
  Request generate_registration_request();

  public:
  StageAssociation(ConnectionInformation server_information, Identifiers &identifiers, MachineState &machine_state)
  StageReturn<AssociationReturnStruct> *run() override;
};

// R should not be nullptr
class StageFullConnection : public ConnectionStage<WiFiServer, TCPListenerClient, bool> {
  private:
  WiFiSSLClient wifi_ssl_object;
  TCPRequestClient request_client;
  ConnectionInformation server_information

  NetworkExceptions handle_incoming();

  public:
  StageFullConnection(ConnectionInformation &server_information);
  Response send_request(Request &request);
  StageReturn<bool>* run() override;
};

class ConnectionManager {
  private:
  ConnectionInformation local_connection_information;
  ConnectionInformation server_information;
  ConnectionInformation multicast_information;

  ConnectionStageIdentifier stage_identifier;
  ConnectionStage* stage;

  StorageManager *global_storage;

  void set_stage_object(ConnectionStageIdentifier stage);
  void handle_returns(StageReturn *return_value);
  void handle_error(NetworkExceptions exception);

  public:
  ConnectionManager(int local_port, StorageManager *global_storage)
  void run();
};

class WiFiWatchdog : public Callable {
  private:
  int wifi_fail_counter = 0;
  MachineState *machine_state;

  void check_wifi_status();

  public:
  ConnectionManager *connected_manager;

  WiFiWatchdog(ConnectionManager *connected_manager, MachineState *machine_state);
  void callback();
};

#endif