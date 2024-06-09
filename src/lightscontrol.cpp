#include <stdlib.h>
#include <FastLED.h>
#include "lightscontrol.h"
struct CRGB longstrip[longstrip_LED_COUNT];
struct CRGB rightstrip[bremslicht_R_LED_COUNT];
struct CRGB leftstrip[bremslicht_L_LED_COUNT];


struct segment_struct {
  uint16_t start;
  uint16_t stop;
};
/**
 *           5-28
 *      +----------------------+
 *      |        seg1          |
 *  0-4 | seg0                 | 29-40
 *                        seg2 |
 * 65-74| seg4                 |
 *      |        seg3          |
 *      +----------------------+
 *          41-64
 * 
 * 75-299 sitz
 * 10 leds = 0.3m -> 30 LEDs/m
*/
// #define NUM_SEGMENTS 6
// segment_struct segments[NUM_SEGMENTS] = {
//   {0,4},
//   {5,28},
//   {29,40},
//   {41,64},
//   {65,74},
//   {75,299}
// };

/**
 *  0-35         
 *      +----------------------+
 *      |        seg0          |
 *      |                      | 
 *                             |
 *      |                      |
 *      |        seg1          |
 *      +----------------------+
 * 36-74
 * 
 * 75-299 sitz
 * 10 leds = 0.3m -> 30 LEDs/m
*/
#define NUM_SEGMENTS 3
segment_struct segments[NUM_SEGMENTS] = {
  {0,35},
  {36,74},
  {75,299}
};


//WS2812FX bremslicht_L = WS2812FX(bremslicht_L_LED_COUNT, bremslicht_L_PIN, NEO_GRB + NEO_KHZ800);
//WS2812FX bremslicht_R = WS2812FX(bremslicht_R_LED_COUNT, bremslicht_R_PIN, NEO_GRB + NEO_KHZ800);
//WS2812FX longstrip = WS2812FX(longstrip_LED_COUNT, longstrip_PIN, NEO_GRB + NEO_KHZ800);


extern QueueHandle_t lightsControlQueue;
extern QueueHandle_t wheelPostionQueue;

// interesting effects:
//TwinkleFOX
//ICU
//Rainbow Fireworks
//Popcorn
double valueAverager[10];
int averagerIndex=0;
uint8_t baseHue = 0;
double lastSpeed_ms=0;
uint8_t activeEffect = 0;
int16_t activeEffectParameter=0;

#define BODENLICHT_INTERVAL 30.0
#define PIXEL_PER_METER 144.0

void setupLEDs() {
  int speed = 2000;
//  uint32_t colors[] = {CRGB::Orange,CRGB::Orange,CRGB::Orange,CRGB::Orange, CRGB::Blue,CRGB::Blue,CRGB::Blue,CRGB::Blue};
  uint32_t colors[] = {CRGB::White,CRGB::White,CRGB::Black,CRGB::Black,CRGB::Black,CRGB::Black,CRGB::Black,CRGB::Black,CRGB::Black,CRGB::Black,CRGB::Black,CRGB::Black};
	FastLED.addLeds<WS2812,longstrip_PIN,GRB>(longstrip,longstrip_LED_COUNT);
	FastLED.addLeds<WS2812,bremslicht_L_PIN,GRB>(leftstrip,bremslicht_L_LED_COUNT);
	FastLED.addLeds<WS2812,bremslicht_R_PIN,GRB>(rightstrip,bremslicht_R_LED_COUNT);
	FastLED.setBrightness(64);
  FastLED.setDither(BINARY_DITHER);
  for (uint8_t i = sizeof(valueAverager) / sizeof(valueAverager[0]); i > 0; )
  {
    valueAverager[--i] = 0.0;
  }
  int colorIndex=0;
  for(uint8_t i=0;i< NUM_SEGMENTS; i++) {
    colorIndex=0;
    for(int16_t k=segments[i].start; k<=segments[i].stop;k++) {
      colorIndex++;
      if(colorIndex==sizeof(colors) / sizeof(CRGB::HTMLColorCode)) colorIndex=0;
      longstrip[k] = colors[colorIndex];
    }
  }
  fill_rainbow_circular( &longstrip[segments[0].start], segments[0].stop-segments[0].start+1, 0);
  fill_rainbow_circular( &longstrip[segments[1].start], segments[1].stop-segments[1].start+1, 0, true);
  fill_rainbow( &leftstrip[0], bremslicht_L_LED_COUNT, 0,9);
  fill_rainbow( &rightstrip[0], bremslicht_R_LED_COUNT, 0, 9);
//  fill_solid( &leftstrip[0], bremslicht_L_LED_COUNT, CRGB::Black);
//  fill_solid( &rightstrip[0], bremslicht_R_LED_COUNT, CRGB::Black);
 for(uint8_t i=0;i< 3; i++) {
    leftstrip[i]=CRGB::Orange;
    rightstrip[i]=CRGB::Orange;
  }
  FastLED.show(); 
  xTaskCreatePinnedToCore(ledtaskFunction, "LEDs",10000, NULL, 0, NULL, 1);
  xTaskCreatePinnedToCore(updateBodenlichtFunction, "LEDs3",10000, NULL, 0, NULL, 0);
  xTaskCreatePinnedToCore(updateRainbowFunction, "LEDs2",10000, NULL, 0, NULL, 0);
}

void updateRainbowFunction( void * parameter) {
  while(1) {
    fill_rainbow_circular( &longstrip[segments[2].start], segments[2].stop-segments[2].start, baseHue);
    baseHue++;
    vTaskDelay(50);
  }
}

void addValueToAverager(double val) {
  valueAverager[averagerIndex] = val;
  averagerIndex++;

  if (averagerIndex == sizeof(valueAverager) / sizeof(valueAverager[0])) averagerIndex = 0;
}
double getAverage() {
  double _sum = 0;
  int averagerSize = sizeof(valueAverager) / sizeof(valueAverager[0]);
  for (uint8_t i = 0; i < averagerSize; i++)
  {
    _sum += valueAverager[i];
  }
  return _sum / (double)averagerSize;
}


void overlayEffect() {
  switch(activeEffect) {
    case overlayEffect_singlePixel: 
      longstrip[segments[2].start+activeEffectParameter] = CRGB::White;
  }
}

void updateRainbowFunction( ) {
    fill_rainbow_circular( &longstrip[segments[2].start], segments[2].stop-segments[2].start, baseHue);
    baseHue++;
}
/**
 * speed m/s * pix/m -> pix/s
*/

void updateBodenlichtFunction( void * parameter) {
  CRGB pixelbuffer;
  double move_pixels;
  double pause = 0;
  uint8_t forwardSegment, backwardSegment;
  while(1) {
    move_pixels = lastSpeed_ms * PIXEL_PER_METER; // pixels per second

    if(lastSpeed_ms!= 0) pause = (1000.0 / move_pixels);
    else pause = 250;
    pause = pause / portTICK_PERIOD_MS;
    // Serial.printf("speed: %f m/s, pause: %fms, %f pix/s \n",lastSpeed_ms,pause,move_pixels);

    if(lastSpeed_ms!=0) {
      // move leds backwards in segment 0 and 1 when driving forwards
      if(lastSpeed_ms>0) {
        backwardSegment = 0;
        forwardSegment = 1;
      }
      if(lastSpeed_ms<0) {
        backwardSegment = 1;
        forwardSegment = 0;
      }
      pixelbuffer=longstrip[segments[backwardSegment].start];
      for(int16_t k=segments[backwardSegment].start; k<segments[backwardSegment].stop;k++) {
          longstrip[k]=longstrip[k+1];
      }
      longstrip[segments[backwardSegment].stop]=pixelbuffer;

      if(lastSpeed_ms>0) {
        pixelbuffer=rightstrip[0];
        for(int16_t k=0; k<bremslicht_R_LED_COUNT-1;k++) {
            rightstrip[k]=rightstrip[k+1];
            leftstrip[k]=rightstrip[k];
        }
        rightstrip[bremslicht_R_LED_COUNT-1]=pixelbuffer;
        leftstrip[bremslicht_L_LED_COUNT-1]=pixelbuffer;
      } else {
        pixelbuffer=rightstrip[bremslicht_R_LED_COUNT-1];
        for(int16_t k=bremslicht_R_LED_COUNT-1; k>0;k--) {
            rightstrip[k]=rightstrip[k-1];
            leftstrip[k]=rightstrip[k];
        }
        rightstrip[0]=pixelbuffer;
        leftstrip[0]=pixelbuffer;

      }

      pixelbuffer=longstrip[segments[forwardSegment].stop];
      for(int16_t k=segments[forwardSegment].stop; k>segments[forwardSegment].start;k--) {
          longstrip[k]=longstrip[k-1];
      }
      longstrip[segments[forwardSegment].start]=pixelbuffer;
    }
    // pause so, that we move 1 pixel per update
    vTaskDelay(abs(pause));
  }
}
void ledtaskFunction( void * parameter) {
  lightsControl_struct messageFromLightsControl;
  wheelPostionMessage_struct wheelPostionMessage;
  wheelPostionMessage_struct lastMessage;
  int16_t messageTimeout=100;
//  WS2812FX::Segment* segment;

  for(;;) {
//    bremslicht_L.service();
//    bremslicht_R.service();
    if (xQueueReceive(wheelPostionQueue, (void *)&wheelPostionMessage, 0) == pdTRUE) {
      addValueToAverager(wheelPostionMessage.speed_ms);
      lastSpeed_ms=getAverage();
      messageTimeout=10;
    } else {
      if(messageTimeout > 0) {
        messageTimeout--;
      } else {
        lastSpeed_ms=0;
      }
    }
    if (xQueueReceive(lightsControlQueue, (void *)&messageFromLightsControl, 0) == pdTRUE) {
        switch (messageFromLightsControl.command) {
          case LIGHTCOMMAND_BRIGHTNESS:
            FastLED.setBrightness(messageFromLightsControl.value);break;
          case LIGHTCOMMAND_EFFECT:
            activeEffect = messageFromLightsControl.value;
          case LIGHTCOMMAND_PARAMETER:
            activeEffectParameter = messageFromLightsControl.value;
        }
    }
    // overlayEffect();
    FastLED.show(); 
    taskYIELD();
  }
}
