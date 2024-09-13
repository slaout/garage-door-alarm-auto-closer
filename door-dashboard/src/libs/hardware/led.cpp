#include <Arduino.h>

#include "led.h"

Led::Led(uint8_t pin)
  : pin(pin)
{
}

void Led::setup()
{
  pinMode(pin, OUTPUT);
  digitalWrite(pin, lit ? HIGH : LOW);
}

void Led::loop()
{
  if (this->pattern == nullptr) {
    return;
  }

  const unsigned long now = millis();
  if (now > this->nextPatternToggleTimestamp) {
    this->currentPatternIndex = (this->currentPatternIndex + 1) % this->pattern->count;
    this->nextPatternToggleTimestamp = now + this->pattern->durations[this->currentPatternIndex];

    this->lit = this->currentPatternIndex % 2 == 0;
    digitalWrite(pin, this->lit ? HIGH : LOW);
  }
}

void Led::set(bool lit)
{
  this->pattern = nullptr;

  this->lit = lit;
  digitalWrite(pin, lit ? HIGH : LOW);
}

void Led::turnOn()
{
  set(true);
}

void Led::turnOff()
{
  set(false);
}

void Led::blink(const LedPattern *pattern)
{
  if (this->pattern == pattern) {
    return;
  }

  this->pattern = pattern;

  if (pattern->durations[0] == 0 && pattern->count > 1) {
    this->currentPatternIndex = 1;
    this->nextPatternToggleTimestamp = millis() + pattern->durations[1];
    digitalWrite(pin, LOW);
  } else {
    this->currentPatternIndex = 0;
    this->nextPatternToggleTimestamp = millis() + pattern->durations[0];
    digitalWrite(pin, HIGH);
  }
}

bool Led::isLit()
{
  return lit;
}
