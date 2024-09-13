#include "wireless.h"

Wireless::Wireless(uint8_t cePin, uint8_t csnPin)
  : cePin(cePin)
  , csnPin(csnPin)
{
}

void Wireless::setup(
  const uint8_t radioId,
  const uint8_t destinationRadioId,

  const uint8_t channel // Sending&receiving channel, can fill 0~128, send and receive must be consistent
)
{
  _destinationRadioId = destinationRadioId;

  // About 5ms to send a message (more if there are retries, but we disabled them)
  // 250KBPS is twice slower than both 1MBPS and 2MBPS, and we do not need the more power-hungry 2MBPS
  if (!_radio.init(radioId, cePin, csnPin, NRFLite::BITRATE1MBPS, channel)) {
    Serial.println("Cannot communicate with radio");
    while (1);
  }
}

void Wireless::loop()
{
  if (!isInReceptionTimeout &&
      receptionTimeoutCallback != nullptr &&
      receptionTimeout != 0 &&
      millis() > nextReceptionTimeoutTimestamp
  ) {
    isInReceptionTimeout = true;
    receptionTimeoutCallback(isInReceptionTimeout);
  }
}

bool Wireless::send(byte *payload, uint8_t size)
{
  // IMPORTANT for some faulty devices to not fall into unrecoverable lock (until physically reset)
  // We will re-send the same message ourself anyway
  // See https://arduino.stackexchange.com/questions/55042/how-to-automatically-reset-the-nrf24l01-with-code
  const NRFLite::SendType noAck = NRFLite::NO_ACK;

  return _radio.send(_destinationRadioId, payload, size, noAck);
}

bool Wireless::receive(void (*receiveCallback)(byte *payload, uint8_t size))
{
    bool received = false;
    uint8_t size;

    while ((size = _radio.hasData())) {
        byte bytes[32];
        _radio.readData(bytes);

        receiveCallback(bytes, size);

        resetReceptionTimeout();
        received = true;
    }

    return received;
}

void Wireless::enableReceptionTimeout(unsigned long receptionTimeout, void (*receptionTimeoutCallback)(bool))
{
  this->receptionTimeout = receptionTimeout;
  this->receptionTimeoutCallback = receptionTimeoutCallback;
  resetReceptionTimeout();
}

void Wireless::resetReceptionTimeout()
{
  if (isInReceptionTimeout &&
      receptionTimeoutCallback != nullptr &&
      receptionTimeout != 0
  ) {
    isInReceptionTimeout = false;
    receptionTimeoutCallback(isInReceptionTimeout);
  }

  nextReceptionTimeoutTimestamp = millis() + receptionTimeout;
}

bool Wireless::inReceptionTimeout()
{
  return isInReceptionTimeout;
}
