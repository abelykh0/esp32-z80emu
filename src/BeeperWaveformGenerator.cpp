#include "esp_log.h"
#include "xtensa/core-macros.h"

#include "settings.h"
#include "BeeperWaveformGenerator.h"

using namespace Sound;

BeeperWaveformGenerator::BeeperWaveformGenerator()
{
    this->_oldTStatesPos = 0;
    this->_index = 0;
    this->_currentBufferIndex = 0;
    this->_currentAudioBuffer = this->_audioBuffer[this->_currentBufferIndex];
}

IRAM_ATTR int BeeperWaveformGenerator::getSample()
{
    int8_t result = this->_currentAudioBuffer[this->_index];
    if (this->_index < SAMPLES_PER_FRAME)
    {
        this->_index++;
    }

    return result;
}

IRAM_ATTR void BeeperWaveformGenerator::setState(bool state, uint32_t tStates)
{
    int tStatesPos = tStates >> 4;
    for (int i = this->_oldTStatesPos; i < tStatesPos - 1; i++)
    {
        this->_buffer[i / 32][i % 32] = this->_oldState ? 1 : 0;
    }

    this->_buffer[tStatesPos / 32][tStatesPos % 32] = state ? 1 : 0;

    this->_oldState = state;
    this->_oldTStatesPos = tStatesPos;
}

IRAM_ATTR void BeeperWaveformGenerator::newFrame()
{
    int tStatesPos = TSTATES_PER_FRAME >> 4;
    for (int i = this->_oldTStatesPos; i < tStatesPos - 1; i++)
    {
        this->_buffer[i / 32][i % 32] = this->_oldState ? 1 : 0;
    }

    for (int i = 0; i < SAMPLES_PER_FRAME; i++)
    {
        this->_currentAudioBuffer[i] = this->_buffer[i].count() * 8;
    }

    this->_oldTStatesPos = 0;
    this->_index = 0;
    this->_currentBufferIndex = this->_currentBufferIndex == 0 ? 1 : 0;
    this->_currentAudioBuffer = this->_audioBuffer[this->_currentBufferIndex];
}

void BeeperWaveformGenerator::setFrequency(int value)
{
}