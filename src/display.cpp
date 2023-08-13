#include <stdlib.h>
#include <TFT_eSPI.h>
#include "display.h"

TFT_eSPI tft = TFT_eSPI();
extern QueueHandle_t screenDataQueue;
extern QueueHandle_t encoderInputQueue;
extern QueueHandle_t lightsControlQueue;
extern QueueHandle_t carControlOnScreenQueue;
extern QueueHandle_t lightsOnScreenQueue;

void setupDisplay() {
    tft.init();
    tft.setRotation(2);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE,TFT_BLACK);
    tft.drawCentreString("lightbox",TFT_WIDTH/2,0,4);
    xTaskCreatePinnedToCore(
        updateDisplayFunction, /* Function to implement the task */
        "display", /* Name of the task */
        20000,  /* Stack size in words */
        NULL,  /* Task input parameter */
        0,  /* Priority of the task */
        NULL,  /* Task handle. */
        0); /* Core where the task should run */
}
void updateDisplayFunction( void * parameter) {
    char msg[40];
    screendata_struct receivedMessage;
    encoderMessage_struct messageFromEncoder;
    lightsControl_struct messageFromLights;
    carcontrolOnScreen_struct messageFromCarControl;
    
    while(1) {
        if (xQueueReceive(screenDataQueue, (void *)&receivedMessage, 0) == pdTRUE) {
            sprintf(msg,"cmd: %d  ",receivedMessage.cmd);
            tft.drawString(msg,0,25,2);
            sprintf(msg,"crc: %d",receivedMessage.crc);
            tft.drawString(msg,0,45,2);
            sprintf(msg,"speed: %d    ",receivedMessage.speed);
            tft.drawString(msg,0,105,4);
            sprintf(msg,"%d V  ",receivedMessage.batVoltage);
            tft.drawString(msg,0,130,2);
        }
        if (xQueueReceive(encoderInputQueue, (void *)&messageFromEncoder, 0) == pdTRUE) {
            sprintf(msg,"encoder: %d %s  ",messageFromEncoder.encoderPosition,messageFromEncoder.direction == 1 ? "^":"v");
            tft.drawString(msg,0,65,2);
        }
        if (xQueueReceive(lightsOnScreenQueue, (void *)&messageFromLights, 0) == pdTRUE) {
            sprintf(msg,"lgt: %d ",messageFromLights.light);
            tft.drawString(msg,0,140,2);
            sprintf(msg,"eff: %d %d",messageFromLights.command,messageFromLights.value);
            tft.drawString(msg,0,160,2);
        }
        if (xQueueReceive(carControlOnScreenQueue, (void *)&messageFromCarControl, 0) == pdTRUE) {
            sprintf(msg,"ctrl: %d rpm",messageFromCarControl.nMotMax);
            tft.drawString(msg,0,180,2);
            sprintf(msg,"ctrl: %d A",messageFromCarControl.curMax);
            tft.drawString(msg,0,200,2);
        }
    }
}
