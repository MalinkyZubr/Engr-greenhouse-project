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




#endif