#ifndef WIFI_HPP
#define WIFI_HPP

#include <Arduino.h>
#include <WiFi101.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include "async.hpp"


#define RECEPTION_PID 6
#define LOCAL_PORT 2211


enum NetworkTypes {
    HOME,
    ENTERPRISE
};

typedef struct {
    NetworkTypes network_type;
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

union Connection {
    WiFiUDP udp;
    WiFiSSLClient standard;
    WiFiServer server(LOCAL_PORT);
};

class ConnectionManager {
    public:
    States state = INITIALIZING;
    WifiInfo wifi_information;
    ConnectionInfo own_information;
    ConnectionInfo server_information;
    TaskManager task_manager;
    Connection state_connection;

    ConnectionManager(TaskManager task_manager);
    ConnectionManager(TaskManager task_manager, WifiInfo wifi_information);
    ConnectionManager(TaskManager task_manager, WifiInfo wifi_information, ConnectionInfo server_information)
    bool set_ssid_config();
    bool initialization();
    bool configuration(); // soft ap mode operations
    ConnectionInfo association(); // to be run inside broadcast when receive confirmation
    ConnectionInfo broadcast();
    void connected();
    bool send();
    DynamicJsonDocument receive();
    void run();
};

#endif