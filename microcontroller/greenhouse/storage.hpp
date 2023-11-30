#ifndef STORAGE_HPP
#define STORAGE_HPP

#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <Arduino.h>
#include <SPIMemory.h>

#define CONFIG_ADDRESS 4000
#define CONFIG_JSON_SIZE 256
#define WEBPAGE_ADDRESS 5000

typedef struct {
  float Temperature;
  float Humidity;
  float Moisture;
  float LightExposure;
  float IRExposure;
  String PresetName;
} Preset;

typedef struct {
  int device_id;
  String server_hostname;
  String device_name;
  String project_name;
  Preset preset;
} Configuration;

class ConfigManager {
  private:
  int configAddr = CONFIG_ADDRESS;
  int webpageAddr = WEBPAGE_ADDRESS;
  SPIFlash flash;

  bool retrieve_configuration_flash(String *output);
  bool write_configuration_flash(DynamicJsonDocument &document);

  String *check_data(String *arriving_data, String *existing_data);
  int *check_data(int *arriving_data, int *existing_data);

  public:
  Configuration config;

  ConfigManager(void){};
  ConfigManager(int configAddr, int webpageAddr);


};

#endif