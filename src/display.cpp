#include <stdlib.h>
#include <TFT_eSPI.h>
#include "display.h"

TFT_eSPI tft = TFT_eSPI();
extern QueueHandle_t screenDataQueue;
extern QueueHandle_t encoderInputQueue;
extern QueueHandle_t lightsControlQueue;
extern QueueHandle_t carControlOnScreenQueue;
extern QueueHandle_t lightsOnScreenQueue;
extern QueueHandle_t otaStatusQueue;

uint8_t updating;
void setupDisplay() {
    tft.init();
    tft.setRotation(2);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE,TFT_BLACK);
    tft.drawCentreString("lightbox",TFT_WIDTH/2,0,4);
    updating = false;
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
    otaMessage_struct otaMessage;

   while(1) {
    taskYIELD();
    if(!updating) {
        if (xQueueReceive(screenDataQueue, (void *)&receivedMessage, 0) == pdTRUE) {
            sprintf(msg,"cmd: %d  ",receivedMessage.cmd);
            tft.drawString(msg,0,25,2);
            sprintf(msg,"crc: %d",receivedMessage.crc);
            tft.drawString(msg,0,45,2);
            sprintf(msg,"speed: %d    ",receivedMessage.speed);
            tft.drawString(msg,0,105,4);
            sprintf(msg,"%2.1f V  ",receivedMessage.batVoltage/100.0);
            tft.drawString(msg,0,130,2);
            sprintf(msg,"%2.1f A  ",receivedMessage.currentDC/100.0);
            tft.drawString(msg,50,130,2);
        }
        if (xQueueReceive(encoderInputQueue, (void *)&messageFromEncoder, 0) == pdTRUE) {
            sprintf(msg,"encoder: %d %s  ",messageFromEncoder.encoderPosition,messageFromEncoder.direction == 1 ? "^":"v");
            tft.drawString(msg,0,65,2);
        }
        if (xQueueReceive(lightsOnScreenQueue, (void *)&messageFromLights, 0) == pdTRUE) {
            sprintf(msg,"lgt: %d ",messageFromLights.light);
            tft.drawString(msg,0,140,2);
            sprintf(msg,"cmd: %d val: %d",messageFromLights.command,messageFromLights.value);
            tft.drawString(msg,0,160,2);
        }
        if (xQueueReceive(carControlOnScreenQueue, (void *)&messageFromCarControl, 0) == pdTRUE) {
            sprintf(msg,"ctrl: %d rpm",messageFromCarControl.nMotMax);
            tft.drawString(msg,0,180,2);
            sprintf(msg,"ctrl: %d A",messageFromCarControl.curMax);
            tft.drawString(msg,0,200,2);
        }
    }
    if (xQueueReceive(otaStatusQueue, (void *)&otaMessage, 0) == pdTRUE) {
        updating=true;
        if(otaMessage.status == MSG_OTA_IDLE && otaMessage.wifi == WIFI_CONNECTING) {
            tft.fillSmoothCircle(10,10,5,TFT_RED,TFT_BLACK);
        }
        if(otaMessage.status == MSG_OTA_IDLE && otaMessage.wifi == WIFI_CONNECTED) {
            tft.fillSmoothCircle(10,10,5,TFT_GREEN,TFT_BLACK);
        }
        if(otaMessage.status == MSG_OTA_START) {
            tft.fillScreen(TFT_DARKGREEN);
            tft.drawCentreString("OTA update",TFT_WIDTH/2,5,4);
            tft.drawRect(0,80,TFT_WIDTH,10,TFT_WHITE);
        }
        if(otaMessage.status == MSG_OTA_END) {
            tft.setTextColor(TFT_WHITE,TFT_DARKGREEN,true);
            tft.drawCentreString("OTA end",TFT_WIDTH/2,5,4);
            updating=false;
        }
        if(otaMessage.status == MSG_OTA_PROGRESS) {
            tft.setTextColor(TFT_WHITE,TFT_DARKGREEN,true);
            sprintf(msg,"%3.0f %%",otaMessage.progress);
            tft.drawCentreString(msg,TFT_WIDTH/2,30,4);
            tft.drawRect(1,81,map(otaMessage.progress,0,100,0,TFT_WIDTH-1),8,TFT_GREEN);
        }
        if(otaMessage.status == MSG_OTA_ERROR) {
            tft.fillScreen(TFT_RED);
            tft.setTextColor(TFT_WHITE, TFT_BLACK,true);
            tft.drawCentreString("OTA ERROR",TFT_WIDTH/2,5,4);
            tft.drawCentreString(otaMessage.message,TFT_WIDTH/2,30,4);
            updating=false;
        }
    }
  }
}
