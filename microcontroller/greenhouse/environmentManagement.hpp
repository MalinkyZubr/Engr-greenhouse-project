#ifndef ENVIRONMENTMANAGEMENT_HPP
#define ENVIRONMENTMANAGEMENT_HPP

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_Sensor.h>
#include <BH1750.h>
#include <DHT.h>
#include "async.hpp"
#include "storage.hpp"

// Device control
#define CONTROL_INTERVAL 5 // in seconds
#define CONTROL_ID 3

#define MILLISECONDS_IN_HOUR 3600000

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

  LedStrip(int pin, int led_count);
  void startup();
  void set_strip_high();
  void set_strip_low();
};

enum DeviceState{DEVICE_HIGH, DEVICE_LOW};

class EnvironmentManager : public Callable {
  private:
  Device pump;
  Device heater;
  Device fan;
  LedStrip led;
  StorageManager *global_storage;

  void set_devices_low(Device devices[]);
  void set_devices_high(Device devices[]);
  void set_device_statuses(MeasurementCompliance current_measurement_classification, Device set_high_when_higher[], Device set_low_when_higher[]);

  public:
  EnvironmentManager::EnvironmentManager(StorageManager *global_storage, int pump_pin, int heater_pin, int fan_pin, int led_pin, int num_led);

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
  StorageManager *global_storage;
  Moisture moisture;
  Light light;
  HumidityTemperature humtemp;

  public:
  Sensors(StorageManager* global_storage, int humtemp_pin, int moisture_pin);

  void submit_readings();
  void callback() override {
    this->submit_readings();
  }
};

#endif