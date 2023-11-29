#include "storage.hpp"

// MUST MODERNIZE ALL THIS SO THAT IT WORKS BETTER IN CONJUNCTION WITH THE STRUCT MEMBERS
ConfigManager::ConfigManager(int configAddr, int webpageAddr) : configAddr(configAddr), webpageAddr(webpageAddr) {
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