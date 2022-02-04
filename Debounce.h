#ifndef Debounce_h
#define Debounce_h

#include <Arduino.h>

class Debounce {
    public:
      Debounce(int pin, int defaultState = LOW);
      bool pressed(void);
    private:
      int _pin;
      int _state = LOW;
      int _lastState = LOW;
      unsigned long _debounceDelay = 50;
      unsigned long _lastDebounceTime = 0;
};

#endif
