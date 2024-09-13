#ifndef ACTION_CHAIN_H
#define ACTION_CHAIN_H

#include "../hardware/buzzer.h"
#include "../hardware/led.h"
#include "../hardware/relay.h"
#include "../hardware/timer.h"

/**
 * Only for internal usage purpose (to implement loops).
 */
enum ActionRole {
  STANDARD,
  LOOP_BEGIN,
  LOOP_END
};

/**
 * Base class for any action to be executed in a chain by an ActionOrchestrator.
 */
class Action
{
  public:
    /**
     * Called when it's the action turn to start in the chain.
     */
    virtual void start() const
    {
    }

    /**
     * The duration of the action in the chain.
     * Zero by default, for one-time actions like setting an output.
     */
    virtual unsigned long duration() const
    {
      return 0;
    }

    /**
     * Called when the duration expired: the next action in the chain will execute.
     */
    virtual void durationExpired() const
    {
    }

    /**
     * Called when the chain ended.
     * This method must undo any execution started in start(), as if the action never started.
     * It must e.g. turn off the LEDs lit on by start(), turn off buzzer, delete timers, etc.
     */
    virtual void destroy() const
    {
    }

    /**
     * Only for internal usage purpose (to implement loops).
     */
    virtual ActionRole role() const
    {
      return STANDARD;
    }
};

/**
 * No-operation action: do nothing.
 * This action is non-blocking: the next action will run just after this one.
 */
const Action NO_ACTION = Action();

/**
 * Start to blink a given LED following a pattern (turn off at chain's end).
 * This action is non-blocking: the next action will run just after this one.
 */
class StartBlinkingLedAction : public Action
{
  private:
    Led *led;
    const LedPattern *pattern;

  public:
    StartBlinkingLedAction(Led *led, const LedPattern *pattern)
      : led(led)
      , pattern(pattern)
    {
    }

    void start() const
    {
      led->blink(pattern);
    }

    void destroy() const
    {
      led->turnOff();
    }
};

/**
 * Start to alternate blinking between two given LEDs (turn off both at chain's end).
 * Each LED is lit `period` milliseconds before the next one is.
 * This action is non-blocking: the next action will run just after this one.
 */
class StartAlternateBlinkingLedsAction : public Action
{
  private:
    Led *led1;
    Led *led2;

    const unsigned int durationArray1[2];
    const unsigned int durationArray2[3];

    const LedPattern pattern1;
    const LedPattern pattern2;

  public:
    StartAlternateBlinkingLedsAction(Led *led1, Led *led2, const unsigned int period)
      : led1(led1)
      , led2(led2)
      , durationArray1{ /*ON*/period, /*OFF*/period }
      , durationArray2{ /*ON*/0, /*OFF*/period, /*ON*/period }
      , pattern1{ 2, durationArray1 }
      , pattern2{ 3, durationArray2 }
    {
    }

    void start() const
    {
      led1->blink(&pattern1);
      led2->blink(&pattern2);
    }

    void destroy() const
    {
      led1->turnOff();
      led2->turnOff();
    }
};

/**
 * Turn on a given LED (turn off at chain's end).
 * This action is non-blocking: the next action will run just after this one.
 */
class TurnOnLedAction : public Action
{
  private:
    Led *led;

  public:
    TurnOnLedAction(Led *led)
      : led(led)
    {
    }

    void start() const
    {
      led->turnOn();
    }

    void destroy() const
    {
      led->turnOff();
    }
};

/**
 * Power on a given relay for the given (usualy short) duration (turn off at chain's end).
 * This action is non-blocking: the next action will run just after this one.
 */
class TemporarilyPowerOnRelayAction : public Action
{
  private:
    Relay *relay;
    unsigned long powerOnDuration;

  public:
    TemporarilyPowerOnRelayAction(Relay *relay, unsigned long powerOnDuration)
      : relay(relay)
      , powerOnDuration(powerOnDuration)
    {
    }

    void start() const
    {
      relay->powerOnDuring(powerOnDuration);
    }

    void destroy() const
    {
      relay->powerOff();
    }
};

/**
 * Start playing the given melody on the buzzer (stop at chain's end).
 * This action is non-blocking: the next action will run just after this one.
 */
class StartPlayingMelodyAction : public Action
{
  private:
    Buzzer *buzzer;
    const BuzzerMelody *melody;
    const bool isMainStateTransition;

    static unsigned long lastMainStateTransitionAction;

  public:
    StartPlayingMelodyAction(Buzzer *buzzer, const BuzzerMelody *melody)
      : buzzer(buzzer)
      , melody(melody)
      , isMainStateTransition(false)
    {
    }

    StartPlayingMelodyAction(Buzzer *buzzer, const BuzzerMelody *melody, const bool isMainStateTransition)
      : buzzer(buzzer)
      , melody(melody)
      , isMainStateTransition(isMainStateTransition)
    {
    }

    void start() const
    {
      if (isMainStateTransition) {
        // No sound for mainStateTransitionMelody => someComboActionMelody => the previous mainStateTransitionMelody
        if (lastMainStateTransitionAction != (unsigned long) this) {
          // No sound on startup, when lastMainStateTransitionAction is 0.
          // Because we play sounds only for user-visible transitions:
          // likely no transition at startup, or worst, after restart by the Restarter class
          if (lastMainStateTransitionAction != 0) {
            buzzer->play(melody);
          }
          lastMainStateTransitionAction = (unsigned long) this;
        }
      } else {
        buzzer->play(melody);
      }
    }

    void destroy() const
    {
      buzzer->stop();
    }
};

unsigned long StartPlayingMelodyAction::lastMainStateTransitionAction = 0;

/**
 * Wait during `waitDuration` milliseconds before running the next action.
 * This action is blocking the chain during its execution.
 */
class WaitAction : public Action
{
  private:
    const unsigned long waitDuration;

  public:
    WaitAction(const unsigned long waitDuration)
      : waitDuration(waitDuration)
    {
    }

    unsigned long duration() const {
      return waitDuration;
    }
};

/**
 * Run a function.
 * This action is non-blocking (except while executing the function): the next action will run just after this one.
 */
class RunnableAction : public Action
{
  private:
    void (*run)();

  public:
    RunnableAction(void (*run)())
      : run(run)
    {
    }

    void start() const
    {
      run();
    }
};

/**
 * Not a real action: marks the begin of a loop, to run the following actions several times:
 * either the given number of iterations, in indefinitely (use the LOOP_BEGIN_ACTION constant for that).
 * This action is non-blocking: the next action will run just after this one.
 */
class LoopBeginAction : public Action
{
  private:
    const unsigned int iterations;

  public:
    LoopBeginAction()
      : iterations(0)
    {
    }

    LoopBeginAction(const unsigned int iterations)
      : iterations(iterations)
    {
    }

    ActionRole role() const
    {
      return LOOP_BEGIN;
    }

    unsigned int totalIterations() const {
      return iterations;
    }

    bool isFinite() const {
      return iterations > 0;
    }
};

/**
 * Marks the begin of an infinite loop, to run the following actions indefinitely.
 * Prefer using this constant for infinite loops.
 * See `new LoopBeginAction(const unsigned int iterations)` to initiate a finite loops.
 */
const LoopBeginAction LOOP_BEGIN_ACTION = LoopBeginAction();

/**
 * Not a real action: marks the end of a loop, to potentially re-run the previous actions several times.
 * Always use the LOOP_END_ACTION constant for that.
 * This action is non-blocking: the next loop or the next action will run just after this one.
 */
class LoopEndAction : public Action
{
  public:
    ActionRole role() const
    {
      return LOOP_END;
    }
};

/**
 * Marks the end of a loop, to potentially re-run the previous actions several times.
 * Always use this constant.
 */
const LoopEndAction LOOP_END_ACTION = LoopEndAction();

/**
 * Orchestrate a chain of actions to run one after the other, in a non-blocking way.
 */
class ActionOrchestrator
{
  private:
    const Action **nextActions = nullptr;
    uint8_t nextSize = 0;

    const Action **actions = nullptr;
    uint8_t size = 0;

    int currentActionIndex = -1;
    unsigned long nextActionSwitchTimestamp = 0; // Irrelevant when currentActionIndex is -1

    int loopBeginIndex = -1;
    int loopEndIndex = -1;
    unsigned int remainingLoopIterations = 0;

    bool isRunning()
    {
      return currentActionIndex >= 0;
    }

    void startNextAction()
    {
      if (isRunning()) {
        expireCurrentAction();
      }

      currentActionIndex++;

      if (currentActionIndex < size) {
        startCurrentAction();
      }
    }

    void startCurrentAction() {
      const Action *action = actions[currentActionIndex];

      if (action->role() == LOOP_BEGIN) {
        const LoopBeginAction *loopBegin = (LoopBeginAction *) action;
        if (loopBegin->isFinite()) {
          // Start the loop on the first time we encounter the begin
          if (loopBeginIndex == -1) {
            remainingLoopIterations = loopBegin->totalIterations();
          }
          loopBeginIndex = currentActionIndex;
          // Start the next iteration (possibly the last one)
          if (remainingLoopIterations > 0) {
            remainingLoopIterations--;
          } else {
            currentActionIndex = loopEndIndex;
          }
        } else {
          loopBeginIndex = currentActionIndex;
        }
        startNextAction();
        return;
      }

      if (action->role() == LOOP_END) {
        if (loopBeginIndex == -1) {
          Serial.println("Ending a not started loop");
          Serial.flush();
          startNextAction();
        } else {
          loopEndIndex = currentActionIndex;
          currentActionIndex = loopBeginIndex;
          startCurrentAction();
        }
        return;
      }

      action->start();

      unsigned long actionDuration = action->duration();
      if (actionDuration == 0) {
        startNextAction();
      } else {
        nextActionSwitchTimestamp = millis() + actionDuration;
      }
    }

    void expireCurrentAction()
    {
      actions[currentActionIndex]->durationExpired();
    }

    void destroyAllActions()
    {
      for (int i = 0; i < size; i++) {
        actions[i]->destroy();
      }
    }

    void resetActionsState(const Action **actions, uint8_t size) {
      this->actions = actions;
      this->size = size;

      currentActionIndex = -1;
      nextActionSwitchTimestamp = 0;

      loopBeginIndex = -1;
      loopEndIndex = -1;
      remainingLoopIterations = 0;
    }

  public:
    ActionOrchestrator()
    {
    }

    /**
     * Ensure to run this function in the Arduino's loop() function, in order to trigger the correct sequence of actions.
     */
    void loop()
    {
      if (nextActions != nullptr) {
        if (isRunning()) {
          stop();
        }

        resetActionsState(nextActions, nextSize);

        nextActions = nullptr;
        nextSize = 0;

        startNextAction();
      } else if (isRunning() && currentActionIndex < size && millis() > nextActionSwitchTimestamp) {
        startNextAction();
      }
    }

    /**
     * Start a new action chain, or restart the current one if passing the current action chain as parameter of this function.
     */
    void start(const Action **actions, uint8_t size)
    {
      nextActions = actions;
      nextSize = size;
    }

    /**
     * Start a new action chain if passing an action chain different that the one currently running as parameter of this function.
     */
    void change(const Action **actions, uint8_t size)
    {
      if (actions != ActionOrchestrator::actions &&
          actions != ActionOrchestrator::nextActions) {
        nextActions = actions;
        nextSize = size;
      }
    }

    /**
     * Stop the execution of the current action chain, if any.
     */
    void stop()
    {
      if (isRunning()) {
        destroyAllActions();
        resetActionsState(nullptr, 0);
      }
    }

    /**
     * Get the currently executed action chain, if any.
     */
    const Action **getCurrentActions()
    {
      return actions;
    }

    /**
     * Get the currently executed action chain size, if any.
     */
    uint8_t getCurrentActionsSize()
    {
      return size;
    }
};

#endif
