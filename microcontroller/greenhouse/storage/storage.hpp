#ifndef STORAGE_HPP
#define STORAGE_HPP

#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <Arduino.h>
#include <SPIMemory.h>
#include "../network/wifi_info.hpp"
#include "../asynchronous/machine_state.hpp"

#define IDENTIFIER_ADDRESS 9000
#define PRESET_ADDRESS 5000
#define WIFI_ADDRESS 1000
#define DATA_STORAGE_START 13000
#define DATA_STORAGE_LIMIT 180000

#define ERASE_BLOCK_SIZE 4000

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

class DataWriter {
  private:
  long current;

  long data_storage_start = DATA_STORAGE_START;

  int partition_size = CONFIG_JSON_SIZE + 10;

  SPIFlash *flash;

  public:
  float reference_datatime = 0;
  bool is_full = false;
  bool is_storing;

  DataWriter(SPIFlash *flash);
  
  bool write_data(DynamicJsonDocument &data);
  bool decrement_read(DynamicJsonDocument &data_output);
  bool erase_all_data();
};

class ConfigManager {
  private:
  MachineState *machine_state;

  SPIFlash flash;

  bool retrieve_configuration_flash(int address, String *output);
  bool write_configuration_flash(int address, DynamicJsonDocument &document);

  public:
  int identifier_address = IDENTIFIER_ADDRESS;
  int preset_address = PRESET_ADDRESS;
  int wifi_address = WIFI_ADDRESS;
  
  bool configured = false; // this must be set when the configuration is read at startup. Should also be set to true as soon as configuration data is written
  
  Configuration config;
  DataWriter *writer;

  ConfigManager(MachineState *machine_state);
  ~ConfigManager();

  void retrieve_config_to_json(int address, DynamicJsonDocument &document);

  void set_reference_datetime(float datetime);

  void serialize_preset(Preset &preset, DynamicJsonDocument &document);
  void deserialize_preset(Preset &preset, DynamicJsonDocument &document);
  bool set_preset(Preset preset);

  void serialize_wifi_configuration(WifiInfo &wifi_info, DynamicJsonDocument &document, bool password);
  bool set_wifi_configuration(WifiInfo wifi_info);

  void serialize_device_identifiers(Identifiers &device_identifiers, DynamicJsonDocument &document);
  void deserialize_device_identifiers(Identifiers &device_identifiers, DynamicJsonDocument &document);
  bool set_device_identifiers(Identifiers device_identifiers);

  void load_device_identifiers(DynamicJsonDocument &document, Identifiers &identifiers);

  void load_wifi_info(DynamicJsonDocument &document, WifiInfo &wifi_info);

  void load_preset_info(DynamicJsonDocument &document, Preset &preset);

  void reset();
};

#endif