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
///////// NetworkClient ///////////////////////////
///////////////////////////////////////////////////

NetworkClient::NetworkClient(Layer4Encryption layer_4_encryption = NONE) : layer_4_encryption_type(layer_4_encryption) {}

Layer4Encryption NetworkClient::get_encryption() {
  return this->layer_4_encryption_type;
}

NetworkExceptions NetworkClient::client_receive_message(WiFiClient &client, int timeout, String *message) {
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

NetworkExceptions NetworkClient::client_send_message(WiFiClient &client, HTTPMessage *message) {
  NetworkExceptions return_error = NETWORK_OKAY;
  String serialized_message = message->serialize();

  bool status = client.println(serialized_message);
  if(!status) {
    return_error = NETWORK_SERVER_CONNECTION_FAILURE;
  }

  delete message;
  return return_error;
}

HTTPMessage* NetworkClient::parse_message(String &unparsed_message) {
  HTTPMessage *received;

  if(message.startsWith("HTTP")) {
    received = new Response(unparsed_message);
  }
  else {
    received = new Request(unparsed_message);
  }

  return received;
}

HTTPMessage* NetworkClient::receive_and_parse(WiFiClient &client, int timeout=5000) {
  String unparsed;
  HTTPMessage *received;

  NetworkExceptions receive_error = this->client_receive_message(client, timeout, &unparsed);
  if(receive_error == NETWORK_OKAY) {
    received = this->parse_message(unparsed);
  }
  else {
    received->set_exception(receive_error);
  }

  return received;
}

///////////////////////////////////////////////////
///////// TCPRequestClient ////////////////////////
///////////////////////////////////////////////////

TCPRequestClient::TCPRequestClient(WiFiClient &client, Layer4Encryption encryption, ConnectionInformation &server_information) : NetworkClient(encryption), client(client), server_information(server_information) {}

Response TCPRequestClient::receive_response(WiFiClient &client) {
  Response *response_pointer;
  Response response;

  response_pointer = this->receive_and_parse(this->client, 10000);

  this->client.stop();

  response = Response(response_pointer);
  return response;
}

Response TCPRequestClient::request(Request *message) {
  Response response;

  switch(this->get_encryption()) {
    case ENCRYPTION_SSL:
      this->client.connectSSL(this->server_information.ip, this->server_information.port);
      break;
    case ENCRYPTION_NONE:
      this->client.connect(this->server_information.ip, this->server_information.port);
      break;
  }

  NetworkExceptions send_exception = this->client_send_message(this->client, message);
  response = this->receive_response();

  return response;
}

///////////////////////////////////////////////////
///////// TCPListenerClient ///////////////////////
///////////////////////////////////////////////////

TCPListenerClient::TCPListenerClient(WiFiServer &listener, Layer4Encryption encryption, Router router) : listener(listener), router(router), NetworkClient(encryption) {
  switch(encryption) {
    case ENCRYPTION_NONE:
      this->listener.begin();
      break;
    case ENCRYPTION_SSL:
      this->listener.beginSSL();
      break;
  }
}

NetworkExceptions TCPListenerClient::respond(Response &response, WiFiClient &client) {
  String response = response.serialize();
  NetworkExceptions exception = NETWORK_OKAY;
  if(!client.println(response)) {
    exception = NETWORK_SERVER_CONNECTION_FAILURE;
  }

  client.stop();
  return exception;
}

NetworkExceptions TCPListenerClient::listen() {
  Request *received_ptr;
  Request received;
  WiFiClient client = this->listener.available();

  received_ptr = this->receive_and_parse(client, 5000);
  if(received_ptr->get_exception() != NETWORK_OKAY) {
    delete received_ptr;
    return received_ptr->get_exception();
  }

  received = Request(received_ptr); // this frees the pointer memory

  Response response = this->router->execute_route(received);
  
  return this->respond(response, client);
}

///////////////////////////////////////////////////
///////// UDPClient ///////////////////////////////
///////////////////////////////////////////////////

UDPClient::UDPClient(WiFiUDP &udp_server, int local_port, ConnectionInformation multicast_information) : udp_server(udp_server), multicast_information(multicast_information) {
  this->udp_server.begin(local_port);
}

NetworkExceptions UDPClient::send_udp(UDPRequest &request) {
  String request_string = request.to_string();

  this->udp_server.beginPacket(this->multicast_information.ip, this->multicast_information.port);
  this->udp_server.write(request_string.c_str());
  this->udp_server.endPacket();

  return NETWORK_OKAY
}

UDPResponse UDPClient::receive_udp(int timeout) {
  UDPResponse response;

  char receive_buffer[32];

  long long start = millis();

  while(!this->udp_server.parsePacket() && (millis() - start < timeout)) {}
  if(timeout < millis() - start) {
    response.set_exception(NETWORK_TIMEOUT);
  }
  else {
    this->udp_server.read(receive_buffer, 32);
    response = UDPResponse(receive_buffer);
  }
  return response;
}

///////////////////////////////////////////////////
///////// ConnectionStage /////////////////////////
///////////////////////////////////////////////////

template <typename L, typename H, typename R>
void ConnectionStage<L, H, R>::configure_message_handler() {
 // need another attribute for this
}

template <typename L, typename H, typename R>
L ConnectionStage<L, H, R>::get_listener() {
  return this->listener;
}

template <typename L, typename H, typename R>
H* ConnectionStage<L, H, R>::get_handler() {
  return this->message_handler;
}

template <typename L, typename H, typename R>
void ConnectionStage<L, H, R>::set_handler(H* message_handler) {
  this->message_handler = message_handler;
}

template <typename L, typename H, typename R>
ConnectionStage<L, H, R>::~ConnectionStage() {
  delete this->message_handler;
}

///////////////////////////////////////////////////
///////// StageReturnBase /////////////////////////
///////////////////////////////////////////////////

StageReturnBase::StageReturnBase(NetworkExceptions exception) : exception(exception) {}

NetworkExceptions StageReturnBase::get_exception() {
  return this->exception;
}

void StageReturnBase::set_exception(NetworkExceptions exception) {
  this->exception = exception;
}

///////////////////////////////////////////////////
///////// StageReturn /////////////////////////////
///////////////////////////////////////////////////

template<typename R>
StageReturn<R>::StageReturn(NetworkExceptions exception, R return_value) : StageReturnBase(exception), return_value(return_value) {};

template<typename R>
R StageReturn<R>::get_return_value() {
  return this->return_value;
}

template<typename R>
void StageReturn<R>::set_return_value(R return_value) {
  this->return_value = return_value;
}

///////////////////////////////////////////////////
///////// StageWifiBaseClass //////////////////////
///////////////////////////////////////////////////

/// @brief connect to an enterprise authenticated AP with username and password
/// @param info WifiInfo reference to wifi info object containing configuration information for AP connection
/// @return bool to say if the connection succeeded or not
bool StageWifiBaseClass::enterprise_connect(WifiInfo &info) { // pls fix my malloc bs
  tstr1xAuthCredentials auth;
  strcpy((char *) auth.au8UserName, info.username.c_str());
  strcpy((char *) auth.au8Passwd, info.password.c_str());

  bool wifi_status = m2m_wifi_connect_sc(info.get_ssid().c_str(), info.get_ssid().length(), M2M_WIFI_SEC_802_1X, &auth, info.channel, false);

  return wifi_status == M2M_SUCCESS;
}

/// @brief simple AP connection function that connects to home wifi AP with just a password
/// @param info wifi information containing credentials necessary for connection
/// @return bool to say if connection succeeded or not
bool StageWifiBaseClass::home_connect(WiFiInfo &info) {
  bool status = WiFi.begin(info.get_ssid(), info.get_password());
  return status == WL_CONNECTED;
}

/// @brief function for switching between different connection functions based on the type enum specified in the wifi info
/// @param info wifi information necessary for connection to an AP, contains the network type
/// @return NetworkReturnErrors enum containing error or okay message after execution
NetworkExceptions StageWifiBaseClass::connect_wifi(WiFiInfo &info) {
  bool result;

  switch(info.type) {
    case WIFI_ENTERPRISE:
      result = this->enterprise_connect(info);
      break;
    case WIFI_HOME:
      result = this->home_connect(info);
      break;
    case WIFI_OPEN:
      break;
  }
  if(!result) {
    return NETWORK_AUTHENTICATION_FAILURE;
  }
  return NETWORK_OKAY;
}

///////////////////////////////////////////////////
///////// StageWifiInitialization /////////////////
///////////////////////////////////////////////////

StageWifiInitialization::StageWifiInitialization(int listen_port) : ConnectionStage(listen_port) {
  Router router;
  router
    .add_route(new ReceiveCredentials("/submit", POST, &this->temporary_wifi_info, &this->wifi_configured_flag))
    .add_route(new SendHTML("/", GET))
    .add_route(new SendCSS("/styles", GET))
    .add_route(new SendJS("/script", GET));
  
  this->set_handler(new TCPListenerClient(this->get_listener(), ENCRYPTION_NONE, router));
}

String StageWifiInitialization::ssid_config() {
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

bool StageWifiInitialization::start_access_point() {
  String ssid;
  bool status;

  ssid = this->ssid_config();
  status = WiFi.beginAP(converted, AP_CONFIG_PASSWORD);
  if (status != WL_AP_LISTENING) {
    return false;
  }
  delay(2000);
  return true;
}

/// @brief function used during wifi provisioning to receive credentials for AP association from served webpage. Serves webpage and submits respones to client
/// @param client wifi client to use for communications with the requesting device
/// @return WifiInfo object to return and apply to the flash memory and to the server
/// @todo upgrade this to an SSL server connection
NetworkExceptions StageWifiInitialization::receive_credentials(WiFiClient &client) {
  NetworkExceptions exception;
  while(!this->wifi_configured) {
    exception = this->get_handler()->listen();

    if(this->wifi_configured) { // better error handling here maybe?
      break;
    }
  }

  return exception
}

StageReturn<WifiInfo>* StageWifiInitialization::run() {
  String ssid; 
  StageReturn<WifiInfo> stage_return = new StageReturn<WifiInfo>();
  NetworkExceptions exception;

  if(!check_wifi_module_status()) {
    stage_return->set_exception(NETWORK_WIFI_FAILURE);
    return stage_return
  }

  bool connection_success = false;

  while(!connection_success) {
    if(!this->start_access_point()) {
      stage_return->set_exception(NETWORK_WIFI_FAILURE);
      return stage_return;
    }

    exception = this->receive_credentials(); // handle errors
    WiFi.end();

    exception = this->connect_wifi(this->temporary_wifi_information); // handle errors, fix later

    switch(exception) {
      case NETWORK_AUTHENTICATION_FAILURE:
        connection_success = false;
        break;
      case NETWORK_OKAY:
        connection_success = true;
        break;
    }
  }

  exception = NETWORK_OKAY;
  stage_return->set_exception(exception);
  stage_return->set_return_value(this->temporary_wifi_information);

  return stage_return;
}

///////////////////////////////////////////////////
///////// StageWifiStartup ////////////////////////
///////////////////////////////////////////////////

StageWifiStartup::StageWifiStartup(WifiInfo &wifi_info) : wifi_info(wifi_info) {}

StageReturn<WifiInfo>* run() {
  NetworkExceptions exception;
  StageReturn<WifiInfo> stage_return = new StageReturn<WifiInfo>();

  exception = this->connect_wifi(this->wifi_info)

  stage_return->set_exception(exception);
  stage_return->set_return_value(this->wifi_info);

  return stage_return
}

///////////////////////////////////////////////////
///////// Broadcasting ////////////////////////////
///////////////////////////////////////////////////

StageBroadcasting::StageBroadcasting(ConnectionInformation local_information, ConnectionInformation multicast_information, const Identifiers &device_identifiers) : local_information(local_information), multicast_information(multicast_information), device_identifiers(device_identifiers) {
  this->set_handler(new UDPClient(this->get_listener(), local_information.port, multicast_information));
}

StageReturn<ConnectionInformation>* StageBroadcasting::run() {
  NetworkExceptions exception;
  ConnectionInformation returned_connection_information;
  StageReturn<ConnectionInformation> stage_return = new StageReturn<ConnectionInformation>();

  UDPRequest request(this->local_information->ip, this->device_identifiers);
  UDPResponse response;

  bool received_acknowledgement = false;

  while(received_acknowledgement) {
    exception = this->get_listener()->send_udp(request);
    response = this->get_listener()->receive_udp();
    exception = response.get_exception();

    switch(exception) {
      case NETWORK_TIMEOUT:
        break;
      case NETWORK_OKAY:
        returned_connection_information.ip = response.get_server_ip();
        returned_connection_information.port = response.get_server_port();
        stage_return->set_return_value(returned_connection_information);

        this->received_acknowledgement = true;
        break;
    }
  }

  return stage_return;
}


///////////////////////////////////////////////////
///////// Association /////////////////////////////
///////////////////////////////////////////////////

StageAssociation::StageAssociation(ConnectionInformation server_information, Identifiers &identifiers, MachineState &machine_state) : server_information(server_information), identifiers(identifiers), machine_state(machine_state) {
  this->set_handler(new TCPRequestClient(this->get_listener(), ENCRYPTION_SSL, this->server_information));
}

/// @brief function designed to connect to server using the object's ssl client. Makes 5 attempts at 5 second intervals to connect
/// @return if the function fails to connect after 5 attempts, return false
NetworkExceptions StageAssociation::test_server_connection() {
  for(int x = 0; x < 3; x++) {
    if(this->get_listener().connectSSL(this->server_information.ip.c_str(), this->server_information.port)) {
      delay(1000);
      this->get_listener().stop();
      return NETWORK_OKAY;
    }
    delay(5000);
  }
  return NETWORK_SERVER_CONNECTION_FAILURE;
}

Request StageAssociation::generate_registration_request() {
  Request registration_request(POST, "/devices/confirm", this->server_information.ip, this->identifiers.get_device_id(), this->machine_state.get_state());
  return registration_request;
}

StageReturn<AssociationReturnStruct>* StageAssociation::run() {
  AssociationReturnStruct received_configuration;
  NetworkExceptions exception;
  Identifiers identifier;
  Preset preset;
  StageReturn<AssociationReturnStruct> stage_return = new StageReturn<AssociationReturnStruct>();

  Request registration_request;
  Response registration_response;

  int failure_count = 0;
  bool associated = false;

  registration_request = this->generate_registration_request();

  while(!associated && failure_count < 3) {
    registration_response = this->get_handler().request(registration_request);
    exception = registration_response.exception;

    received_configuration.identifier.from_json(registration_response.get_body());
    received_configuration.preset.from_json(registration_response.get_body());

    switch(exception) {
      case NETWORK_SERVER_CONNECTION_FAILURE:
        failure_count++;
        if(failure_count >= 3) {
          stage_return->set_exception(exception);
        }
        break;
      case NETWORK_OKAY:
        associated = true;
        break;
    }
  }

  return stage_return
}

///////////////////////////////////////////////////
///////// FullConnection //////////////////////////
///////////////////////////////////////////////////

StageFullConnection::StageFullConnection(ConnectionInformation &server_information) : request_client(this->wifi_ssl_object, ENCRYPTION_SSL, server_information), server_information(server_information) {
  Router router;
  router
    .add_route(new NetworkReset("/reset/network", POST))
    .add_route(new DeviceReset("/reset/hard", DELETE))
    .add_route(new SetTime("/time", POST, global_storage))
    .add_route(new ConfigureDeviceIds("/configure/id", POST, global_storage))
    .add_route(new ConfigureDevicePreset("/configure/preset", POST, global_storage))
    .add_route(new PauseDevice("/configure/status", PUT, global_storage));

  this->set_handler(new TCPListenerClient(this->get_listener(), ENCRYPTION_SSL, router));
}

NetworkExceptions StageFullConnection::handle_incoming() {
  NetworkExceptions exception;
  int failure_count = 0;

  while(failure_count < 7) {
    exception = this->get_handler().listen();

    switch(exception) {
      case NETWORK_TIMEOUT:
        break;
      case NETWORK_SERVER_CONNECTION_FAILURE:
        failure_count++;
        break;
      case NETWORK_OKAY:
        failure_count = 0;
        break;
    }
  }
  return exception;
}

Response StageFullConnection::send_request(Request &request) {
  Response response;

  response = this->request_client.request(&request);

  return response;
}

StageReturn<bool>* StageFullConnection::run() {
  StageReturn<bool> stage_return = new StageReturn<bool>();
  stage_return->set_return_value(true);

  NetworkExceptions exception;

  exception = this->handle_incoming();

  stage_return->set_exception(exception);

  return stage_return;
}

///////////////////////////////////////////////////
///////// Connectionmanager ///////////////////////
///////////////////////////////////////////////////

ConnectionManager::ConnectionManager(ConnectionInformation local_connection_information, ConnectionInformation multicast_information, StorageManager *global_storage) : local_connection_information(local_connection_information), multicast_information(multicast_information), (global_storage) {

}

void ConnectionManager::set_stage_object(ConnectionStageIdentifier stage) {
  delete this->stage;

  switch(this->stage_identifier) {
    case STAGE_INITIALIZING:
      this->stage = new StageWifiInitialization(this->local_port);
      break;
    case STAGE_STARTUP:
      this->stage = new StageWifiStartup(this->global_storage->get_wifi());
      break;
    case STAGE_BROADCASTING:
      this->stage = new StageBroadcasting(this->server_information, this->multicast_information, this->global_storage->get_identifiers());
      break;
    case STAGE_ASSOCIATING:
      this->stage = new StageAssociation(this->server_information, this->global_storage->get_identifiers(), this->global_storage->get_machine_state().get_state());
      break;
    case STAGE_CONNECTED:
      this->stage = new StageFullConnection(this->server_information);
      break;
  }
}

void ConnectionManager::handle_error(NetworkExceptions exception) {
  switch(exception) {
    case NETWORK_WIFI_FAILURE:
      this->stage_identifier = STAGE_STARTUP;
      break;
    case NETWORK_AUTHENTICATION_FAILURE:
      this->stage_identifier = STAGE_INITIALIZING;
      break;
    case NETWORK_SERVER_CONNECTION_FAILURE:
      this->stage_identifier = STAGE_BROADCASTING;
      break;
  }
}

void ConnectionManager::handle_returns(StageReturnBase *return_value) {
  if(*return_value->get_exception() != NETWORK_OKAY) {
    this->handle_error(*return_value->get_exception());
    return
  }

  switch(this->stage_identifier) {
    case STAGE_INITIALIZING:
      this->global_storage->get_wifi().copy(*return_value->get_return_value());
      this->stage_identifier = STAGE_BROADCASTING;
      break;
    case STAGE_STARTUP:
      this->stage_identifier = STAGE_BROADCASTING;
      break;
    case STAGE_BROADCASTING:
      this->server_information = *return_value->get_return_value();
      this->stage_identifier = STAGE_ASSOCIATING;
      break;
    case STAGE_ASSOCIATING:
      this->stage_identifier = STAGE_CONNECTED;
      break;
    case STAGE_CONNECTED:
      break;
  }

  delete return_value;
}

void ConnectionManager::run() {
  StageReturnBase *return_value;

  return_value = this->stage->run();

  this->handle_returns(return_value);
}

/// @brief Core network management code. State driven, running different functions based on machine network state. This should be the only function in void loop, everythign else is a pseudo-async task in the device task manager
void ConnectionManager::run() { // goes in void_loop
  bool status;
  switch(this->stage) {
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