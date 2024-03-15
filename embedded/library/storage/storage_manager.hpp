#ifndef STORAGE_HPP
#define STORAGE_HPP

#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <Arduino.h>
#include <SPIMemory.h>
#include "../exceptions.hpp"

#define LUX_THRESHOLD 800

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
  bool write_flash_configuration(ConfigType configuration, const DynamicJsonDocument &to_write);
  bool write_flash_configuration(WifiInfo wifi_info);

  void set_network_state(NetworkState state);
  NetworkState get_network_state() const;

  void set_common_data(CommonDataBuffer &common);
  CommonDataBuffer& get_common_data();

  void hard_reset();
};

#endif