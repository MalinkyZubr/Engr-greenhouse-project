#include "wifi.hpp"


ConnectionManager::ConnectionManager(TaskManager task_manager) : task_manager(task_manager) {
  this->state = INITIALIZING;
  this->run();
}

ConnectionManager::ConnectionManager(TaskManager task_manager, WifiInfo wifi_information) : wifi_information(wifi_information), task_manager(task_manager) {
  this->state = BROADCAST;
  this->run();
}

ConnectionManager::ConnectionManager(TaskManager task_manager, WifiInfo wifi_information, ConnectionInfo connection_information) : wifi_information(wifi_information), connection_information(connection_information), task_manager(task_manager) {
  this->state = CONNECTED;
  this->run();
}

bool ConnectionManager::set_ssid_config() {
  String ssid = "REMOTE_GREENHOUSE";
  int num_networks = WiFi.scanNetworks();
  int ssid_number = 0;
  bool allowed = false;
  if(num_networks == -1) {
    return false;
  }

  while(!allowed) {
    allowed = true;
    for(int network = 0; network < num_networks; network++) {
      if(ssid.concat(ssid_number).equals(WiFi.SSID(network))) {
        allowed = false;
        ssid_number++;
        break;
      }
    }
  }
  this->wifi_information.ssid = ssid.concat(ssid_number);
  return true;
}

bool  ConnectionManager::initialization() {
  this->state_connection.server = WiFiServer(LOCAL_PORT);
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    return false;
  }
  this->set_ssid_config();
  int status = WL_IDLE_STATUS;

  status = WiFi.beginAP(this->wifi_information.ssid);
  if (status != WL_AP_LISTENING) {
    return false;
  }
  delay(2000);
  this->state_connection.server.begin(); // start the web server
  while(true) {
    WiFiClient client = this->state_connection.server.available();

    if(client) {
      String data = "";
      while(client.connected()) {
        if(client.available()) {
          char c = client.read();
          client.write()
        }
      }
    }
  }
}

ConnectionInfo ConnectionManager::association() {

}

ConnectionInfo ConnectionManager::broadcast() {

}

void ConnectionManager::connected() {

}

bool ConnectionManager::send() {

}

DynamicJsonDocument ConnectionManager::receive() {

}

void ConnectionManager::run() {
  while(this->state != DOWN) {
    switch(this->state) {
      case INITIALIZING:
        this->initialization();
        break;
      case BROADCASTING:
        this->broadcast();
        break;
      case ASSOCIATING:
        break;
      case CONNECTED:
        break;
    }
  }
}