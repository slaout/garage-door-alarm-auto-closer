#ifndef REMOTE_BUTTONS_SENDER_H
#define REMOTE_BUTTONS_SENDER_H

const uint8_t KEEP_OPEN_BUTTON_ID = 1;
const uint8_t OPEN_BUTTON_ID = 2;
const uint8_t ACK_SUCCESSFUL_CLOSE_BUTTON_ID = 3;

class RemoteButtonsSender {
  public:
    static void onButtonPressed(uint8_t buttonIndex)
    {
      isSending = true;

      currentEventId++;
      if (currentEventId == 0) { // It did overflow
        currentEventId++; // Never send eventId=0, because when receiving an ACK, 0 means no eventId to acknowledge
      }

      currentEventButtonIndex = buttonIndex;
    }

    static void ackEventId(uint8_t eventId)
    {
      if (isSending && currentEventId == eventId) {
        isSending = false;
      }
    }

     static uint8_t getCurrentEventId()
     {
       return isSending ? currentEventId : 0;
     }

     static uint8_t getCurrentEventButtonIndex()
     {
       return isSending ? currentEventButtonIndex : 0;
     }

  private:
    static bool isSending;
    static uint8_t currentEventId;
    static uint8_t currentEventButtonIndex;
};

bool RemoteButtonsSender::isSending = false;
uint8_t RemoteButtonsSender::currentEventId = 0;
uint8_t RemoteButtonsSender::currentEventButtonIndex = 0;

#endif
