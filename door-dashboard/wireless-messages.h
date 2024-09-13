#ifndef WIRELESS_MESSAGES_H
#define WIRELESS_MESSAGES_H

// Bytes have both 1s and 0s to make sure noise don't send false signals

// Common to controller and dashboard
const byte MESSAGE_HEADER                         = 0b11100111;

// Sent by controller
const byte MESSAGE_STATE_DOOR_SENSOR_ANOMALY      = 0b01101110;
const byte MESSAGE_STATE_CLOSED                   = 0b10101010;
const byte MESSAGE_STATE_OPEN                     = 0b01101000;
const byte MESSAGE_STATE_KEPT_OPEN                = 0b00101011;
const byte MESSAGE_STATE_WILL_CLOSE_SOON          = 0b11110111;
const byte MESSAGE_STATE_CLOSING                  = 0b10010110;
const byte MESSAGE_STATE_CLOSING_FAILED           = 0b11010001;

// Sent by dashboard
const byte MESSAGE_POLLING                        = 0; // Requesting the controller to reply with its status
const byte MESSAGE_PRESSED_BUTTON_KEEP_OPEN       = 0b10110110;
const byte MESSAGE_PRESSED_BUTTON_CLOSE           = 0b11001100;
const byte MESSAGE_PRESSED_BUTTON_ACK_AUTO_CLOSED = 0b00110011;
const byte MESSAGE_PRESSED_COMBO_TOGGLE_DEMO_MODE = 0b01100011;

#endif
