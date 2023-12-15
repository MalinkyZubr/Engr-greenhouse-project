#include "storage.hpp"


////////////////////////////////////////////////////////////////////
//////////////// ConfigStruct //////////////////////////////////////
////////////////////////////////////////////////////////////////////

ConfigStruct::ConfigStruct(SPIFlash *flash, int address) : flash(flash), flash_address(address) {}

StorageException ConfigStruct::write(DynamicJsonDocument &data) {
  String to_write;

  StorageException conversion_exception = this->from_json(data);
  if(conversion_exception != STORAGE_OKAY) {
    return conversion_exception;
  }
  this->erase();

  serializeJson(data, to_write);
  if(!this->flash->writeStr(this->flash_address, to_write)) {
    return STORAGE_WRITE_FAILURE;
  };
}

bool ConfigStruct::read() {
  String to_read;
  String to_save;
  DynamicJsonDocument to_deserialize(CONFIG_JSON_SIZE);

  this->flash->readStr(this->flash_address, to_read);

  if(to_read.equals("")) {
    this->set_unconfigured();
    return false;
  }
  deserializeJson(to_deserialize, to_save);

  this->from_json(to_deserialize);
  this->set_configured();

  return true;
}

bool ConfigStruct::erase() {
  this->is_configured = false;
  return this->flash->eraseSector(this->flash_address);
}

void ConfigStruct::set_configured() {
  this->is_configured = true;
}

void ConfigStruct::set_unconfigured() {
  this->is_configured = false;
}

bool ConfigStruct::check_is_configured() {
  return this->is_configured;
}

////////////////////////////////////////////////////////////////////
//////////////// Identifiers ///////////////////////////////////////
////////////////////////////////////////////////////////////////////

Identifiers::Identifiers(SPIFlash *flash, int flash_address) : ConfigStruct(flash, flash_address) {}

int Identifiers::get_device_id() {
  return this->device_id;
}

int Identifiers::get_project_id() {
  return this->project_id;
}

String Identifiers::get_device_name() {
  return this->device_name;
}

StorageException Identifiers::from_json(DynamicJsonDocument &data) {
  if(!data.containsKey("device_id") || !data.containsKey("project_id") || !data.containsKey("device_name")) {
    return STORAGE_IDENTIFIER_FIELD_MISSING;
  }
  this->device_id = data["device_id"];
  this->project_id = data["project_id"];
  this->device_name = (char *)&data["device_name"];

  return STORAGE_OKAY;
}

DynamicJsonDocument Identifiers::to_json() {
  DynamicJsonDocument data(CONFIG_JSON_SIZE);

  data["device_id"] = this->device_id;
  data["device_name"] = this->device_name;
  data["project_id"] = this->project_id;

  return data;
}

////////////////////////////////////////////////////////////////////
//////////////// Machine State /////////////////////////////////////
////////////////////////////////////////////////////////////////////

MachineState::MachineState(SPIFlash *flash, int flash_address) : ConfigStruct(flash, flash_address) {}

MachineOperationalState MachineState::get_state() {
  return this->operational_state;
}

void MachineState::set_state(MachineOperationalState state) {
  this->operational_state = state;
}

StorageException MachineState::from_json(DynamicJsonDocument &data) {
  if(!data.containsKey("operational_state")) {
    return STORAGE_IDENTIFIER_FIELD_MISSING;
  }

  char reported_operational_state = (char)data["operational_state"];
  switch(reported_operational_state) {
    case 'p':
      this->operational_state = MACHINE_PAUSED;
      break;
    case 'a':
      this->operational_state = MACHINE_ACTIVE;
      break;
  }
  return STORAGE_OKAY;
}

DynamicJsonDocument MachineState::to_json() {
  DynamicJsonDocument data(CONFIG_JSON_SIZE);

  data["operational_state"] = (char)this->operational_state;

  return data;
}

////////////////////////////////////////////////////////////////////
//////////////// Preset ////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

Preset::Preset(SPIFlash *flash, int flash_address) : ConfigStruct(flash, flash_address) {}

float Preset::get_temperature() {
  return this->temperature;
}

float Preset::get_humidity() {
  return this->humidity;
}

float Preset::get_moisture() {
  return this->moisture;
}

float Preset::get_hours_daylight() {
  return this->hours_daylight;
}

float Preset::get_preset_id() {
  return this->preset_id;
}

StorageException Preset::from_json(DynamicJsonDocument &data) {
  if(!data.containsKey("preset_id") || !data.containsKey("temperature") || !data.containsKey("humidity") || !data.containsKey("moisture") || !data.containsKey("hours_daylight")) {
    return STORAGE_PRESET_FIELD_MISSING;
  }

  this->preset_id = data["preset_id"];
  this->temperature = data["temperature"];
  this->humidity = data["humidity"];
  this->moisture = data["moisture"];
  this->hours_daylight = data["hours_daylight"];

  return STORAGE_OKAY;
}

DynamicJsonDocument Preset::to_json() {
  DynamicJsonDocument data(CONFIG_JSON_SIZE);

  data["preset_id"] = this->preset_id;
  data["temperature"] = this->temperature;
  data["humidity"] = this->humidity;
  data["moisture"] = this->moisture;
  data["hours_daylight"] = this->hours_daylight;

  return data;
}


////////////////////////////////////////////////////////////////////
//////////////// WifiInfo //////////////////////////////////////////
////////////////////////////////////////////////////////////////////

WifiInfo::WifiInfo(SPIFlash *flash, int flash_address) : ConfigStruct(flash, flash_address) {}

WifiInfo::WifiInfo(String ssid, int channel, String password, String username) : ssid(ssid), channel(channel), username(username), password(password), type(WIFI_ENTERPRISE) {}

WifiInfo::WifiInfo(String ssid, int channel, String password) : ssid(ssid), channel(channel), password(password), type(WIFI_HOME) {}

WifiInfo::WifiInfo(String ssid, int channel) : ssid(ssid), channel(channel), type(WIFI_OPEN) {}

WifiNetworkTypes WifiInfo::get_type() {
  return this->type;
}

String WifiInfo::get_ssid() {
  return this->ssid;
}

String WifiInfo::get_username() {
  return this->username;
}

String WifiInfo::get_password() {
  return this->password;
}

int WifiInfo::get_channel() {
  return this->channel;
}

bool WifiInfo::copy(WifiInfo &to_copy) {
  this->type = to_copy.type;
  this->channel = to_copy.channel;
  this->ssid = to_copy.ssid;
  this->username = to_copy.username;
  this->password = to_copy.password;

  DynamicJsonDocument wifi_info_json = to_copy.to_json();
  return this->write(wifi_info_json);
}

StorageException WifiInfo::from_json(DynamicJsonDocument &data) {
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

DynamicJsonDocument WifiInfo::to_json() {
  DynamicJsonDocument data(CONFIG_JSON_SIZE);

  data["type"] = (char)this->type;
  data["ssid"] = this->ssid;
  data["username"] = this->username;
  data["password"] = this->password;
  data["channel"] = this->channel;

  return data;
}

////////////////////////////////////////////////////////////////////
//////////////// DataManager ///////////////////////////////////////
////////////////////////////////////////////////////////////////////

/// @brief the data writer is a utility class to write data to the flash memory in cases where data can no longer be uploaded to the server
/// @param flash reference to the flash memory unit driver object to be used for writing data
DataManager::DataManager(SPIFlash *flash, int start_address, int max_size) : flash(flash), flash_address(start_address), max_size(max_size) {}

/// @brief write a piece of data in Json format to the flash memory in the event connection is lost
/// @param data json document representing data to be written to flash
/// @return bool to represent if the write was successful or not
bool DataManager::write_data(DynamicJsonDocument &data) {
  String serialized;
  bool status;

  this->is_storing = true;

  if(!this->is_full) {
    data["next"] = this->current + this->partition_size;
    data["previous"] = this->current - this->partition_size;
    data["seconds_from_start"] = millis() / 1000;
    data["reference_datetime"] = this->reference_datetime;

    serializeJson(data, serialized);

    status = this->flash->writeStr(this->current, serialized);

    this->current += this->partition_size;

    if(this->current + partition_size >= DATA_STORAGE_LIMIT) {
      this->is_full = true;
    }
  }
  else {
    status = false;
  }
  return status;
}


/// @brief when connection is re-established after writing data to the flash memory, this function consecutively reads a 266 bit json document from the flash, and loads it into memory to be sent.
/// This is necessary since loading everything into RAM at once would be way too costly. When the entirety of the flash data has been read
/// @param data_output reference to the json object to write the data to once read
/// @return bool to represent if reading was successful
/// @todo what abut the case in which the connection is lost while flushing? the memory should be erased from the point it rests and then writing can resume
bool DataManager::read_data(DynamicJsonDocument &data_output) {
  String serialized;

  this->current -= partition_size;
  this->flash->readStr(this->current, serialized);
  deserializeJson(data_output, serialized);
  
  if(current <= this->flash_address) {
    return true; // the data segment has been completely flushed
  }
}


/// @brief erase all the data from the data section of the flash memory
/// @return bool to tell if the operation succeeded
bool DataManager::erase_data() {
  int current_address = this->current - BLOCK_SIZE >= this->flash_address ? this->current - BLOCK_SIZE : this->flash_address;

  while(current_address >= this->flash_address) {
    this->flash->eraseSector(current_address);

    if(current_address - BLOCK_SIZE <= this->flash_address) {
      current_address = this->flash_address;
    }
  }
  this->is_storing = false;
  return true;
}

void DataManager::set_reference_datetime(int timestamp) {
  this->reference_datetime = timestamp;
}

int DataManager::get_end_address() {
  return this->flash_address + this->max_size;
}

////////////////////////////////////////////////////////////////////
//////////////// DeviceResetter ////////////////////////////////////
////////////////////////////////////////////////////////////////////

DeviceResetter::DeviceResetter(int reset_pin) : reset_pin(reset_pin) {
  pinMode(reset_pin, OUTPUT);
}

void DeviceResetter::trigger_reset() {
  digitalWrite(this->reset_pin, HIGH);
}

////////////////////////////////////////////////////////////////////
//////////////// StorageManager ////////////////////////////////////
////////////////////////////////////////////////////////////////////

StorageManager::StorageManager(int start_address, int data_size, int device_reset_pin) 
  : resetter(device_reset_pin), 
  identifier_info(&this->flash, start_address + BLOCK_SIZE),
  preset_info(&this->flash, start_address + BLOCK_SIZE * 2),
  wifi_info(&this->flash, start_address + BLOCK_SIZE * 3),
  machine_state(&this->flash, start_address + BLOCK_SIZE * 4),
  data_manager(&this->flash, start_address + BLOCK_SIZE * 5, data_size) {
  if(this->data_manager.get_end_address() > this->flash.getCapacity()) { 
    return;
  }
  this->load_flash_configuration();
}

void StorageManager::load_flash_configuration() {
  this->identifier_info.read();
  this->preset_info.read();
  this->wifi_info.read();
}

DataManager& StorageManager::get_data_manager() {
  return this->data_manager;
}

WifiInfo& StorageManager::get_wifi() {
  return this->wifi_info;
}

Preset& StorageManager::get_preset() {
  return this->preset_info;
}

Identifiers& StorageManager::get_identifiers() {
  return this->identifier_info;
}

MachineState& StorageManager::get_machine_state() {
  return this->machine_state;
}

bool StorageManager::write_flash_configuration(ConfigType configuration, DynamicJsonDocument &to_write) {
  switch(configuration) {
    case IDENTIFIER:
      return this->identifier_info.write(to_write);
      break;
    case PRESET:
      return this->preset_info.write(to_write);
      break;
  }
}

bool StorageManager::write_flash_configuration(WifiInfo &wifi_info) {
  this->wifi_info.copy(wifi_info);
}

void StorageManager::hard_reset() {
  this->flash.eraseChip();
  this->resetter.trigger_reset();
}