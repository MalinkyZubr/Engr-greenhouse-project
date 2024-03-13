////////////////////////////////////////////////////////////////////
//////////////// Identifiers ///////////////////////////////////////
////////////////////////////////////////////////////////////////////

Identifiers::Identifiers(SPIFlash *flash, int flash_address) : ConfigStruct(flash, flash_address) {}

int Identifiers::get_device_id() const {
  return this->device_id;
}

int Identifiers::get_project_id() const {
  return this->project_id;
}

String Identifiers::get_device_name() const {
  return this->device_name;
}

StorageException Identifiers::from_json(const DynamicJsonDocument &data) {
  if(!data.containsKey("device_id") || !data.containsKey("project_id") || !data.containsKey("device_name")) {
    return STORAGE_IDENTIFIER_FIELD_MISSING;
  }
  this->device_id = data["device_id"];
  this->project_id = data["project_id"];
  this->device_name = (char *)&data["device_name"];

  return STORAGE_OKAY;
}

const DynamicJsonDocument Identifiers::to_json() const {
  DynamicJsonDocument data(CONFIG_JSON_SIZE);

  data["device_id"] = this->device_id;
  data["device_name"] = this->device_name;
  data["project_id"] = this->project_id;

  return data;
}