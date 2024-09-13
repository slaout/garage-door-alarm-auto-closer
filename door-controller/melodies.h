#ifndef MELODIES_H
#define MELODIES_H

#include "src/libs/hardware/buzzer-melody.h"
#include "src/libs/hardware/buzzer-pitches.h"

const BuzzerMelody WILL_CLOSE_SOON_MELODY = INIT_BUZZER_MELODY(
  NOTE_E5, 200,
  0, 50,
  NOTE_E5, 200,
  0, 50,
  NOTE_E5, 200,
  0, 50
);

#endif
