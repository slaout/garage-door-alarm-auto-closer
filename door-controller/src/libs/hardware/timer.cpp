#include <Arduino.h>

#include "timer.h"

Timer::Timer(unsigned long duration, void (*runTask)())
  : duration(duration)
  , runTask(runTask)
{
}

void Timer::loop()
{
  if (!started || millis() < runAtTimestamp) {
    return;
  }

  if (infinite) {
    runAtTimestamp += duration;
  } else {
    started = false;
  }

  runTask();
}

void Timer::startOnce()
{
  start(false);
}

void Timer::startInfinite()
{
  start(true);
}

void Timer::stop()
{
  started = false;
}

void Timer::start(bool infinite)
{
  started = true;
  this->infinite = infinite;
  runAtTimestamp = millis() + duration;
}
