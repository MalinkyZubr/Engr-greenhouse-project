#include "async.hpp"

//////////////////////////////////////////////////////////////
// Task Object  //////////////////////////////////////////////
//////////////////////////////////////////////////////////////

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

void TimedTask::execute(MachineConnectionState mode) {
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

  this->callback->callback();
}

void TimedTask::disable() {
  this->enabled = false;
}

//////////////////////////////////////////////////////////////
// Task Manager  /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

TaskManager::TaskManager(MachineState *machine_state) : machine_state(machine_state) {}

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

bool TaskManager::remove_task(int id) {
  for(int i = 0; i < MAX_TASKS; i++) {
    if(this->task_list[i].id == id) {
      this->task_list[i] = TimedTask();
      return true;
    }
  }
  return false;
}

void TaskManager::execute_actions(Actions action) {
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
          task_list[i].execute(this->machine_state->connection_state);
          break;
      }
    }
  }
}