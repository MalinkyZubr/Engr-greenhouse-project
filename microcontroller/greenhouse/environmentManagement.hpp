#ifndef ENVIRONMENTMANAGEMENT_HPP
#define ENVIRONMENTMANAGEMENT_HPP

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_Sensor.h>
#include <BH1750.h>
#include <DHT.h>
#include "async.hpp"
#include "machine_state.hpp"
#include "storage.hpp"

#define MILLISECONDS_IN_HOUR 3600000
#define LUX_THRESHOLD 800

// Device control
#define CONTROL_INTERVAL 5 // in seconds
#define CONTROL_ID 3

long long hours_ms(int hours);

class Device {
  public:
  bool status;
  int pin;

  Device(int pin);
  void set_high();
  void set_low();
};

class LedStrip {
  private:
  int led_count = 16;
  int standard_color[3] = {214, 83, 211};
  int startup_sequence_colors[3][3] = {
    {0,0,255},
    {0,255,0},
    {255,0,0}
  };
  int pin;

  Adafruit_NeoPixel led_strip;

  void set_strip(int color[], int delay_time, bool sequence);

  public:
  bool status;

  LedStrip(int pin);
  void startup();
  void set_strip_high();
  void set_strip_low();
};

class Interval {
  public:
  float upper;
  float lower;

  enum relation {HIGHER, LOWER, OKAY};

  Interval(float desired_value);
  void set_interval(float desired_value);
  relation check_current_value(float value);
};

class EnvironmentManager : public Callable {
  private:
  Interval temperature;
  Interval humidity;
  Interval moisture;
  long long hours_sunlight_ms;

  MachineState *machine_state;

  Device pump;
  Device heater;
  Device fan;
  LedStrip led;
  CommonData *common_data;
  Configuration *config;

  Interval::relation check_status();

  public:
  EnvironmentManager(MachineState *machine_state, Configuration *config, CommonData *common_data, int pump_pin, int heating_pin, int fan_pin, int led_pin, float desired_temperature, float desired_humidity, float desired_moisture, int hours_sunlight);

  void update_interfaces();

  void device_activation();
  void callback() override;
};

class Sensor {
  public:
  virtual float sense();
};

class Moisture : public Sensor{
  private:
  int pin;

  public:
  Moisture(int pin);
  float sense();
};

class Light : public Sensor {
  private:
  BH1750 light_sensor;
  bool status;

  public:
  Light();
  float sense();
};

class HumidityTemperature {
  private:
  int pin;

  DHT dht_sensor;

  public:
  typedef struct EnvReading {
    float temperature_c;
    float humidity_percent;
  }EnvReading;

  HumidityTemperature(int pin);
  EnvReading sense();
};

class Sensors : public Callable {
  private:
  MachineState *machine_state;
  CommonData *common_data;
  Moisture moisture;
  Light light;
  HumidityTemperature humtemp;

  public:
  Sensors(MachineState *machine_state, CommonData *common_data, int humtemp_pin, int moisture_pin);

  void submit_readings();
  void callback() override {
    this->submit_readings();
  }
};

#endif