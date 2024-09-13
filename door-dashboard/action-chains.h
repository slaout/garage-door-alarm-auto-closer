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

const unsigned long OPEN_REMINDER_DELAY_MS = OPEN_DURATION_MS - 2 * MINUTES_AS_MS;
const unsigned long OPEN_REMINDER_DELAY_MS_DEMO = OPEN_DURATION_MS_DEMO - 2 * SECONDS_AS_MS;

const unsigned long KEPT_OPEN_FOR_TOO_LONG_DURATION_MS = 1 * HOURS_AS_MS;
const unsigned long KEPT_OPEN_FOR_TOO_LONG_DURATION_MS_DEMO = 10 * SECONDS_AS_MS;

ActionOrchestrator actionOrchestrator = ActionOrchestrator();

// Actions in a chain are run one at a time.
// Only one action-chain is active at a given time: there is only one ActionOrchestrator.
// When a chain is replaced by another one, previously started actions are all cancelled: turn off LEDs, buzzer...

const Action *WAITING_FIRST_SIGNAL_ACTION_CHAIN[] = {
};
const uint8_t WAITING_FIRST_SIGNAL_ACTION_CHAIN_SIZE = sizeof(WAITING_FIRST_SIGNAL_ACTION_CHAIN) / sizeof(Action*);

const Action *DISCONNECTED_ACTION_CHAIN[] = {
  new StartBlinkingLedAction(&disconnectedLed, &DISCONNECTED_LED_PATTERN)
  // new StartPlayingMelodyAction(&buzzer, &DISCONNECTED_MELODY)
};
const uint8_t DISCONNECTED_ACTION_CHAIN_SIZE = sizeof(DISCONNECTED_ACTION_CHAIN) / sizeof(Action*);

const Action* DOOR_SENSOR_ANOMALY_ACTION_CHAIN[] = {
  new StartAlternateBlinkingLedsAction(&openLed, &closingLed, 300),
  new StartPlayingMelodyAction(&buzzer, &DOOR_SENSOR_ANOMALY_MELODY, true)
};
const uint8_t DOOR_SENSOR_ANOMALY_ACTION_CHAIN_SIZE = sizeof(DOOR_SENSOR_ANOMALY_ACTION_CHAIN) / sizeof(Action*);

const Action* CLOSED_ACTION_CHAIN[] = {
  new StartPlayingMelodyAction(&buzzer, &CLOSED_MELODY, true)
};
const uint8_t CLOSED_ACTION_CHAIN_SIZE = sizeof(CLOSED_ACTION_CHAIN) / sizeof(Action*);

const Action* OPEN_ACTION_CHAIN[] = {
  new TurnOnLedAction(&openLed),
  new StartPlayingMelodyAction(&buzzer, &OPEN_MELODY, true),
  new DemoModeAwareWaitAction(
    OPEN_REMINDER_DELAY_MS,
    OPEN_REMINDER_DELAY_MS_DEMO),
  new StartBlinkingLedAction(&openLed, &OPEN_FOR_TOO_LONG_LED_PATTERN),
  new StartPlayingMelodyAction(&buzzer, &OPEN_FOR_TOO_LONG_MELODY)
};
const uint8_t OPEN_ACTION_CHAIN_SIZE = sizeof(OPEN_ACTION_CHAIN) / sizeof(Action*);

const Action* WILL_CLOSE_SOON_ACTION_CHAIN[] = {
  new StartBlinkingLedAction(&openLed, &WILL_AUTO_CLOSE_SOON_LED_PATTERN),
  new StartPlayingMelodyAction(&buzzer, &OPEN_FOR_TOO_LONG_MELODY, true)
};
const uint8_t WILL_CLOSE_SOON_ACTION_CHAIN_SIZE = sizeof(WILL_CLOSE_SOON_ACTION_CHAIN) / sizeof(Action*);

const Action* CLOSING_ACTION_CHAIN[] = {
  new TurnOnLedAction(&closingLed),
  new StartPlayingMelodyAction(&buzzer, &CLOSING_MELODY, true)
};
const uint8_t CLOSING_ACTION_CHAIN_SIZE = sizeof(CLOSING_ACTION_CHAIN) / sizeof(Action*);

const Action* CLOSING_FAILED_ACTION_CHAIN[] = {
  new StartBlinkingLedAction(&closingLed, &CLOSING_FAILED_LED_PATTERN),
  new StartPlayingMelodyAction(&buzzer, &CLOSING_FAILED_MELODY, true)
};
const uint8_t CLOSING_FAILED_ACTION_CHAIN_SIZE = sizeof(CLOSING_FAILED_ACTION_CHAIN) / sizeof(Action*);

const Action* KEPT_OPEN_ACTION_CHAIN[] = {
  new StartPlayingMelodyAction(&buzzer, &KEPT_OPEN_MELODY, true),
  &LOOP_BEGIN_ACTION,
  new TurnOnLedAction(&keptOpenLed),
  new DemoModeAwareWaitAction(
    KEPT_OPEN_FOR_TOO_LONG_DURATION_MS,
    KEPT_OPEN_FOR_TOO_LONG_DURATION_MS_DEMO),
  new StartBlinkingLedAction(&keptOpenLed, &KEPT_OPEN_FOR_TOO_LONG_LED_PATTERN),
  new WaitAction(KEPT_OPEN_FOR_TOO_LONG_LED_PATTERN.totalDuration()),
  &LOOP_END_ACTION
};
const uint8_t KEPT_OPEN_ACTION_CHAIN_SIZE = sizeof(KEPT_OPEN_ACTION_CHAIN) / sizeof(Action*);

bool isAnOpenActionChain(const Action **actions) {
  return
    actions == OPEN_ACTION_CHAIN ||
    actions == WILL_CLOSE_SOON_ACTION_CHAIN ||
    actions == CLOSING_ACTION_CHAIN ||
    actions == CLOSING_FAILED_ACTION_CHAIN ||
    actions == KEPT_OPEN_ACTION_CHAIN;
}

// Special button-combos

extern void stopRunningComboFeedback();

Led *led1 = &openLed;
Led *led2 = &keptOpenLed;
Led *led3 = &closingLed;
Led *led4 = &autoClosedLed;
Led *led5 = &disconnectedLed;

TurnOnLedAction *turnOnLed1Action = new TurnOnLedAction(led1);
TurnOnLedAction *turnOnLed2Action = new TurnOnLedAction(led2);
TurnOnLedAction *turnOnLed3Action = new TurnOnLedAction(led3);
TurnOnLedAction *turnOnLed4Action = new TurnOnLedAction(led4);
TurnOnLedAction *turnOnLed5Action = new TurnOnLedAction(led5);

const Action* VOLUME_FEEDBACK_ACTION_CHAIN[] = {
  turnOnLed5Action,
  turnOnLed4Action,
  turnOnLed3Action,
  turnOnLed2Action,
  turnOnLed1Action,
  new StartPlayingMelodyAction(&buzzer, &VOLUME_FEEDBACK_MELODY),
  new WaitAction(1 * SECONDS_AS_MS),
  new RunnableAction(&stopRunningComboFeedback)
};

const Action** getActionChainForVolumeStep(uint8_t step)
{
  return VOLUME_FEEDBACK_ACTION_CHAIN + (5 - step);
}

uint8_t getActionChainSizeForVolumeStep(uint8_t step)
{
  return step + 3;
}

class SetLedStripAction : public Action
{
  private:
    const char *strip;

    void set(const char *strip) const
    {
      led1->set(strip[0] != ' ');
      led2->set(strip[1] != ' ');
      led3->set(strip[2] != ' ');
      led4->set(strip[3] != ' ');
      led5->set(strip[4] != ' ');
    }

  public:
    SetLedStripAction(const char *strip)
      : strip(strip)
    {
    }

    void start() const
    {
      set(strip);
    }

    void destroy() const
    {
      set("     ");
    }
};

SetLedStripAction *turnOnOnlyLed1Action = new SetLedStripAction("O    ");
SetLedStripAction *turnOnOnlyLed2Action = new SetLedStripAction(" O   ");
SetLedStripAction *turnOnOnlyLed3Action = new SetLedStripAction("  O  ");
SetLedStripAction *turnOnOnlyLed4Action = new SetLedStripAction("   O ");
SetLedStripAction *turnOnOnlyLed5Action = new SetLedStripAction("    O");

const unsigned long DEMO_MODE_ANIMATION_FRAME_DURATION_MS = 400;
const unsigned long DEMO_MODE_ANIMATION_FRAME_DURATION_MS_DEMO = 75;
const Action *DEMO_MODE_ANIMATION_FRAME_WAIT = new DemoModeAwareWaitAction(
  DEMO_MODE_ANIMATION_FRAME_DURATION_MS,
  DEMO_MODE_ANIMATION_FRAME_DURATION_MS_DEMO);
const Action* DEMO_MODE_TOGGLE_ACTION_CHAIN[] = {
  turnOnOnlyLed1Action, DEMO_MODE_ANIMATION_FRAME_WAIT,
  turnOnOnlyLed2Action, DEMO_MODE_ANIMATION_FRAME_WAIT,
  turnOnOnlyLed3Action, DEMO_MODE_ANIMATION_FRAME_WAIT,
  turnOnOnlyLed4Action, DEMO_MODE_ANIMATION_FRAME_WAIT,
  turnOnOnlyLed5Action, DEMO_MODE_ANIMATION_FRAME_WAIT,
  new RunnableAction(&stopRunningComboFeedback)
};
const uint8_t DEMO_MODE_TOGGLE_ACTION_CHAIN_SIZE = sizeof(DEMO_MODE_TOGGLE_ACTION_CHAIN) / sizeof(Action*);

SetLedStripAction *stripLedBigAction = new SetLedStripAction("OOOOO");
SetLedStripAction *stripLedMediumAction = new SetLedStripAction(" OOO ");
SetLedStripAction *stripLedSmallAction = turnOnOnlyLed3Action;
SetLedStripAction *stripLedNoneAction = new SetLedStripAction("     ");
const Action *MUTE_SOUND_UNTIL_NEXT_CLOSE_WAIT = new WaitAction(200);

const Action* MUTE_SOUND_UNTIL_NEXT_CLOSE_ON_ACTION_CHAIN[] = {
  stripLedBigAction, MUTE_SOUND_UNTIL_NEXT_CLOSE_WAIT,
  stripLedMediumAction, MUTE_SOUND_UNTIL_NEXT_CLOSE_WAIT,
  stripLedSmallAction, MUTE_SOUND_UNTIL_NEXT_CLOSE_WAIT,
  stripLedNoneAction, MUTE_SOUND_UNTIL_NEXT_CLOSE_WAIT,
  new RunnableAction(&stopRunningComboFeedback)
};
const uint8_t MUTE_SOUND_UNTIL_NEXT_CLOSE_ON_ACTION_CHAIN_SIZE = sizeof(MUTE_SOUND_UNTIL_NEXT_CLOSE_ON_ACTION_CHAIN) / sizeof(Action*);

const Action* MUTE_SOUND_UNTIL_NEXT_CLOSE_OFF_ACTION_CHAIN[] = {
  stripLedNoneAction, MUTE_SOUND_UNTIL_NEXT_CLOSE_WAIT,
  stripLedSmallAction, MUTE_SOUND_UNTIL_NEXT_CLOSE_WAIT,
  stripLedMediumAction, MUTE_SOUND_UNTIL_NEXT_CLOSE_WAIT,
  stripLedBigAction, MUTE_SOUND_UNTIL_NEXT_CLOSE_WAIT,
  new RunnableAction(&stopRunningComboFeedback)
};
const uint8_t MUTE_SOUND_UNTIL_NEXT_CLOSE_OFF_ACTION_CHAIN_SIZE = sizeof(MUTE_SOUND_UNTIL_NEXT_CLOSE_OFF_ACTION_CHAIN) / sizeof(Action*);

#endif
