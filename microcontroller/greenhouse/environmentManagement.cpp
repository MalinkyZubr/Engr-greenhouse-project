#include "environmentManagement.hpp"


long long hours_ms(int hours) {
  long long milliseconds;
  milliseconds = MILLISECONDS_IN_HOUR * hours;
  return milliseconds;
}

Interval::Interval(float upper, float lower) : upper(upper), lower(lower) {}

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

Device::Device(int pin) : pin(pin), status(false) {}

void Device::set_high() {
  this->status = true;
  digitalWrite(this->pin, HIGH);
}

void Device::set_low() {
  this->status = false;
  digitalWrite(this->pin, LOW);
}

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

void LedStrip::set_strip_high() {
  this->set_strip(this->standard_color, 0, false);
  this->status = true;
}

void LedStrip::set_strip_low() {
  this->led_strip.clear();
  this->led_strip.show();
  this->status = false;
}

LedStrip::LedStrip(int pin) {
  this->led_strip = Adafruit_NeoPixel(this->led_count, this->pin, NEO_GRB + NEO_KHZ800);
  this->led_strip.clear();
  this->startup();
}

EnvironmentManager::Intervals::Intervals(float desired_temperature, float desired_humidity, float desired_moisture, int hours_sunlight) : temperature(desired_temperature - 2, desired_temperature + 2),  humidity(desired_humidity - 2, desired_humidity + 2), moisture(desired_moisture - 2, desired_moisture + 2), hours_sunlight_ms(hours_ms(hours_sunlight)){}

EnvironmentManager::EnvironmentManager(MachineState *machine_state, CommonData *common_data, int pump_pin, int heating_pin, int fan_pin, int led_pin, float desired_temperature, float desired_humidity, float desired_moisture, int hours_sunlight) : machine_state(machine_state), common_data(common_data), intervals(desired_temperature, desired_humidity, desired_moisture, hours_sunlight), pump(pump_pin), heater(heating_pin), fan(fan_pin), led(led_pin) {}

void EnvironmentManager::device_activation() {
  // check conditions for light
  if(this->machine_state->operational_state != MACHINE_PAUSED) {
    if(this->common_data->nighttime && this->intervals.hours_sunlight_ms > (this->common_data->light_increment * CONTROL_INTERVAL * 1000) && !this->led.status) {
      this->led.set_strip_high();
    }
    else if(this->led.status){
      this->common_data->light_increment++;
    }
    else if((this->intervals.hours_sunlight_ms <= (this->common_data->light_increment * CONTROL_INTERVAL * 1000)) || !this->common_data->nighttime) {
      this->led.set_strip_low();
    }

    // check conditions for temperature
    switch(this->intervals.temperature.check_current_value(this->common_data->temperature)) {
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
    switch(this->intervals.humidity.check_current_value(this->common_data->humidity)) {
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
}

Moisture::Moisture(int pin) {
  this->pin = pin;
  pinMode(this->pin, INPUT);
}

float Moisture::sense() {
  long result;
  int raw_data;

  raw_data = analogRead(this->pin);
  result = map(raw_data, 0, 1023, 0, 100);

  return result;
}

Light::Light() {
  if(this->light_sensor.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    this->status = true;
  }
  else {
    this->status = false;
  }
}

float Light::sense() {
  return this->light_sensor.readLightLevel();
}

HumidityTemperature::HumidityTemperature(int pin) : dht_sensor(pin, DHT11) {
  this->pin = pin;
  this->dht_sensor.begin();
}

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

Sensors::Sensors(MachineState *machine_state, CommonData *common_data, int humtemp_pin, int moisture_pin) : machine_state(machine_state), common_data(common_data), humtemp(humtemp_pin), moisture(moisture_pin) {}

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