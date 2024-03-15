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