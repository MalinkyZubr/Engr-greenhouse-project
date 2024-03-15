#include "async.hpp"

//////////////////////////////////////////////////////////////
// Task Object  //////////////////////////////////////////////
//////////////////////////////////////////////////////////////


/// @brief constructor for a new timed task class. These can be placed inside a task manager object to be run periodically pseudo-asynchronously
/// @param callback Callable object which contains a callback function. This is the actual code that will be called at every specified interval 
/// @param interval the interval at which the callback is called, must be a multiple of 
/// @see TIMER_INTERVAL_S
/// @param id the task ID, must be unique, and less than
/// @see MAX_TASKS
/// @param disconnected_slowdown flag specifiying if the interval should increase if the machine connection disconnects. Increases interval by a factor of 20 if set to true
TimedTask::TimedTask(Callable *callback, int interval, int id, bool disconnected_slowdown) : callback(callback), id(id), enabled(false), disconnected_slowdown(disconnected_slowdown) {
  if(interval < TIMER_INTERVAL_S) {
    this->interval = TIMER_INTERVAL_S;
  } 
  while(interval % TIMER_INTERVAL_S) {
    interval--;
  }
  interval *= 1000; // convert to milliseconds
  this->callback = callback;
}

void TimedTask::enable() {
  this->enabled = true;
  this->target_time = millis() + this->interval;
}


/// @brief function that calls the callback of the Callable object composing the TimedTask object
/// @param mode enum, MACHINE_DISCONNETED or MACHINE_CONNECTED, which determines if the interval is increased by a factor of 20
void TimedTask::execute(NetworkState mode) {
  if(millis() >= target_time && this->enabled) {
    this->callback->callback();
  }

  switch(mode) {
    case MACHINE_DISCONNECTED:
      if(disconnected_slowdown) {
        this->target_time = millis() + this->interval * 20;
        break; // if the disconnected slowdown flag isnt set, the swithc statement should fall through
      }
    case MACHINE_CONNECTED:
      this->target_time = millis() + this->interval;
      break;
  }
}


/// @brief disable a task so that it isnt exeecuted
void TimedTask::disable() {
  this->enabled = false;
}