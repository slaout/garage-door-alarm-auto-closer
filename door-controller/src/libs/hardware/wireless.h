#ifndef WIRELESS_H
#define WIRELESS_H

#include <SPI.h>
#include <NRFLite.h>

class Wireless {
  public:
    Wireless(const uint8_t cePin, const uint8_t csnPin);

    void setup(
      const uint8_t radioId,
      const uint8_t destinationRadioId,

      const uint8_t channel = 100 // Sending&receiving channel, can fill 0~128, send and receive must be consistent
    );
    void enableReceptionTimeout(unsigned long receptionTimeout, void (*receptionTimeoutCallback)(bool));
    bool inReceptionTimeout();

    void loop();

    bool send(byte *payload, uint8_t size);
    bool receive(void (*receiveCallback)(byte *payload, uint8_t size));

  private:
    const uint8_t cePin;
    const uint8_t csnPin;

    NRFLite _radio;

    uint8_t _destinationRadioId;

    unsigned long receptionTimeout;
    unsigned long nextReceptionTimeoutTimestamp; // Irrelevant when receptionTimeout is 0
    void (*receptionTimeoutCallback)(bool); // Irrelevant when receptionTimeout is 0
    bool isInReceptionTimeout;

    void (*errorHandler)();

    void resetReceptionTimeout();
};

#endif
