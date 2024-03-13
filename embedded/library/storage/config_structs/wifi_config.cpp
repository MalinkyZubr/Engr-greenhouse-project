////////////////////////////////////////////////////////////////////
//////////////// WifiInfo //////////////////////////////////////////
////////////////////////////////////////////////////////////////////

WifiInfo::WifiInfo(SPIFlash *flash, int flash_address) : ConfigStruct(flash, flash_address) {}

WifiInfo::WifiInfo(String ssid, int channel, String password, String username) : ssid(ssid), channel(channel), username(username), password(password), type(WIFI_ENTERPRISE) {}

WifiInfo::WifiInfo(String ssid, int channel, String password) : ssid(ssid), channel(channel), password(password), type(WIFI_HOME) {}

WifiInfo::WifiInfo(String ssid, int channel) : ssid(ssid), channel(channel), type(WIFI_OPEN) {}

WifiNetworkTypes WifiInfo::get_type() const {
  return this->type;
}

String WifiInfo::get_ssid() const {
  return this->ssid;
}

String WifiInfo::get_username() const {
  return this->username;
}

String WifiInfo::get_password() const {
  return this->password;
}

int WifiInfo::get_channel() const {
  return this->channel;
}

bool WifiInfo::copy(WifiInfo to_copy) {
  this->type = to_copy.type;
  this->channel = to_copy.channel;
  this->ssid = to_copy.ssid;
  this->username = to_copy.username;
  this->password = to_copy.password;

  DynamicJsonDocument wifi_info_json = to_copy.to_json();
  return this->write(wifi_info_json);
}

StorageException WifiInfo::from_json(const DynamicJsonDocument &data) {
  String type = data["type"];
  
  switch(type.c_str()[0]) {
    case 'h':
      this->type = WIFI_HOME;
      this->password = (char *)&data["password"];
      break;
    case 'e':
      this->type = WIFI_ENTERPRISE;
      this->password = (char *)&data["password"];
      this->username = (char *)&data["username"];
      break;
    case 'o':
      this->type = WIFI_OPEN;
  } 
  this->channel = data["channel"];

  this->ssid = (char *)&data["ssid"];
}

const DynamicJsonDocument WifiInfo::to_json() const {
  DynamicJsonDocument data(CONFIG_JSON_SIZE);

  data["type"] = (char)this->type;
  data["ssid"] = this->ssid;
  data["username"] = this->username;
  data["password"] = this->password;
  data["channel"] = this->channel;

  return data;
}