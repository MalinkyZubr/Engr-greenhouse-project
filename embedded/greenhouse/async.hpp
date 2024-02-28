#ifndef ASYNC_HPP
#define ASYNC_HPP

#define TIMER_INTERRUPT_DEBUG         0
#define _TIMERINTERRUPT_LOGLEVEL_     0

#define USE_TIMER_1     true

#if ( defined(__AVR_ATmega644__) || defined(__AVR_ATmega644A__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644PA__)  || \
        defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO) || defined(ARDUINO_AVR_MINI) ||    defined(ARDUINO_AVR_ETHERNET) || \
        defined(ARDUINO_AVR_FIO) || defined(ARDUINO_AVR_BT)   || defined(ARDUINO_AVR_LILYPAD) || defined(ARDUINO_AVR_PRO)      || \
        defined(ARDUINO_AVR_NG) || defined(ARDUINO_AVR_UNO_WIFI_DEV_ED) || defined(ARDUINO_AVR_DUEMILANOVE) || defined(ARDUINO_AVR_FEATHER328P) || \
        defined(ARDUINO_AVR_METRO) || defined(ARDUINO_AVR_PROTRINKET5) || defined(ARDUINO_AVR_PROTRINKET3) || defined(ARDUINO_AVR_PROTRINKET5FTDI) || \
        defined(ARDUINO_AVR_PROTRINKET3FTDI) )
#endif

#include <SimpleTimer.h>  
#include <ArduinoJson.h>
#include "storage.hpp"

#define MAX_TASKS 10
#define MESSAGE_QUEUE_SIZE 20

#define TIMER_INTERVAL_S 5


enum Actions {ENABLE, DISABLE, EXECUTE};


class Callable {
  public:
  virtual void callback() {};
};

class TimedTask {
  private:
  Callable *callback;
  int interval;
  long long target_time = 0;
  bool disconnected_slowdown = true;

  public:
  int id;
  bool enabled;
  TimedTask() {
    this->id = -1;
  }
  TimedTask(Callable *callback, int interval, int id, bool disconnected_slowdown);

  void execute(NetworkState mode);
  void enable();
  void disable();
};


class TaskManager {
  private:
  TimedTask task_list[MAX_TASKS];
  StorageManager *global_storage;

  public:
  TaskManager(StorageManager *global_storage);
  bool add_task(Callable *callback, int interval, int id, bool disconnected_slowdown);
  bool remove_task(int id);
  void execute(Actions action);
};


#endif