#ifndef AUTO_CLOSE_FEEDBACK_H
#define AUTO_CLOSE_FEEDBACK_H

#include <EEPROM.h>

class AutoCloseFeedback
{
  private:
    const int EEPROM_ADDRESS = 0;
    const byte AUTO_CLOSED_VALUE = 0b10011001; // Not 0 nor 1, to avoid reading a value written by a previous program

    bool autoClosed = false;

  public:
    void setup()
    {
      byte value = EEPROM.read(EEPROM_ADDRESS);
      autoClosed = (value == AUTO_CLOSED_VALUE);
    }

    void registerSuccessfulAutoClose()
    {
      autoClosed = true;
      EEPROM.update(EEPROM_ADDRESS, AUTO_CLOSED_VALUE);
    }

    void acknowledge()
    {
      autoClosed = false;
      EEPROM.update(EEPROM_ADDRESS, 0);
    }
    
    bool isAutoClosed() const
    {
      return autoClosed;
    }
};

#endif
