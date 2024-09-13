#ifndef LED_PATTERN_H
#define LED_PATTERN_H

struct LedPattern {
  const unsigned int count;
  const unsigned int* durations; // 65536 ms maximum

  unsigned int totalDuration() const;
};

#define INIT_LED_PATTERN(...) \
    { \
        .count = sizeof((unsigned int[]) {__VA_ARGS__}) / sizeof(unsigned int), \
        .durations = (const unsigned int[]) {__VA_ARGS__} \
    }

#endif
