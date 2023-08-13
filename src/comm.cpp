#include "HardwareSerial.h"
#include "comm.h"
#include "display.h"
#include "buttons.h"

HardwareSerial ButtonboxPort(1);
HardwareSerial HovercarPort(2);
extern QueueHandle_t screenDataQueue;
extern QueueHandle_t serialDataQueue;
extern QueueHandle_t lightsOnScreenQueue;
extern QueueHandle_t carControlOnScreenQueue;
extern QueueHandle_t lightsControlQueue;

void setupCommports() {
    Serial.begin(115200);
    ButtonboxPort.begin(115200,SERIAL_8N1,BUTTONBOX_RX_PIN,BUTTONBOX_TX_PIN);
    HovercarPort.begin(115200,SERIAL_8N1,HOVERCAR_RX_PIN,HOVERCAR_TX_PIN);
    xTaskCreatePinnedToCore( serialReadButtonBoxFunction, "serialR", 10000, NULL, 0, NULL, 0);
    xTaskCreatePinnedToCore( serialSendFunction, "serialS", 10000, NULL, 0, NULL, 0);
    xTaskCreatePinnedToCore( serialReadHovercarFunction, "readCar", 1000, NULL, 0, NULL, 0);
}
void serialReadHovercarFunction(void* pvParameters) {
    uint8_t idx = 0;                            // Index for new data pointer
    uint16_t bufStartFrame;                     // Buffer Start Frame
    char *p;                                    // Pointer declaration for the new received data
    char incomingByte;
    char incomingBytePrev;
    screendata_struct messageToScreen;
    while(1) {

        // Check for new data availability in the Serial buffer
        if (HovercarPort.available()) {
            incomingByte 	= HovercarPort.read();                                   // Read the incoming byte
            bufStartFrame	= ((uint16_t)(incomingByte) << 8) | incomingBytePrev;       // Construct the start frame
        }
        else {
            continue;
        }

        // Copy received data
        if (bufStartFrame == START_FRAME_CARDATA) {
            p       = (char *)&carData;
            *p++    = incomingBytePrev;
            *p++    = incomingByte;
            idx     = 2;	
        } else if (idx >= 2 && idx < sizeof(HovercarDataPacket)) {  // Save the new received data
            *p++    = incomingByte; 
            idx++;
        }	
        
        // Check if we reached the end of the package
        if (idx == sizeof(HovercarDataPacket)) {
            uint16_t checksum;
            checksum   = (uint16_t)(carData.start ^ carData.cmd1 ^ carData.cmd2 ^ carData.speedR_meas ^ carData.speedL_meas 
                                           ^ carData.batVoltage ^ carData.boardTemp ^ carData.cmdLed);

            // Check validity of the new data
            // and send Message to screen task
            // and send Packet out to buttonbox
            if (carData.start == START_FRAME_CARDATA && checksum == carData.checksum) {
                messageToScreen.speed = carData.speedL_meas;
                messageToScreen.batVoltage = carData.batVoltage;
                messageToScreen.crc = checksum;
                messageToScreen.cmd = carData.cmd1-carData.cmd2;
                xQueueSend(screenDataQueue, (void*)&messageToScreen, 0);
                ButtonboxPort.write((uint8_t *) &carData, sizeof(carData));
            }
            idx = 0;    // Reset the index (it prevents to enter in this if condition in the next cycle)
        }

        // Update previous states
        incomingBytePrev = incomingByte;
    }

}

// receive Lightdata or CarControldata from Buttonbox
void serialReadButtonBoxFunction( void * parameter) {
    uint8_t idx = 0;                            // Index for new data pointer
    uint16_t bufStartFrame;                     // Buffer Start Frame
    char *p;                                    // Pointer declaration for the new received data
    char incomingByte;
    char incomingBytePrev;
    uint16_t frameType = 0;

    lightsControl_struct lightsMessageToLEDs;
    carcontrolOnScreen_struct carControlMessageToScreen;
    while(1) {

        // Check for new data availability in the Serial buffer
        if (ButtonboxPort.available()) {
            incomingByte 	= ButtonboxPort.read();                                   // Read the incoming byte
            bufStartFrame	= ((uint16_t)(incomingByte) << 8) | incomingBytePrev;       // Construct the start frame
        }
        else {
            continue;
        }

        // Copy received data
        if (bufStartFrame == START_FRAME_LIGHTSDATA || bufStartFrame == START_FRAME_CARCONTROLDATA) {
            if(bufStartFrame == START_FRAME_LIGHTSDATA) {
                p       = (char *)&LightsData;
                frameType = START_FRAME_LIGHTSDATA;
            }
            if(bufStartFrame == START_FRAME_CARCONTROLDATA) {
                p       = (char *)&carControl;
                frameType = START_FRAME_CARCONTROLDATA;
            }
            *p++    = incomingBytePrev;
            *p++    = incomingByte;
            idx     = 2;	
        } else if (idx >= 2) { // Save the new received data
            if( ( frameType == START_FRAME_LIGHTSDATA && idx < sizeof(LightsDataPacket) )
             || ( frameType == START_FRAME_CARCONTROLDATA && idx < sizeof(HovercarControlPacket) )
            )
            {
                *p++    = incomingByte; 
                idx++;
            }
        }	
        
        // Check if we reached the end of LightsDataPacket
        if ( frameType == START_FRAME_LIGHTSDATA && idx == sizeof(LightsDataPacket) ) {
            uint16_t checksum;
            checksum = (uint16_t)(LightsData.start ^ LightsData.light ^ LightsData.command ^ LightsData.value);

            // Check validity of the new data
            // and send Message to screen task
            if (LightsData.start == START_FRAME_LIGHTSDATA && checksum == LightsData.checksum) {
                lightsMessageToLEDs.light = LightsData.light;
                lightsMessageToLEDs.command = LightsData.command;
                lightsMessageToLEDs.value = LightsData.value;
                xQueueSend(lightsOnScreenQueue, (void*)&lightsMessageToLEDs, 0);
                xQueueSend(lightsControlQueue, (void*)&lightsMessageToLEDs, 0);                
            }
            idx = 0;    // Reset the index (it prevents to enter in this if condition in the next cycle)
        }

        // Check if we reached the end of HovercarControlPacket
        if ( frameType == START_FRAME_CARCONTROLDATA && idx == sizeof(HovercarControlPacket) ) {
            uint16_t checksum;
            checksum = (uint16_t)(carControl.start ^ carControl.nMotMax ^ carControl.curMax ^ carControl.mode);

            // Check validity of carControl data
            // and send to Hovercar
            if (carControl.start == START_FRAME_CARCONTROLDATA && checksum == carControl.checksum) {
                HovercarPort.write((uint8_t *) &carControl, sizeof(carControl));
                carControlMessageToScreen.curMax = carControl.curMax;
                carControlMessageToScreen.nMotMax = carControl.nMotMax;
                xQueueSend(carControlOnScreenQueue, (void*)&carControlMessageToScreen, 0);
            }
            idx = 0;    // Reset the index (it prevents to enter in this if condition in the next cycle)
        }
        // Update previous states
        incomingBytePrev = incomingByte;
    }
}

void serialSendFunction(void* pvParameters) {
    encoderMessage_struct encoderMessage;
    while(1) {
        // listen to serialDataQueue an send RemoteButtonDataPacket
        if (xQueueReceive(serialDataQueue, (void *)&encoderMessage, portMAX_DELAY ) == pdTRUE) {
          remoteButtonData.start	        = (uint16_t)START_FRAME_REMOTEINPUTDATA;
          remoteButtonData.encoderPosition= encoderMessage.encoderPosition;
          remoteButtonData.encoderDirection= encoderMessage.direction;
          remoteButtonData.checksum       = (uint16_t)(remoteButtonData.start ^ remoteButtonData.encoderPosition ^ remoteButtonData.encoderDirection);
          ButtonboxPort.write((uint8_t *) &remoteButtonData, sizeof(remoteButtonData));
        }
    }
}
