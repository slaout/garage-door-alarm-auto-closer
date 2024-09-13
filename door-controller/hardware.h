#ifndef HARDWARE_H
#define HARDWARE_H

#include <Arduino.h>

#include "src/libs/hardware/button.h"
#include "src/libs/hardware/buzzer.h"
#include "src/libs/hardware/led.h"
#include "src/libs/hardware/redundant-sensor.h"
#include "src/libs/hardware/relay.h"
#include "src/libs/hardware/wireless.h"

#include "auto-close-feedback.h"
#include "melodies.h"

Led keptOpenLed = Led(3); // Yellow (disabled alarm & closing)
Led disconnectedLed = Led(6); // White

AutoCloseFeedback autoCloseFeedback = AutoCloseFeedback();

Button keepOpenButton = Button(A5, INTERNAL_PULL_UP_RESISTOR);

RedundantSensor doorSensor = RedundantSensor(7, 8, INTERNAL_PULL_UP_RESISTOR);

Buzzer buzzer = Buzzer(VOLUME_10_OF_10);

Relay doorRelay1 = Relay(A2);
Relay doorRelay2 = Relay(A3);

//////// Wireless radio communications ////////

Wireless wireless = Wireless(/*CE=*/A0, /*CSN=*/A1);

const static uint8_t WIRELESS_RADIO_ID = 1; // WIRELESS_RADIO_ID and WIRELESS_DESTINATION_RADIO_ID must be inverted in dashboard and controller
const static uint8_t WIRELESS_DESTINATION_RADIO_ID = 0; // WIRELESS_RADIO_ID and WIRELESS_DESTINATION_RADIO_ID must be inverted in dashboard and controller
const static uint8_t WIRELESS_CHANNEL = 100; // Sending&receiving channel, can fill 0~128, dashboard and controller must use the same channel

const static unsigned long WIRELESS_RECEPTION_TIMEOUT_MS = 5 * SECONDS_AS_MS; // Needs to be at least longer than a restart (about 3 seconds) because of Restarter

#endif
