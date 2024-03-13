///////////////////////////////////////////////////
///////// StageWifiBaseClass //////////////////////
///////////////////////////////////////////////////

/// @brief connect to an enterprise authenticated AP with username and password
/// @param info WifiInfo reference to wifi info object containing configuration information for AP connection
/// @return bool to say if the connection succeeded or not
bool StageWifiBaseClass::enterprise_connect(WifiInfo &info) { // pls fix my malloc bs
  tstr1xAuthCredentials auth;
  strcpy((char *) auth.au8UserName, info.get_username().c_str());
  strcpy((char *) auth.au8Passwd, info.get_password().c_str());

  bool wifi_status = m2m_wifi_connect_sc((char*)info.get_ssid().c_str(), info.get_ssid().length(), M2M_WIFI_SEC_802_1X, &auth, info.get_channel(), false);

  return wifi_status == M2M_SUCCESS;
}

/// @brief simple AP connection function that connects to home wifi AP with just a password
/// @param info wifi information containing credentials necessary for connection
/// @return bool to say if connection succeeded or not
bool StageWifiBaseClass::home_connect(WifiInfo &info) {
  bool status = WiFi.begin(info.get_ssid(), info.get_password());
  return status == WL_CONNECTED;
}

/// @brief function for switching between different connection functions based on the type enum specified in the wifi info
/// @param info wifi information necessary for connection to an AP, contains the network type
/// @return NetworkReturnErrors enum containing error or okay message after execution
NetworkException StageWifiBaseClass::connect_wifi(WifiInfo &info) {
  bool result;

  switch(info.get_type()) {
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