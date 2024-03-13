/// @brief DHT sensor initializer
/// @param pin pin to intiailize DHT sensor on
HumidityTemperature::HumidityTemperature(int pin) : dht_sensor(pin, DHT11) {
  this->pin = pin;
  this->dht_sensor.begin();
}


/// @brief sense temperature and humidity levels
/// @return return a struct containing both humidity and temperature
HumidityTemperature::EnvReading HumidityTemperature::sense() {
  float humidity = this->dht_sensor.readHumidity();
  float temperature = this->dht_sensor.readTemperature();

  if(isnan(humidity)) {
    Serial.println("Humidity reading failed");
  }
  if(isnan(temperature)) {
    Serial.println("Temperature reading failed");
  }

  float celsius_adjusted = this->dht_sensor.computeHeatIndex(temperature, humidity, false);

  EnvReading environment_reading;
  environment_reading.humidity_percent = humidity;
  environment_reading.temperature_c = celsius_adjusted;

  return environment_reading;
}