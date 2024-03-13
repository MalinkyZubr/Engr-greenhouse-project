////////////////////////////////////////////////////////////////////
//////////////// Preset ////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

Preset::Preset(SPIFlash *flash, int flash_address) : ConfigStruct(flash, flash_address) {}

float Preset::get_temperature() const {
  return this->temperature;
}

float Preset::get_humidity() const {
  return this->humidity;
}

float Preset::get_moisture() const {
  return this->moisture;
}

float Preset::get_hours_daylight() const {
  return this->hours_daylight;
}

float Preset::get_preset_id() const {
  return this->preset_id;
}

StorageException Preset::from_json(const DynamicJsonDocument &data) {
  if(!data.containsKey("preset_id") || !data.containsKey("temperature") || !data.containsKey("humidity") || !data.containsKey("moisture") || !data.containsKey("hours_daylight")) {
    return STORAGE_PRESET_FIELD_MISSING;
  }

  this->preset_id = data["preset_id"];
  this->temperature = data["temperature"];
  this->humidity = data["humidity"];
  this->moisture = data["moisture"];
  this->hours_daylight = data["hours_daylight"];

  return STORAGE_OKAY;
}

const DynamicJsonDocument Preset::to_json() const {
  DynamicJsonDocument data(CONFIG_JSON_SIZE);

  data["preset_id"] = this->preset_id;
  data["temperature"] = this->temperature;
  data["humidity"] = this->humidity;
  data["moisture"] = this->moisture;
  data["hours_daylight"] = this->hours_daylight;

  return data;
}

MeasurementCompliance Preset::check_measurement_compliance(float desired, float real) const {
  if(desired < real - 2) {
    return LOWER;
  }
  else if(desired > real + 2) {
    return HIGHER;
  }
  return COMPLIANT;
}

MeasurementCompliance Preset::check_temperature_compliance(float measured) const {
  return this->check_measurement_compliance(this->temperature, measured);
}

MeasurementCompliance Preset::check_humidity_compliance(float measured) const {
  return this->check_measurement_compliance(this->humidity, measured);
}

MeasurementCompliance Preset::check_moisture_compliance(float measured) const {
  return this->check_measurement_compliance(this->moisture, measured);
}