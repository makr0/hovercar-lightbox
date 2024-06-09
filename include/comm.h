#include <stdlib.h>

#define START_FRAME_CARDATA         0xABCD  // [-] Dataframe from Hoverboard
#define START_FRAME_CARCONFIGDATA  0xABCE  // [-] Dataframe to Hoverboard
#define START_FRAME_REMOTEINPUTDATA 0xF002  // [-] contains buttons and encoderposition (from Lightbox to Buttonbox)
#define START_FRAME_LIGHTSDATA      0xF003  // [-] contains control data for lights (from Buttonbox to Lightbox)
#define BUTTONBOX_RX_PIN 32
#define BUTTONBOX_TX_PIN 33
#define HOVERCAR_RX_PIN 21
#define HOVERCAR_TX_PIN 22

void serialReadButtonBoxFunction( void * parameter);
void serialSendToButtonboxFunction(void* pvParameters);
void serialReadHovercarFunction(void* pvParameters);
void setupCommports();
void serialSendToHovercarFunction(void* pvParameters);

typedef struct{
  uint16_t  start;
  int16_t   cmd1;
  int16_t   cmd2;
  int16_t   speedR_meas;
  int16_t   speedL_meas;
  int16_t   batVoltage;
  int16_t   cruiseCtrlTarget;
  uint16_t  cruiseCtrlP; // cruise control Controller P value
  uint16_t  cruiseCtrlI; // cruise control Controller I value
  int16_t   boardTemp;
  int16_t   currentDC; // A * 100
  uint8_t   drivingBackwards;
  uint32_t  revolutions_l;
  uint32_t  revolutions_r;
  uint8_t   overdrive;
  uint16_t  checksum;
} HovercarDataPacket;
static HovercarDataPacket carData;

#define OPEN_MODE       0               // [-] OPEN mode
#define VLT_MODE        1               // [-] VOLTAGE mode
#define SPD_MODE        2               // [-] SPEED mode
#define TRQ_MODE        3               // [-] TORQUE mode

#define COM_CTRL        0               // [-] Commutation Control Type
#define SIN_CTRL        1               // [-] Sinusoidal Control Type
#define FOC_CTRL        2               // [-] Field Oriented Control (FOC) Type

typedef struct {
  uint16_t       start;
  uint16_t  controlMode; // OPEN_MODE | VLT_MODE | SPD_MODE | TRQ_MODE
  uint16_t  controlType; // COM_CTRL | SIN_CTRL | FOC_CTRL
  uint16_t      maxRPM;
   int16_t       curMax; // max Current (A)
  uint16_t  fieldWeakEn; // Enable field weakening
   int16_t  fieldWeakHi; // Field weakening high RPM
   int16_t  fieldWeakLo; // Field weakening low RPM
   int16_t  fieldWeakMax;// Field weakening max current A(FOC)
   int16_t  phaAdvMax;   // Max Phase Adv angle Deg(SIN)
  uint16_t  cruiseCtrlTarget;
  uint16_t  cruiseCtrlP; // cruise control Controller P value
  uint16_t  cruiseCtrlI; // cruise control Controller I value
  uint16_t  checksum;
} HovercarConfigsetPacket;
static HovercarConfigsetPacket HovercarConfigset;

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
  uint8_t   value;
  uint16_t  checksum;
} LightsDataPacket;
static LightsDataPacket LightsData;
