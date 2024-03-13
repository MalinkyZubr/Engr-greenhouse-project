#include "tones.hpp"


/// @brief instantiate the tones object for making cute noises at different events, like turn on
/// @param pin pin to set the buzzer to use
Tones::Tones(int pin) {
  this->pin = pin; // make an enum so that user can only select PWM pins
  pinMode(this->pin, OUTPUT);
}


/// @brief play a tone based on a 2 d array of integers. The first array represents the pitch of the tone, the second the delay until the next tone
/// @param tones reference to the 2d array
void Tones::play_tone(int tones[][5]) {
  for(int x = 0; x < this->tone_length; x++) {
    analogWrite(this->pin, tones[0][x]);
    delay(tones[1][x]);
    analogWrite(this->pin, LOW);
  }
}


/// @brief play a tone based on the enum value
/// @param tone enum value to play tone by
void Tones::select_tone(switchTones tone) {
  switch(tone) {
    case STARTUP:
      play_tone(this->startup);
      break;
    case READY:
      play_tone(this->ready);
      break;
  }
}