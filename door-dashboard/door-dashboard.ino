#include "action-chains.h" // ➡ You can configure behaviors for each state (lit or blinking LEDs, and melodies to play)
#include "hardware.h" // ➡ You can configure all Arduino pins and wireless configuration
#include "led-patterns.h" // ➡ You can configure how LEDs blink for each state
#include "melodies.h" // ➡ You can configure melodies to play on the buzzer for each state or event

#include "wireless-messages.h" // Defines messages exchanged by radio

#include "src/libs/constants/duration-units.h"
#include "src/libs/hardware/remote-buttons-sender.h"
#include "src/libs/hardware/restarter.h"
#include "src/libs/hardware/timer.h"

// COMBOS
bool muteSoundUntilNextClose = false;

bool isRunningComboFeedback = false;
const Action **actionsAfterComboFeedback = nullptr;
uint8_t actionsAfterComboFeedbackSize = 0;
void startComboAction(const Action **actions, uint8_t size)
{
  isRunningComboFeedback = true;
  actionOrchestrator.start(actions, size);
}
void changeNormalAction(const Action **actions, uint8_t size)
{
  actionsAfterComboFeedback = actions;
  actionsAfterComboFeedbackSize = size;

  if (!isRunningComboFeedback) {
    const Action **oldActions = actionOrchestrator.getCurrentActions();
    actionOrchestrator.change(actions, size);

    // Unmute AFTER orchestrator change, to not play the "Door closed" melody
    if (muteSoundUntilNextClose && isAnOpenActionChain(oldActions) && actions == CLOSED_ACTION_CHAIN) {
      actionOrchestrator.loop(); // change(...) is "asynchronous": trigger another loop() to "apply" the change and start the chain, playing a mute melody just before unmuting the buzzer
      muteSoundUntilNextClose = false;
      buzzer.unmute();
    }
  }
}
void stopRunningComboFeedback()
{
  isRunningComboFeedback = false;
  if (actionsAfterComboFeedback != nullptr) {
    changeNormalAction(actionsAfterComboFeedback, actionsAfterComboFeedbackSize);
  }
}
// COMBOS

void setup()
{
  Serial.begin(115200);
  Serial.println("Door Dashboard");

  disconnectedLed.setup();
  openLed.setup();
  keptOpenLed.setup();
  closingLed.setup();
  autoClosedLed.setup();

  keepOpenButton.setup();
  closeButton.setup();
  acknowledgeAutoClosedButton.setup();

  keepOpenButton.setOnPress(&handleKeepOpenButtonPress);
  closeButton.setOnPress(&handleCloseButtonPress);
  acknowledgeAutoClosedButton.setOnPress(&handleAcknowledgeAutoClosedPress);
  acknowledgeAutoClosedButton.setOnLongPress(5 * SECONDS_AS_MS, &handleAcknowledgeAutoClosedLongPress);
  acknowledgeAutoClosedButton.setOnMultiPress(&handleAcknowledgeAutoClosedMultiPress);

  buzzer.setup();

  buzzerVolumeManager.setup(DEFAULT_BUZZER_VOLUME_STEP);

  wireless.setup(WIRELESS_RADIO_ID, WIRELESS_DESTINATION_RADIO_ID, WIRELESS_CHANNEL);
  wireless.enableReceptionTimeout(WIRELESS_RECEPTION_TIMEOUT_MS, &onReceptionTimeout);

  // See documentation of the class Restarter for delay recommendations
  Restarter::setup(
    /* gracefulRestartDelay = */ 48 * DAYS_AS_MS + 12 * HOURS_AS_MS, // Hopefully, once in 3 months, it will be during night for nobody to notice the restart
    /* forcedRestartDelay = */ 12 * HOURS_AS_MS,
    []() {
      return !wireless.inReceptionTimeout() &&
        actionOrchestrator.getCurrentActions() == CLOSED_ACTION_CHAIN;
    });

  changeNormalAction(WAITING_FIRST_SIGNAL_ACTION_CHAIN, WAITING_FIRST_SIGNAL_ACTION_CHAIN_SIZE);
}

void loop()
{
  disconnectedLed.loop();
  openLed.loop();
  keptOpenLed.loop();
  closingLed.loop();
  autoClosedLed.loop();

  keepOpenButton.loop();
  closeButton.loop();
  acknowledgeAutoClosedButton.loop();

  buzzer.loop();

  loopWireless();

  actionOrchestrator.loop();

  Restarter::loop();
}

unsigned long nextSendingTime = 0;

void loopWireless()
{
  if (nextSendingTime <= millis()) {
    sendMessage();
  }
  wireless.receive(&handleWirelessDataReceived);
  wireless.loop();
}

void onReceptionTimeout(bool timeout)
{
  if (timeout) {
    changeNormalAction(DISCONNECTED_ACTION_CHAIN, DISCONNECTED_ACTION_CHAIN_SIZE);
  }
}

void showVolumeStepChangeFeedback(uint8_t step)
{
  startComboAction(
    getActionChainForVolumeStep(step),
    getActionChainSizeForVolumeStep(step));
}

bool comboStartedForAcknowledgeAutoClosedButton = false;

void handleKeepOpenButtonPress()
{
  if (acknowledgeAutoClosedButton.isPressed()) {
    comboStartedForAcknowledgeAutoClosedButton = true;
    buzzerVolumeManager.decrease();
  } else {
    RemoteButtonsSender::onButtonPressed(MESSAGE_PRESSED_BUTTON_KEEP_OPEN);
  }
}

void handleCloseButtonPress()
{
  if (acknowledgeAutoClosedButton.isPressed()) {
    comboStartedForAcknowledgeAutoClosedButton = true;
    buzzerVolumeManager.increase();
  } else {
    RemoteButtonsSender::onButtonPressed(MESSAGE_PRESSED_BUTTON_CLOSE);
  }
}

void handleAcknowledgeAutoClosedPress()
{
  comboStartedForAcknowledgeAutoClosedButton = false;
  RemoteButtonsSender::onButtonPressed(MESSAGE_PRESSED_BUTTON_ACK_AUTO_CLOSED);
}

void handleAcknowledgeAutoClosedLongPress(unsigned int repeatNumber)
{
  if (!comboStartedForAcknowledgeAutoClosedButton) {
    if (repeatNumber == 1) {
      RemoteButtonsSender::onButtonPressed(MESSAGE_PRESSED_COMBO_TOGGLE_DEMO_MODE);
    } else if (repeatNumber == 2) {
      testLedBrightness();
    }
  }
}

void testLedBrightness()
{
  led1->turnOn();
  led2->turnOn();
  led3->turnOn();
  led4->turnOn();
  led5->turnOn();

  // Wait for the combo press to be finished
  while (acknowledgeAutoClosedButton.isPressed()) {
    acknowledgeAutoClosedButton.loop();
  }

  // Wait for a new press to stop the combo
  while (!acknowledgeAutoClosedButton.isPressed()) {
    acknowledgeAutoClosedButton.loop();
  }

  led1->turnOff();
  led2->turnOff();
  led3->turnOff();
  led4->turnOff();
  led5->turnOff();

  stopRunningComboFeedback();
}

void handleAcknowledgeAutoClosedMultiPress(unsigned int pressCount)
{
  if (pressCount == 4) {
    muteSoundUntilNextClose = !muteSoundUntilNextClose;
    if (muteSoundUntilNextClose) {
      buzzer.mute();
      startComboAction(
        MUTE_SOUND_UNTIL_NEXT_CLOSE_ON_ACTION_CHAIN,
        MUTE_SOUND_UNTIL_NEXT_CLOSE_ON_ACTION_CHAIN_SIZE);
    } else {
      buzzer.unmute();
      startComboAction(
        MUTE_SOUND_UNTIL_NEXT_CLOSE_OFF_ACTION_CHAIN,
        MUTE_SOUND_UNTIL_NEXT_CLOSE_OFF_ACTION_CHAIN_SIZE);
    }
  }
}

void handleWirelessDataReceived(byte data[], uint8_t size)
{
  const byte header = data[0];
  const byte stateMessage = data[1];
  const bool autoClosed = data[2];
  const bool newIsDemoMode = data[3];
  const byte ackedButtonPressEventId = data[4];

  bool messageIsErroneous = size != 5 || header != MESSAGE_HEADER;

  if (!messageIsErroneous) {
    messageIsErroneous = handleStateMessageReceived(stateMessage);
  }

  if (!messageIsErroneous) {
    if (!isRunningComboFeedback) {
      autoClosedLed.set(autoClosed);
    } // else: no need to save it for after the feedback: we pressed ACK to start a combo, so the LED is OFF
    if (isDemoMode != newIsDemoMode) {
      isDemoMode = newIsDemoMode;
      startComboAction(
        DEMO_MODE_TOGGLE_ACTION_CHAIN,
        DEMO_MODE_TOGGLE_ACTION_CHAIN_SIZE);
    }
    if (ackedButtonPressEventId != 0) {
      RemoteButtonsSender::ackEventId(ackedButtonPressEventId);
    }
  }

  if (messageIsErroneous) {
    Serial.print("Received erroneous message (size ");
    Serial.print(size);
    Serial.print("): ");
    Serial.print(data[0]);
    Serial.print(" ");
    Serial.print(data[1]);
    Serial.print(" ");
    Serial.print(data[2]);
    Serial.print(" ");
    Serial.print(data[3]);
    Serial.print(" ");
    Serial.println(data[4]);
    // Serial.flush();
  }
}

bool handleStateMessageReceived(const byte stateMessage)
{
  if (stateMessage == MESSAGE_STATE_DOOR_SENSOR_ANOMALY) {
    changeNormalAction(DOOR_SENSOR_ANOMALY_ACTION_CHAIN, DOOR_SENSOR_ANOMALY_ACTION_CHAIN_SIZE);
  }

  else if (stateMessage == MESSAGE_STATE_CLOSED) {
    changeNormalAction(CLOSED_ACTION_CHAIN, CLOSED_ACTION_CHAIN_SIZE);
  }

  else if (stateMessage == MESSAGE_STATE_OPEN) {
    changeNormalAction(OPEN_ACTION_CHAIN, OPEN_ACTION_CHAIN_SIZE);
  }

  else if (stateMessage == MESSAGE_STATE_KEPT_OPEN) {
    changeNormalAction(KEPT_OPEN_ACTION_CHAIN, KEPT_OPEN_ACTION_CHAIN_SIZE);
  }

  else if (stateMessage == MESSAGE_STATE_WILL_CLOSE_SOON) {
    changeNormalAction(WILL_CLOSE_SOON_ACTION_CHAIN, WILL_CLOSE_SOON_ACTION_CHAIN_SIZE);
  }

  else if (stateMessage == MESSAGE_STATE_CLOSING) {
    changeNormalAction(CLOSING_ACTION_CHAIN, CLOSING_ACTION_CHAIN_SIZE);
  }

  else if (stateMessage == MESSAGE_STATE_CLOSING_FAILED) {
    changeNormalAction(CLOSING_FAILED_ACTION_CHAIN, CLOSING_FAILED_ACTION_CHAIN_SIZE);
  }

  else {
    Serial.print("Received erroneous state message: ");
    Serial.println(stateMessage);
    return true;
  }

  return false;
}

void sendMessage()
{
  nextSendingTime = millis() + WIRELESS_POLL_DELAY_MS;

  uint8_t eventId = RemoteButtonsSender::getCurrentEventId();
  uint8_t buttonIndex = RemoteButtonsSender::getCurrentEventButtonIndex();

  byte payload[] = { MESSAGE_HEADER, eventId, buttonIndex };
  wireless.send(payload, sizeof(payload));
}
