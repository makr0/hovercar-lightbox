#include "HardwareSerial.h"
#include "comm.h"
#include "display.h"
#include "buttons.h"

HardwareSerial ButtonboxPort(1);
HardwareSerial HovercarPort(2);
extern QueueHandle_t screenDataQueue;
extern QueueHandle_t serialDataForButtonboxQueue;
extern QueueHandle_t cardataPassthroughQueue;
extern QueueHandle_t carConfigPassthroughQueue;
extern QueueHandle_t wheelPostionQueue;
extern QueueHandle_t lightsOnScreenQueue;
extern QueueHandle_t carControlOnScreenQueue;
extern QueueHandle_t lightsControlQueue;

void setupCommports() {
    Serial.begin(115200);
    ButtonboxPort.begin(115200,SERIAL_8N1,BUTTONBOX_RX_PIN,BUTTONBOX_TX_PIN);
    HovercarPort.begin(115200,SERIAL_8N1,HOVERCAR_RX_PIN,HOVERCAR_TX_PIN);
    xTaskCreatePinnedToCore( serialReadButtonBoxFunction, "serialR", 10000, NULL, 0, NULL, 0);
    xTaskCreatePinnedToCore( serialSendToButtonboxFunction, "serialS", 10000, NULL, 0, NULL, 0);
    xTaskCreatePinnedToCore( serialReadHovercarFunction, "readCar", 5000, NULL, 0, NULL, 0);
    xTaskCreatePinnedToCore( serialSendToHovercarFunction, "writeCar", 1000, NULL, 0, NULL, 0);
}

void serialReadHovercarFunction(void* pvParameters) {
    uint8_t idx = 0;                            // Index for new data pointer
    uint16_t bufStartFrame;                     // Buffer Start Frame
    char *p;                                    // Pointer declaration for the new received data
    char incomingByte;
    char incomingBytePrev;
    screendata_struct messageToScreen;
    screendata_struct lastMessage;
    wheelPostionMessage_struct wheelPostionMessage;
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
                                           ^ carData.batVoltage ^ carData.cruiseCtrlTarget ^ carData.cruiseCtrlP ^ carData.cruiseCtrlI 
                                           ^ carData.boardTemp ^ carData.currentDC ^ carData.drivingBackwards
                                           ^ carData.revolutions_l ^ carData.revolutions_r ^ carData.overdrive
                                           );

            // Check validity of the new data
            // and send Message to screen task
            // and send Packet out to buttonbox
            if (carData.start == START_FRAME_CARDATA && checksum == carData.checksum) {
                messageToScreen.speed = carData.speedL_meas;
                messageToScreen.batVoltage = carData.batVoltage;
                messageToScreen.crc = checksum;
                messageToScreen.cmd = carData.cmd1-carData.cmd2;
                messageToScreen.drivingBackwards = carData.drivingBackwards;
                messageToScreen.currentDC = carData.currentDC;
                messageToScreen.revolutions_l = carData.revolutions_l;
                messageToScreen.revolutions_r = carData.revolutions_r;
                xQueueSend(screenDataQueue, (void*)&messageToScreen, 10);
                xQueueSend(cardataPassthroughQueue,(void*)&carData,10);
                if(messageToScreen.revolutions_l != lastMessage.revolutions_l
                || messageToScreen.revolutions_r != lastMessage.revolutions_r)
                {
                    wheelPostionMessage.wheelPosition_l = carData.revolutions_l;
                    wheelPostionMessage.wheelPosition_r = carData.revolutions_r;
                    wheelPostionMessage.speed_raw = (carData.speedL_meas - carData.speedR_meas) / 2.0; // average rpm, speeR is negative
                    wheelPostionMessage.speed_ms = wheelPostionMessage.speed_raw
                    / 60.0 * 0.817; // m/s
                    xQueueSend(wheelPostionQueue,(void*)&wheelPostionMessage,10);
                }
                lastMessage = messageToScreen;

            } else {
                Serial.println("motor CRC error");
            }
            idx = 0;    // Reset the index (it prevents to enter in this if condition in the next cycle)
        }


        // Update previous states
        incomingBytePrev = incomingByte;
    }

}


// receive Lightdata or CarConfigdata from Buttonbox
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
        if (bufStartFrame == START_FRAME_LIGHTSDATA || bufStartFrame == START_FRAME_CARCONFIGDATA) {
            if(bufStartFrame == START_FRAME_LIGHTSDATA) {
                p       = (char *)&LightsData;
                frameType = START_FRAME_LIGHTSDATA;
                Serial.print("L");
            }
            if(bufStartFrame == START_FRAME_CARCONFIGDATA) {
                p       = (char *)&HovercarConfigset;
                frameType = START_FRAME_CARCONFIGDATA;
                Serial.print("C");
            }
            *p++    = incomingBytePrev;
            *p++    = incomingByte;
            idx     = 2;	
        } else if (idx >= 2) { // Save the new received data
            if( ( frameType == START_FRAME_LIGHTSDATA && idx < sizeof(LightsDataPacket) )
             || ( frameType == START_FRAME_CARCONFIGDATA && idx < sizeof(HovercarConfigsetPacket) )
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
            Serial.print("received light command ");

            // Check validity of the new data
            // and send Message to screen task
            if (LightsData.start == START_FRAME_LIGHTSDATA && checksum == LightsData.checksum) {
                lightsMessageToLEDs.light = LightsData.light;
                lightsMessageToLEDs.command = LightsData.command;
                lightsMessageToLEDs.value = LightsData.value;
                xQueueSend(lightsOnScreenQueue, (void*)&lightsMessageToLEDs, 0);
                xQueueSend(lightsControlQueue, (void*)&lightsMessageToLEDs, 0);   
                Serial.print("value ");
                Serial.println(LightsData.value);          
            } else {
                Serial.println("Light CRC error");
            }
            idx = 0;
        }

        // Check if we reached the end of HovercarConfigPacket
        if ( frameType == START_FRAME_CARCONFIGDATA && idx == sizeof(HovercarConfigsetPacket) ) {
            uint16_t checksum;
            checksum = (uint16_t)(HovercarConfigset.start ^ HovercarConfigset.controlMode ^ HovercarConfigset.controlType ^ HovercarConfigset.maxRPM ^ HovercarConfigset.curMax ^ HovercarConfigset.fieldWeakEn ^ HovercarConfigset.fieldWeakHi ^ HovercarConfigset.fieldWeakLo ^ HovercarConfigset.fieldWeakMax ^ HovercarConfigset.phaAdvMax ^ HovercarConfigset.cruiseCtrlTarget ^ HovercarConfigset.cruiseCtrlP ^ HovercarConfigset.cruiseCtrlI);

            // Check validity of carControl data
            // and send to Hovercar
            if (HovercarConfigset.start == START_FRAME_CARCONFIGDATA && checksum == HovercarConfigset.checksum) {
                xQueueSend(carConfigPassthroughQueue,(void*)&HovercarConfigset,1000);
                carControlMessageToScreen.curMax = HovercarConfigset.curMax;
                carControlMessageToScreen.nMotMax = HovercarConfigset.maxRPM;
                xQueueSend(carControlOnScreenQueue, (void*)&carControlMessageToScreen, 0);
            }
            idx = 0;
        }
        // Update previous states
        incomingBytePrev = incomingByte;
    }
}

void serialSendToButtonboxFunction(void* pvParameters) {
    encoderMessage_struct encoderMessage;
    HovercarDataPacket carData;
    while(1) {
        // listen to serialDataForButtonboxQueue an send RemoteButtonDataPacket
        if (xQueueReceive(serialDataForButtonboxQueue, (void *)&encoderMessage, 0 ) == pdTRUE) {
          remoteButtonData.start	        = (uint16_t)START_FRAME_REMOTEINPUTDATA;
          remoteButtonData.encoderPosition= encoderMessage.encoderPosition;
          remoteButtonData.encoderDirection= encoderMessage.direction;
          remoteButtonData.checksum       = (uint16_t)(remoteButtonData.start ^ remoteButtonData.encoderPosition ^ remoteButtonData.encoderDirection);
          ButtonboxPort.write((uint8_t *) &remoteButtonData, sizeof(remoteButtonData));
        }
        // pass through carData packet
        if (xQueueReceive(cardataPassthroughQueue, (void *)&carData, 0 ) == pdTRUE) {
          ButtonboxPort.write((uint8_t *) &carData, sizeof(carData));
        }

    }
}

void serialSendToHovercarFunction(void* pvParameters) {
    HovercarConfigsetPacket carConfigData;
    while(1) {
        // pass through carConfig packet
        if (xQueueReceive(carConfigPassthroughQueue, (void *)&carConfigData, 0 ) == pdTRUE) {
            HovercarPort.write((uint8_t *) &carConfigData, sizeof(carConfigData));
/**         Serial.println("sent HovercarConfigset");
            Serial.print("HovercarConfigset.start ");
            Serial.println(HovercarConfigset.start);
            Serial.print("HovercarConfigset.controlMode ");
            Serial.println(HovercarConfigset.controlMode);
            Serial.print("HovercarConfigset.controlType ");
            Serial.println(HovercarConfigset.controlType);
            Serial.print("HovercarConfigset.maxRPM ");
            Serial.println(HovercarConfigset.maxRPM);
            Serial.print("HovercarConfigset.curMax ");
            Serial.println(HovercarConfigset.curMax);
            Serial.print("HovercarConfigset.fieldWeakEn ");
            Serial.println(HovercarConfigset.fieldWeakEn);
            Serial.print("HovercarConfigset.fieldWeakHi ");
            Serial.println(HovercarConfigset.fieldWeakHi);
            Serial.print("HovercarConfigset.fieldWeakLo ");
            Serial.println(HovercarConfigset.fieldWeakLo);
            Serial.print("HovercarConfigset.fieldWeakMax ");
            Serial.println(HovercarConfigset.fieldWeakMax);
            Serial.print("HovercarConfigset.phaAdvMax ");
            Serial.println(HovercarConfigset.phaAdvMax);

            Serial.print("HovercarConfigset.cruiseCtrlTarget ");
            Serial.println(HovercarConfigset.cruiseCtrlTarget);
            Serial.print("HovercarConfigset.cruiseCtrlP ");
            Serial.println(HovercarConfigset.cruiseCtrlP);
            Serial.print("HovercarConfigset.cruiseCtrlI ");
            Serial.println(HovercarConfigset.cruiseCtrlI);
            
            Serial.print("HovercarConfigset.checksum ");
            Serial.println(HovercarConfigset.checksum);
            **/
        }
    }
}
