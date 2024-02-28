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

//////////////////////////////////////////////////////////////
// Task Manager  /////////////////////////////////////////////
//////////////////////////////////////////////////////////////


/// @brief constructor for the TaskManager object, which stores an array of TimedTasks to be periodically executed pseudo-asynchronously
/// @param machine_state reference to the machine state so the object can read global machine state
TaskManager::TaskManager(StorageManager *global_storage) : global_storage(global_storage) {}


/// @brief add a task to the TimedTask array for regular execution
/// @param callback reference to a Callable object to be called periodically
/// @param interval the interval at which the callback will be executed 
/// @param id the id of the task for tracking in the array
/// @param disconnected_slowdown flag to determine if the task interval increases when the machine disconnects
/// @return bool, did the insertion of the task succeed?
bool TaskManager::add_task(Callable *callback, int interval, int id, bool disconnected_slowdown) {
  TimedTask new_task = TimedTask(callback, interval, id, disconnected_slowdown);
  for(int i = 0; i < MAX_TASKS; i++) {
    if(this->task_list[i].id == -1)
    {
      task_list[i] = new_task;
      return true;
    }
  }
  return false;
}


/// @brief pop a task from the TimedTask array
/// @param id 
/// @return 
bool TaskManager::remove_task(int id) {
  for(int i = 0; i < MAX_TASKS; i++) {
    if(this->task_list[i].id == id) {
      this->task_list[i] = TimedTask();
      return true;
    }
  }
  return false;
}

void TaskManager::execute(Actions action) {
  for(int i = 0; i < MAX_TASKS; i++) {
    if(this->task_list[i].id != -1)
    {
      switch(action) {
        case ENABLE:
          task_list[i].enable();
          break;
        case DISABLE:
          task_list[i].disable();
          break;
        case EXECUTE:
          if(this->global_storage->get_machine_state().get_state() == MACHINE_PAUSED) {
            task_list[i].disable();
          }
          else {
            task_list[i].execute(this->global_storage->get_network_state());
          }
          break;
      }
    }
  }
}