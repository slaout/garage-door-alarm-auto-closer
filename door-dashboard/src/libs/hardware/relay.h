#ifndef RELAY_H
#define RELAY_H

class Relay {
  private:
    /**
     * The output pin to write to.
     */
    const uint8_t pin;

    unsigned long nextPowerOffTimestamp; // 0 if no power-off to do

  public:
    Relay(uint8_t pin)
      : pin(pin)
    {
    }

    void setup()
    {
      pinMode(pin, OUTPUT);
      powerOff();
    }

    void loop()
    {
      if (nextPowerOffTimestamp != 0 && millis() > nextPowerOffTimestamp) {
        powerOff();
      }
    }

    void powerOnDuring(unsigned long duration)
    {
      powerOn();
      nextPowerOffTimestamp = millis() + duration;
    }

    void powerOn() {
      digitalWrite(pin, HIGH);
      nextPowerOffTimestamp = 0;
    }

    void powerOff() {
      digitalWrite(pin, LOW);
      nextPowerOffTimestamp = 0;
    }
};

#endif
