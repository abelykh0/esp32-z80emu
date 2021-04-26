#include "wifi.h"
#include "arduino.h"
#include "emulator.h"

static TaskHandle_t mainTaskHandle;

void setup()
{
    Serial.begin(115200); 
    Serial.write("in setup()\r\n");

    xTaskCreatePinnedToCore(&EmulatorTaskMain, "emulatorTask", 1024 * 8, NULL, 5, &mainTaskHandle, tskNO_AFFINITY);
}

void loop()
{
}
