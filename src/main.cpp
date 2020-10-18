#include "wifi.h"
#include "arduino.h"
#include "FS.h"
#include "FFat.h"

#include "emulator.h"
#include "bubblebobble.h"

TaskHandle_t mainTaskHandle;

void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("- failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}


void setup()
{
    Serial.begin(115200); 
    Serial.write("in setup()\r\n");

    //xTaskCreatePinnedToCore(&EmulatorTaskMain, "emulatorTask", 1024 * 4, NULL, 5, &mainTaskHandle, tskNO_AFFINITY);

    if(!FFat.begin()){
        Serial.println("FFat Mount Failed");
        return;
    }
    Serial.printf("Total space: %10u\n", FFat.totalBytes());
    Serial.printf("Free space: %10u\n", FFat.freeBytes());
    listDir(FFat, "/", 0);
}

void loop()
{
}
