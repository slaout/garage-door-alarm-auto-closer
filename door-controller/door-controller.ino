#include "action-chains.h" // ➡ You can configure behaviors for each state (lit or blinking LEDs, and melodies to play)
#include "door-state-machine.h"
#include "hardware.h" // ➡ You can configure all Arduino pins and wireless configuration
#include "melodies.h" // ➡ You can configure melodies to play on the buzzer for each state or event

#include "wireless-messages.h" // Defines messages exchanged by radio

#include "src/libs/constants/duration-units.h"
#include "src/libs/hardware/restarter.h"
#include "src/libs/hardware/timer.h"

extern bool sensingDoorIsOpen();

void setup()
{
  Serial.begin(115200);
  Serial.println("Door Controller");

  keptOpenLed.setup();
  disconnectedLed.setup();

  keepOpenButton.setup();
  keepOpenButton.setOnPress(&handleKeepOpenButtonPress);

  doorSensor.setOnChange(&handleDoorSensorChange);
  doorSensor.setup();

  buzzer.setup();

  doorRelay1.setup();
  doorRelay2.setup();

  autoCloseFeedback.setup();

  wireless.setup(WIRELESS_RADIO_ID, WIRELESS_DESTINATION_RADIO_ID, WIRELESS_CHANNEL);
  wireless.enableReceptionTimeout(WIRELESS_RECEPTION_TIMEOUT_MS, &onReceptionTimeout);

  // See documentation of the class Restarter for delay recommendations
  Restarter::setup(
    /* gracefulRestartDelay = */ 48 * DAYS_AS_MS + 12 * HOURS_AS_MS, // Hopefully, once in 3 months, it will be during night for nobody to notice the restart
    /* forcedRestartDelay = */ 12 * HOURS_AS_MS,
    []() {
      return !sensingDoorIsOpen();
    });

  doorStateMachine.start(sensingDoorIsOpen() ? &OPEN_STATE : &CLOSED_STATE);
}

void loop()
{
  keptOpenLed.loop();
  disconnectedLed.loop();

  keepOpenButton.loop();

  doorSensor.loop();

  buzzer.loop();

  doorRelay1.loop();
  doorRelay2.loop();

  loopWireless();

  actionOrchestrator.loop();

  Restarter::loop();
}

extern void sendDoorStatus();

void loopWireless()
{
  bool justReceived = wireless.receive(&handleWirelessDataReceived);
  if (justReceived) {
    sendDoorStatus();
  }

  wireless.loop();
}

void onReceptionTimeout(bool timeout)
{
  if (timeout) {
    disconnectedLed.blink(&DISCONNECTED_LED_PATTERN);
  } else {
    disconnectedLed.turnOff();
  }
}

void handleKeepOpenButtonPress()
{
  doorStateMachine.handleEvent(&EVENT_PRESSED_BUTTON_KEEP_OPEN);
}

void handleDoorSensorChange(RedundantSensor::State state)
{
  if (state == RedundantSensor::ANOMALY) {
    doorStateMachine.handleEvent(&EVENT_DETECTED_DOOR_SENSOR_ANOMALY);
  } else {
    doorStateMachine.handleEvent(sensingDoorIsOpen() ? &EVENT_SENSED_DOOR_IS_OPEN : &EVENT_SENSED_DOOR_IS_CLOSED);
  }
}

void handleWirelessDataReceived(byte data[], uint8_t size)
{
  const byte header = data[0];
  const byte eventId = data[1];
  const byte buttonIndex = data[2];

  bool messageIsErroneous = size != 3 || header != MESSAGE_HEADER;

  if (!messageIsErroneous) {
    messageIsErroneous = handleButtonPressedMessageReceived(eventId, buttonIndex);
  }

  if (messageIsErroneous) {
    Serial.print("Received erroneous message (size ");
    Serial.print(size);
    Serial.print("): ");
    Serial.print(data[0]);
    Serial.print(" ");
    Serial.print(data[1]);
    Serial.print(" ");
    Serial.println(data[2]);
    // Serial.flush();
  }
}

byte ackedButtonPressEventId;

bool handleButtonPressedMessageReceived(uint8_t eventId, uint8_t buttonIndex)
{
  bool isPolling = (eventId == 0 && buttonIndex == MESSAGE_POLLING);
  if (isPolling || eventId == ackedButtonPressEventId) {
    return false;
  }

  if (buttonIndex == MESSAGE_PRESSED_BUTTON_KEEP_OPEN) {
    doorStateMachine.handleEvent(&EVENT_PRESSED_BUTTON_KEEP_OPEN);

  } else if (buttonIndex == MESSAGE_PRESSED_BUTTON_CLOSE) {
    doorStateMachine.handleEvent(&EVENT_PRESSED_BUTTON_CLOSE);

  } else if (buttonIndex == MESSAGE_PRESSED_BUTTON_ACK_AUTO_CLOSED) {
    if (doorSensor.getState() == RedundantSensor::ANOMALY) {
      doorSensor.tryClearAnomaly();
    } else {
      autoCloseFeedback.acknowledge();
    }

  } else if (buttonIndex == MESSAGE_PRESSED_COMBO_TOGGLE_DEMO_MODE) {
    isDemoMode = !isDemoMode;

  } else {
    Serial.print("Received erroneous button press message: ");
    Serial.println(buttonIndex);
    return true;
  }

  ackedButtonPressEventId = eventId;
  return false;
}

void sendDoorStatus()
{
  byte payload[] = {
    MESSAGE_HEADER,
    stateToMessage(doorStateMachine.getCurrentState()),
    autoCloseFeedback.isAutoClosed(),
    isDemoMode,
    ackedButtonPressEventId
  };
  wireless.send(payload, sizeof(payload));
}

byte stateToMessage(const State *state)
{
  if (state == &DOOR_SENSOR_ANOMALY_STATE) {
    return MESSAGE_STATE_DOOR_SENSOR_ANOMALY;
  } else if (state == &CLOSED_STATE) {
    return MESSAGE_STATE_CLOSED;
  } else if (state == &OPEN_STATE) {
    return MESSAGE_STATE_OPEN;
  } else if (state == &KEPT_OPEN_STATE) {
    return MESSAGE_STATE_KEPT_OPEN;
  } else if (state == &WILL_CLOSE_SOON_STATE) {
    return MESSAGE_STATE_WILL_CLOSE_SOON;
  } else if (state == &CLOSING_STATE) {
    return MESSAGE_STATE_CLOSING;
  } else if (state == &CLOSING_FAILED_STATE) {
    return MESSAGE_STATE_CLOSING_FAILED;
  } else {
    Serial.println("Unhandled state in stateToMessage()");
    return 0;
  }
}

bool sensingDoorIsOpen()
{
  RedundantSensor::State state = doorSensor.getState();

  return state == RedundantSensor::PRESSED ||
    state == RedundantSensor::DISAGREEING_TEMPORARILY; // React early to door opening, at first sensor reaction
}
