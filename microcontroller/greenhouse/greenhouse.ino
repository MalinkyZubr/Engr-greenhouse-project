#include <Wire.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include "async.hpp"
#include "environmentManagement.hpp"
#include "wifi.hpp"
#include "storage.hpp"
#include "router.hpp"

#include "TimerInterrupt.h"
#include "ISR_Timer.h"
#include "machine_state.hpp"

#define TESTING true // should serial be connected?

#define MOISTURE_PIN A0
#define PUMP_PIN A1
#define BUZZER_PIN 3
#define DHT_PIN 7
#define LED_PIN 6
#define FAN_PIN 5
#define HEAT_PIN 4


// Data sending
#define DATA_SEND_INTERVAL 30 // in seconds
#define DATA_SEND_ID 1 // task id for data sending

// Sensing
#define SENSOR_INTERVAL 5 // in seconds
#define SENSOR_ID 2

ISR_Timer ISR_timer; // hardware timer instantiation

MachineOperationalState state = MACHINE_PAUSED; // the machine should start in a paused state so that the environmental controls remain off

CommonData common_data; // struct to hold data feed from environmental systems
TaskManager task_manager; // pseudo asynchronous, for executing periodic actions
Callable envmgr = EnvironmentManager(&common_data, (int) PUMP_PIN, HEAT_PIN, FAN_PIN, LED_PIN, 1.0, 2.0, 3.0, 6); // control devices (heaters and such)
Callable sensors = Sensors(&common_data, DHT_PIN, (int) MOISTURE_PIN); // sensor systems to feed into data struct
MessageQueue message_queue; 

Router router; // router for executing tasks based on received requests
ConfigManager config_manager;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  SPI.begin();
  
  //task_manager.add_task(enqueue_data, DATA_SEND_INTERVAL, DATA_SEND_ID);
  task_manager.add_task(&envmgr, CONTROL_INTERVAL, CONTROL_ID);
  task_manager.add_task(&sensors, SENSOR_INTERVAL, SENSOR_ID);
  //task_manager.add_task();

  ITimer1.init();
  while(!ITimer1.attachInterruptInterval(TIMER_INTERVAL_S * 1000, TimedTasks)){}

  task_manager.execute_actions(ENABLE);
}

void loop() {
  // listen for requests here forever
}

void TimedTasks() // executes every 5 seconds
{
  ISR_timer.run();
  task_manager.execute_actions(EXECUTE);
}

void enqueue_data(ConfigManager config_manager) { // this should be called by sensor reader
  DynamicJsonDocument message_body(1024);
  message_body["project_name"] = config_manager.config.project_name;
  message_body["device_name"] = config_manager.config.device_name;
  message_body["temperature"] = common_data.temperature;
  message_body["moisture"] = common_data.moisture;
  message_body["humidity"] = common_data.humidity;
  message_body["light_exposure"] = common_data.light_level;

  message_queue.enqueue_message(config_manager.config.server_hostname, String("/interface/data/") + config_manager.config.device_name, String("POST"), message_body);
}