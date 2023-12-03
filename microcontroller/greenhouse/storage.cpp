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
      break;
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

bool ConfigManager::write_configuration_flash(int address, DynamicJsonDocument &document) {
  String serialized;
  serializeJson(document, serialized);
  this->flash.eraseSector(address);
  return this->flash.writeStr(address, serialized);
}

void ConfigManager::serialize_preset(Preset &preset, DynamicJsonDocument &document) {
  document["preset_name"] = preset.preset_name;
  document["temperature"] = preset.temperature;
  document["humidity"] = preset.humidity;
  document["moisture"] = preset.moisture;
  document["hours_daylight"] = preset.hours_daylight;
}

bool ConfigManager::set_preset(Preset preset) {
  this->config.preset = preset;

  DynamicJsonDocument document(CONFIG_JSON_SIZE);
  this->serialize_preset(preset, document);

  this->flash.eraseSector(this->preset_address);
  return this->write_configuration_flash(this->preset_address, document);
}

void ConfigManager::serialize_wifi_configuration(WifiInfo &wifi_configuration, DynamicJsonDocument &document, bool password = false) {
  document["type"] = wifi_configuration.type;
  document["ssid"] = wifi_configuration.ssid;
  document["username"] = wifi_configuration.username;
  document["password"] = wifi_configuration.password;
  document["channel"] = wifi_configuration.channel;
}

bool ConfigManager::set_wifi_configuration(WifiInfo wifi_info) {
  this->config.wifi_information = wifi_info;

  DynamicJsonDocument document(CONFIG_JSON_SIZE);
  this->serialize_wifi_configuration(this->config.wifi_information, document);

  this->flash.eraseSector(this->wifi_address);
  return this->write_configuration_flash(this->wifi_address, document);
}

void serialize_device_identifiers(Identifiers &device_identifiers, DynamicJsonDocument &document) {
  document["device_id"] = device_identifiers.device_id;
  document["server_hostname"] = device_identifiers.server_hostname;
  document["device_name"] = device_identifiers.device_name;
  document["project_name"] = device_identifiers.project_name;
}

bool ConfigManager::set_device_identifiers(Identifiers device_identifiers) {
  this->config.identifying_information = device_identifiers;

  DynamicJsonDocument document(CONFIG_JSON_SIZE);
  this->serialize_device_identifiers(this->config.identifying_information, document);

  this->flash.eraseSector(this->identifier_address);
  return this->write_configuration_flash(this->identifier_address, document);
}

void ConfigManager::reset() {
  this->flash.eraseChip();
  this->configured = false;
}