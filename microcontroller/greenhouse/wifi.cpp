#include "wifi.hpp"


ConnectionManager::ConnectionManager(NetworkTypes type, TaskManager task_manager, ConfigManager storage) : type(type), task_manager(task_manager), storage(storage) {
  this->state = INITIALIZING;
  this->run();
}

ConnectionManager::ConnectionManager(NetworkTypes type, TaskManager task_manager, ConfigManager storage, WifiInfo wifi_information) : type(type), wifi_information(wifi_information), storage(storage), task_manager(task_manager) {
  this->state = BROADCASTING;
  this->run();
}

ConnectionManager::ConnectionManager(NetworkTypes type, TaskManager task_manager, ConfigManager storage, WifiInfo wifi_information, ConnectionInfo connection_information) : type(type), wifi_information(wifi_information), storage(storage), server_information(connection_information), task_manager(task_manager) {
  this->state = CONNECTED;
  this->run();
}

ParsedMessage ConnectionManager::rest_receive(WiFiClient &client) {
  String message;
  String current_line = "";
  while(client.connected()) {
    if(client.available()) {
      char c = client.read();

      if(c == *"\n") {
        if(current_line.length() == 0) {
          break;
        }
        else {
          message.concat(current_line + "\n");
          current_line = "";
        }
      }

      else if(c != *"\r") {
        current_line.concat(c);
      }
    }
  }

  ParsedMessage received;
  if(message.startsWith("HTTP")) {
    received.response = Responses::parse_response(message);
    received.type = RESPONSE;
  }
  else {
    received.request = Requests::parse_request(message);
    received.type = REQUEST;
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

int ConnectionManager::get_ap_channel(String &ssid) {
  int num_networks = WiFi.scanNetworks();

  for(int network = 0; network < num_networks; network++) {
    if(ssid.equals(WiFi.SSID(network))) {
      return WiFi.channel(network);
    }
  }
  return -1;
}

WifiInfo ConnectionManager::receive_credentials(WiFiClient &client) {
  bool credentials_received = false;
  WifiInfo temporary_wifi_information;
  while(!credentials_received) {
    ParsedMessage message = this->rest_receive(client);
    String response;
    switch(message.type) {
      case(REQUEST):
        ParsedRequest& request = message.request;
        String& route = request.route;
        if(route.equals("/submit")) { // there should probably be some validation here!
          temporary_wifi_information.ssid = request.body["ssid"].as<String>();
          temporary_wifi_information.password = request.body["password"].as<String>();
          if(strcmp(request.body["type"], "enterprise")) {
            temporary_wifi_information.username = request.body["username"].as<String>();
            temporary_wifi_information.type = ENTERPRISE;
          }
          temporary_wifi_information.type = HOME;
          int channel = this->get_ap_channel(temporary_wifi_information.ssid);
          if(channel == -1) {
            response = Responses::response(503); // if the network isnt found, return 503 error
          }
          else {
            temporary_wifi_information.channel = channel;
            response = Responses::response(200); // otherwise tell the client everything worked
            credentials_received = true;
          }
        }
        else if(route.equals("/")) {
          response = Responses::file_response(webpage_html, HTML);
        }
        else if(route.equals("/styles.css")) {
          response = Responses::file_response(webpage_css, CSS);
        }
        else if(route.equals("/app.js")) {
          response = Responses::file_response(webpage_js, JS);
        }
        else {
          response = Responses::response(404);
        }
        break;
      case(RESPONSE):
        break;
      client.println(response);
    }
  }
  return temporary_wifi_information;
}

bool ConnectionManager::enterprise_connect(WifiInfo &info) {
  int ssid_size = info.ssid.length() * sizeof(char);

  void* ssid_temp = malloc(ssid_size);
  char* ssid_converted = static_cast<char*>(ssid_temp);

  info.ssid.toCharArray(ssid_converted, ssid_size);

  tstr1xAuthCredentials auth;
  strcpy((char *) auth.au8UserName, info.username.c_str());
  strcpy((char *) auth.au8Passwd, info.password.c_str());

  bool wifi_status = m2m_wifi_connect_sc(ssid_converted, ssid_size, M2M_WIFI_SEC_802_1X, &auth, info.channel, true);

  free(ssid_temp);
  free(ssid_converted);

  return wifi_status;
}

bool ConnectionManager::home_connect(WifiInfo &info) {
  bool status = WiFi.begin(info.ssid, info.password);
  return status;
}

bool ConnectionManager::initialization() {
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    return false;
  }
  this->set_ssid_config();
  int status = WL_IDLE_STATUS;

  int size = this->wifi_information.ssid.length() * sizeof(char);

  void* temp = malloc(size);
  char* converted = static_cast<char*>(temp);

  this->wifi_information.ssid.toCharArray(converted, size);

  status = WiFi.beginAP(converted);
  if (status != WL_AP_LISTENING) {
    return false;
  }
  delay(2000);

  bool success = false;
  while(!success) {
    WiFiClient client; 
    this->state_connection.Startupserver.begin(); // start the web server
    while(!client) {
      client = this->state_connection.Startupserver.available();
    }

    WifiInfo temporary_wifi_information = this->receive_credentials(client);
    WiFi.end();

    switch(temporary_wifi_information.type) {
      case ENTERPRISE:
        success = this->enterprise_connect(temporary_wifi_information);
        break;
      case HOME:
        success = this->home_connect(temporary_wifi_information);
        break;
    }
  }
  free(temp);
  free(converted);
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