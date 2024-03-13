////////////////////////////////////////////////////////////////////
//////////////// DeviceResetter ////////////////////////////////////
////////////////////////////////////////////////////////////////////

DeviceResetter::DeviceResetter(int reset_pin) : reset_pin(reset_pin) {
  pinMode(reset_pin, OUTPUT);
}

void DeviceResetter::trigger_reset() {
  digitalWrite(this->reset_pin, HIGH);
}