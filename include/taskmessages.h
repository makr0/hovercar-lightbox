#include <stdlib.h>

#ifndef TASKMESSAGES_H
#define TASKMESSAGES_H

typedef struct {
  int16_t  cmd;
  int16_t  speed;
  int16_t  batVoltage;
  uint16_t  crc;
} screendata_struct;

typedef struct {
  long encoderPosition;
  uint8_t direction;
} encoderMessage_struct;

typedef struct {
  int8_t   light;
  int8_t   command;
  int8_t   value;
} lightsControl_struct;

typedef struct {
  int8_t   nMotMax;
  int8_t   curMax;
} carcontrolOnScreen_struct;

#endif