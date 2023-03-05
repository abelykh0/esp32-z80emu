#include "BeeperWaveformGenerator.h"

using namespace Sound;

BeeperWaveformGenerator::BeeperWaveformGenerator()
{
}

int BeeperWaveformGenerator::getSample()
{
    return this->_state;
}

void BeeperWaveformGenerator::setState(bool state)
{
    this->_state = state ? -127 : 127;
}

void BeeperWaveformGenerator::setFrequency(int value)
{
}