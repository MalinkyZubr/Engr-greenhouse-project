////////////////////////////////////////////////////////////////////
//////////////// CommonDataBuffer //////////////////////////////////
////////////////////////////////////////////////////////////////////

void CommonDataBuffer::increment_light(bool increment) {
  if(increment) {
    this->ms_sunlight += millis() - this->last_measurement;
  }
  this->last_measurement = millis();
}

void CommonDataBuffer::set_common_data(float temperature, float humidity, float moisture, float light_exposure) {
  this->temperature = temperature;
  this->humidity = humidity;
  this->moisture = moisture;
  this->light_exposure = light_exposure;

  if(light_exposure > 800) {
    this->increment_light(true);
  }
  else {
    this->increment_light(false);
  }
}

const float CommonDataBuffer::get_temperature() const {
  return this->temperature;
}

const float CommonDataBuffer::get_humidity() const {
  return this->humidity;
}

const float CommonDataBuffer::get_moisture() const {
  return this->moisture;
}

const float CommonDataBuffer::get_light_exposure() const {
  return this->light_exposure;
}

const long long CommonDataBuffer::get_ms_light() const {
  return this->ms_sunlight;
}

const bool CommonDataBuffer::is_night_time() const {
  return this->night_time;
}

const DynamicJsonDocument CommonDataBuffer::to_json() const {
  DynamicJsonDocument data(CONFIG_JSON_SIZE);

  data["temperature"] = this->temperature;
  data["humidity"] = this->humidity;
  data["moisture"] = this->moisture;
  data["light_exposure"] = this->light_exposure;

  return data;
}

void CommonDataBuffer::set_night_time(bool night) {
  this->night_time = night;
}