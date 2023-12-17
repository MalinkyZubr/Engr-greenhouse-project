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


/// @brief connection info for server and client connections
/// @param ip: string representing target IP
/// @param mac: string representing target mac address
/// @param exception: exception for when returning connection info from methods
typedef struct {
    String ip;
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

enum Layer4Encryption {
  ENCRYPTION_SSL,
  ENCRYPTION_NONE
};

class NetworkClient {
  private:
  Layer4Encryption layer_4_encryption_type = ENCRYPTION_NONE;

  HTTPMessage* parse_message(String &unparsed_message);
  NetworkException client_receive_message(WiFiClient &client, int timeout, String *message);

  public:
  NetworkClient(Layer4Encryption layer_4_encryption);
  Layer4Encryption get_encryption();

  NetworkException client_send_message(WiFiClient &client, HTTPMessage *message); // must free dynamically allocated message
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

  NetworkException respond(Response &response, WiFiClient &client);

  public:
  TCPListenerClient(WiFiServer &listener, Layer4Encryption layer_4_encryption, Router router);
  NetworkException listen();
};

class UDPClient {
  private:
  WiFiUDP &udp_server;
  ConnectionInformation multicast_information;

  public:
  UDPClient(WiFiUDP &udp_server, ConnectionInformation &local_information, ConnectionInformation &multicast_information);
  NetworkException send_udp(UDPRequest &request);
  UDPResponse receive_udp(int timeout);
};

typedef struct {
  Identifiers identifier;
  Preset preset;
} AssociationReturnStruct;

class StageReturnBase {
  NetworkException exception;

  public:
  StageReturnBase() {};
  StageReturnBase(NetworkException exception);

  virtual NetworkException get_exception() = 0;
  virtual void set_exception(NetworkException exception) = 0;
};

template<typename R>
class StageReturn : public StageReturnBase {
  private:
  R return_value;

  public:
  StageReturn() {};
  StageReturn(NetworkException exception, R return_value);

  R get_return_value();
  void set_return_value(R return_value);

  NetworkException get_exception();
  void set_exception(NetworkException exception);
};

class ConnectionStageBase {
  public:
  virtual StageReturnBase *run() = 0;
  virtual ~ConnectionStageBase() {};
};

template<typename L, typename H, typename R>
class ConnectionStage : public ConnectionStageBase {
  private:
  L listener;
  H *message_handler;

  public:
  ConnectionStage() {};
  ConnectionStage(int listener_port) : listener(listener_port) {};

  L& get_listener();

  void set_handler(H* message_handler);
  H* get_handler();

  virtual StageReturnBase *run() override = 0;

  ~ConnectionStage();
};

class StageWifiBaseClass : public ConnectionStage<WiFiServer, TCPListenerClient, WifiInfo> {
  private:
  bool enterprise_connect(WifiInfo &info);
  bool home_connect(WifiInfo &info);

  public:
  StageWifiBaseClass() {};
  StageWifiBaseClass(int listener_port) : ConnectionStage(listener_port) {};
  NetworkException connect_wifi(WifiInfo &info);
};

// add common templated interface for the message handler
class StageWifiInitialization : public StageWifiBaseClass {
  private:
  WifiInfo temporary_wifi_information;
  bool wifi_configured = false;

  String ssid_config();
  bool start_access_point();
  NetworkException receive_credentials();

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
  const Identifiers &device_identifiers;
  bool received_acknowledgement = false;

  public:
  StageBroadcasting(ConnectionInformation local_information, ConnectionInformation multicast_information, const Identifiers &device_identifiers);
  StageReturn<ConnectionInformation> *run() override;
};

class StageAssociation : public ConnectionStage<WiFiSSLClient, TCPRequestClient, AssociationReturnStruct> {
  private:
  ConnectionInformation server_information;
  Identifiers &identifiers;
  MachineState &machine_state;

  NetworkException test_server_connection();
  Request generate_registration_request();

  public:
  StageAssociation(ConnectionInformation server_information, Identifiers &identifiers, MachineState &machine_state);
  StageReturn<AssociationReturnStruct> *run() override;
};

// R should not be nullptr
class StageFullConnection : public ConnectionStage<WiFiServer, TCPListenerClient, bool> {
  private:
  WiFiSSLClient wifi_ssl_object;
  TCPRequestClient request_client;
  ConnectionInformation server_information;

  NetworkException handle_incoming();

  public:
  StageFullConnection(ConnectionInformation &server_information, StorageManager *global_storage);
  Response send_request(Request &request);
  StageReturn<bool>* run() override;
};

class ConnectionManager {
  private:
  ConnectionStageBase* stage;
  ConnectionInformation local_connection_information;
  ConnectionInformation server_information;
  ConnectionInformation multicast_information;

  ConnectionStageIdentifier stage_identifier;

  StorageManager *global_storage;

  void set_stage_object(ConnectionStageIdentifier stage);
  void handle_returns(StageReturnBase *return_value);
  void handle_error(NetworkException exception);

  public:
  ConnectionManager(ConnectionInformation local_connection_information, ConnectionInformation multicast_information, StorageManager *global_storage);

  void set_stage_identifier(ConnectionStageIdentifier stage);
  ConnectionStageIdentifier get_stage_identifier();

  Response send_request(Request &request);

  void run();
};

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

#endif