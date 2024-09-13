#ifndef REDUNDANT_SENSOR
#define REDUNDANT_SENSOR

#include "button.h"

class RedundantSensor {
  public:
    enum State {
      UNPRESSED = 0,
      PRESSED,
      DISAGREEING_TEMPORARILY,
      ANOMALY
    };

    RedundantSensor(
      uint8_t pin1,
      uint8_t pin2,
      ButtonResistor resistor
    )
      : sensor1(pin1, resistor)
      , sensor2(pin2, resistor)
    {
      sensor1.setOnChange(&RedundantSensor::onSensorChangeProxy);
      sensor2.setOnChange(&RedundantSensor::onSensorChangeProxy);
    }

    void setup()
    {
      sensor1.setup();
      sensor2.setup();

      if (sensor1.isPressed() || sensor2.isPressed()) {
        onSensorChange();
      }
    }

    void setOnChange(void (*callback)(State state))
    {
      onChangeCallback = callback;
    }

    void loop()
    {
      callbackThis = this;
      sensor1.loop();

      callbackThis = this; // In case loop() triggered external code calling loop() on another RedundantSensor
      sensor2.loop();

      if (nextAnomalyTimeMs != 0 && millis() >= nextAnomalyTimeMs) {
        if (!anomaly) {
          anomaly = true;
          callChangeCallback();
        }
        nextAnomalyTimeMs = 0;
      }
    }

    State getState()
    {
      if (anomaly) {
        return ANOMALY;
      }

      if (pressed1 == pressed2) {
        return pressed1 ? PRESSED : UNPRESSED;
      } else {
        return DISAGREEING_TEMPORARILY;
      }
    }

    void tryClearAnomaly()
    {
      if (anomaly && pressed1 == pressed2) {
        anomaly = false;
        callChangeCallback();
      }
    }

  private:
    Button sensor1;
    Button sensor2;

    bool pressed1;
    bool pressed2;

    bool anomaly;
    unsigned long nextAnomalyTimeMs;

    void (*onChangeCallback)(State) = nullptr;

    static RedundantSensor *callbackThis;

    void onSensorChange()
    {
      pressed1 = sensor1.isPressed();
      pressed2 = sensor2.isPressed();

      if (!anomaly) {
        if (pressed1 != pressed2) {
          nextAnomalyTimeMs = millis() + 1000;
        } else {
          nextAnomalyTimeMs = 0;
        }

        callChangeCallback();
      }
    }

    void callChangeCallback()
    {
      if (onChangeCallback != nullptr) {
        onChangeCallback(getState());
      }
    }

    #pragma GCC diagnostic ignored "-Wunused-parameter"
    static void onSensorChangeProxy(bool unusedParameterButNeededByButtonCallback)
    {
      RedundantSensor::callbackThis->onSensorChange();
    }

};

RedundantSensor *RedundantSensor::callbackThis = nullptr;

#endif
