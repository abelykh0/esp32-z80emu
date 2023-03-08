#include "esp_log.h"

#include "settings.h"
#include "BeeperWaveformGenerator.h"

using namespace Sound;

int _oldIndex;
int _count = 0;

#define BUFFER_SIZE (FABGL_SOUNDGEN_DEFAULT_SAMPLE_RATE / 60)

BeeperWaveformGenerator::BeeperWaveformGenerator()
{
    this->_index = 0;
    this->_buffer1 = (int8_t*)malloc(BUFFER_SIZE);
    this->_buffer2 = (int8_t*)malloc(BUFFER_SIZE);
    this->_currentBuffer = this->_buffer1;
    this->_shadowBuffer = this->_buffer2;
}

IRAM_ATTR int BeeperWaveformGenerator::getSample()
{
    if (this->_index < BUFFER_SIZE - 1)
    {
        this->_index++;
    }

    this->_shadowBuffer[this->_index] = this->_state;
    return this->_currentBuffer[this->_index];
}

IRAM_ATTR void BeeperWaveformGenerator::setState(bool state)
{
    if (_oldIndex == this->_index)
    {
        _count++;
    }
    else
    {
        if (_count > 0)
        {
            ESP_LOGW(TAG, "count %i, index %i", _count, this->_index);
        }
        _count = 0;
    _oldIndex = this->_index;
    }

    this->_state = state ? -127 : 127;
}

IRAM_ATTR void BeeperWaveformGenerator::newFrame()
{
    this->_index = 0;
    int8_t* temp = this->_currentBuffer;
    this->_currentBuffer = this->_shadowBuffer;
    this->_shadowBuffer = temp;
}

void BeeperWaveformGenerator::setFrequency(int value)
{
}