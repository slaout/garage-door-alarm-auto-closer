#ifndef BUZZER_VOLUME_MANAGER_H
#define BUZZER_VOLUME_MANAGER_H

#include <EEPROM.h>

#include "buzzer.h"

class BuzzerVolumeManager
{
  private:
    static const int EEPROM_ADDRESS;
    static const byte MIN_STEP_VALUE;

    Buzzer *buzzer;
    const BuzzerVolume *volumeSteps;
    const byte stepCount;
    void (*showVolumeStepChangeFeedback)(uint8_t step);

    uint8_t step;

    void changeToStep(uint8_t newStep)
    {
      if (step != newStep) {
        step = newStep;
        EEPROM.update(EEPROM_ADDRESS, MIN_STEP_VALUE + step);
        setBuzzerVolume();
        showVolumeStepChangeFeedback(step);
      }
    }

    void setBuzzerVolume()
    {
      buzzer->setVolume(volumeSteps[step]);
    }

  public:
    BuzzerVolumeManager(
      Buzzer *buzzer,
      const BuzzerVolume volumeSteps[],
      const byte stepCount,
      void (*showVolumeStepChangeFeedback)(uint8_t step)
    )
      : buzzer(buzzer)
      , volumeSteps(volumeSteps)
      , stepCount(stepCount)
      , showVolumeStepChangeFeedback(showVolumeStepChangeFeedback)
    {
    }

    void setup(uint8_t defaultStep)
    {
      byte value = EEPROM.read(EEPROM_ADDRESS);

      byte maxStepValue = MIN_STEP_VALUE + stepCount - 1;
      if (value >= MIN_STEP_VALUE && value <= maxStepValue) {
        step = value - MIN_STEP_VALUE;
      } else {
        step = defaultStep;
      }

      setBuzzerVolume();
    }

    void increase()
    {
      changeToStep((step + 1) % stepCount);
    }

    void decrease()
    {
      changeToStep((step == 0 ? stepCount : step) - 1);
    }
};

const int BuzzerVolumeManager::EEPROM_ADDRESS = 1;
const byte BuzzerVolumeManager::MIN_STEP_VALUE = 0b01010110; // Avoid reading a value written by a previous program

#endif
