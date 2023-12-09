#ifndef STORAGE_HPP
#define STORAGE_HPP

#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <Arduino.h>
#include <SPIMemory.h>
#include "wifi_info.hpp"
#include "machine_state.hpp"

#define IDENTIFIER_ADDRESS 9000
#define PRESET_ADDRESS 5000
#define WIFI_ADDRESS 1000
#define DATA_STORAGE_START 13000
#define DATA_STORAGE_LIMIT 180000

#define BLOCK_SIZE 4000

#define CONFIG_JSON_SIZE 128

// there must be a memory sector for machine state, so that if the device disconnects while paused, it will stay paused after reconnecting

class ConfigStruct {
  private:
  int flash_address;
  SPIFlash *flash;

  public:
  ConfigStruct() {};
  ConfigStruct(SPIFlash *flash, int flash_address);

  virtual DynamicJsonDocument to_json() = 0;
  virtual void from_json(DynamicJsonDocument &data) = 0;

  bool write(DynamicJsonDocument &data);
  bool read();

  bool erase();
};

class Preset : public ConfigStruct {
  private:
  float temperature;
  float humidity;
  float moisture;
  float hours_daylight;
  int preset_id = -1;

  public:
  Preset(SPIFlash *flash, int flash_address);

  float get_temperature();
  float get_humidity();
  float get_moisture();
  float get_hours_daylight();
  float get_preset_id();

  void from_json(DynamicJsonDocument &data) override;
  DynamicJsonDocument to_json() override;
};

class Identifiers : public ConfigStruct {
  private:
  int device_id = -1;
  int project_id = -1;
  String device_name;

  public:
  Identifiers(SPIFlash *flash, int flash_address);
  int get_device_id();
  int get_project_id();
  String get_device_name();

  void from_json(DynamicJsonDocument &data) override;
  DynamicJsonDocument to_json() override;
};

class WifiInfo : public ConfigStruct{
  private:
  NetworkTypes type;
  int channel;
  String ssid;
  String username = "";
  String password = "";

  public:
  WifiInfo::WifiInfo(SPIFlash *flash, int flash_address);
  WifiInfo(String ssid, int channel);
  WifiInfo(String ssid, int channel, String password);
  WifiInfo(String ssid, int channel, String password, String username);

  NetworkTypes get_type();
  String get_ssid();
  String get_username();
  String get_password();
  int get_channel();

  bool copy(WifiInfo &to_copy);

  void from_json(DynamicJsonDocument &data) override;
  DynamicJsonDocument to_json() override;
};

class DataManager {
  private:
  SPIFlash *flash;

  int current;
  int partition_size = CONFIG_JSON_SIZE + 10;

  int flash_address;
  int max_size;

  public:
  float reference_datetime = 0;
  bool is_full = false;
  bool is_storing;

  DataManager(SPIFlash *flash, int start_address, int max_size);
  void set_reference_datetime(int timestamp);
  int get_end_address();
  
  bool write_data(DynamicJsonDocument &data);
  bool read_data(DynamicJsonDocument &data_output);
  bool erase_data();
};


class DeviceResetter {
  private:
  int reset_pin;

  public:
  DeviceResetter(int reset_pin);
  void trigger_reset();
};


enum ConfigType {
  IDENTIFIER,
  WIFI,
  PRESET
};


class StorageManager {
  private:
  MachineState machine_state;
  DeviceResetter resetter;

  DataManager data_manager;
  
  WifiInfo wifi_info;
  Preset preset_info;
  Identifiers identifier_info;

  SPIFlash flash;

  public:
  bool configured = false; // this must be set when the configuration is read at startup. Should also be set to true as soon as configuration data is written

  StorageManager(int start_address, int data_size, int device_reset_pin);

  void load_flash_configuration();
  bool write_flash_configuration(ConfigType configuration, DynamicJsonDocument &to_write);
  bool write_flash_configuration(WifiInfo &wifi_info);

  void hard_reset();
};

#endif