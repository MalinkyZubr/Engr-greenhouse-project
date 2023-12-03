#ifndef STORAGE_HPP
#define STORAGE_HPP

#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <Arduino.h>
#include <SPIMemory.h>
#include "wifi_info.hpp"
#include "machine_state.hpp"

#define IDENTIFIER_ADDRESS 12000
#define PRESET_ADDRESS 7000
#define WIFI_ADDRESS 2000

#define CONFIG_JSON_SIZE 256


typedef struct {
  float temperature;
  float humidity;
  float moisture;
  float hours_daylight;
  String preset_name;
} Preset;

typedef struct {
  int device_id = -1;
  String server_hostname;
  String device_name;
  String project_name;
} Identifiers;

typedef struct {
  Identifiers identifying_information;
  WifiInfo wifi_information;
  Preset preset;
} Configuration;

class ConfigManager {
  private:
  int identifier_address = IDENTIFIER_ADDRESS;
  int preset_address = PRESET_ADDRESS;
  int wifi_address = WIFI_ADDRESS;

  MachineState *machine_state;

  SPIFlash flash;

  bool retrieve_configuration_flash(int address, String *output);
  bool write_configuration_flash(int address, DynamicJsonDocument &document);

  public:
  bool configured = false; // this must be set when the configuration is read at startup. Should also be set to true as soon as configuration data is written
  Configuration config;

  ConfigManager(MachineState *machine_state);

  void serialize_preset(Preset &preset, DynamicJsonDocument &document);
  bool set_preset(Preset preset);

  void serialize_wifi_configuration(WifiInfo &wifi_info, DynamicJsonDocument &document, bool password);
  bool set_wifi_configuration(WifiInfo wifi_info);

  void serialize_device_identifiers(Identifiers &device_identifiers, DynamicJsonDocument &document);
  bool set_device_identifiers(Identifiers device_identifiers);

  void load_device_identifiers(DynamicJsonDocument &document);

  void load_wifi_info(DynamicJsonDocument &document);

  void load_preset_info(DynamicJsonDocument &document);

  void reset();
};

#endif