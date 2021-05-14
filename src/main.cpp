#include "wifi.h"
#include "arduino.h"
#include "emulator.h"

static TaskHandle_t mainTaskHandle;

void setup()
{
    Serial.begin(115200); 

    uint32_t freeHeap32 = heap_caps_get_free_size(MALLOC_CAP_32BIT);
    uint32_t freeHeap8 = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    Serial.printf("Free heap 32BIT: %d, free heap 8BIT: %d\r\n", freeHeap32 - freeHeap8, freeHeap8);

    xTaskCreatePinnedToCore(&EmulatorTaskMain, "emulatorTask", 1024 * 10, NULL, 5, &mainTaskHandle, tskNO_AFFINITY);
    vTaskDelete(NULL);
}

void loop()
{
}
