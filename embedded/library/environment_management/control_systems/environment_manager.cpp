/// @brief environment manager manages all peripherals, including sensors and control systems
/// @param machine_state global machine state controlling state driven operations
/// @param config configmanager config field, storing the preset information necessary for control operations
/// @param common_data global common data struct to feed sensor data into
/// @param pump_pin physical pin for the pump system
/// @param heating_pin physical pin for the heating system
/// @param fan_pin physical pin for the fan system
/// @param led_pin physical pin for the LED system
/// @param desired_temperature desired temperature to maintain
/// @param desired_humidity desired humidity percentage to maintain
/// @param desired_moisture desired moisture to maintain
/// @param hours_sunlight hours sunlight to achieve per 24 hour period
EnvironmentManager::EnvironmentManager(StorageManager *global_storage, int pump_pin, int heater_pin, int fan_pin, int led_pin, int num_led) : global_storage(global_storage), pump(pump_pin), heater(heater_pin), fan(fan_pin), led(led_pin, num_led) {}

void EnvironmentManager::set_devices_low(Device devices[]) {
  for(int x = 0; x < sizeof(devices) / sizeof(Device); x++) {
    devices[x].set_low();
  }
}

void EnvironmentManager::set_devices_high(Device devices[]) {
  for(int x = 0; x < sizeof(devices) / sizeof(Device); x++) {
    devices[x].set_high();
  }
}

void EnvironmentManager::set_device_statuses(MeasurementCompliance current_measurement_classification, Device set_high_when_higher[], Device set_low_when_higher[]) {
  switch(current_measurement_classification) {
    case HIGHER:
      this->set_devices_high(set_high_when_higher);
      this->set_devices_low(set_low_when_higher);
      break;
    case LOWER:
      this->set_devices_high(set_low_when_higher);
      this->set_devices_low(set_high_when_higher);
      break;
    case COMPLIANT:
      this->set_devices_low(set_low_when_higher);
      this->set_devices_low(set_high_when_higher);
      break;
  }
}

/// @brief main function for environment manager, reads all sensor data and activates or deactivates control systems accordingly
void EnvironmentManager::device_activation() {
  Preset& preset = this->global_storage->get_preset();
  CommonDataBuffer& common_data = this->global_storage->get_common_data();

  long long desired_ms_daylight = hours_ms(this->global_storage->get_preset().get_hours_daylight());

  // check conditions for light
  if(common_data.is_night_time() && desired_ms_daylight > common_data.get_ms_light() && !this->led.status) {
    this->led.set_strip_high();
  }
  else if(this->led.status){
    common_data.increment_light(true);
  }
  else if(desired_ms_daylight <= common_data.get_ms_light() || !common_data.is_night_time()) {
    this->led.set_strip_low();
  }

  // check conditions for temperature
  set_device_statuses(this->global_storage->get_preset().check_temperature_compliance(common_data.get_temperature()), {&this->fan}, {&this->heater});
  set_device_statuses(this->global_storage->get_preset().check_humidity_compliance(common_data.get_humidity()), {&this->fan}, {&this->pump});
  set_device_statuses(this->global_storage->get_preset().check_moisture_compliance(common_data.get_moisture()), {}, {&this->pump});
}


/// @brief pseudo asynchronous callback for the environment manager
void EnvironmentManager::callback() {
  this->device_activation();
  if(this->global_storage->get_machine_state().get_state() != MACHINE_PAUSED) {
    this->device_activation();
  }
}