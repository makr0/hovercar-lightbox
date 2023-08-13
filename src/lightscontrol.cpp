#include <stdlib.h>
#include "lightscontrol.h"


WS2812FX bremslicht_L = WS2812FX(bremslicht_L_LED_COUNT, bremslicht_L_PIN, NEO_GRB + NEO_KHZ800);
WS2812FX bremslicht_R = WS2812FX(bremslicht_R_LED_COUNT, bremslicht_R_PIN, NEO_GRB + NEO_KHZ800);
WS2812FX longstrip = WS2812FX(longstrip_LED_COUNT, longstrip_PIN, NEO_GRB + NEO_KHZ800);
extern QueueHandle_t lightsControlQueue;

// interesting effects:
//TwinkleFOX
//ICU
//Rainbow Fireworks
//Popcorn

void setupLEDs() {
  int speed = 2000;
  uint32_t colors[] = {ORANGE, BLACK};
  longstrip.init();
  longstrip.setBrightness(200);
  longstrip.setSpeed(2000);
  longstrip.setMode(FX_MODE_RAINBOW_CYCLE);
  longstrip.setSegment(0, 0, 150, FX_MODE_RAINBOW_CYCLE,  colors , speed, false);
  longstrip.setSegment(1, 151, longstrip_LED_COUNT, FX_MODE_RAINBOW_CYCLE,  colors , speed, false);
  longstrip.start();

  bremslicht_L.init();
  bremslicht_L.setBrightness(200);
  bremslicht_L.setSpeed(2000);
  bremslicht_R.init();
  bremslicht_R.setBrightness(200);
  bremslicht_R.setSpeed(2000);

  bremslicht_L.setSegment(0, 0, 15, FX_MODE_RAINBOW_CYCLE,  colors , speed, false);
  bremslicht_L.setSegment(1, 16, 16+15, FX_MODE_RAINBOW_CYCLE,  colors  , speed, true);
  bremslicht_L.setSegment(2, 32, 32+15, FX_MODE_RAINBOW_CYCLE,  colors  , speed, false);
  bremslicht_L.setSegment(3, 48, 48+15, FX_MODE_RAINBOW_CYCLE,  colors  , speed, true);
  bremslicht_R.setSegment(0, 0, 15, FX_MODE_RAINBOW_CYCLE,  colors , speed, false);
  bremslicht_R.setSegment(1, 16, 16+15, FX_MODE_RAINBOW_CYCLE,  colors  , speed, true);
  bremslicht_R.setSegment(2, 32, 32+15, FX_MODE_RAINBOW_CYCLE,  colors  , speed, false);
  bremslicht_R.setSegment(3, 48, 48+15, FX_MODE_RAINBOW_CYCLE,  colors  , speed, true);
  bremslicht_L.start();
  bremslicht_R.start();
  xTaskCreatePinnedToCore(ledtaskFunction, "LEDs",10000, NULL, 0, NULL, 0);
}

void ledtaskFunction( void * parameter) {
  lightsControl_struct messageFromLightsControl;
  WS2812FX::Segment* segment;

  for(;;) {
    bremslicht_L.service();
    bremslicht_R.service();
    longstrip.service();
    if (xQueueReceive(lightsControlQueue, (void *)&messageFromLightsControl, 0) == pdTRUE) {
        if(messageFromLightsControl.light == LIGHT_SEAT) {
          switch (messageFromLightsControl.command) {
            case LIGHTCOMMAND_BRIGHTNESS:
              longstrip.setBrightness(messageFromLightsControl.value);break;
            case LIGHTCOMMAND_SPEED:
                longstrip.setSpeed(1,messageFromLightsControl.value);break;
            case LIGHTCOMMAND_EFFECT:
                longstrip.setMode(1,messageFromLightsControl.value);break;
        }

        }
        if(messageFromLightsControl.light == LIGHT_BOTTOM) {
          switch (messageFromLightsControl.command) {
            case LIGHTCOMMAND_BRIGHTNESS:
              longstrip.setBrightness(messageFromLightsControl.value);break;
            case LIGHTCOMMAND_SPEED:
                longstrip.setSpeed(0,messageFromLightsControl.value);break;
            case LIGHTCOMMAND_EFFECT:
                longstrip.setMode(0,messageFromLightsControl.value);break;
          }
        }
        if(messageFromLightsControl.light == LIGHT_L) {
          switch (messageFromLightsControl.command) {
            case LIGHTCOMMAND_BRIGHTNESS:
              bremslicht_L.setBrightness(messageFromLightsControl.value);break;
            case LIGHTCOMMAND_SPEED:
              bremslicht_L.setSpeed(0,messageFromLightsControl.value);break;
              bremslicht_L.setSpeed(1,messageFromLightsControl.value);break;
              bremslicht_L.setSpeed(2,messageFromLightsControl.value);break;
              bremslicht_L.setSpeed(3,messageFromLightsControl.value);break;
            case LIGHTCOMMAND_EFFECT:
              bremslicht_L.setMode(0,messageFromLightsControl.value);break;
              bremslicht_L.setMode(1,messageFromLightsControl.value);break;
              bremslicht_L.setMode(2,messageFromLightsControl.value);break;
              bremslicht_L.setMode(3,messageFromLightsControl.value);break;
          }
        }
        if(messageFromLightsControl.light == LIGHT_R) {
          switch (messageFromLightsControl.command) {
            case LIGHTCOMMAND_BRIGHTNESS:
              bremslicht_R.setBrightness(messageFromLightsControl.value);break;
            case LIGHTCOMMAND_SPEED:
              bremslicht_R.setSpeed(0,messageFromLightsControl.value);break;
              bremslicht_R.setSpeed(1,messageFromLightsControl.value);break;
              bremslicht_R.setSpeed(2,messageFromLightsControl.value);break;
              bremslicht_R.setSpeed(3,messageFromLightsControl.value);break;
            case LIGHTCOMMAND_EFFECT:
              bremslicht_R.setMode(0,messageFromLightsControl.value);break;
              bremslicht_R.setMode(1,messageFromLightsControl.value);break;
              bremslicht_R.setMode(2,messageFromLightsControl.value);break;
              bremslicht_R.setMode(3,messageFromLightsControl.value);break;
          }
        
        }
        
    }
  }
}