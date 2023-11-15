#include "sensing.hpp"


Moisture::Moisture(int pin) {
  this->pin = pin;
  pinMode(this->pin, INPUT);
}

Moisture::get_moisture_percentage() {
  long result;
  int raw_data;

  raw_data = analogRead(this->pin);
  result = map(raw_data, 0, 1023, 0, 100);

  return result;
}

Light::Light() {
  if(this->light_sensor.begin()) {
    Serial.println("BH1750 Light Sensor Initialized");
  }
  else {
    Serial.println("BH1750 Light Sensor Failure");
  }
}

float Light::get_reading() {
  return this->light_sensor.readLightLevel();
}

HumidityTemperature::HumidityTemperature(int pin) : dht_sensor(pin, DHT11) {
  this->pin = pin;
  this->dht_sensor.begin();
}

HumidityTemperature::EnvReading HumidityTemperature::get_temp_humidity() {
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


