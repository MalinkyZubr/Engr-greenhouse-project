///////////////////////////////////////////////////
///////// StageWifiInitialization /////////////////
///////////////////////////////////////////////////

StageWifiInitialization::StageWifiInitialization(int listen_port) : StageWifiBaseClass(listen_port) {
  Router router;
  router
    .add_route(new ReceiveCredentials("/submit", POST, &this->temporary_wifi_information, &this->wifi_configured))
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
    return "";
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
  status = WiFi.beginAP((char*) ssid.c_str(), AP_CONFIG_PASSWORD);
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
NetworkException StageWifiInitialization::receive_credentials() {
  NetworkException exception;
  while(!this->wifi_configured) {
    exception = this->get_handler()->listen();

    if(this->wifi_configured) { // better error handling here maybe?
      break;
    }
  }

  return exception;
}

StageReturn<WifiInfo>* StageWifiInitialization::run() {
  String ssid; 
  StageReturn<WifiInfo>* stage_return = new StageReturn<WifiInfo>();
  NetworkException exception;

  if(!check_wifi_module_status()) {
    stage_return->set_exception(NETWORK_WIFI_FAILURE);
    return stage_return;
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
