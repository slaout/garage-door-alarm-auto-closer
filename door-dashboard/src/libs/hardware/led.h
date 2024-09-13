#ifndef LED_H
#define LED_H

#include "led-pattern.h"

class Led {
  private:
    /**
     * The output pin to write to.
     */
    const uint8_t pin;

    bool lit;

    const LedPattern *pattern;
    uint8_t currentPatternIndex; // Irrelevant when pattern is nullptr
    unsigned long nextPatternToggleTimestamp; // Irrelevant when pattern is nullptr

  public:
    Led(uint8_t pin);
    void setup();
    void loop();
    void set(bool lit);
    void turnOn();
    void turnOff();
    void blink(const LedPattern *pattern);
    bool isLit();
};

#endif
