#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

enum ButtonResistor {
  /**
   * Use the internal pull-up resistor built into the Atmega chip.
   * See https://docs.arduino.cc/learn/microcontrollers/digital-pins/#properties-of-pins-configured-as-input_pullup
   * Pressed means the input reads LOW.
   */
  INTERNAL_PULL_UP_RESISTOR,

  /**
   * Use an external pull-up resistor you add to the circuit.
   * Pressed means the input reads LOW.
   */
  EXTERNAL_PULL_UP_RESISTOR,

  /**
   * Use an external pull-down resistor you add to the circuit.
   * Pressed means the input reads HIGH.
   */
  EXTERNAL_PULL_DOWN_RESISTOR
};

class Button {
  private:
    /**
     * The input pin to read during setup and loop.
     */
    const uint8_t pin;

    /**
     * The type of resistor used to set a known input voltage when the button is not pressed.
     */
    const ButtonResistor resistor;

    /**
     * The current known stable (validated after a debouncing duration) reading from the input pin.
     */
    bool pressed = false;

    /**
     * The very last reading from the input pin (during debouncing).
     */
    bool pressedAtLastRead = false;

    /**
     * The last time the input pin was toggled (it resets the debouncing duration).
     */
    unsigned long lastDebounceTime = 0;

    /**
     * The state read from digitalRead() when the button is pressed:
     * HIGH with a pull-down resistor and LOW with a pull-up resistor.
     */
    bool pressedState() const;

    void (*onChangeCallback)(bool) = nullptr;
    void (*onPressCallback)() = nullptr;
    void (*onLongPressCallback)(unsigned int repeatNumber) = nullptr;
    void (*onMultiPressCallback)(unsigned int pressCount) = nullptr;

    unsigned long longPressPeriod = 0;
    unsigned long longPressNextRepeatNumber = 0;
    unsigned long longPressNextTriggerTime = 0;

    unsigned long multiPressCount = 0;
    unsigned long multiPressNextTriggerTime = 0;

    void handlePressChange();

  public:
    /**
     * Register a button on the given pin, with the given resistor configuration (INTERNAL_PULL_UP_RESISTOR is advised).
     */
    Button(uint8_t pin, ButtonResistor resistor);

    /**
     * Ensure to run this function in the Arduino's setup() function, in order to correctly initialize the button.
     */
    void setup();

    /**
     * Set or replace the function to call when the button changes its state: from or to pressed state.
     * The callback function takes a bool argument
     * indicating if the change event is for the button having just being pressed (true) or not (false).
     * The button is already debounced.
     */
    void setOnChange(void (*callback)(bool));

    /**
     * Set or replace the function to call when the button changes its state from not-pressed to pressed.
     * The button is already debounced.
     */
    void setOnPress(void (*callback)());

    /**
     * Set or replace the function to call when the button is pressed for a given time.
     * The function is called periodically until the button is unpressed.
     * E.g. for period=1000 (in milliseconds), the callback function is called after one second of pressing, and then after each second.
     * The callback function takes an int argument indicating the number of periods that elapsed since pressing the button (1, 2, 3...).
     * The button is already debounced.
     */
    void setOnLongPress(unsigned long period, void (*callback)(unsigned int repeatNumber));

    /**
     * Set or replace the function to call when the button is pressed several times in a row, in a short period of time.
     * E.g. to catch double-clicks...
     * The function is called after each press happening under a certain duration apart from the previous press.
     * The callback function takes an int argument indicating the number presses during the last short perios.
     * Example:
     * - the button is pressed the first time, the function is not called,
     * - then within a few milliseconds, the button is pressed again, the function is called with the argument pressCount=2 to signal a double-click,
     * - if pressed again after a few milliseconds, the function is called with the argument pressCount=3 to signal a triple-click...
     * The button is already debounced.
     */
    void setOnMultiPress(void (*callback)(unsigned int pressCount));

    /**
     * Ensure to run this function in the Arduino's loop() function, in order to correctly react to the button.
     */
    void loop();

    /**
     * Returns true if the button is currently pressed.
     * The button is already debounced.
     */
    bool isPressed() const;
};

#endif
