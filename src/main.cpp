#include "comm.h"
#include "display.h"
#include "buttons.h"
#include "lightscontrol.h"
#include "ota.h"

QueueHandle_t screenDataQueue = xQueueCreate(2, sizeof(screendata_struct));
QueueHandle_t carControlOnScreenQueue = xQueueCreate(2, sizeof(carcontrolOnScreen_struct));
QueueHandle_t cardataPassthroughQueue = xQueueCreate(10, sizeof(HovercarDataPacket));
QueueHandle_t carConfigPassthroughQueue = xQueueCreate(10, sizeof(HovercarConfigsetPacket));
QueueHandle_t wheelPostionQueue =  xQueueCreate(10, sizeof(wheelPostionMessage_struct));
QueueHandle_t otaStatusQueue =  xQueueCreate(10, sizeof(otaMessage_struct));

QueueHandle_t serialDataForButtonboxQueue = xQueueCreate(8, sizeof(encoderMessage_struct));
QueueHandle_t encoderInputQueue = xQueueCreate(8, sizeof(encoderMessage_struct));
QueueHandle_t lightsControlQueue = xQueueCreate(8, sizeof(lightsControl_struct));
QueueHandle_t lightsOnScreenQueue = xQueueCreate(8, sizeof(lightsControl_struct));

void setup() {
  setupLEDs();
  setupCommports();
  setupDisplay();
  setupButtons();
//  setupOTA();
}

void loop() {
  delay(1000);
}
