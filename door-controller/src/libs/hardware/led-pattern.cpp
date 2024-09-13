#include "led-pattern.h"

unsigned int LedPattern::totalDuration() const {
  unsigned int duration = 0;
  for (unsigned int i = 0; i < this->count; i++) {
    duration += this->durations[i];
  }
  return duration;
}
