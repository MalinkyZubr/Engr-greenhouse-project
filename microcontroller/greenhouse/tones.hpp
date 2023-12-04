#ifndef TONES_HPP
#define TONES_HPP

#include <Arduino.h>


enum switchTones {STARTUP, READY};


class Tones {
    private:
    int pin;
    int tone_length = 5;
    int startup[2][5] = {{70, 170, 255, 170, 70},
                         {100, 100, 100, 100, 100}};
    int ready[2][5] = {{70, 170, 70, 255, 255},
                         {100, 300, 100, 100, 300}};

    void play_tone(int tones[][5]);

    public:
    Tones(int pin);
    void select_tone(switchTones tone);
};

#endif