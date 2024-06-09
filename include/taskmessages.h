#include <stdlib.h>

#ifndef TASKMESSAGES_H
#define TASKMESSAGES_H

typedef struct {
  int16_t  cmd;
  int16_t  speed;
  int16_t  batVoltage;
  int16_t  currentDC;
  uint8_t   drivingBackwards;
  uint32_t  revolutions_l;
  uint32_t  revolutions_r;
  uint16_t  crc;
} screendata_struct;

#define MSG_OTA_IDLE 0
#define MSG_OTA_START 1
#define MSG_OTA_PROGRESS 2
#define MSG_OTA_END 3
#define MSG_OTA_ERROR 4

#define WIFI_CONNECTING 1
#define WIFI_CONNECTED 2

typedef struct {
  uint8_t   wifi;
  uint8_t   status;
  float   progress;
  char message[20];
} otaMessage_struct;

typedef struct {
  long encoderPosition;
  uint8_t direction;
} encoderMessage_struct;

typedef struct {
  long wheelPosition_l;
  long wheelPosition_r;
  double  speed_ms;
  double  speed_raw;
} wheelPostionMessage_struct;

typedef struct {
  int8_t   light;
  int8_t   command;
  uint8_t   value;
} lightsControl_struct;

typedef struct {
  uint16_t   nMotMax;
  int16_t    curMax;
} carcontrolOnScreen_struct;

#endif