////////////////////////////////////////////////////////////////////
//////////////// Machine State /////////////////////////////////////
////////////////////////////////////////////////////////////////////

MachineState::MachineState(SPIFlash *flash, int flash_address) : ConfigStruct(flash, flash_address) {}

MachineOperationalState MachineState::get_state() const {
  return this->operational_state;
}

void MachineState::set_state(MachineOperationalState state) {
  this->operational_state = state;
}

StorageException MachineState::from_json(const DynamicJsonDocument &data) {
  if(!data.containsKey("operational_state")) {
    return STORAGE_IDENTIFIER_FIELD_MISSING;
  }

  char reported_operational_state = (char)data["operational_state"];
  switch(reported_operational_state) {
    case 'p':
      this->operational_state = MACHINE_PAUSED;
      break;
    case 'a':
      this->operational_state = MACHINE_ACTIVE;
      break;
  }
  return STORAGE_OKAY;
}

const DynamicJsonDocument MachineState::to_json() const {
  DynamicJsonDocument data(CONFIG_JSON_SIZE);

  data["operational_state"] = (char)this->operational_state;

  return data;
}