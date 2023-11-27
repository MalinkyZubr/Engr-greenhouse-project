#include "wifi.hpp"


ConnectionManager::ConnectionManager(TaskManager task_manager, ConfigManager storage) : task_manager(task_manager), storage(storage) {
  this->state = INITIALIZING;
  this->run();
}

ConnectionManager::ConnectionManager(TaskManager task_manager, ConfigManager storage, WifiInfo wifi_information) : wifi_information(wifi_information), storage(storage), task_manager(task_manager) {
  this->state = BROADCAST;
  this->run();
}

ConnectionManager::ConnectionManager(TaskManager task_manager, ConfigManager storage, WifiInfo wifi_information, ConnectionInfo connection_information) : wifi_information(wifi_information), storage(storage), server_information(connection_information), task_manager(task_manager) {
  this->state = CONNECTED;
  this->run();
}

ParsedMessage ConnectionManager::rest_receive(WiFiClient client, ReceiveType type) {
  String message;
  String current_line = "";
  while(client.connected()) {
    if(client.available()) {
      char c = client.read();

      if(c == "\n") {
        if(current_line.length() == 0) {
          break;
        }
        else {
          message.concat(current_line + "\n");
          current_line = "";
        }
      }

      else if(c != "\r") {
        current_line.concat(c);
      }
    }
  }

  ParsedMessage received;
  switch(type) {
    case REQUEST:
      received.request = Requests::parse_request(message);
      break;
    case RESPONSE:
      received.response = Responses::parse_response(message);
      break;
  }
  return received;
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
      if((ssid + ssid_number).equals(WiFi.SSID(network))) {
        allowed = false;
        ssid_number++;
        break;
      }
    }
  }
  this->wifi_information.ssid = ssid + ssid_number;
  return true;
}

bool ConnectionManager::initialization() {
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
  WiFiClient client; 
  while(!client) {
    client = server.available();
  }

  ParsedMessage request = this->rest_receive(REQUEST);
  
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