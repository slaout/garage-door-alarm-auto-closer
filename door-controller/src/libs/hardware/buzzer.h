#ifndef BUZZER_H
#define BUZZER_H

// Comment to use regular tone() from Arduino
// See https://playground.arduino.cc/Code/ToneAC/
#define USE_TONE_AC

#include "buzzer-melody.h"

#ifdef USE_TONE_AC
enum BuzzerVolume {
  VOLUME_OFF = 0,
  VOLUME_1_OF_10 = 1,
  VOLUME_2_OF_10 = 2,
  VOLUME_3_OF_10 = 3,
  VOLUME_4_OF_10 = 4,
  VOLUME_5_OF_10 = 5,
  VOLUME_6_OF_10 = 6,
  VOLUME_7_OF_10 = 7,
  VOLUME_8_OF_10 = 8,
  VOLUME_9_OF_10 = 9,
  VOLUME_10_OF_10 = 10
};
#endif

class Buzzer {
  private:
#ifdef USE_TONE_AC
    /**
     * The volume to output sound, from 1 to 10, 0 for no sound.
     */
    BuzzerVolume volume;
#else
    /**
     * The output pin to write to.
     */
    const uint8_t pin;
#endif

    bool muted;

    const BuzzerMelody *melody;
    uint8_t currentNoteIndex; // Irrelevant when melody is nullptr
    unsigned long nextNoteChangeTimestamp; // Irrelevant when melody is nullptr

    void playCurrentNote();

    void playNote(const unsigned int frequency);
    void playNothing();

  public:
#ifdef USE_TONE_AC
    Buzzer(BuzzerVolume volume = VOLUME_5_OF_10); // Defaults to the Arduino's tone() power
#else
    /**
     * (3), 5, 6, 9, 10, (11) have PWM on most Arduino.
     * (3 and 11 are not advised on https://www.arduino.cc/reference/en/language/functions/advanced-io/tone/)
     */
    Buzzer(uint8_t pin);
#endif
    void setup();
    void loop();
    void play(const BuzzerMelody *melody);
    void playSynchronously(const BuzzerMelody *melody);
    void stop();
    void mute();
    void unmute();
    void playAllNotes();
#ifdef USE_TONE_AC
    void setVolume(BuzzerVolume volume);
#endif
};

#endif
