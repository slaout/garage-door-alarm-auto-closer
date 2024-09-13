#ifndef LED_PATTERNS_H
#define LED_PATTERNS_H

#include "src/libs/hardware/led-pattern.h"

const LedPattern DISCONNECTED_LED_PATTERN = INIT_LED_PATTERN(/*ON*/200, /*OFF*/300, /*ON*/200, /*OFF*/300, /*ON*/300);

#endif
