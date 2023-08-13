#include <stdlib.h>
#include <WS2812FX.h>
#include "taskmessages.h"

#define bremslicht_L_LED_COUNT 64
#define bremslicht_L_PIN 2
#define bremslicht_R_LED_COUNT 144
#define bremslicht_R_PIN 15
#define longstrip_LED_COUNT 300
#define longstrip_PIN 13

#define LIGHT_L 1
#define LIGHT_R 2
#define LIGHT_SEAT 3
#define LIGHT_BOTTOM 4

#define LIGHTCOMMAND_BRIGHTNESS 1
#define LIGHTCOMMAND_EFFECT 2
#define LIGHTCOMMAND_SPEED 3

void updateDisplayFunction( void * parameter);
void setupDisplay();
void ledtaskFunction( void * parameter);
void setupLEDs();