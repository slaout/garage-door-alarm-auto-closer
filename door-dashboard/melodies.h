#ifndef MELODIES_H
#define MELODIES_H

#include "src/libs/hardware/buzzer-melody.h"
#include "src/libs/hardware/buzzer-pitches.h"

const BuzzerMelody DISCONNECTED_MELODY = INIT_BUZZER_MELODY(
  NOTE_F4, 400,
  NOTE_NONE, 200,
  NOTE_DS4, 400
);

const BuzzerMelody OPEN_MELODY = INIT_BUZZER_MELODY(
  NOTE_C3, 200,
  NOTE_D3, 200,
  NOTE_E3, 200
);

const BuzzerMelody OPEN_FOR_TOO_LONG_MELODY = INIT_BUZZER_MELODY(
  NOTE_C4, 100,
  NOTE_D4, 100,
  NOTE_E4, 100,
  NOTE_NONE, 100,
  NOTE_C4, 100,
  NOTE_D4, 100,
  NOTE_E4, 100
);

const BuzzerMelody CLOSING_MELODY = INIT_BUZZER_MELODY(
  NOTE_E3, 200,
  NOTE_D3, 200,
  NOTE_C3, 200
);

const BuzzerMelody CLOSED_MELODY = INIT_BUZZER_MELODY(
  NOTE_C3, 300,
  NOTE_NONE, 100,
  NOTE_G2, 300
);

const BuzzerMelody CLOSING_FAILED_MELODY = INIT_BUZZER_MELODY(
  NOTE_G4, 200,
  NOTE_NONE, 100,
  NOTE_DS4, 200,
  NOTE_NONE, 100,
  NOTE_B3, 200,
  NOTE_NONE, 100,
  NOTE_G4, 200
);

const BuzzerMelody DOOR_SENSOR_ANOMALY_MELODY = INIT_BUZZER_MELODY(
  NOTE_B3, 200,
  NOTE_NONE, 100,
  NOTE_DS4, 200,
  NOTE_NONE, 100,
  NOTE_G4, 200,
  NOTE_NONE, 100,
  NOTE_B3, 200
);

const BuzzerMelody KEPT_OPEN_MELODY = INIT_BUZZER_MELODY(
  NOTE_C3, 200,
  NOTE_G3, 200,
  NOTE_C4, 200
);

const BuzzerMelody VOLUME_FEEDBACK_MELODY = INIT_BUZZER_MELODY(
  NOTE_C3, 200, // A low and a high note...
  NOTE_G4, 200 // (their volumes are not felt the same)
);

#endif
