#include "wifi.hpp"


WiFiWatchdog::WiFiWatchdog(CommonData *common_data, ConnectionManager *connected_manager) : common_data(common_data), connected_manager(connected_manager) {}

void WiFiWatchdog::check_wifi_status() {
  int Wifi_status = WiFi.status();
  bool connected;

  // check if the wifi is disconnected, first and foremost
  if((Wifi_status == WL_CONNECTION_LOST || Wifi_status == WL_DISCONNECTED) && (this->connected_manager->state != DOWN && this->connected_manager->state != INITIALIZING)) {
    connected = this->handle_wifi_down();
  }
}

bool WiFiWatchdog::handle_wifi_down() {
  bool wifi_connected = this->connected_manager->connect_wifi(this->connected_manager->wifi_information);
  if(!wifi_connected) {
    this->common_data->connected = false;
    this->connected_manager->state = DOWN;
    this->wifi_fail_counter++;
  }
  else {
    return true;
  }
  if(this->wifi_fail_counter == 10) {
    this->wifi_fail_counter = 0;
    this->connected_manager->state = INITIALIZING;
  }
  return false;
}

void WiFiWatchdog::callback() {
  this->check_wifi_status();
}

ConnectionManager::ConnectionManager(TaskManager *task_manager, Router *routes, ConfigManager *storage, MessageQueue *message_queue) : task_manager(task_manager), routes(routes), storage(storage), message_queue(message_queue) {
  this->state = INITIALIZING;
  this->run();
}

ConnectionManager::ConnectionManager(TaskManager *task_manager, Router *routes, ConfigManager *storage, MessageQueue *message_queue, WifiInfo wifi_information) : wifi_information(wifi_information), routes(routes), storage(storage), message_queue(message_queue), task_manager(task_manager) {
  this->state = BROADCASTING;
  this->run();
}

ConnectionManager::ConnectionManager(TaskManager *task_manager, Router *routes, ConfigManager *storage, MessageQueue *message_queue, WifiInfo wifi_information, ConnectionInfo connection_information) : wifi_information(wifi_information), storage(storage), message_queue(message_queue), server_information(connection_information), task_manager(task_manager) {
  this->state = CONNECTED;
  this->run();
}

ParsedMessage ConnectionManager::rest_receive(WiFiClient &client, int timeout=30000) {
  String message;
  String current_line = "";
  ParsedMessage received;

  if(!client.connected()) {
    received.error = CONNECTION_FAILURE;
    return received;
  }

  long start_time = millis();
  while(client.connected() && ((millis() - start_time) < timeout)) {
    if(this->state == DOWN) {
      received.error = WIFI_FAILURE;
      return received;
    }
    if(client.available()) {
      char c = client.read();

      start_time = millis();
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
  if(millis() - start_time > timeout) {
    received.error = TIMEOUT;
    return received;
  }

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

  while(!credentials_received && this->state != DOWN) {
    ParsedMessage message = this->rest_receive(client, 10000);

    if(message.type == CONNECTION_FAILURE) {
      temporary_wifi_information.error = CONNECTION_FAILURE;
      break;
    }
    else if(message.error == WIFI_FAILURE) {
      temporary_wifi_information.error = WIFI_FAILURE;
      break;
    }

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

  if(!credentials_received) {
    temporary_wifi_information.error = WIFI_FAILURE;
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

  return wifi_status == M2M_SUCCESS;
}

bool ConnectionManager::home_connect(WifiInfo &info) {
  bool status = WiFi.begin(info.ssid, info.password);
  return status == WL_CONNECTED;
}

bool ConnectionManager::connect_wifi(WifiInfo &info) {
  bool result;
  switch(info.type) {
    case ENTERPRISE:
      result = this->enterprise_connect(info);
      break;
    case HOME:
      result = this->home_connect(info);
      break;
    case OPEN:
      break;
  }
  return result;
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

  status = WiFi.beginAP(converted, AP_CONFIG_PASSWORD);
  if (status != WL_AP_LISTENING) {
    return false;
  }
  delay(2000);

  bool success = false;
  WiFiClient client; 
  WifiInfo temporary_wifi_information;

  this->state_connection.Startupserver.begin(); // start the web server
  
  while(!success && this->state != DOWN) {
    while(!client) {
      client = this->state_connection.Startupserver.available();
    }

    temporary_wifi_information = this->receive_credentials(client);

    switch(temporary_wifi_information.error) {
      case CONNECTION_FAILURE:
      case WIFI_FAILURE:
        client.stop();
        continue;
    }

    client.stop();
    WiFi.end();

    success = this->connect_wifi(temporary_wifi_information);
  }

  free(temp);
  free(converted);

  if(!success) {
    return false;
  }

  this->wifi_information = temporary_wifi_information;
  this->state = BROADCASTING;

  return true;
}

ReturnErrors ConnectionManager::send_broadcast(String &json_data, IPAddress &address, char *receive_buffer, int buff_size, DynamicJsonDocument &receive_json, int timeout = 10000) {
  this->state_connection.UDPserver.beginPacket(address, EXTERNAL_PORT);
  this->state_connection.UDPserver.write(json_data.c_str());
  this->state_connection.UDPserver.endPacket();

  ReturnErrors return_code;

  long start = millis();
  while(!this->state_connection.UDPserver.parsePacket() && this->state != DOWN && (millis() - start < timeout)) {}
  if(this->state == DOWN) {
    return_code = WIFI_FAILURE;
  }
  else if(timeout < millis() - start) {
    return_code = TIMEOUT;
  }
  else {
    this->state_connection.UDPserver.read(receive_buffer, buff_size);
    deserializeJson(receive_json, receive_buffer);
    String server_ip = receive_json["server_ip"];
    this->server_information.ip = server_ip;
    return_code = OKAY;
  }
  return return_code;
}

bool ConnectionManager::broadcast() {
  this->state_connection.UDPserver.begin(LOCAL_PORT);

  DynamicJsonDocument doc(sizeof(this->own_information) + 20);
  doc["ip"] = this->own_information.ip;
  doc["mac"] = this->own_information.mac;
  doc["name"] = this->storage->config.device_name;

  DynamicJsonDocument received(32);

  String json_data;

  IPAddress external_address_object;
  external_address_object.fromString(UDPMulticastAddress);

  serializeJson(doc, json_data);

  char receive_buffer[UDPReceiveBuffSize];

  bool discovered = false;
  ReturnErrors return_value;

  while(!discovered && this->state != DOWN) {
    return_value = this->send_broadcast(json_data, external_address_object, receive_buffer, UDPReceiveBuffSize, received);

    switch(return_value) {
      case OKAY:
        discovered = true;
        break;
      case TIMEOUT:
        continue;
      case WIFI_FAILURE:
        break;
    }
    delay(10000);
  }

  if(!discovered) {
    return false;
  }

  this->state_connection.UDPserver.stop();
  this->state = ASSOCIATING;
  return true;
}

bool ConnectionManager::connect_to_server() {
  for(int x = 0; x < 5; x++) {
    if(this->state_connection.SSLclient.connect(this->server_information.ip.c_str(), EXTERNAL_PORT)) {
      return true;
    }
    delay(5000);
  }
  return false;
}

const char* ConnectionManager::prepare_identifier_field(String &field_value) {
  if(field_value.equals("")) {
    return nullptr;
  }
  return field_value.c_str();
}

int* ConnectionManager::prepare_identifier_field(int &field_value) {
  if(!field_value) {
    return nullptr;
  }
  return &field_value;
}

void ConnectionManager::package_identifying_info(DynamicJsonDocument &to_package) {
  to_package["device_name"] = this->prepare_identifier_field(this->storage->config.device_name);
  to_package["device_id"] = *this->prepare_identifier_field(this->storage->config.device_id);
  to_package["device_ip"] = this->own_information.ip;
  to_package["device_mac"] = this->own_information.mac;
  to_package["preset_name"] = this->prepare_identifier_field(this->storage->config.preset.PresetName);
  to_package["project_name"] = this->prepare_identifier_field(this->storage->config.project_name);
  to_package["device_status"] = true;
}

void ConnectionManager::write_identifying_info(ParsedResponse &response) {
  
}

bool ConnectionManager::association() { // make first ssl request to associate with server
  bool associated = false;
  int fail_counter = 0;
  
  while(!associated && fail_counter < 3) {
    if(!this->state_connection.SSLclient.connected() && !this->connect_to_server()) {
      this->state = BROADCASTING; // if server connection fails, go back to broadcasting
      return false;
    }

    DynamicJsonDocument doc(CONFIG_JSON_SIZE);
    this->package_identifying_info(doc);

    String data = Requests::request(POST, String("/devices/confirm"), this->server_information.ip, doc);
    this->state_connection.SSLclient.println(data);

    ParsedMessage message = this->rest_receive(this->state_connection.SSLclient, 10000);

    switch(message.error) {
      case WIFI_FAILURE:
        return false;
        break;
      case CONNECTION_FAILURE:
      case TIMEOUT:
        fail_counter++;
        continue;
      case OKAY:
        // REMEMBER TO WRITE TO CONFIG HERE
        
        associated = true;
    }
  }

  if(!associated) {
    return false;
  }

  this->state = CONNECTED;

  return true;
}

ReturnErrors ConnectionManager::handle_requests() {

}

bool ConnectionManager::connected() {

}

bool ConnectionManager::send() {

}

void ConnectionManager::run() { // goes in void_loop
  switch(this->state) {
    case INITIALIZING:
      this->initialization();
      break;
    case BROADCASTING:
      this->broadcast();
      break;
    case ASSOCIATING:
      this->association();
      break;
    case CONNECTED:
      break;
    case DOWN:
      break;
  }
}