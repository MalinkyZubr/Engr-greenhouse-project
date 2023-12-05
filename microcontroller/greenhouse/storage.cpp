#include "storage.hpp"



/// @brief the data writer is a utility class to write data to the flash memory in cases where data can no longer be uploaded to the server
/// @param flash reference to the flash memory unit driver object to be used for writing data
DataWriter::DataWriter(SPIFlash *flash) : flash(flash) {}


/// @brief write a piece of data in Json format to the flash memory in the event connection is lost
/// @param data json document representing data to be written to flash
/// @return bool to represent if the write was successful or not
bool DataWriter::write_data(DynamicJsonDocument &data) {
  String serialized;
  bool status;

  this->is_storing = true;

  if(!this->is_full) {
    data["next"] = this->current + this->partition_size;
    data["previous"] = this->current - this->partition_size;
    data["seconds_from_start"] = millis() / 1000;
    data["reference_datetime"] = this->reference_datatime;

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
bool DataWriter::decrement_read(DynamicJsonDocument &data_output) {
  String serialized;

  this->current -= partition_size;
  this->flash->readStr(this->current, serialized);
  deserializeJson(data_output, serialized);
  
  if(current <= this->data_storage_start) {
    return true; // the data segment has been completely flushed
  }
}


/// @brief erase all the data from the data section of the flash memory
/// @return bool to tell if the operation succeeded
bool DataWriter::erase_all_data() {
  long current_address = this->current - ERASE_BLOCK_SIZE >= this->data_storage_start ? this->current - ERASE_BLOCK_SIZE : this->data_storage_start;

  while(current_address >= this->data_storage_start) {
    this->flash->eraseSector(current_address);

    if(current_address - ERASE_BLOCK_SIZE <= this->data_storage_start) {
      current_address = this->data_storage_start;
    }
  }
  this->is_storing = false;
  return true;
}


/// @brief Configuration manager is the steward of the flash memory system. It is responsible for writing and reading configurations, and device data
/// @param machine_state reference to the machine state, so that if the device ever has its project or preset deleted, it will enter a paused state and no longer take measurements or maintain the environment
/// @param device_reset_pin digital output pin number for device resets. When a device reset request is called, this will be set to high
ConfigManager::ConfigManager(MachineState *machine_state, int device_reset_pin) : machine_state(machine_state), device_reset_pin(device_reset_pin) {
  this->flash.begin();

  pinMode(device_reset_pin, OUTPUT);

  this->writer = new DataWriter(&this->flash);

  DynamicJsonDocument doc(CONFIG_JSON_SIZE);
  
  // load existing configuration to the storage manager
  this->load_device_identifiers(doc, this->config.identifying_information);
  doc.clear();
  this->load_preset_info(doc, this->config.preset);
  doc.clear();
  this->load_wifi_info(doc, this->config.wifi_information);
  doc.clear();

  if(this->config.identifying_information.device_id != -1) {
    this->configured = true;
  }
}


/// @brief sets the reference datetime for the data writer. Necessary in cases where connection is lost. This will keep data time synced (to a reasonable extent)
/// @param datetime unix timestamp representing the datetime the device was associated with its server
void ConfigManager::set_reference_datetime(float datetime) {
  this->writer->reference_datatime = datetime;
}


/// @brief retrieve data from the flash memory unit at specified address
/// @param address the address to extract data from
/// @param output reference to string to write to
/// @return bool to say if reading succeeded or not
bool ConfigManager::retrieve_configuration_flash(int address, String *output) {
  return this->flash.readStr(address, *output);
}


/// @brief retrieve data from flash memory and deserialize it to a json object
/// @param address address to get data from
/// @param document reference to json object to write to
void ConfigManager::retrieve_config_to_json(int address, DynamicJsonDocument &document) {
  String data;
  this->retrieve_configuration_flash(address, &data);
  deserializeJson(document, data);
}


/// @brief device identifiers like device_id and device_name must be stored on the flash for configuration persistence. 
/// This function loads data from the flash into memory to be stored as attributes of the ConfigManager class
/// @param document json document to be used in the writing processed for data formatting
/// @param identifiers identifiers object to be parsed to object
void ConfigManager::load_device_identifiers(DynamicJsonDocument &document, Identifiers &identifiers) {
  String temp_string;
  this->retrieve_configuration_flash(this->identifier_address, &temp_string);

  if(temp_string.equals("")) {
    return;
  }

  deserializeJson(document, temp_string);
  this->deserialize_device_identifiers(identifiers, document);
}


/// @brief wifi info is necessary for reconnection capabilities in case there is a power disruption to the device. This function loads wifi
/// information from the device into memory as attributes of the ConfigManager class
/// @param document json document to be used in the writing process for data formatting
/// @param wifi_info WifiInfo object to be saved as an object member
void ConfigManager::load_wifi_info(DynamicJsonDocument &document, WifiInfo &wifi_info) {
  String temp_string;

  this->retrieve_configuration_flash(this->identifier_address, &temp_string);

  if(temp_string.equals("")) {
    return;
  }

  deserializeJson(document, temp_string);

  String type = document["type"];
  
  switch(type.c_str()[0]) {
    case 'h':
      wifi_info.type = HOME;
      wifi_info.password = (char *)&document["password"];
      break;
    case 'e':
      wifi_info.type = ENTERPRISE;
      wifi_info.password = (char *)&document["password"];
      wifi_info.username = (char *)&document["username"];
      break;
    case 'o':
      wifi_info.type = OPEN;
  } 
  wifi_info.channel = document["channel"];

  wifi_info.ssid = (char *)&document["ssid"];
}


/// @brief preset information persistence is necessary in order to ensure the greenhouse can continue maintaining its environment, even in cases where connection to the server or power is unstable. 
/// This function loads assigned preset data into memory as a member of the ConfigManager class
/// @param document reference json document object for data formatting
/// @param preset reference to Preset object for deserialization into usable format
void ConfigManager::load_preset_info(DynamicJsonDocument &document, Preset &preset) {
  String temp_string;
  this->retrieve_configuration_flash(this->preset_address, &temp_string);

  if(temp_string.equals("")) {
    return;
  }

  deserializeJson(document, temp_string);

  this->deserialize_preset(preset, document);
}


/// @brief write a json document to an address on the flash memory
/// @param address address at which to write the data
/// @param document json document to serialize and write to the flash memory
/// @return bool to say if the writing succeeded or not
bool ConfigManager::write_configuration_flash(int address, DynamicJsonDocument &document) {
  String serialized;
  serializeJson(document, serialized);
  this->flash.eraseSector(address);
  return this->flash.writeStr(address, serialized);
}


/// @brief convert a Preset object into json serializable document object
/// @param preset preset object to be converted
/// @param document document object to load the data into
void ConfigManager::serialize_preset(Preset &preset, DynamicJsonDocument &document) {
  document["preset_id"] = preset.preset_id;
  document["temperature"] = preset.temperature;
  document["humidity"] = preset.humidity;
  document["moisture"] = preset.moisture;
  document["hours_daylight"] = preset.hours_daylight;
}


/// @brief convert a json document to a preset object for use in code
/// @param preset preset object to write json data to
/// @param document json document object to deserialize into preset object
void ConfigManager::deserialize_preset(Preset &preset, DynamicJsonDocument &document) {
  preset.preset_id = document["preset_id"];
  preset.temperature = document["temperature"];
  preset.humidity = document["humidity"];
  preset.moisture = document["moisture"];
  preset.hours_daylight = document["hours_daylight"];
}


/// @brief function that takes a preset object, sets the manager object preset field to that preset, and writes the preset to the flash in json format for persistence.
/// If the preset ID is -1 eg the preset is deleted, the device goes into a MACHINE_PAUSED state
/// @param preset preset object to set in memory and in flash
/// @return bool to say if operation completed successfully
bool ConfigManager::set_preset(Preset preset) {
  this->config.preset = preset;
  if(this->config.preset.preset_id == -1) { // machine shouldnt run if no preset
    this->machine_state->operational_state = MACHINE_PAUSED;
  }
  else {
    this->machine_state->operational_state = MACHINE_ACTIVE;
  }

  DynamicJsonDocument document(CONFIG_JSON_SIZE);
  this->serialize_preset(preset, document);

  this->flash.eraseSector(this->preset_address);
  return this->write_configuration_flash(this->preset_address, document);
}


/// @brief function to convert wifi info object to json serializable document object.
/// @param wifi_configuration wifi configuration object to convert
/// @param document document to write data to
/// @param password if this is set to false, the wifi password stored in memory will not be serialized, for security purposes
void ConfigManager::serialize_wifi_configuration(WifiInfo &wifi_configuration, DynamicJsonDocument &document, bool password = false) {
  document["type"] = wifi_configuration.type;
  document["ssid"] = wifi_configuration.ssid;
  document["username"] = wifi_configuration.username;
  if(password) {
    document["password"] = wifi_configuration.password;
  }
  document["channel"] = wifi_configuration.channel;
}


/// @brief write a wifi configuration to memory of ConfigManager object, and to flash for persistence
/// @param wifi_info wifi info object to write
/// @return bool to tell if the operation succeeded or not
bool ConfigManager::set_wifi_configuration(WifiInfo wifi_info) {
  this->config.wifi_information = wifi_info;

  DynamicJsonDocument document(CONFIG_JSON_SIZE);
  this->serialize_wifi_configuration(this->config.wifi_information, document);

  this->flash.eraseSector(this->wifi_address);
  return this->write_configuration_flash(this->wifi_address, document);
}



/// @brief function to convert device identifier object to json serializable json document object
/// @param device_identifiers device identifier object to be converted
/// @param document document object to write to 
void ConfigManager::serialize_device_identifiers(Identifiers &device_identifiers, DynamicJsonDocument &document) {
  document["device_id"] = device_identifiers.device_id;
  document["server_hostname"] = device_identifiers.server_hostname;
  document["device_name"] = device_identifiers.device_name;
  document["project_id"] = device_identifiers.project_id;
}


/// @brief convert from a json document back to an identifiers object
/// @param device_identifiers identifiers object to write to
/// @param document json document to read data from
void ConfigManager::deserialize_device_identifiers(Identifiers &device_identifiers, DynamicJsonDocument &document) {
  device_identifiers.device_id = document["device_id"];
  device_identifiers.project_id = document["project_name"];
  device_identifiers.device_name = (char *)&document["device_name"];
  device_identifiers.server_hostname = (char *)&document["server_name"];
}


/// @brief write device identifiers to the identifiers field of the ConfigManager and to flash memory for persistence. If the project ID is -1, eg if the project
/// is deleted or the device is removed from its project, the device enters a paused state
/// @param device_identifiers device identifiers object to write
/// @return bool to say if operation was successful
bool ConfigManager::set_device_identifiers(Identifiers device_identifiers) {
  this->config.identifying_information = device_identifiers;
  if(this->config.identifying_information.project_id == -1) { // machine shouldnt run if no preset
    this->machine_state->operational_state = MACHINE_PAUSED;
  }
  else {
    this->machine_state->operational_state = MACHINE_ACTIVE;
  }

  DynamicJsonDocument document(CONFIG_JSON_SIZE);
  this->serialize_device_identifiers(this->config.identifying_information, document);

  this->flash.eraseSector(this->identifier_address);
  return this->write_configuration_flash(this->identifier_address, document);
}


/// @brief reset the device and empty the flash memory. This should also be triggered when millis() overflows
void ConfigManager::reset() {
  this->flash.eraseChip();
  this->configured = false;

  digitalWrite(this->device_reset_pin, HIGH);
}


/// @brief custom destructor because data writer is heap allocated
ConfigManager::~ConfigManager() {
  delete this->writer;
}