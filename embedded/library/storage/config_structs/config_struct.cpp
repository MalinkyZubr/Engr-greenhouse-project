////////////////////////////////////////////////////////////////////
//////////////// ConfigStruct //////////////////////////////////////
////////////////////////////////////////////////////////////////////

ConfigStruct::ConfigStruct(SPIFlash *flash, int address) : flash(flash), flash_address(address) {}

StorageException ConfigStruct::write(const DynamicJsonDocument &data) {
  String to_write;

  StorageException conversion_exception = this->from_json(data);
  if(conversion_exception != STORAGE_OKAY) {
    return conversion_exception;
  }
  this->erase();

  serializeJson(data, to_write);
  if(!this->flash->writeStr(this->flash_address, to_write)) {
    return STORAGE_WRITE_FAILURE;
  };
}

bool ConfigStruct::read() {
  String to_read;
  String to_save;
  DynamicJsonDocument to_deserialize(CONFIG_JSON_SIZE);

  this->flash->readStr(this->flash_address, to_read);

  if(to_read.equals("")) {
    this->set_unconfigured();
    return false;
  }
  deserializeJson(to_deserialize, to_save);

  this->from_json(to_deserialize);
  this->set_configured();

  return true;
}

bool ConfigStruct::erase() {
  this->is_configured = false;
  return this->flash->eraseSector(this->flash_address);
}

void ConfigStruct::set_configured() {
  this->is_configured = true;
}

void ConfigStruct::set_unconfigured() {
  this->is_configured = false;
}

bool ConfigStruct::check_is_configured() const {
  return this->is_configured;
}