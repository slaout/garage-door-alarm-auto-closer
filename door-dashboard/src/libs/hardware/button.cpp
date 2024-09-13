#include <Arduino.h>

#include "button.h"

// During tests, the fastest a button is toggled by a human is 48ms
// Bouncing happens in 0 to 1 ms...
// Rare triggers happen in 16ms: human or bouncing?!
const unsigned long DEBOUNCE_DELAY_MS = 20;
const unsigned long MULTI_PRESS_MAX_GAP_MS = 600;

Button::Button(uint8_t pin, ButtonResistor resistor)
  : pin(pin)
  , resistor(resistor)
{
}

void Button::setup()
{
  pinMode(pin, resistor == INTERNAL_PULL_UP_RESISTOR ? INPUT_PULLUP : INPUT);

  // Initialize state, in case it's pressed at startup (but don't emit callbacks)
  pressed = digitalRead(pin) == pressedState();
  pressedAtLastRead = pressed;
  lastDebounceTime = millis();
}

void Button::setOnChange(void (*callback)(bool))
{
  onChangeCallback = callback;
}

void Button::setOnPress(void (*callback)())
{
  onPressCallback = callback;
}

void Button::setOnLongPress(unsigned long period, void (*callback)(unsigned int repeatNumber))
{
  longPressPeriod = period;
  onLongPressCallback = callback;
}

void Button::setOnMultiPress(void (*callback)(unsigned int pressCount))
{
  onMultiPressCallback = callback;
}

void Button::loop()
{
  const bool currentlyPressed = digitalRead(pin) == pressedState();

  // check to see if you just pressed the button,
  // and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (currentlyPressed != pressedAtLastRead) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  // The current reading has been stable for long enough to be considered bouncing-free by now:
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY_MS) {
    // Update & emit new state if the button state has changed after debouncing
    if (currentlyPressed != pressed) {
      pressed = currentlyPressed;
      handlePressChange();
    }
  }

  pressedAtLastRead = currentlyPressed;

  if (onLongPressCallback != nullptr && longPressNextTriggerTime > 0 && millis() >= longPressNextTriggerTime) {
    longPressNextRepeatNumber++;
    longPressNextTriggerTime = millis() + longPressPeriod;
    onLongPressCallback(longPressNextRepeatNumber);
  }
}

void Button::handlePressChange()
{
  // Change
  if (onChangeCallback != nullptr) {
    (*onChangeCallback)(pressed);
  }

  // Press
  if (pressed && onPressCallback != nullptr) {
    (*onPressCallback)();
  }

  // Long press
  longPressNextRepeatNumber = 0;
  longPressNextTriggerTime = (pressed && longPressPeriod > 0 ? millis() + longPressPeriod : 0);

  // Multi-press
  if (pressed) {
    if (millis() < multiPressNextTriggerTime) {
      multiPressCount++;
      if (onMultiPressCallback != nullptr) {
        onMultiPressCallback(multiPressCount);
      }
    } else {
      multiPressCount = 1;
    }
    multiPressNextTriggerTime = millis() + MULTI_PRESS_MAX_GAP_MS;
  }
}

bool Button::isPressed() const
{
  return pressed;
}

bool Button::pressedState() const
{
  return resistor == EXTERNAL_PULL_DOWN_RESISTOR ? HIGH : LOW;
}
