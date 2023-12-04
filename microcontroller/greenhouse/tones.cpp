#include "tones.hpp"


Tones::Tones(int pin) {
  this->pin = pin; // make an enum so that user can only select PWM pins
  pinMode(this->pin, OUTPUT);
}

void Tones::play_tone(int tones[][5]) {
  for(int x = 0; x < this->tone_length; x++) {
    analogWrite(this->pin, tones[0][x]);
    delay(tones[1][x]);
    analogWrite(this->pin, LOW);
  }
}

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