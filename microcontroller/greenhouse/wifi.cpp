#include "wifi.hpp"


WiFiWatchdog::WiFiWatchdog(CommonData *common_data, ConnectionManager *connected_manager, MachineState *machine_state) : common_data(common_data), connected_manager(connected_manager), machine_state(machine_state) {}

void WiFiWatchdog::check_wifi_status() {
  int Wifi_status = WiFi.status();

  // check if the wifi is disconnected, first and foremost
  if((Wifi_status == WL_CONNECTION_LOST || Wifi_status == WL_DISCONNECTED) && (this->connected_manager->network_state != DOWN && this->connected_manager->network_state != INITIALIZING)) {
    this->machine_state->connection_state = MACHINE_DISCONNECTED;
    this->connected_manager->network_state = DOWN;
  }
}

void WiFiWatchdog::callback() {
  this->check_wifi_status();
}

ConnectionManager::ConnectionManager(TaskManager *task_manager, Router *routes, ConfigManager *storage, MachineState *machine_state) : task_manager(task_manager), router(routes), storage(storage), machine_state(machine_state) {
  this->network_state = INITIALIZING;
  this->run();
}

ConnectionManager::ConnectionManager(TaskManager *task_manager, Router *routes, ConfigManager *storage, MachineState *machine_state, WifiInfo wifi_information) : wifi_information(wifi_information), router(routes), storage(storage), machine_state(machine_state), task_manager(task_manager) {
  this->network_state = STARTUP;
  this->run();
}

ConnectionManager::ConnectionManager(TaskManager *task_manager, Router *routes, ConfigManager *storage, MachineState *machine_state, WifiInfo wifi_information, ConnectionInfo connection_information) : wifi_information(wifi_information), storage(storage), machine_state(machine_state), server_information(connection_information), task_manager(task_manager) {
  if(this->storage->config.identifying_information.device_id == -1) {
    this->network_state = STARTUP;
  }
  else {
    this->network_state = CONNECTED;
  }
  this->run();
}

bool ConnectionManager::check_ssid_existence(String &ssid) {
    int num_networks = WiFi.scanNetworks();

    for(int network = 0; network < num_networks; network++) {
      if(strcmp(WiFi.SSID(network), ssid.c_str())) {
        return true;
      }
      return false;
    }
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
    if(this->network_state == DOWN) {
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

  while(!credentials_received && this->network_state != DOWN) {
    ParsedMessage message = this->rest_receive(client, 10000);

    if(message.error == CONNECTION_FAILURE || message.error == WIFI_FAILURE) {
      temporary_wifi_information.error = message.error;
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

NetworkReturnErrors ConnectionManager::connect_wifi(WifiInfo &info) {
  bool result;

  if(!this->check_ssid_existence(info.ssid)) {
    return NOT_FOUND;
  }
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
  if(!result) {
    return AUTHENTICATION_FAILURE;
  }
  return OKAY;
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

  NetworkReturnErrors success;
  success = ERROR;

  WiFiClient client; 
  WifiInfo temporary_wifi_information;

  this->state_connection.Startupserver.begin(); // start the web server
  
  while(success != OKAY && this->network_state != DOWN) {
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

  if(success != OKAY) {
    return false;
  }

  this->wifi_information = temporary_wifi_information;
  this->network_state = BROADCASTING;

  return true;
}

NetworkReturnErrors ConnectionManager::send_broadcast(String &json_data, IPAddress &address, char *receive_buffer, int buff_size, DynamicJsonDocument &receive_json, int timeout = 10000) {
  this->state_connection.UDPserver.beginPacket(address, EXTERNAL_PORT);
  this->state_connection.UDPserver.write(json_data.c_str());
  this->state_connection.UDPserver.endPacket();

  NetworkReturnErrors return_code;

  long start = millis();
  while(!this->state_connection.UDPserver.parsePacket() && this->network_state != DOWN && (millis() - start < timeout)) {}
  if(this->network_state == DOWN) {
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

bool ConnectionManager::broadcast(bool expidited) {
  this->state_connection.UDPserver.begin(LOCAL_PORT);

  DynamicJsonDocument doc(sizeof(this->own_information) + 20);
  doc["ip"] = this->own_information.ip;
  doc["mac"] = this->own_information.mac;
  doc["name"] = this->storage->config.identifying_information.device_name;
  doc["id"] = this->storage->config.identifying_information.device_id;
  doc["expidited"] = expidited;

  DynamicJsonDocument received(32);

  String json_data;

  IPAddress external_address_object;
  external_address_object.fromString(UDPMulticastAddress);

  serializeJson(doc, json_data);

  char receive_buffer[UDPReceiveBuffSize];

  bool discovered = false;
  NetworkReturnErrors return_value;

  while(!discovered && this->network_state != DOWN) {
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
  this->network_state = ASSOCIATING;
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

bool ConnectionManager::association() { // make first ssl request to associate with server
  bool associated = false;
  int fail_counter = 0;

  Identifiers device_identifiers;
  Preset device_preset;
  
  while(!associated && fail_counter < 3) {
    if(!this->state_connection.SSLclient.connected() && !this->connect_to_server()) {
      this->network_state = BROADCASTING; // if server connection fails, go back to broadcasting
      return false;
    }

    DynamicJsonDocument doc(CONFIG_JSON_SIZE);
    DynamicJsonDocument preset(CONFIG_JSON_SIZE);

    this->storage->retrieve_config_to_json(this->storage->identifier_address, doc);
    this->storage->retrieve_config_to_json(this->storage->preset_address, preset);

    doc["preset"] = preset;

    String data = Requests::request(POST, String("/devices/confirm"), this->server_information.ip, this->storage->config.identifying_information.device_id, doc); // the device_id should default to 0 before reading from flash
    this->state_connection.SSLclient.println(data); // THERE MUST BE A WAY TO PACKAGE THE PRESET NAME IN HERE TOO

    ParsedMessage message = this->rest_receive(this->state_connection.SSLclient, 10000); // the device should receive a response here, make sure the server actually does that

    this->state_connection.SSLclient.stop(); // remember to close the connection after each request

    
    switch(message.error) {
      case WIFI_FAILURE:
        return false;
        break;
      case CONNECTION_FAILURE:
      case TIMEOUT:
        fail_counter++;
        continue;
      case OKAY:
        this->storage->deserialize_device_identifiers(device_identifiers, message.response.body);
        this->storage->set_device_identifiers(device_identifiers);

        preset = message.response.body["preset"];

        this->storage->deserialize_preset(device_preset, preset);
        this->storage->set_preset(device_preset);
        
        associated = true;
    }
    if(!this->storage->writer->reference_datatime) {
      this->storage->set_reference_datetime(message.response.body["reference_time"]);
    }
  }

  if(!associated) {
    return false;
  }

  this->network_state = CONNECTED;
  this->machine_state->connection_state = MACHINE_CONNECTED;

  return true;
}

NetworkReturnErrors ConnectionManager::listener() {
  this->state_connection.SSLlistener.beginSSL();
  WiFiClient client;
  ParsedMessage message;
  String response;
  NetworkReturnErrors error;

  int connection_failure_counter = 0;
  bool fail = false;

  while(this->network_state == CONNECTED && connection_failure_counter < 10) {
    client = this->state_connection.SSLlistener.available();
    message = this->rest_receive(client, 3000);

    error = message.error;
    switch(error) {
      case CONNECTION_FAILURE: // connection failures are bad, if 10 happen in a row that means the server probably died
        connection_failure_counter++;
        if(connection_failure_counter > 10) {
          fail = true;
        }
        break;
      case TIMEOUT: // timeouts are totally fine and can just be ignored :)
        continue;
      case WIFI_FAILURE:
        fail = true;
        break;
      default:
        connection_failure_counter = 0;
    }
    if(fail) {
      break;
    }
    // assuming the message is in fact a request, deal with deciding that later
    NetworkReturnErrors result = this->router->execute_route(message.request, &response);

    client.println(response);

    client.stop();
  }

  return error;
}

void ConnectionManager::listener_error_handler(NetworkReturnErrors error) {
  switch(error) {
    case WIFI_FAILURE:
      this->network_state = DOWN;
      break;
    case CONNECTION_FAILURE:
      this->network_state = BROADCASTING;
  }
}

ParsedResponse ConnectionManager::connected_send(String &request) {
  this->connect_to_server();

  this->state_connection.SSLclient.println(request);

  ParsedResponse response = this->rest_receive(this->state_connection.SSLclient, 10000).response;

  this->state_connection.SSLclient.stop();

  return response;
}

void ConnectionManager::run() { // goes in void_loop
  bool status;
  switch(this->network_state) {
    case INITIALIZING:
      status = this->initialization();
      break;
    case BROADCASTING:
      if(this->storage->configured) { // the server device_initializer object will have an expidited field, that can tell the association setup thing if the setup should be expidited (no async event)
        status = this->broadcast(true);
      }
      else {
        status = this->broadcast(false);
      }
      break;
    case ASSOCIATING:
      status = this->association();
      break;
    case CONNECTED:
      this->listener();
      break;
    case STARTUP:
    case DOWN:
      switch(this->connect_wifi(this->wifi_information)) {
        case NOT_FOUND:
          break;
        case AUTHENTICATION_FAILURE:
          this->network_state = INITIALIZING; // if credentials are no longer valid, reset the wifi data
          break;
        case OKAY:
          this->network_state = BROADCASTING;
          break;
      }
      delay(5000);
      break;
  }
}