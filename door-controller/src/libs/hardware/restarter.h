#ifndef RESTARTER_H
#define RESTARTER_H

void (*resetArduino)() = 0;

// See https://www.arduino.cc/reference/en/language/functions/time/millis/
// millis() uses an unsigned long: "This number will overflow (go back to zero), after approximately" 49.71 days
// Automatically restart the controller before that delay, to avoid being stuck in untested states (e.g. going back to the "past" and waiting for a "next" tick far into the future...)
class Restarter {
  private:
    static bool (*canRestartNow)();
    static Timer *gracefulRestartTimer;
    static Timer *forcedRestartTimer;

    static bool enteredGracefulRestart;

    static void enterGracefulRestart()
    {
      Serial.println("Graceful restart"); // Initiated: restart as soon as conditions are favorable
      enteredGracefulRestart = true;
      restartNowIfPossible();
      forcedRestartTimer->startOnce();
    }

    static void restartNowIfPossible() {
      if (enteredGracefulRestart && canRestartNow()) {
        Serial.println("Can restart"); // Now: conditions are favorable
        Serial.flush();
        resetArduino();
      }
    }

    static void proceedToForcedRestart()
    {
      Serial.println("Forced restart"); // Now: conditions were not favorable soon enough
      Serial.flush();
      resetArduino();
    }

  public:
    static void setup(
      const unsigned long gracefulRestartDelay,
      const unsigned long forcedRestartDelay,
      bool (*canRestartNow)()
    ) {
      Restarter::canRestartNow = canRestartNow;

      gracefulRestartTimer = new Timer(gracefulRestartDelay, enterGracefulRestart);
      forcedRestartTimer = new Timer(forcedRestartDelay, proceedToForcedRestart);

      gracefulRestartTimer->startOnce();
    }

    static void loop()
    {
      gracefulRestartTimer->loop();
      forcedRestartTimer->loop();
      restartNowIfPossible();
    }
};

bool (*Restarter::canRestartNow)() = nullptr;
Timer *Restarter::gracefulRestartTimer = nullptr;
Timer *Restarter::forcedRestartTimer = nullptr;

bool Restarter::enteredGracefulRestart = false;

#endif
