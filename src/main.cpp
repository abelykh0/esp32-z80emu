#include "wifi.h"
#include "arduino.h"

#include "emulator.h"
#include "bubblebobble.h"

void setup()
{
  Serial.begin(115200); 
  Serial.write("in setup()\r\n");

  //mainTaskHandle = xTaskGetCurrentTaskHandle();

  startVideo();

	MainScreen.ShowScreenshot((const uint8_t*)bubblebobble);
  showHelp();
}

void loop()
{
}
