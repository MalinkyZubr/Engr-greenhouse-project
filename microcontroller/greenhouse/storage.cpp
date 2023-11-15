#include "storage.hpp"


ConfigManager::ConfigManager() {
  this->flash.begin();
}
void ConfigManager::de_power() {
  this->flash.powerDown();
}
bool ConfigManager::power(String outputString) {
  this->flash.powerUp();
  return retrieve_configuration(outputString);
}
bool ConfigManager::retrieve_configuration(String outputString) {
  for(int x = 0; x < 3; x++)
  {
    if(this->flash.readStr(this->configAddr, outputString))
    {
      return true;
    }
  }
  return false;
}
bool ConfigManager::write_configuration(DynamicJsonDocument toSerialize) {
  String serialized = "";
  serializeJson(toSerialize, serialized);
  for(int x = 0; x < 3; x++)
  {
    if(this->flash.writeStr(this->configAddr, serialized))
    {
      return true;
    }
  }
  return false;
}