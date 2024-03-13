/// @brief light sensor. No pin because I2C based
Light::Light() {
  if(this->light_sensor.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    this->status = true;
  }
  else {
    this->status = false;
  }
}


/// @brief read current light exposure levels
/// @return the current light reading in lux
float Light::sense() {
  return this->light_sensor.readLightLevel();
}