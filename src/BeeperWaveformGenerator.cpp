#include "esp_log.h"
#include "xtensa/core-macros.h"

#include "settings.h"
#include "BeeperWaveformGenerator.h"

using namespace Sound;

#define BUFFER_SIZE (FABGL_SOUNDGEN_DEFAULT_SAMPLE_RATE / 60)

BeeperWaveformGenerator::BeeperWaveformGenerator()
{
}

IRAM_ATTR int BeeperWaveformGenerator::getSample()
{
    uint32_t count = (uint32_t)XTHAL_GET_CCOUNT();

    if (this->_state)
    {
        this->_accumOne += count - this->_countChangeOrSample;
    }
    else
    {
        this->_accumZero += count - this->_countChangeOrSample;
    }

    // PWM
    int volume = (255 * this->_accumOne / (this->_accumOne + this->_accumZero)) - 128;

    this->_accumZero = 0;
    this->_accumOne = 0;
    this->_countChangeOrSample = count;

    return volume;
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
}

void BeeperWaveformGenerator::setFrequency(int value)
{
}