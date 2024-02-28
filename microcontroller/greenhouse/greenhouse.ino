/*
  Project Name: Engineering 131 Greenhouse Project
  Description: For Purdue freshman engineering 131, team was assigned to develop an efficient greenhouse system including microcontroller systems for environmental regulation
  We decided to develop a full stack wifi based system for easy user configuration. This is the embedded systems portion of the project
  Author: Michael Ray (MalinkyZubr on github)
  Date: 2023-10-10
  License: MIT license
*/


#include <Wire.h>
#include <SPI.h>
#include <ArduinoJson.h>

#include "async.hpp"
#include "storage.hpp"
#include "environmentManagement.hpp"
#include "router.hpp"
#include "data_sender.hpp"
#include "http.hpp"
#include "wifi.hpp"

#include "TimerInterrupt.h"
#include "ISR_Timer.h"

#define TESTING true // should serial be connected?

#define MOISTURE_PIN A0
#define PUMP_PIN A1
#define BUZZER_PIN 3
#define DHT_PIN 7
#define LED_PIN 6
#define FAN_PIN 5
#define HEAT_PIN 4
#define DEVICE_RESET_PIN 11


// Data sending
#define DATA_SEND_INTERVAL 30 // in seconds
#define DATA_SEND_ID 1 // task id for data sending

// Sensing
#define SENSOR_INTERVAL 5 // in seconds
#define SENSOR_ID 2

ISR_Timer ISR_timer; // hardware timer instantiation

StorageManager storage_manager(0, BLOCK_SIZE, DEVICE_RESET_PIN);
TaskManager task_manager(&storage_manager); // pseudo asynchronous, for executing periodic actions
Callable envmgr = EnvironmentManager(&storage_manager, (int) PUMP_PIN, HEAT_PIN, FAN_PIN, LED_PIN, 16); // control devices (heaters and such)
Callable sensors = Sensors(&storage_manager, DHT_PIN, (int) MOISTURE_PIN); // sensor systems to feed into data struct
Router router; // router for executing tasks based on received requests

void setup() {
  Serial.begin(115200);
  Wire.begin();
  SPI.begin();
  
  //task_manager.add_task(enqueue_data, DATA_SEND_INTERVAL, DATA_SEND_ID);
  task_manager.add_task(&envmgr, CONTROL_INTERVAL, CONTROL_ID, false);
  task_manager.add_task(&sensors, SENSOR_INTERVAL, SENSOR_ID, false);
  //task_manager.add_task();

  ITimer1.init();
  while(!ITimer1.attachInterruptInterval(TIMER_INTERVAL_S * 1000, TimedTasks)){}

  task_manager.execute(ENABLE);
}

void loop() {
  // listen for requests here forever
}

void TimedTasks() // executes every 5 seconds
{
  ISR_timer.run();
  task_manager.execute(EXECUTE);
}