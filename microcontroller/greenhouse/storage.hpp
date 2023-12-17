#ifndef STORAGE_HPP
#define STORAGE_HPP

#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <Arduino.h>
#include <SPIMemory.h>
#include "exceptions.hpp"

#define IDENTIFIER_ADDRESS 9000
#define PRESET_ADDRESS 5000
#define WIFI_ADDRESS 1000
#define DATA_STORAGE_START 13000
#define DATA_STORAGE_LIMIT 180000

#define BLOCK_SIZE 4000

#define CONFIG_JSON_SIZE 128

// DO BELOW
// there must be a memory sector for machine state, so that if the device disconnects while paused, it will stay paused after reconnecting

/// @brief global machine connection state for tracking if the device is connected to the server or not
enum NetworkState {
  MACHINE_CONNECTED,
  MACHINE_DISCONNECTED
};

/// @brief global machine operational state for tracking if the machine should be paused or not
enum MachineOperationalState {
    MACHINE_PAUSED = 'p', // in this case all devices should stop taking measurements
    MACHINE_ACTIVE = 'a' // in this case devices should send data to the server
};

/// @brief enum of possible wifi connection authentication types supported by the network management software
enum WifiNetworkTypes {
  WIFI_HOME = 'h',
  WIFI_ENTERPRISE = 'e',
  WIFI_OPEN = 'o'
};

typedef struct {
  float temperature;
  float humidity;
  float moisture;
  float light_exposure;
} CommonDataBuffer;

class ConfigStruct {
  private:
  int flash_address;
  SPIFlash *flash;
  bool is_configured = false;

  public:
  ConfigStruct() {};
  ConfigStruct(SPIFlash *flash, int flash_address);

  virtual DynamicJsonDocument to_json() = 0;
  virtual StorageException from_json(DynamicJsonDocument &data) = 0;

  void set_configured();
  void set_unconfigured();
  bool check_is_configured() const;

  StorageException write(DynamicJsonDocument &data);
  bool read();
  bool erase();
};

/// @brief global device state tracker for both connection and operational states. Critical for state driven operations on the device
class MachineState : public ConfigStruct {
  private:
  MachineOperationalState operational_state;

  public:
  MachineState(SPIFlash *flash, int flash_address);

  MachineOperationalState get_state() const;
  void set_state(MachineOperationalState state);

  StorageException from_json(DynamicJsonDocument &data);
  DynamicJsonDocument to_json();
};

class Preset : public ConfigStruct {
  private:
  float temperature;
  float humidity;
  float moisture;
  float hours_daylight;
  int preset_id = -1;

  public:
  Preset() {};
  Preset(SPIFlash *flash, int flash_address);

  float get_temperature() const;
  float get_humidity() const;
  float get_moisture() const;
  float get_hours_daylight() const;
  float get_preset_id() const;

  StorageException from_json(DynamicJsonDocument &data) override;
  DynamicJsonDocument to_json() override;
};

class Identifiers : public ConfigStruct {
  private:
  int device_id = -1;
  int project_id = -1;
  String device_name;

  public:
  Identifiers() {};
  Identifiers(SPIFlash *flash, int flash_address);

  int get_device_id() const;
  int get_project_id() const;
  String get_device_name() const;

  StorageException from_json(DynamicJsonDocument &data) override;
  DynamicJsonDocument to_json() override;
};

class WifiInfo : public ConfigStruct {
  private:
  WifiNetworkTypes type;
  int channel;
  String ssid;
  String username = "";
  String password = "";

  public:
  WifiInfo::WifiInfo(SPIFlash *flash, int flash_address);
  WifiInfo() {};
  WifiInfo(String ssid, int channel);
  WifiInfo(String ssid, int channel, String password);
  WifiInfo(String ssid, int channel, String password, String username);

  WifiNetworkTypes get_type() const;
  String get_ssid() const;
  String get_username() const;
  String get_password() const;
  int get_channel() const;

  bool copy(WifiInfo to_copy);

  StorageException from_json(DynamicJsonDocument &data) override;
  DynamicJsonDocument to_json() override;
};

class DataManager {
  private:
  SPIFlash *flash;

  int current;
  int partition_size = CONFIG_JSON_SIZE + 10;

  int flash_address;
  int max_size;

  bool is_full = false;
  bool is_storing;

  public:
  float reference_datetime = 0;

  DataManager(SPIFlash *flash, int start_address, int max_size);
  void set_reference_datetime(int timestamp);
  int get_end_address() const;

  bool check_is_storing() const;
  
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
  MachineState machine_state;
  NetworkState network_state;
  CommonDataBuffer common_data;

  SPIFlash flash;

  public:
  bool configured = false; // this must be set when the configuration is read at startup. Should also be set to true as soon as configuration data is written

  StorageManager(int start_address, int data_size, int device_reset_pin);

  DataManager& get_data_manager();
  WifiInfo& get_wifi();
  Preset& get_preset();
  Identifiers& get_identifiers();
  MachineState& get_machine_state();

  void load_flash_configuration();
  bool write_flash_configuration(ConfigType configuration, DynamicJsonDocument &to_write);
  bool write_flash_configuration(WifiInfo wifi_info);

  void set_network_state(NetworkState state);
  NetworkState get_network_state() const;

  void set_common_data(CommonDataBuffer &common);
  CommonDataBuffer get_common_data() const;

  void hard_reset();
};

#endif