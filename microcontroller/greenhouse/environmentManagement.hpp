#ifndef ENVIRONMENTMANAGEMENT_HPP
#define ENVIRONMENTMANAGEMENT_HPP

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

class Device {
  public:
  int pin;
  int status;

  Device(int pin);
  void toggle();
};

class LedStrip : public Device {
  private:
  int led_count = 16;
  int standard_color[3] = {214, 83, 211};
  int startup_sequence_colors[3][3] = {
    {0,0,255},
    {0,255,0},
    {255,0,0}
  };

  Adafruit_NeoPixel led_strip;

  void set_strip(int color[], int delay_time, bool sequence);
  void startup();
  void set_strip_high();
  void set_strip_low();

  public:
  LedStrip(int pin);
  void toggle();
};

class EnvironmentManager
{
  private:
  Device *pump;
  Device *heater;
  Device *fan;
  LedStrip *led;

  public:
  enum Options {PUMP, HEAT, FAN, LED};
  EnvironmentManager(int pump_pin, int heating_pin, int fan_pin, int led_pin);
  void toggle(Options option);
  ~EnvironmentManager();
};


#endif