#include <stdlib.h>
#include <TFT_eSPI.h>
#include <ESP32Encoder.h>
#include "buttons.h"
#include "display.h"

extern QueueHandle_t serialDataForButtonboxQueue;
extern QueueHandle_t screenDataQueue;
extern QueueHandle_t encoderInputQueue;

ESP32Encoder knobOne;

void setupButtons() {
    ESP32Encoder::useInternalWeakPullResistors=UP;
	knobOne.attachHalfQuad(ENCODER_PIN2, ENCODER_PIN1);
	knobOne.clearCount();
    xTaskCreatePinnedToCore(
        readButtonsTask, /* Function to implement the task */
        "buttons", /* Name of the task */
        10000,  /* Stack size in words */
        NULL,  /* Task input parameter */
        0,  /* Priority of the task */
        NULL,  /* Task handle. */
        0); /* Core where the task should run */

}

void readButtonsTask( void * parameter) {
  int32_t encoderPosition, lastEncoderPosition = 0;
  encoderMessage_struct messageToSerial;
  screendata_struct messageToScreen;
  encoderMessage_struct encoderMessage;
  while(1) {
    encoderPosition = knobOne.getCount();
    
    if (lastEncoderPosition != encoderPosition) {
        messageToSerial.encoderPosition=encoderPosition;
        messageToSerial.direction = encoderPosition > lastEncoderPosition ? 1:2;
        encoderMessage.encoderPosition = encoderPosition;
        encoderMessage.direction = messageToSerial.direction;
        xQueueSend(serialDataForButtonboxQueue, (void*)&messageToSerial, 0);
        xQueueSend(encoderInputQueue, (void*)&encoderMessage, 0);
        lastEncoderPosition = encoderPosition;
    }
  }
}
