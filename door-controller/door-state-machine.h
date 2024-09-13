#ifndef DOOR_STATE_MACHINE_H
#define DOOR_STATE_MACHINE_H

#include "src/libs/state-machine/state-machine.h"

#include "action-chains.h"

extern StateMachine doorStateMachine;

// States: start an action chain when entering another state

const State OPEN_STATE = State([]() {
  Serial.println("In OPEN_STATE");
  actionOrchestrator.start(OPEN_ACTION_CHAIN, OPEN_ACTION_CHAIN_SIZE);
});

const State KEPT_OPEN_STATE = State([]() {
  Serial.println("In KEPT_OPEN_STATE");
  actionOrchestrator.start(KEPT_OPEN_ACTION_CHAIN, KEPT_OPEN_ACTION_CHAIN_SIZE);
});

// Its action-chain could have been appended to the OPEN_STATE,
// but having a separate state allows to:
// * trace WILL_CLOSE_SOON=>CLOSING=>CLOSED to detect a successful automatic-closing and
// * send a separate command to the door-controller to raise an alarm
const State WILL_CLOSE_SOON_STATE = State([]() {
  Serial.println("In WILL_CLOSE_SOON_STATE");
  actionOrchestrator.start(WILL_CLOSE_SOON_ACTION_CHAIN, WILL_CLOSE_SOON_ACTION_CHAIN_SIZE);
});

const State CLOSING_STATE = State([]() {
  Serial.println("In CLOSING_STATE");
  actionOrchestrator.start(CLOSING_ACTION_CHAIN, CLOSING_ACTION_CHAIN_SIZE);
});

const State CLOSING_FAILED_STATE = State([]() {
  Serial.println("In CLOSING_FAILED_STATE");
  actionOrchestrator.start(CLOSING_FAILED_ACTION_CHAIN, CLOSING_FAILED_ACTION_CHAIN_SIZE);
});

const State CLOSED_STATE = State([]() {
  if (doorStateMachine.getBeforeLastState() == &WILL_CLOSE_SOON_STATE &&
      doorStateMachine.getLastState() == &CLOSING_STATE) {
    autoCloseFeedback.registerSuccessfulAutoClose();
  }

  Serial.println("In CLOSED_STATE");
  actionOrchestrator.start(CLOSED_ACTION_CHAIN, CLOSED_ACTION_CHAIN_SIZE);
});

const State DOOR_SENSOR_ANOMALY_STATE = State([]() {
  Serial.println("In DOOR_SENSOR_ANOMALY_STATE");
  actionOrchestrator.start(DOOR_SENSOR_ANOMALY_ACTION_CHAIN, DOOR_SENSOR_ANOMALY_ACTION_CHAIN_SIZE);
});

// Events: they are the only triggers that can act on the state machine

const Event EVENT_DETECTED_DOOR_SENSOR_ANOMALY = Event();
const Event EVENT_SENSED_DOOR_IS_CLOSED = Event();
const Event EVENT_SENSED_DOOR_IS_OPEN = Event();

const Event EVENT_PRESSED_BUTTON_KEEP_OPEN = Event();
const Event EVENT_PRESSED_BUTTON_CLOSE = Event();
const Event EVENT_START_WILL_CLOSE_SOON = Event();
const Event EVENT_START_AUTO_CLOSE = Event();
const Event EVENT_START_CLOSING_FAILED = Event();

// Transitions: **from** a given state, when an **event** is triggered, transition **to** the new state

const Transition ANY_TO_DOOR_SENSOR_ANOMALY_TRANSITION = Transition(&State::ANY, &EVENT_DETECTED_DOOR_SENSOR_ANOMALY, &DOOR_SENSOR_ANOMALY_STATE);
const Transition ANY_TO_CLOSED_TRANSITION = Transition(&State::ANY, &EVENT_SENSED_DOOR_IS_CLOSED, &CLOSED_STATE);
const Transition DOOR_SENSOR_ANOMALY_TO_OPEN_TRANSITION = Transition(&DOOR_SENSOR_ANOMALY_STATE, &EVENT_SENSED_DOOR_IS_OPEN, &OPEN_STATE);

const Transition CLOSED_TO_OPEN_TRANSITION = Transition(&CLOSED_STATE, &EVENT_SENSED_DOOR_IS_OPEN, &OPEN_STATE);

const Transition OPEN_TO_KEPT_OPEN_TRANSITION       = Transition(&OPEN_STATE, &EVENT_PRESSED_BUTTON_KEEP_OPEN, &KEPT_OPEN_STATE);
const Transition OPEN_TO_CLOSING_TRANSITION         = Transition(&OPEN_STATE, &EVENT_PRESSED_BUTTON_CLOSE,     &CLOSING_STATE);
const Transition OPEN_TO_WILL_CLOSE_SOON_TRANSITION = Transition(&OPEN_STATE, &EVENT_START_WILL_CLOSE_SOON,    &WILL_CLOSE_SOON_STATE);

const Transition KEPT_OPEN_TO_OPEN_TRANSITION    = Transition(&KEPT_OPEN_STATE, &EVENT_PRESSED_BUTTON_KEEP_OPEN, &OPEN_STATE);
const Transition KEPT_OPEN_TO_CLOSING_TRANSITION = Transition(&KEPT_OPEN_STATE, &EVENT_PRESSED_BUTTON_CLOSE,     &CLOSING_STATE);

const Transition WILL_CLOSE_SOON_TO_CLOSING_BY_BUTTON_TRANSITION = Transition(&WILL_CLOSE_SOON_STATE, &EVENT_PRESSED_BUTTON_CLOSE,     &CLOSING_STATE);
const Transition WILL_CLOSE_SOON_TO_KEPT_OPEN_TRANSITION         = Transition(&WILL_CLOSE_SOON_STATE, &EVENT_PRESSED_BUTTON_KEEP_OPEN, &KEPT_OPEN_STATE);
const Transition WILL_CLOSE_SOON_TO_CLOSING_TRANSITION           = Transition(&WILL_CLOSE_SOON_STATE, &EVENT_START_AUTO_CLOSE,         &CLOSING_STATE);

const Transition CLOSING_TO_OPEN_TRANSITION           = Transition(&CLOSING_STATE, &EVENT_PRESSED_BUTTON_CLOSE,     &OPEN_STATE);
const Transition CLOSING_TO_KEPT_OPEN_TRANSITION      = Transition(&CLOSING_STATE, &EVENT_PRESSED_BUTTON_KEEP_OPEN, &KEPT_OPEN_STATE);
const Transition CLOSING_TO_CLOSING_FAILED_TRANSITION = Transition(&CLOSING_STATE, &EVENT_START_CLOSING_FAILED,     &CLOSING_FAILED_STATE);

const Transition* DOOR_TRANSITION_VALUES[] = {
  &ANY_TO_DOOR_SENSOR_ANOMALY_TRANSITION,
  &ANY_TO_CLOSED_TRANSITION,
  &DOOR_SENSOR_ANOMALY_TO_OPEN_TRANSITION,

  &CLOSED_TO_OPEN_TRANSITION,

  &OPEN_TO_KEPT_OPEN_TRANSITION,
  &OPEN_TO_CLOSING_TRANSITION,
  &OPEN_TO_WILL_CLOSE_SOON_TRANSITION,

  &KEPT_OPEN_TO_OPEN_TRANSITION,
  &KEPT_OPEN_TO_CLOSING_TRANSITION,

  &WILL_CLOSE_SOON_TO_CLOSING_BY_BUTTON_TRANSITION,
  &WILL_CLOSE_SOON_TO_KEPT_OPEN_TRANSITION,
  &WILL_CLOSE_SOON_TO_CLOSING_TRANSITION,

  &CLOSING_TO_OPEN_TRANSITION,
  &CLOSING_TO_KEPT_OPEN_TRANSITION,
  &CLOSING_TO_CLOSING_FAILED_TRANSITION
};

const Transitions DOOR_TRANSITIONS = {
  .length = sizeof(DOOR_TRANSITION_VALUES) / sizeof(Transition*),
  .values = DOOR_TRANSITION_VALUES
};

// State Machine

StateMachine doorStateMachine = StateMachine(&DOOR_TRANSITIONS);

// Actions to send events elsewhere

void sendEventStartWillCloseSoon()
{
  doorStateMachine.handleEvent(&EVENT_START_WILL_CLOSE_SOON);
}

void sendEventStartAutoClose()
{
  doorStateMachine.handleEvent(&EVENT_START_AUTO_CLOSE);
};

void sendEventStartClosingFailed()
{
  doorStateMachine.handleEvent(&EVENT_START_CLOSING_FAILED);
}

#endif
