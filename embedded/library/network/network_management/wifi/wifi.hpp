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
#include "ip_extended.hpp"


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
    String ip = "";
    int port = -1;
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
  ConnectionStageIdentifier get_stage_identifier() const;

  Response send_request(Request &request) const;

  const ConnectionInformation& get_server_information() const;

  void run();
};



#endif