#include "storage.hpp"

// MUST MODERNIZE ALL THIS SO THAT IT WORKS BETTER IN CONJUNCTION WITH THE STRUCT MEMBERS
ConfigManager::ConfigManager(int configAddr, int webpageAddr) : configAddr(configAddr), webpageAddr(webpageAddr) {
  this->flash.begin();
}

bool ConfigManager::retrieve_configuration_flash(String *output) {
  return this->flash.readStr(this->configAddr, *output);
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

bool ConfigManager::write_configuration_flash(DynamicJsonDocument &document) {
  this->config.device_name = *this->check_data(document["device_name"], &this->config.device_name);
  this->config.device_name = *this->check_data(document["device_id"], &this->config.device_id);
  this->config.device_name = *this->check_data(document["project_name"], &this->config.project_name);
  this->config.device_name = *this->check_data(document["preset_name"], &this->config.preset.PresetName);

  String serialized;
  serializeJson(document, serialized);
  this.flash.eraseSector(this->configAddr);
  return this->flash.writeStr(this->configAddr, serialized);
}