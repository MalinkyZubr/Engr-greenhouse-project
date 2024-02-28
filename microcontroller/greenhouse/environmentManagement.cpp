#include "environmentManagement.hpp"


/// @brief helper function for converting hours to milliseconds
/// @param hours number of hours to convert to ms
/// @return milliseconds in the specified number of hours
long long hours_ms(int hours) {
  long long milliseconds;
  milliseconds = MILLISECONDS_IN_HOUR * hours;
  return milliseconds;
}

/// @brief base object for environment management devices
/// @param pin pin to set for the device
Device::Device(int pin) : pin(pin), status(false) {}


/// @brief digital write the device to high to activate it
void Device::set_high() {
  this->status = true;
  digitalWrite(this->pin, HIGH);
}


/// @brief digital write the device to low to deactivate it
void Device::set_low() {
  this->status = false;
  digitalWrite(this->pin, LOW);
}


/// @brief function to set the status of the LED strip on the greenhouse unit
/// @param color the color array (RGB) to set the LED ring to
/// @param delay_time the delay value between the setting of each pixel
/// @param sequence flag to say if the call of the function is part of a sequence. If this is true, the led strip will be set low after finishing execution
void LedStrip::set_strip(int color[], int delay_time, bool sequence) {
  for(int pixel = 0; pixel < this->led_count; pixel++)
  {
    this->led_strip.setPixelColor(pixel, this->led_strip.Color(
      color[0],
      color[1],
      color[2]
    ));
    delay(delay_time);
    this->led_strip.show();
  }
  if(sequence){
    this->set_strip_low();
  }
}


/// @brief startup animation sequence for LED ring
void LedStrip::startup() {
  for(int sequence = 0; sequence < 3; sequence++)
  {
    this->set_strip(this->startup_sequence_colors[sequence], 50, true);
  }
  for(int sequence = 0; sequence < 3; sequence++)
  {
    this->set_strip(this->standard_color, 0, false);
    delay(200);
    this->set_strip_low();
    delay(200);
  }
}


/// @brief set all pixels to be the standard operation color
void LedStrip::set_strip_high() {
  this->set_strip(this->standard_color, 0, false);
  this->status = true;
}


/// @brief set all pixels on the ring to be dark
void LedStrip::set_strip_low() {
  this->led_strip.clear();
  this->led_strip.show();
  this->status = false;
}


/// @brief constructor for the LED ring controller object
/// @param pin the control pin for the led ring
LedStrip::LedStrip(int pin, int num_led) : led_count(num_led) {
  this->led_strip = Adafruit_NeoPixel(this->led_count, this->pin, NEO_GRB + NEO_KHZ800);
  this->led_strip.clear();
  this->startup();
}


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


/// @brief object for managing the moisture sensor
/// @param pin physical pin to assign
Moisture::Moisture(int pin) {
  this->pin = pin;
  pinMode(this->pin, INPUT);
}


/// @brief get the moisture reading from the sensor
/// @return the % moisture in the soil
float Moisture::sense() {
  long result;
  int raw_data;

  raw_data = analogRead(this->pin);
  result = map(raw_data, 0, 1023, 0, 100);

  return result;
}


/// @brief light sensor. No pin because I2C based
Light::Light() {
  if(this->light_sensor.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    this->status = true;
  }
  else {
    this->status = false;
  }
}


/// @brief read current light exposure levels
/// @return the current light reading in lux
float Light::sense() {
  return this->light_sensor.readLightLevel();
}


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