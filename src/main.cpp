#include <SPI.h>
#include <WS2812FX.h>
#include <TFT_eSPI.h> // Hardware-specific library

#define bremslicht_L_LED_COUNT 64
#define bremslicht_L_PIN 2
#define bremslicht_R_LED_COUNT 144
#define bremslicht_R_PIN 15
#define RX_PIN 33
#define TX_PIN 32

WS2812FX bremslicht_L = WS2812FX(bremslicht_L_LED_COUNT, bremslicht_L_PIN, NEO_GRB + NEO_KHZ800);
WS2812FX bremslicht_R = WS2812FX(bremslicht_R_LED_COUNT, bremslicht_R_PIN, NEO_GRB + NEO_KHZ800);
TFT_eSPI tft = TFT_eSPI();       // Invoke tft library

uint8_t effects[] = { FX_MODE_RAINBOW_CYCLE, FX_MODE_ICU, FX_MODE_RAINBOW_FIREWORKS, FX_MODE_POPCORN};
//uint8_t effects[] = { FX_MODE_RAINBOW_CYCLE, FX_MODE_BREATH};

// interesting effects:
//TwinkleFOX
//ICU
//Rainbow Fireworks
//Popcorn

struct serialcommand {
  uint16_t effect;
  uint16_t speed;
  uint16_t light;
};

TaskHandle_t ledtask;
TaskHandle_t modechangetask;
void ledtaskFunction( void * parameter);
void modechangeFunction( void * parameter);
void serialcommFunction( void * parameter);


void setup() {
  tft.init();
  tft.setRotation(2);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE,TFT_BLACK);
  tft.drawCentreString("lightbox",TFT_WIDTH/2,0,2);
  Serial.begin(115200);
  bremslicht_L.init();
  bremslicht_L.setBrightness(255);
  bremslicht_L.setSpeed(2000);
  bremslicht_L.setMode(FX_MODE_TWINKLEFOX);
  bremslicht_L.start();
  bremslicht_R.init();
  bremslicht_R.setBrightness(255);
  bremslicht_R.setSpeed(2000);
  bremslicht_R.setMode(FX_MODE_TWINKLEFOX);
  bremslicht_R.start();
  xTaskCreatePinnedToCore(
      ledtaskFunction, /* Function to implement the task */
      "LEDs", /* Name of the task */
      10000,  /* Stack size in words */
      NULL,  /* Task input parameter */
      0,  /* Priority of the task */
      &ledtask,  /* Task handle. */
      0); /* Core where the task should run */
  xTaskCreatePinnedToCore(
      modechangeFunction, /* Function to implement the task */
      "modechange", /* Name of the task */
      10000,  /* Stack size in words */
      NULL,  /* Task input parameter */
      0,  /* Priority of the task */
      &modechangetask,  /* Task handle. */
      0); /* Core where the task should run */
//  xTaskCreatePinnedToCore(
//      serialcommFunction, /* Function to implement the task */
//      "serial", /* Name of the task */
//      10000,  /* Stack size in words */
//      NULL,  /* Task input parameter */
//      0,  /* Priority of the task */
//      NULL,  /* Task handle. */
//      0); /* Core where the task should run */
}


void loop() {
  int speed = 5000;
  delay(250);
  uint32_t colors[] = {ORANGE, BLACK};
  for( int modenumber=0; modenumber < sizeof(effects); modenumber++ ) {
    Serial.println(MODE_NAME(effects[modenumber]));
    tft.fillRect(0,0,TFT_WIDTH,TFT_HEIGHT/2,TFT_BLACK);
    tft.drawCentreString(MODE_NAME(effects[modenumber]),TFT_WIDTH/2,10,2);
    bremslicht_L.setSegment(0, 0, 15, effects[modenumber],  colors , speed, false);
    bremslicht_L.setSegment(1, 16, 16+15, effects[modenumber],  colors  , speed, true);
    bremslicht_L.setSegment(2, 32, 32+15, effects[modenumber],  colors  , speed, false);
    bremslicht_L.setSegment(3, 48, 48+15, effects[modenumber],  colors  , speed, true);
    delay(30000);
    bremslicht_L.resetSegments();
  }
}
void modechangeFunction( void * parameter) {
  for(;;) {
    for( int modenumber=0; modenumber < sizeof(effects); modenumber++ ) {
      tft.fillRect(0,TFT_HEIGHT/2,TFT_WIDTH,TFT_HEIGHT,TFT_BLACK);
      tft.drawCentreString(MODE_NAME(effects[modenumber]),TFT_WIDTH/2,TFT_HEIGHT/2+5,2);
      bremslicht_R.setMode(effects[modenumber]);
      delay(10000);
    }
  }
}

void ledtaskFunction( void * parameter) {
  for(;;) {
    bremslicht_L.service();
    bremslicht_R.service();
  }
}

void serialcommFunction( void * parameter) {
  uint16_t counter;
  byte incomingByte;
  while(1) {
    Serial2.write(counter++);
    if (Serial2.available()) {
      incomingByte 	= Serial.read();
    }

  }
}
