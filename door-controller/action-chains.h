#ifndef ACTION_CHAINS_H
#define ACTION_CHAINS_H

#include "src/libs/action-chain/action-chain.h"
#include "src/libs/constants/duration-units.h"

#include "demo-mode.h"
#include "hardware.h"
#include "led-patterns.h"
#include "melodies.h"

// Options configuration

const unsigned long OPEN_DURATION_MS = 10 * MINUTES_AS_MS; // Keep synced between controller and dashboard
const unsigned long OPEN_DURATION_MS_DEMO = 10 * SECONDS_AS_MS; // Keep synced between controller and dashboard

const unsigned long WILL_CLOSE_SOON_DURATION_MS = 2 * MINUTES_AS_MS;
const unsigned long WILL_CLOSE_SOON_DURATION_MS_DEMO = 10 * SECONDS_AS_MS;
const unsigned int  WILL_CLOSE_SOON_MELODY_COUNT = 2;

const unsigned long CLOSING_RETRY_DELAY_MS = 25 * SECONDS_AS_MS; // Must be more than enough for the door to open or close completely (timed at 20 seconds: add some margin)
const unsigned long CLOSING_RETRY_DELAY_MS_DEMO = 5 * SECONDS_AS_MS;

ActionOrchestrator actionOrchestrator = ActionOrchestrator();

extern void sendEventStartWillCloseSoon();
extern void sendEventStartAutoClose();
extern void sendEventStartClosingFailed();

// Actions in a chain are run one at a time.
// Only one action-chain is active at a given time: there is only one ActionOrchestrator.
// When a chain is replaced by another one, previously started actions are all cancelled: turn off LEDs, buzzer...

const Action* DOOR_SENSOR_ANOMALY_ACTION_CHAIN[] = {
};
const uint8_t DOOR_SENSOR_ANOMALY_ACTION_CHAIN_SIZE = sizeof(DOOR_SENSOR_ANOMALY_ACTION_CHAIN) / sizeof(Action*);

const Action* CLOSED_ACTION_CHAIN[] = {
};
const uint8_t CLOSED_ACTION_CHAIN_SIZE = sizeof(CLOSED_ACTION_CHAIN) / sizeof(Action*);

const Action* OPEN_ACTION_CHAIN[] = {
  new DemoModeAwareWaitAction(
    OPEN_DURATION_MS,
    OPEN_DURATION_MS_DEMO),
  new RunnableAction([]() { sendEventStartWillCloseSoon(); })
};
const uint8_t OPEN_ACTION_CHAIN_SIZE = sizeof(OPEN_ACTION_CHAIN) / sizeof(Action*);

const Action* WILL_CLOSE_SOON_ACTION_CHAIN[] = {
  new LoopBeginAction(WILL_CLOSE_SOON_MELODY_COUNT),
  new StartPlayingMelodyAction(&buzzer, &WILL_CLOSE_SOON_MELODY),
  new DemoModeAwareWaitAction(
    WILL_CLOSE_SOON_DURATION_MS / WILL_CLOSE_SOON_MELODY_COUNT,
    WILL_CLOSE_SOON_DURATION_MS_DEMO / WILL_CLOSE_SOON_MELODY_COUNT),
  &LOOP_END_ACTION,
  new RunnableAction([]() { sendEventStartAutoClose(); })
};
const uint8_t WILL_CLOSE_SOON_ACTION_CHAIN_SIZE = sizeof(WILL_CLOSE_SOON_ACTION_CHAIN) / sizeof(Action*);

const Action* CLOSING_ACTION_CHAIN[] = {
  // IMPORTANT:
  // * Odd number of attempts, in case the door sensor is bad:
  //   if we opened the door, make sure we close it; then abort attempts to close it.
  // * At least 2 attempts, in case the door someone aborted a "closing" action:
  //   powering on the door will first re-open it, and then close it
  // * In-between retries, make sure to wait enough time to completely open or close the door
  //   to be sure we try our best for a successful closing
  new LoopBeginAction(4),
  new TemporarilyPowerOnRelayAction(&doorRelay1, 1000),
  new WaitAction(150), // Not at the same time, to avoid too much power draw at once: that would render the NRF24L01+ unusable until a hard-reset
  new TemporarilyPowerOnRelayAction(&doorRelay2, 1000),
  new DemoModeAwareWaitAction(
    CLOSING_RETRY_DELAY_MS,
    CLOSING_RETRY_DELAY_MS_DEMO),
  &LOOP_END_ACTION,
  new RunnableAction([]() { sendEventStartClosingFailed(); })
};
const uint8_t CLOSING_ACTION_CHAIN_SIZE = sizeof(CLOSING_ACTION_CHAIN) / sizeof(Action*);

const Action* CLOSING_FAILED_ACTION_CHAIN[] = {
};
const uint8_t CLOSING_FAILED_ACTION_CHAIN_SIZE = sizeof(CLOSING_FAILED_ACTION_CHAIN) / sizeof(Action*);

const Action* KEPT_OPEN_ACTION_CHAIN[] = {
  new TurnOnLedAction(&keptOpenLed)
};
const uint8_t KEPT_OPEN_ACTION_CHAIN_SIZE = sizeof(KEPT_OPEN_ACTION_CHAIN) / sizeof(Action*);

#endif
