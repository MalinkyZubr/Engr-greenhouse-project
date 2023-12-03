#include "storage.hpp"


ConfigManager::ConfigManager(int configAddr) : configAddr(configAddr) {
  this->flash.begin();
}

void ConfigManager::de_power() {
  this->flash.powerDown();
}

bool ConfigManager::power(String *output) {
  this->flash.powerUp();
  return retrieve_configuration(output);
}

bool ConfigManager::retrieve_configuration(String *output) {
  return flash.readStr(this->configAddr, *output);
}

bool ConfigManager::write_configuration(DynamicJsonDocument *document) {
  String serialized;
  serializeJson(*document, serialized);
  return this->flash.writeStr(this->configAddr, serialized);
}