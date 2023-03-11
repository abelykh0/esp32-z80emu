#include "esp_log.h"
#include "xtensa/core-macros.h"

#include "settings.h"
#include "BeeperWaveformGenerator.h"

using namespace Sound;

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
    uint32_t count = (uint32_t)XTHAL_GET_CCOUNT();

    if (this->_index < BUFFER_SIZE - 1)
    {
        this->_index++;
    }

    if (this->_state)
    {
        this->_accumOne += count - this->_countChangeOrSample;
    }
    else
    {
        this->_accumZero += count - this->_countChangeOrSample;
    }

    int volume;
    if (this->_accumOne == 0)
    {
        // stayed at 0
        volume = -128;
    }
    else if (this->_accumZero == 0)
    {
        // stayed at 1
        volume = 127;
    }
    else
    {
        // PWM
        volume = (255 * this->_accumOne / (this->_accumOne + this->_accumZero))- 128;
    }

    this->_accumZero = 0;
    this->_accumOne = 0;
    this->_countChangeOrSample = count;

    this->_shadowBuffer[this->_index] = volume;
    return this->_currentBuffer[this->_index];
}

IRAM_ATTR void BeeperWaveformGenerator::setState(bool state)
{
    uint32_t count = (uint32_t)XTHAL_GET_CCOUNT();

    if (this->_state)
    {
        // 0 > 1
        this->_accumZero += count - this->_countChangeOrSample;
    }
    else
    {
        // 1 > 0
        this->_accumOne += count - this->_countChangeOrSample;
    }

    this->_state = state;
    this->_countChangeOrSample = count;
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