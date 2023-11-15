#include "environmentManagement.hpp"


Device::Device(int pin) {
    this->pin = pin;
    this->status = 0;
}

void Device::toggle() {
  switch(this->status) {
    case 0:
      this->status = 1;
      digitalWrite(this->pin, HIGH);
      break;
    case 1:
      this->status = 0;
      digitalWrite(this->pin, LOW);
      break;
  }
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
}

void LedStrip::set_strip_low() {
  this->led_strip.clear();
  this->led_strip.show();
}

LedStrip::LedStrip(int pin) : Device(pin) {
  this->led_strip = Adafruit_NeoPixel(this->led_count, this->pin, NEO_GRB + NEO_KHZ800);
  this->led_strip.clear();
  this->startup();
}

void LedStrip::toggle() {
  switch(this->status) {
    case 0:
      this->status = 1;
      this->set_strip_high();
      break;
    case 1:
      this->status = 0;
      this->set_strip_low();
      break;
  }
}

EnvironmentManager::EnvironmentManager(int pump_pin, int heating_pin, int fan_pin, int led_pin) {
  this->pump = new Device(pump_pin);
  this->heater = new Device(heating_pin);
  this->fan = new Device(fan_pin);
  this->led = new LedStrip(led_pin);
}

void EnvironmentManager::toggle(Options option) {
  switch(option)
  {
    case PUMP:
      this->pump->toggle();
      break;
    case HEAT:
      this->heater->toggle();
      break;
    case FAN:
      this->fan->toggle();
      break;
    case LED:
      this->led->toggle();
      break;
  }
}
EnvironmentManager::~EnvironmentManager() {
  delete this->led;
  delete this->fan;
  delete this->pump;
  delete this->heater;
}