#ifndef SENSING_HPP
#define SENSING_HPP

#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <BH1750.h>
#include <DHT.h>


class Moisture {
  private:
  int pin;

  public:
  Moisture(int pin);
  int get_moisture_percentage();
};

class Light {
  private:
  BH1750 light_sensor;

  public:
  Light();
  float get_reading();
};

class HumidityTemperature {
  private:
  int pin;

  DHT dht_sensor;
  typedef struct EnvReading {
    float temperature_c;
    float humidity_percent;
  }EnvReading;

  public:
  HumidityTemperature(int pin);
  EnvReading get_temp_humidity();
};

#endif

