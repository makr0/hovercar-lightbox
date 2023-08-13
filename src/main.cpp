#include "comm.h"
#include "display.h"
#include "buttons.h"
#include "lightscontrol.h"

QueueHandle_t screenDataQueue = xQueueCreate(8, sizeof(screendata_struct));
QueueHandle_t carControlOnScreenQueue = xQueueCreate(8, sizeof(carcontrolOnScreen_struct));
QueueHandle_t serialDataQueue = xQueueCreate(8, sizeof(encoderMessage_struct));
QueueHandle_t encoderInputQueue = xQueueCreate(8, sizeof(encoderMessage_struct));
QueueHandle_t lightsControlQueue = xQueueCreate(8, sizeof(lightsControl_struct));
QueueHandle_t lightsOnScreenQueue = xQueueCreate(8, sizeof(lightsControl_struct));


void setup() {
  setupLEDs();
  setupCommports();
  setupDisplay();
  setupButtons();
}

void loop() {
  delay(1000);
}
