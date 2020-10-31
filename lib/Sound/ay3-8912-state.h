#ifndef _AY3_8912_STATE_H
#define _AY3_8912_STATE_H

#include <stdint.h>

namespace Sound
{

class Ay3_8912_state
{
public:
	// Registers
	uint8_t finePitchChannelA = 0xFF;
	uint8_t coarsePitchChannelA = 0xFF;
	uint8_t finePitchChannelB = 0xFF;
	uint8_t coarsePitchChannelB = 0xFF;
	uint8_t finePitchChannelC = 0xFF;
	uint8_t coarsePitchChannelC = 0xFF;
	uint8_t noisePitch = 0xFF;
	uint8_t mixer = 0xFF;
	uint8_t volumeChannelA = 0xFF;
	uint8_t volumeChannelB = 0xFF;
	uint8_t volumeChannelC = 0xFF;
	uint8_t envelopeFineDuration = 0xFF;
	uint8_t envelopeCoarseDuration = 0xFF;
	uint8_t envelopeShape = 0xFF;
	uint8_t ioPortA = 0xFF;

	// Status
	uint8_t selectedRegister = 0xFF;
	uint8_t channelVolume[3] = { 0xFF, 0xFF, 0xFF };
	uint16_t channelFrequency[3] = { 0xFFFF, 0xFFFF, 0xFFFF };

	void selectRegister(uint8_t registerNumber);
	void setRegisterData(uint8_t data);
	uint8_t getRegisterData();

    void Initialize();
    void StopSound();
    void ResumeSound();

private:
	void updated();
};

}

#endif
