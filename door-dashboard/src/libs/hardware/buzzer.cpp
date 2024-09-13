#include <Arduino.h>

#include "buzzer.h"

#ifdef USE_TONE_AC
#include <toneAC.h>

Buzzer::Buzzer(BuzzerVolume volume)
  : volume(volume)
{
}
#else
Buzzer::Buzzer(uint8_t pin)
  : pin(pin)
{
}
#endif

void Buzzer::setup()
{
#ifndef USE_TONE_AC
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
#endif
}

void Buzzer::loop()
{
  if (this->melody == nullptr) {
    return;
  }

  const unsigned long now = millis();
  if (now > this->nextNoteChangeTimestamp) {
    this->currentNoteIndex++;

    if (this->currentNoteIndex == this->melody->noteCount) {
      this->stop();
      return;
    }

    this->playCurrentNote();
  }
}

void Buzzer::play(const BuzzerMelody *melody) {
  if (muted || this->melody == melody) {
    return;
  }

  this->melody = melody;
  this->currentNoteIndex = 0;

  this->playCurrentNote();
}

void Buzzer::playSynchronously(const BuzzerMelody *melody) {
  if (muted) {
    return;
  }

  for (unsigned int noteIndex = 0; noteIndex < melody->noteCount; noteIndex++) {
    const unsigned int currentNoteFrequency = melody->frequencyDurationPairs[noteIndex * 2];
    const unsigned int currentNoteDuration = melody->frequencyDurationPairs[noteIndex * 2 + 1];

    playNote(currentNoteFrequency);
    delay(currentNoteDuration);
  }
  playNothing();
}

void Buzzer::stop() {
  this->melody = nullptr;
  playNothing();
}

void Buzzer::mute()
{
  muted = true;
  stop();
}

void Buzzer::unmute()
{
  muted = false;
}

void Buzzer::playCurrentNote() {
  const unsigned int currentNoteFrequency = melody->frequencyDurationPairs[this->currentNoteIndex * 2];
  const unsigned int currentNoteDuration = melody->frequencyDurationPairs[this->currentNoteIndex * 2 + 1];

  this->nextNoteChangeTimestamp = millis() + currentNoteDuration;
  playNote(currentNoteFrequency);
}

void Buzzer::playAllNotes() {
  if (muted) {
    return;
  }

  for (unsigned int i = 0; i <= 10000; i += 500) { // In theory, could play up to 65535 Hz... Human is limited to 20 kHz
    Serial.println(i);
    playNote(i);
    delay(400);
  }
  playNothing();
}

void Buzzer::playNote(const unsigned int frequency)
{
#ifdef USE_TONE_AC
  toneAC(frequency, volume, PLAY_FOREVER, /*background=*/true);
#else
  tone(pin, frequency);
#endif
}

void Buzzer::playNothing()
{
#ifdef USE_TONE_AC
  noToneAC();
#else
  noTone(pin);
#endif
}

#ifdef USE_TONE_AC
void Buzzer::setVolume(BuzzerVolume newVolume)
{
  volume = newVolume;
}
#endif
