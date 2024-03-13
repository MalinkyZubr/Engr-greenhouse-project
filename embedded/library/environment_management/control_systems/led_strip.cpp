/// @brief function to set the status of the LED strip on the greenhouse unit
/// @param color the color array (RGB) to set the LED ring to
/// @param delay_time the delay value between the setting of each pixel
/// @param sequence flag to say if the call of the function is part of a sequence. If this is true, the led strip will be set low after finishing execution
void LedStrip::set_strip(int color[], int delay_time, bool sequence) {
  for(int pixel = 0; pixel < this->led_count; pixel++)
  {
    this->led_strip.setPixelColor(pixel, this->led_strip.Color(
      color[0],
      color[1],
      color[2]
    ));
    delay(delay_time);
    this->led_strip.show();
  }
  if(sequence){
    this->set_strip_low();
  }
}


/// @brief startup animation sequence for LED ring
void LedStrip::startup() {
  for(int sequence = 0; sequence < 3; sequence++)
  {
    this->set_strip(this->startup_sequence_colors[sequence], 50, true);
  }
  for(int sequence = 0; sequence < 3; sequence++)
  {
    this->set_strip(this->standard_color, 0, false);
    delay(200);
    this->set_strip_low();
    delay(200);
  }
}


/// @brief set all pixels to be the standard operation color
void LedStrip::set_strip_high() {
  this->set_strip(this->standard_color, 0, false);
  this->status = true;
}


/// @brief set all pixels on the ring to be dark
void LedStrip::set_strip_low() {
  this->led_strip.clear();
  this->led_strip.show();
  this->status = false;
}


/// @brief constructor for the LED ring controller object
/// @param pin the control pin for the led ring
LedStrip::LedStrip(int pin, int num_led) : led_count(num_led) {
  this->led_strip = Adafruit_NeoPixel(this->led_count, this->pin, NEO_GRB + NEO_KHZ800);
  this->led_strip.clear();
  this->startup();
}