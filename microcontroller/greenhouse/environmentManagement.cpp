#include "environmentManagement.hpp"


/// @brief helper function for converting hours to milliseconds
/// @param hours number of hours to convert to ms
/// @return milliseconds in the specified number of hours
long long hours_ms(int hours) {
  long long milliseconds;
  milliseconds = MILLISECONDS_IN_HOUR * hours;
  return milliseconds;
}


Interval::Interval(float desired_value) {
  this->set_interval(desired_value);
}


void Interval::set_interval(float desired_value) {
  this->upper = desired_value + 1;
  this->lower = desired_value - 1;
}


/// @brief function to check whether a value is within the data interval or not
/// @param value the value to check compliance for
/// @return Interval::relation: enum to say if the value is higher, lower, than the desired range, or within the range
Interval::relation Interval::check_current_value(float value) {
  if(value < this->lower) {
    return LOWER;
  }
  else if(value > this->upper) {
    return HIGHER;
  }
  else {
    return OKAY;
  }
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
LedStrip::LedStrip(int pin) {
  this->led_strip = Adafruit_NeoPixel(this->led_count, this->pin, NEO_GRB + NEO_KHZ800);
  this->led_strip.clear();
  this->startup();
}


/// @brief grabs the preset info from storage manager's Configuraiton object and loads them to this object
void EnvironmentManager::update_interfaces() {
  this->temperature.set_interval(this->config->preset.temperature);
  this->humidity.set_interval(this->config->preset.humidity);
  this->moisture.set_interval(this->config->preset.moisture);
  this->hours_sunlight_ms = hours_ms(this->config->preset.hours_daylight);
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
EnvironmentManager::EnvironmentManager(MachineState *machine_state, Configuration *config, CommonData *common_data, int pump_pin, int heating_pin, int fan_pin, int led_pin, float desired_temperature, float desired_humidity, float desired_moisture, int hours_sunlight) : machine_state(machine_state), config(config), common_data(common_data), pump(pump_pin), heater(heating_pin), fan(fan_pin), led(led_pin), temperature(desired_temperature), humidity(desired_humidity), moisture(desired_moisture) {}


/// @brief main function for environment manager, reads all sensor data and activates or deactivates control systems accordingly
void EnvironmentManager::device_activation() {
  // check conditions for light
  if(this->common_data->nighttime && this->hours_sunlight_ms > (this->common_data->light_increment * CONTROL_INTERVAL * 1000) && !this->led.status) {
    this->led.set_strip_high();
  }
  else if(this->led.status){
    this->common_data->light_increment++;
  }
  else if((this->hours_sunlight_ms <= (this->common_data->light_increment * CONTROL_INTERVAL * 1000)) || !this->common_data->nighttime) {
    this->led.set_strip_low();
  }

  // check conditions for temperature
  switch(this->temperature.check_current_value(this->common_data->temperature)) {
    case Interval::HIGHER:
      this->heater.set_low();
      this->fan.set_high();
      break;
    case Interval::LOWER:
      this->heater.set_high();
      this->fan.set_low();
      break;
    case Interval::OKAY:
      this->heater.set_low();
      this->fan.set_low();
      break;
  }

  // check humidity conditions
  switch(this->humidity.check_current_value(this->common_data->humidity)) {
    case Interval::HIGHER:
      this->pump.set_high();
      this->fan.set_low();
      break;
    case Interval::LOWER:
      this->pump.set_low();
      this->fan.set_high();
      break;
    case Interval::OKAY:
      this->pump.set_low();
      this->fan.set_low();
      break;
  }
}


/// @brief pseudo asynchronous callback for the environment manager
void EnvironmentManager::callback() {
  this->update_interfaces();
  if(this->machine_state->operational_state != MACHINE_PAUSED) {
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
Sensors::Sensors(MachineState *machine_state, CommonData *common_data, int humtemp_pin, int moisture_pin) : machine_state(machine_state), common_data(common_data), humtemp(humtemp_pin), moisture(moisture_pin) {}


/// @brief write sensor readings to the global common_data struct
void Sensors::submit_readings() {
  if(this->machine_state->operational_state != MACHINE_PAUSED) {
    HumidityTemperature::EnvReading environment_reading = this->humtemp.sense();

    this->common_data->moisture = this->moisture.sense();
    this->common_data->temperature = environment_reading.temperature_c;
    this->common_data->humidity = environment_reading.humidity_percent;
    this->common_data->light_level = this->light.sense();

    if((this->common_data->light_level) >= LUX_THRESHOLD) {
      this->common_data->light_increment++;
    }
  }
}