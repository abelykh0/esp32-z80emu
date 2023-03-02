#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "settings.h"
#include "emulator.h"
// #include <stdio.h>
// #include <cstring>
// #include "fabutils.h"

extern "C" void app_main()
{
    uint32_t freeHeap32 = heap_caps_get_free_size(MALLOC_CAP_32BIT);
    uint32_t freeHeap8 = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    ESP_LOGI(TAG, "Free heap 32BIT: %d, free heap 8BIT: %d", freeHeap32 - freeHeap8, freeHeap8);

    EmulatorTaskMain(nullptr);
}
