#ifndef HARDWARE_H
#define HARDWARE_H

#include <Arduino.h>

#include "src/libs/hardware/button.h"
#include "src/libs/hardware/buzzer.h"
#include "src/libs/hardware/buzzer-volume-manager.h"
#include "src/libs/hardware/led.h"
#include "src/libs/hardware/wireless.h"

#include "melodies.h"

//////// LEDs ////////

Led openLed = Led(2); // Red (blinking when left open for too long)
Led keptOpenLed = Led(3); // Yellow (disabled alarm & closing)
Led closingLed = Led(4); // Green (blinking when closing failed)
Led autoClosedLed = Led(5); // Blue
Led disconnectedLed = Led(6); // White

//////// Buttons ////////

Button keepOpenButton = Button(A5, INTERNAL_PULL_UP_RESISTOR);
Button closeButton = Button(A4, INTERNAL_PULL_UP_RESISTOR);
Button acknowledgeAutoClosedButton = Button(A3, INTERNAL_PULL_UP_RESISTOR);

//////// Buzzer & its volumes ////////

Buzzer buzzer = Buzzer(/* Volume managed by the buzzerVolumeManager below */);

const uint8_t DEFAULT_BUZZER_VOLUME_STEP = 1; // Starts at 0
const BuzzerVolume VOLUME_STEPS[] = {
  VOLUME_OFF,
  VOLUME_1_OF_10,
  VOLUME_4_OF_10,
  VOLUME_7_OF_10,
  VOLUME_9_OF_10,
  VOLUME_10_OF_10
};

const byte VOLUME_STEPS_COUNT = sizeof(VOLUME_STEPS) / sizeof(BuzzerVolume);
extern void showVolumeStepChangeFeedback(uint8_t step);
BuzzerVolumeManager buzzerVolumeManager = BuzzerVolumeManager(
  &buzzer,
  VOLUME_STEPS,
  VOLUME_STEPS_COUNT,
  &showVolumeStepChangeFeedback);

//////// Wireless radio communications ////////

Wireless wireless = Wireless(/*CE=*/A0, /*CSN=*/A1);

const static uint8_t WIRELESS_RADIO_ID = 0; // WIRELESS_RADIO_ID and WIRELESS_DESTINATION_RADIO_ID must be inverted in dashboard and controller
const static uint8_t WIRELESS_DESTINATION_RADIO_ID = 1; // WIRELESS_RADIO_ID and WIRELESS_DESTINATION_RADIO_ID must be inverted in dashboard and controller
const static uint8_t WIRELESS_CHANNEL = 100; // Sending&receiving channel, can fill 0~128, dashboard and controller must use the same channel

const static unsigned long WIRELESS_POLL_DELAY_MS = 50; // No less than 15ms, so buttons stay responsive and/or messages can be sent without overloading radio too much
const static unsigned long WIRELESS_RECEPTION_TIMEOUT_MS = 5 * SECONDS_AS_MS; // Needs to be at least longer than a restart (about 3 seconds) because of Restarter

#endif
