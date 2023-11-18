#include "async.hpp"

//////////////////////////////////////////////////////////////
// Task Object  //////////////////////////////////////////////
//////////////////////////////////////////////////////////////

TimedTask::TimedTask(Callable *callback, int interval, int id) : callback(callback), id(id), enabled(false) {
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

void TimedTask::execute() {
  if(millis() >= target_time && this->enabled) {
    this->callback->callback();
    this->target_time = millis() + this->interval;
  }
  this->callback->callback();
}

void TimedTask::disable() {
  this->enabled = false;
}

//////////////////////////////////////////////////////////////
// Task Manager  /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool TaskManager::add_task(Callable *callback, int interval, int id) {
  TimedTask new_task = TimedTask(callback, interval, id);
  for(int i = 0; i < MAX_TASKS; i++) {
    if(this->task_list[i].id == -1)
    {
      task_list[i] = new_task;
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
          task_list[i].execute();
          break;
      }
    }
  }
}

//////////////////////////////////////////////////////////////
// Message Queue  ////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool MessageQueue::enqueue_message(String server_hostname, String route, String method, DynamicJsonDocument body) {
  if(this->tail < MESSAGE_QUEUE_SIZE - 1) {
    Message new_message;
    new_message.server_hostname = server_hostname;
    new_message.route = route;
    new_message.method = method;
    new_message.body = body;

    this->queue[tail] = new_message;
    this->tail++;

    return true;
  }
  return false;
}

Message MessageQueue::dequeue_message() {
  if(this->queue[0].route.equals("*")) {
    Message head = this->queue[0];
    for(int i = 0; i < MESSAGE_QUEUE_SIZE; i++) {
      this->queue[i] = this->queue[i + 1];
    }
    return head;
  }
  Message empty;
  return empty;
}