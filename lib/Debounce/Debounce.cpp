#include "Arduino.h"
#include "Debounce.h"

Debounce::Debounce(int pin) {
    pinMode(pin, INPUT);
    _pin = pin;
}

bool Debounce::pressed(void) {
    // read the state of the switch into a local variable:
  int reading = digitalRead(_pin);
  bool buttonPressed = false;

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != _lastState) {
    // reset the debouncing timer
    _lastDebounceTime = millis();
  }

  if ((millis() - _lastDebounceTime) > _debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != _state) {
      _state = reading;

      // only toggle the LED if the new button state is HIGH
      if (_state == HIGH) {
        buttonPressed = true;
      }
    }
  }

  // save the reading. Next time through the loop, it'll be the _lastState:
  _lastState = reading;

  return buttonPressed;
}