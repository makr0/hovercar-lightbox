#include "ota.h"

#define WIFI_SSID "hovercar-ota"     // WiFi network
#define WIFI_PASSWORD "deadbeef42" // WiFi network password

extern QueueHandle_t otaStatusQueue;

void setupOTA() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.mode(WIFI_STA);
  xTaskCreatePinnedToCore(handleOTAFunction, "ota", 10000, NULL, 0, NULL, 0);
}

void handleOTAFunction( void * parameter) {
  otaMessage_struct otaMessage;
  while(WiFi.status() != WL_CONNECTED) {
    vTaskDelay(250);
    otaMessage.status = MSG_OTA_IDLE;
    otaMessage.wifi = WIFI_CONNECTING;
    Serial.print('.');
    xQueueSend(otaStatusQueue, (void*)&otaMessage, 10);
  }
  Serial.print("Connected");
  Serial.println(WiFi.localIP());
  otaMessage.status = MSG_OTA_IDLE;
  otaMessage.wifi = WIFI_CONNECTED;
  xQueueSend(otaStatusQueue, (void*)&otaMessage, 10);
  ArduinoOTA.setHostname("lightbox");
  ArduinoOTA.onStart([&otaMessage]() {
    otaMessage.status = MSG_OTA_START;
    xQueueSend(otaStatusQueue, (void*)&otaMessage, 10);
  });
  ArduinoOTA.onEnd([&otaMessage]() {
    otaMessage.status = MSG_OTA_END;
    xQueueSend(otaStatusQueue, (void*)&otaMessage, 10);
  });
  ArduinoOTA.onProgress([&otaMessage](unsigned int progress, unsigned int total) {
    otaMessage.status = MSG_OTA_PROGRESS;
    otaMessage.progress = progress / (total / 100);
    xQueueSend(otaStatusQueue, (void*)&otaMessage, 10);
  });
  ArduinoOTA.onError([&otaMessage](ota_error_t error) {
    otaMessage.status = MSG_OTA_ERROR;
    if (error == OTA_AUTH_ERROR) sprintf(otaMessage.message,"Auth Failed");
    else if (error == OTA_BEGIN_ERROR) sprintf(otaMessage.message,"Begin Failed");
    else if (error == OTA_CONNECT_ERROR) sprintf(otaMessage.message,"Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) sprintf(otaMessage.message,"Receive Failed");
    else if (error == OTA_END_ERROR) sprintf(otaMessage.message,"End Failed");
    xQueueSend(otaStatusQueue, (void*)&otaMessage, 10);
  });
  ArduinoOTA.begin();
  while(1) {
    ArduinoOTA.handle();
    vTaskDelay(100);
  }
}
