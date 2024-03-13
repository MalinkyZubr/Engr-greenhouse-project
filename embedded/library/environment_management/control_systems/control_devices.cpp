/// @brief base object for environment management devices
/// @param pin pin to set for the device
Device::Device(int pin) : pin(pin), status(false) {}
/// @brief digital write the device to high to activate it


void Device::set_high() {
  this->status = true;
  digitalWrite(this->pin, HIGH);
}


/// @brief digital write the device to low to deactivate it
void Device::set_low() {
  this->status = false;
  digitalWrite(this->pin, LOW);
}