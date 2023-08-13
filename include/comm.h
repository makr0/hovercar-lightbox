#include <stdlib.h>

#define START_FRAME_CARDATA         0xABCD  // [-] Dataframe from Hoverboard
#define START_FRAME_CARCONTROLDATA  0xABCE  // [-] Dataframe to Hoverboard
#define START_FRAME_REMOTEINPUTDATA 0xF002  // [-] contains buttons and encoderposition (from Lightbox to Buttonbox)
#define START_FRAME_LIGHTSDATA      0xF003  // [-] contains control data for lights (from Buttonbox to Lightbox)
#define BUTTONBOX_RX_PIN 32
#define BUTTONBOX_TX_PIN 33
#define HOVERCAR_RX_PIN 21
#define HOVERCAR_TX_PIN 22

void serialReadButtonBoxFunction( void * parameter);
void serialSendFunction(void* pvParameters);
void serialReadHovercarFunction(void* pvParameters);
void setupCommports();

typedef struct{
  uint16_t  start;
  int16_t   cmd1;
  int16_t   cmd2;
  int16_t   speedR_meas;
  int16_t   speedL_meas;
  int16_t   batVoltage;
  int16_t   boardTemp;
  uint32_t 	rotations;
  uint16_t  checksum;
} HovercarDataPacket;
static HovercarDataPacket carData;

typedef struct{
  uint16_t  start;
  int16_t   nMotMax; // max rotation speed (rpm)
  int16_t   curMax; // max Current (A)
  int16_t   mode;    // driving mode (volt,torque,speed)
  uint16_t  checksum;
} HovercarControlPacket;
static HovercarControlPacket carControl;



typedef struct{
  uint16_t  start;
  int16_t   encoderPosition;
  int16_t   encoderDirection;
  int16_t   button1;
  int16_t   button2;
  uint16_t  checksum;
} RemoteButtonDataPacket;
static RemoteButtonDataPacket remoteButtonData;

#define LIGHT_L 1
#define LIGHT_R 2
#define LIGHT_SEAT 3
#define LIGHT_BOTTOM 4
#define LIGHTS_ALL 0xF

#define LIGHTCOMMAND_BRIGHTNESS 1
#define LIGHTCOMMAND_EFFECT 2
#define LIGHTCOMMAND_SPEED 3

typedef struct{
  uint16_t  start;
  int8_t   light;
  int8_t   command;
  int8_t   value;
  uint16_t  checksum;
} LightsDataPacket;
static LightsDataPacket LightsData;
