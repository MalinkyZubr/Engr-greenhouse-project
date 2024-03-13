////////////////////////////////////////////////////////////////////
//////////////// StorageManager ////////////////////////////////////
////////////////////////////////////////////////////////////////////

StorageManager::StorageManager(int start_address, int data_size, int device_reset_pin) 
  : resetter(device_reset_pin), 
  identifier_info(&this->flash, start_address + BLOCK_SIZE),
  preset_info(&this->flash, start_address + BLOCK_SIZE * 2),
  wifi_info(&this->flash, start_address + BLOCK_SIZE * 3),
  machine_state(&this->flash, start_address + BLOCK_SIZE * 4),
  data_manager(&this->flash, start_address + BLOCK_SIZE * 5, data_size) {
  if(this->data_manager.get_end_address() > this->flash.getCapacity()) { 
    return;
  }
  this->load_flash_configuration();
}

void StorageManager::load_flash_configuration() {
  this->identifier_info.read();
  this->preset_info.read();
  this->wifi_info.read();
}

DataManager& StorageManager::get_data_manager() {
  return this->data_manager;
}

WifiInfo& StorageManager::get_wifi() {
  return this->wifi_info;
}

Preset& StorageManager::get_preset() {
  return this->preset_info;
}

Identifiers& StorageManager::get_identifiers() {
  return this->identifier_info;
}

MachineState& StorageManager::get_machine_state() {
  return this->machine_state;
}

bool StorageManager::write_flash_configuration(ConfigType configuration, const DynamicJsonDocument &to_write) {
  switch(configuration) {
    case IDENTIFIER:
      return this->identifier_info.write(to_write);
      break;
    case PRESET:
      return this->preset_info.write(to_write);
      break;
  }
}

bool StorageManager::write_flash_configuration(WifiInfo wifi_info) {
  this->wifi_info.copy(wifi_info);
}

void StorageManager::set_network_state(NetworkState state) {
  this->network_state = state;
}

NetworkState StorageManager::get_network_state() const {
  return this->network_state;
}

void StorageManager::set_common_data(CommonDataBuffer &common) {
  this->common_data = common;
}

CommonDataBuffer& StorageManager::get_common_data() {
  return this->common_data;
}

void StorageManager::hard_reset() {
  this->flash.eraseChip();
  this->resetter.trigger_reset();
}