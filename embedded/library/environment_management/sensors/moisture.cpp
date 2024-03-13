/// @brief object for managing the moisture sensor
/// @param pin physical pin to assign
Moisture::Moisture(int pin) {
  this->pin = pin;
  pinMode(this->pin, INPUT);
}


/// @brief get the moisture reading from the sensor
/// @return the % moisture in the soil
float Moisture::sense() {
  long result;
  int raw_data;

  raw_data = analogRead(this->pin);
  result = map(raw_data, 0, 1023, 0, 100);

  return result;
}
