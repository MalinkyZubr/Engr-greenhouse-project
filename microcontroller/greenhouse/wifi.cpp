#include "wifi.hpp"


///////////////////////////////////////////////////
///////// WifiWatchdog ////////////////////////////
///////////////////////////////////////////////////

/// @brief the wifi watchdog watches to see if the connection to the wireless AP is stable. if connection is lost, it sets the network state to down. This is meant to be a Task manager task @see TaskManager
/// @param connected_manager the connection manager for which the watchdog is checking wifi status
/// @param machine_state machine state with which the watchdog tracks and sets machine connection state on wifi failure
WiFiWatchdog::WiFiWatchdog(ConnectionManager *connected_manager, MachineState *machine_state) : connected_manager(connected_manager), machine_state(machine_state) {}


/// @brief checks the wifi status of the connection manager. if the connection to AP is down, machine network state is set to MACHINE_DISCONNETED and the network state is set to down
void WiFiWatchdog::check_wifi_status() {
  int Wifi_status = WiFi.status();

  // check if the wifi is disconnected, first and foremost
  if((Wifi_status == WL_CONNECTION_LOST || Wifi_status == WL_DISCONNECTED) && (this->connected_manager->network_state != DOWN && this->connected_manager->network_state != INITIALIZING)) {
    this->machine_state->connection_state = MACHINE_DISCONNECTED;
    this->connected_manager->network_state = DOWN;
  }
}


/// @brief callback for the wifi watchdog to be run by the task manager
void WiFiWatchdog::callback() {
  this->check_wifi_status();
}

///////////////////////////////////////////////////
///////// TCPClient ///////////////////////////////
///////////////////////////////////////////////////

TCPClient::TCPClient(Layer4Encryption layer_4_encryption = NONE) : layer_4_encryption_type(layer_4_encryption) {}

Layer4Encryption TCPClient::get_encryption() {
  return this->layer_4_encryption_type;
}

NetworkExceptions TCPClient::client_receive_message(WiFiClient &client, int timeout, String *message) {
  String current_line = "";

  long long start_time = millis();

  while(client.connected() && ((millis() - start_time) < timeout)) {
    if(client.available()) {
      char c = client.read();

      start_time = millis();
      if(c == "\n" && current_line.equals("")) {
        if(current_line.length() == 0) {
          break;
        }
        else {
          message->concat(current_line + "\n");
          current_line = "";
        }
      }
      else if(c != *"\r") {
        current_line.concat(c);
      }
    }
  }
  if(!client.connected()) {
    return NETWORK_SERVER_CONNECTION_FAILURE;
  }
  else if(((millis() - start_time) > timeout)) {
    return NETWORK_TIMEOUT;
  }
  return NETWORK_OKAY
}

NetworkExceptions TCPClient::client_send_message(WiFiClient &client, HTTPMessage *message) {
  NetworkExceptions return_error = NETWORK_OKAY;
  String serialized_message = message->serialize();

  bool status = client.println(serailized_message);
  if(!status) {
    return_error = NETWORK_SERVER_CONNECTION_FAILURE;
  }

  delete message;
  return return_error;
}

HTTPMessage* TCPClient::parse_message(String &unparsed_message) {
  HTTPMessage *received;

  if(message.startsWith("HTTP")) {
    received = new Response(unparsed_message);
  }
  else {
    received = new Request(unparsed_message);
  }

  return received;
}

HTTPMessage* TCPClient::receive_and_parse(WiFiClient &client, int timeout=5000) {
  String unparsed;
  HTTPMessage *received;

  NetworkExceptions receive_error = this->receive_message(timeout, &unparsed);
  if(receive_error == NETWORK_OKAY) {
    received = this->parse_message(message);
  }
  else {
    received->set_exception(receive_error);
  }

  return message;
}

///////////////////////////////////////////////////
///////// TCPRequestClient ////////////////////////
///////////////////////////////////////////////////

TCPRequestClient::TCPRequestClient(WiFiClient &client, Layer4Encryption encryption, ServerInformation server_information) : TCPClient(encryption), client(client), server_information(server_information) {}

Response TCPRequestClient::request(Request *message) {
  Response *response_pointer;
  Response response;

  switch(this->get_encryption()) {
    case SSL:
      this->client.connectSSL(this->server_information.ip, this->server_information.port);
      break;
    case NONE:
      this->client.connect(this->server_information.ip, this->server_information.port);
      break;
  }

  NetworkExceptions send_exception = this->client_send_message(this->client, message);
  response_pointer = this->receive_and_parse(this->client, 10000);

  this->client.stop();

  response = Response(response_pointer);
  return response;
}

///////////////////////////////////////////////////
///////// TCPListenerClient ///////////////////////
///////////////////////////////////////////////////

template <typename R>
TCPListenerClient<R>::TCPListenerClient(WiFiServer listener, Layer4Encryption encryption, Router router) : listener(listener), router(router), TCPClient(encryption) {
  switch(encryption) {
    case NONE:
      this->listener.begin();
      break;
    case SSL:
      this->listener.beginSSL();
      break;
  }
}

template <typename R>
R TCPListenerClient<R>::listen() {
  Request *received_ptr;
  Request received;
  WiFiClient client = this->listener.available();

  received_ptr = this->receive_and_parse(client, 5000);
  received = Request(received_ptr);

  this->router
}

///////////////////////////////////////////////////
///////// ConnectionStage /////////////////////////
///////////////////////////////////////////////////

template <typename L, typename C>
L ConnectionStage<L, C>::get_listener() {
  return this->listener;
}

template <typename L, typename C>
NetworkExceptionHandler *get_exception_handler() {
  return this->handler;
}

template <typename L, typename C>
ParsedMessage ConnectionStage<L, C>::receive(C &client) {
  TCPRequestClient tcp_client(client);
  ParsedMessage message = tcp_client.receive(5000);

  return message;
}

///////////////////////////////////////////////////
///////// WiFiInitialization //////////////////////
///////////////////////////////////////////////////

WifiInitialization::WiFiInitialization(int listen_port) : ConnectionStage(listen_port) {}

String WifiInitialization::ssid_config() {
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
  
  ssid.concat(ssid_number);
  return ssid;
}

/// @brief get the channel of a specified wireless AP in preparation to connect
/// @param ssid the SSID of the AP in question
/// @return integer representation of the AP channel. if -1, the SSID is not associated with any active AP
int WifiInitialization::get_ap_channel(String &ssid) {
  int num_networks = WiFi.scanNetworks();

  for(int network = 0; network < num_networks; network++) {
    if(ssid.equals(WiFi.SSID(network))) {
      return WiFi.channel(network);
    }
  }
  return -1;
}

WifiInfo WifiInitialization::handle_credendtials() {
  temporary_wifi_information.ssid = request.body["ssid"].as<String>();
  temporary_wifi_information.password = request.body["password"].as<String>();
  if(strcmp(request.body["type"], "enterprise")) {
    temporary_wifi_information.username = request.body["username"].as<String>();
    temporary_wifi_information.type = ENTERPRISE;
  }
  temporary_wifi_information.type = HOME;
  int channel = this->get_ap_channel(temporary_wifi_information.ssid);
  if(channel == -1) {
    response = Responses::response(503, false); // if the network isnt found, return 503 error
  }
  else {
    temporary_wifi_information.channel = channel;
    response = Responses::response(200, false); // otherwise tell the client everything worked
    credentials_received = true;
  }
}

String WifiInitialization::return_webpage_response(String &route) {
  Response response;

  if(route.equals("/submit")) { // there should probably be some validation here!
    
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
    response = Responses::response(404, false);
  }

  return response;
}

/// @brief function used during wifi provisioning to receive credentials for AP association from served webpage. Serves webpage and submits respones to client
/// @param client wifi client to use for communications with the requesting device
/// @return WifiInfo object to return and apply to the flash memory and to the server
/// @todo upgrade this to an SSL server connection
WifiInfo WifiInitialization::receive_credentials(WiFiClient &client) {
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
            response = Responses::response(503, false); // if the network isnt found, return 503 error
          }
          else {
            temporary_wifi_information.channel = channel;
            response = Responses::response(200, false); // otherwise tell the client everything worked
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
          response = Responses::response(404, false);
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


///////////////////////////////////////////////////
///////// WiFiInitialization //////////////////////
///////////////////////////////////////////////////


/// @brief instantiate a connection manager with minimal information. This is designed to be used on first startup after a reset
/// @param task_manager device task manager for the management of wifi based tasks in tandem with network operations
/// @param routes router object for handling http requests to the device from the server
/// @param storage storage manager for interfacing with flash memory
/// @param machine_state global machine state for tracking and managing device behaviors
ConnectionManager::ConnectionManager(TaskManager *task_manager, Router *routes, ConfigManager *storage, MachineState *machine_state) : task_manager(task_manager), router(routes), storage(storage), machine_state(machine_state) {
  this->network_state = INITIALIZING;
  this->run();
}


/// @brief instantiate a connection manager with wifi information pre-specified. This is for when the device has already connected to the network before
/// @param task_manager device task manager for the management of wifi based tasks in tandem with network operations
/// @param routes router object for handling http requests to the device from the server
/// @param storage storage manager for interfacing with flash memory
/// @param machine_state global machine state for tracking and managing device behaviors
/// @param wifi_information wifi information to preconfigure device authentication infromation, this should be retrieved from flash memory most likely
ConnectionManager::ConnectionManager(TaskManager *task_manager, Router *routes, ConfigManager *storage, MachineState *machine_state, WifiInfo wifi_information) : wifi_information(wifi_information), router(routes), storage(storage), machine_state(machine_state), task_manager(task_manager) {
  this->network_state = STARTUP;
  this->run();
}


/// @brief instantiate a connection manager with wifi information and server information pre specified. Designed for use if the device has already associated with a server in the past
/// @param task_manager device task manager for the management of wifi based tasks in tandem with network operations
/// @param routes router object for handling http requests to the device from the server
/// @param storage storage manager for interfacing with flash memory
/// @param machine_state global machine state for tracking and managing device behaviors
/// @param wifi_information wifi information to preconfigure device authentication infromation, this should be retrieved from flash memory most likely
/// @param connection_info struct identifying server information for the connection
ConnectionManager::ConnectionManager(TaskManager *task_manager, Router *routes, ConfigManager *storage, MachineState *machine_state, WifiInfo wifi_information, ConnectionInfo connection_information) : wifi_information(wifi_information), storage(storage), machine_state(machine_state), server_information(connection_information), task_manager(task_manager) {
  if(this->storage->config.identifying_information.device_id == -1) {
    this->network_state = STARTUP;
  }
  else {
    this->network_state = CONNECTED;
  }
  this->run();
}


/// @brief check if the specified ssid is in range of wifi card
/// @param ssid SSID of the desired network
/// @return bool to signify if the network exists or not
bool ConnectionManager::check_ssid_existence(String &ssid) {
  int num_networks = WiFi.scanNetworks();

  for(int network = 0; network < num_networks; network++) {
    if(strcmp(WiFi.SSID(network), ssid.c_str())) {
      return true;
    }
    return false;
  }
}


/// @brief connect to an enterprise authenticated AP with username and password
/// @param info WifiInfo reference to wifi info object containing configuration information for AP connection
/// @return bool to say if the connection succeeded or not
bool ConnectionManager::enterprise_connect(WifiInfo &info) {
  int ssid_size = info.ssid.length() * sizeof(char);

  void* ssid_temp = malloc(ssid_size);
  char* ssid_converted = static_cast<char*>(ssid_temp);

  info.ssid.toCharArray(ssid_converted, ssid_size);

  tstr1xAuthCredentials auth;
  strcpy((char *) auth.au8UserName, info.username.c_str());
  strcpy((char *) auth.au8Passwd, info.password.c_str());

  bool wifi_status = m2m_wifi_connect_sc(ssid_converted, ssid_size, M2M_WIFI_SEC_802_1X, &auth, info.channel, false);

  free(ssid_temp);
  free(ssid_converted);

  return wifi_status == M2M_SUCCESS;
}


/// @brief simple AP connection function that connects to home wifi AP with just a password
/// @param info wifi information containing credentials necessary for connection
/// @return bool to say if connection succeeded or not
bool ConnectionManager::home_connect(WifiInfo &info) {
  bool status = WiFi.begin(info.ssid, info.password);
  return status == WL_CONNECTED;
}


/// @brief function for switching between different connection functions based on the type enum specified in the wifi info
/// @param info wifi information necessary for connection to an AP, contains the network type
/// @return NetworkReturnErrors enum containing error or okay message after execution
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


/// @brief comprehensive function to enter wifi card AP mode (serve a wifi network), wait for a connection from a client, and serve the provisioning webpage. 
/// After, the AP deactivates, and the device attempts to connect to the specified AP ssid with the provided credentials. If the authentication fails, restart AP mode and try again
/// @return bool to say if the initialization succeeded
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


/// @brief send a multicast UDP packet to the specified server multicast address. Sends packet, and waits until a timeout for a response. 
/// @param json_data json schema to send to the server
/// @param address multicast address to send the UDP packet to 
/// @param receive_buffer char buffer for receiving responses from the server
/// @param buff_size size of the char buffer
/// @param receive_json json object to deserialize received responses into
/// @param timeout timeout interval in milliseconds. Defaults to 10000
/// @return NetworkReturnErrors, contains error code to be handled by calling function
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


/// @brief function to govern broadcasting operations. This makes the server aware of the existience of this device, so it can be placed in the registration queue, associate, and begin normal operations.
/// This is basically a layer 3 version of LLDP protocol https://en.wikipedia.org/wiki/Link_Layer_Discovery_Protocol
/// @param expidited this flag is set when the device has previously connected to the server (eg, if the device has a device ID stored in memory). Setting this flag will allow the device to skip manual registration by the user, and be put directly into full operation with the server.
/// @return bool to say if broadcasting succeeded
/// @todo is this function actually configuring server information on the device side?
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


/// @brief function designed to connect to server using the object's ssl client. Makes 5 attempts at 5 second intervals to connect
/// @return if the function fails to connect after 5 attempts, return false
bool ConnectionManager::connect_to_server() {
  for(int x = 0; x < 5; x++) {
    if(this->state_connection.SSLclient.connect(this->server_information.ip.c_str(), EXTERNAL_PORT)) {
      return true;
    }
    delay(5000);
  }
  return false;
}


/// @brief after the server acknowledges udp broadcast, see broadcast method, the device must make the first ssl connection request to the server, so that it can sync its configuration with the server database and enter full operation
/// @return bool to say if the association succeeded
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

    String data = Requests::request(POST, String("/devices/confirm"), this->server_information.ip, this->storage->config.identifying_information.device_id, this->machine_state->operational_state, doc); // the device_id should default to 0 before reading from flash
    this->state_connection.SSLclient.println(data); // THERE MUST BE A WAY TO PACKAGE THE PRESET NAME IN HERE TOO

    ParsedMessage message = this->rest_receive(this->state_connection.SSLclient, 10000); // the device should receive a response here, make sure the server actually does that

    this->state_connection.SSLclient.stop();

    String response;

    switch(message.type) {
      case RESPONSE:
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
        break;
      case REQUEST: // this needs work. If the request is an unregister, that should be handleded accordingly. Also check to see that routes are closing connections right
        this->connect_to_server();
        this->router->execute_route(message.request, &response);
        this->state_connection.SSLclient.stop();
        break;
    }

    this->state_connection.SSLclient.stop(); // remember to close the connection after each request

    if(!this->storage->writer->reference_datetime) {
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


/// @brief listener function to listen for incoming requests from the server, and forward them to the router for processing.
/// @return NetworkReturnErrors to say if the operation succeeded, or if it failed, what caused the failure
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

    client.stop(); // more robust client closing in the case several requests occur within the span of one operation
  }

  return error;
}


/// @brief process listener errors, and sets network + machine state accordingly
/// @param error the error 
void ConnectionManager::listener_error_handler(NetworkReturnErrors error) {
  switch(error) {
    case WIFI_FAILURE:
      this->network_state = DOWN;
      this->machine_state->connection_state = MACHINE_DISCONNECTED;
      break;
    case CONNECTION_FAILURE:
      this->network_state = BROADCASTING;
      this->machine_state->connection_state = MACHINE_DISCONNECTED;
      break;
  }
}


/// @brief send a request to the server over SSL, and wait for a response
/// @param request request string, formatted using HTTP 
/// @return ParsedResponse containing the response from the server
ParsedResponse ConnectionManager::connected_send(String &request) {
  this->connect_to_server();

  this->state_connection.SSLclient.println(request);

  ParsedResponse response = this->rest_receive(this->state_connection.SSLclient, 10000).response;

  this->state_connection.SSLclient.stop();

  return response;
}


/// @brief Core network management code. State driven, running different functions based on machine network state. This should be the only function in void loop, everythign else is a pseudo-async task in the device task manager
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