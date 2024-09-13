#ifndef BUZZER_MELODY_H
#define BUZZER_MELODY_H

struct BuzzerMelody {
  const unsigned int noteCount;
  const unsigned int* frequencyDurationPairs; // frequency is 10000??? maximum; duration is 65536 ms maximum

  unsigned long duration() const {
    unsigned long duration = 0;
    for (unsigned int i = 0; i < noteCount; i++) {
      duration += frequencyDurationPairs[i * 2 + 1];
    }
    return duration;
  }
};

#define INIT_BUZZER_MELODY(...) \
    { \
        .noteCount = sizeof((unsigned int[]) {__VA_ARGS__}) / sizeof(unsigned int) / 2, \
        .frequencyDurationPairs = (const unsigned int[]) {__VA_ARGS__} \
    }

#endif
