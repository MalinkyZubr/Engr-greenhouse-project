/// @brief manager class for all sensors
/// @param machine_state global machine state to read from to run state based operations
/// @param common_data global common data struct for outputting sensor readings to
/// @param humtemp_pin physical pin for DHT sensor
/// @param moisture_pin physical pin for moisture sensor
Sensors::Sensors(StorageManager *global_storage, int humtemp_pin, int moisture_pin) : global_storage(global_storage), humtemp(humtemp_pin), moisture(moisture_pin) {}

/// @brief write sensor readings to the global common_data struct
void Sensors::submit_readings() {
  CommonDataBuffer& common_data = this->global_storage->get_common_data();

  HumidityTemperature::EnvReading environment_reading = this->humtemp.sense();

  common_data.set_common_data(environment_reading.temperature_c, environment_reading.humidity_percent, this->moisture.sense(), this->light.sense());
}