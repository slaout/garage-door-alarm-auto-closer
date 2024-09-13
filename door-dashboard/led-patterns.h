#ifndef LED_PATTERNS_H
#define LED_PATTERNS_H

#include "src/libs/hardware/led-pattern.h"

const LedPattern DISCONNECTED_LED_PATTERN = INIT_LED_PATTERN(/*ON*/200, /*OFF*/300, /*ON*/200, /*OFF*/300, /*ON*/300);

const LedPattern OPEN_FOR_TOO_LONG_LED_PATTERN = INIT_LED_PATTERN(/*ON*/400, /*OFF*/100);

const LedPattern WILL_AUTO_CLOSE_SOON_LED_PATTERN = INIT_LED_PATTERN(/*ON*/100, /*OFF*/50);

const LedPattern KEPT_OPEN_FOR_TOO_LONG_LED_PATTERN = INIT_LED_PATTERN(
  /*ON*/0,
  /*OFF*/200, /*ON*/400, /*OFF*/200, /*ON*/400, /*OFF*/200,  // Blink out 3 times
  /*ON*/2000,                                                // Wait a bit for the previous animation to be noticed by someone
  /*OFF*/200, /*ON*/400, /*OFF*/200, /*ON*/400, /*OFF*/200); // Blink out again, now that someone is perhaps looking at it

const LedPattern CLOSING_FAILED_LED_PATTERN = INIT_LED_PATTERN(/*ON*/100, /*OFF*/50);

#endif
