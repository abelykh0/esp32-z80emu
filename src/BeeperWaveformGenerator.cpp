#include "esp_log.h"

#include "settings.h"
#include "BeeperWaveformGenerator.h"

using namespace Sound;

BeeperWaveformGenerator::BeeperWaveformGenerator()
{
    this->_index = 0;
}

IRAM_ATTR int BeeperWaveformGenerator::getSample()
{
    return this->_state;
}

IRAM_ATTR void BeeperWaveformGenerator::setState(bool state)
{
    this->_index++;
    this->_state = state ? -127 : 127;
}

IRAM_ATTR void BeeperWaveformGenerator::newFrame(int frame)
{
    ESP_LOGW(TAG, "frame %i, index %i", frame, this->_index);
    this->_index = 0;
}

void BeeperWaveformGenerator::setFrequency(int value)
{
}