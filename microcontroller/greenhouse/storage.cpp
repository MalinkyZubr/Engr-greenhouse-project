#include "storage.hpp"

// MUST MODERNIZE ALL THIS SO THAT IT WORKS BETTER IN CONJUNCTION WITH THE STRUCT MEMBERS
ConfigManager::ConfigManager() {
  this->flash.begin();
  DynamicJsonDocument doc(CONFIG_JSON_SIZE);
  
  this->load_device_identifiers(doc);
  doc.clear();
  this->load_preset_info(doc);
  doc.clear();
  this->load_wifi_info(doc);
  doc.clear();

  if(this->config.identifying_information.device_id != -1) {
    this->configured = true;
  }
}

bool ConfigManager::retrieve_configuration_flash(int address, String *output) {
  return this->flash.readStr(address, *output);
}

void ConfigManager::load_device_identifiers(DynamicJsonDocument &document) {
  String temp_string;
  this->retrieve_configuration_flash(this->identifier_address, &temp_string);

  if(temp_string.equals("")) {
    return;
  }

  deserializeJson(document, temp_string);

  this->config.identifying_information.device_id = document["device_id"];
  this->config.identifying_information.project_name = (char *)document["project_name"];
  this->config.identifying_information.device_name = (char *)document["device_name"];
  this->config.identifying_information.server_hostname = (char *)document["server_name"];
}

void ConfigManager::load_wifi_info(DynamicJsonDocument &document) {
  String temp_string;
  this->retrieve_configuration_flash(this->identifier_address, &temp_string);

  if(temp_string.equals("")) {
    return;
  }

  deserializeJson(document, temp_string);

  char type = document["type"];
  switch(type) {
    case 'h':
      this->config.wifi_information.type = HOME;
      this->config.wifi_information.password = (char *)document["password"];
      break;
    case 'e':
      this->config.wifi_information.type = ENTERPRISE;
      this->config.wifi_information.password = (char *)document["password"];
      this->config.wifi_information.username = (char *)document["username"];
    case 'o':
      this->config.wifi_information.type = OPEN;
  } 
  this->config.wifi_information.channel = document["channel"];

  this->config.wifi_information.ssid = (char *)document["ssid"];
}

void ConfigManager::load_preset_info(DynamicJsonDocument &document) {
  String temp_string;
  this->retrieve_configuration_flash(this->preset_address, &temp_string);

  if(temp_string.equals("")) {
    return;
  }

  deserializeJson(document, temp_string);

  this->config.preset.preset_name = (char *)document["preset_name"];
  this->config.preset.temperature = document["temperature"];
  this->config.preset.humidity = document["humidity"];
  this->config.preset.moisture = document["moisture"];
  this->config.preset.hours_daylight = document["hours_daylight"];
}

String *ConfigManager::check_data(String *arriving_data, String *existing_data) {
  if(arriving_data == nullptr) {
    return existing_data;
  }
  return arriving_data;
}

int *ConfigManager::check_data(int *arriving_data, int *existing_data) {
  if(arriving_data == nullptr) {
    return existing_data;
  }
  return arriving_data;
}

// bool ConfigManager::write_configuration_flash(DynamicJsonDocument &document) {
//   this->config.device_name = *this->check_data(document["device_name"], &this->config.device_name);
//   this->config.device_name = *this->check_data(document["device_id"], &this->config.device_id);
//   this->config.device_name = *this->check_data(document["project_name"], &this->config.project_name);
//   this->config.device_name = *this->check_data(document["preset_name"], &this->config.preset.PresetName);

//   String serialized;
//   serializeJson(document, serialized);
//   this->flash.eraseSector(this->configAddr);
//   return this->flash.writeStr(this->configAddr, serialized);
// }

bool ConfigManager::set_preset(Preset preset) {
  this->config.preset = preset;
}

bool ConfigManager::set_wifi_configuration(WifiInfo wifi_info) {
  this->config.wifi_information = wifi_info;
}

bool ConfigManager::set_device_identifiers(String server_hostname, String device_name, String project_name) {

}