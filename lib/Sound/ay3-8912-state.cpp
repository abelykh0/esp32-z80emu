#include "ay3-8912-state.h"
#include "volume.h"
#include "fabgl.h"

using namespace fabgl;

static SoundGenerator _soundGenerator;
static SquareWaveformGenerator _channel[3];

namespace Sound
{

void Ay3_8912_state::Initialize()
{
    _soundGenerator.setVolume(126);
    _soundGenerator.play(true);
	for (int8_t channel = 0; channel < 3; channel++)
	{
        _soundGenerator.attach(&_channel[channel]);
        _channel[channel].enable(true);
        _channel[channel].setVolume(0);
	}
}

void Ay3_8912_state::StopSound()
{
    _soundGenerator.play(false);
}

void Ay3_8912_state::ResumeSound()
{
    _soundGenerator.play(true);
}

void Ay3_8912_state::updated()
{
	uint16_t oldChannelFrequency[3];
	uint8_t oldChannelVolume[3];

	for (int8_t channel = 0; channel < 3; channel++)
	{
		uint8_t channelVolume = this->channelVolume[channel];
		oldChannelFrequency[channel] = channelVolume ? this->channelFrequency[channel] : 0xFFFF;
		oldChannelVolume[channel] = channelVolume;
	}

	int16_t pitch;

	switch (this->selectedRegister)
	{
	case 0:
	case 1:
		pitch = ((this->coarsePitchChannelA << 8) | this->finePitchChannelA) & 0x0FFF;
		this->channelFrequency[0] = pitch;
		break;
	case 2:
	case 3:
		pitch = ((this->coarsePitchChannelB << 8) | this->finePitchChannelB) & 0x0FFF;
		this->channelFrequency[1] = pitch;
		break;
	case 4:
	case 5:
		pitch = ((this->coarsePitchChannelC << 8) | this->finePitchChannelC) & 0x0FFF;
		this->channelFrequency[2] = pitch;
		break;
	case 6:
		// noisePitch - ignored for now
		break;
	case 7:
		this->channelVolume[0] = (this->mixer & 0x01) ? 0 : volume[this->volumeChannelA & 0x0F];
		this->channelVolume[1] = (this->mixer & 0x02) ? 0 : volume[this->volumeChannelB & 0x0F];
		this->channelVolume[2] = (this->mixer & 0x04) ? 0 : volume[this->volumeChannelC & 0x0F];
		break;
	case 8:
		this->channelVolume[0] = (this->mixer & 0x01) ? 0 : volume[this->volumeChannelA & 0x0F];
		break;
	case 9:
		this->channelVolume[1] = (this->mixer & 0x02) ? 0 : volume[this->volumeChannelB & 0x0F];
		break;
	case 10:
		this->channelVolume[2] = (this->mixer & 0x04) ? 0 : volume[this->volumeChannelC & 0x0F];
		break;
	case 11:
		// envelopeFineDuration - ignored for now
		break;
	case 12:
		// envelopeCoarseDuration - ignored for now
		break;
	case 13:
		// envelopeShape - ignored for now
		break;
	case 14:
		// ioPortA - ignored for now
		break;
	}

	for (int8_t channel = 0; channel < 3; channel++)
	{
		if (this->channelVolume[channel] != oldChannelVolume[channel])
		{
            _channel[channel].setVolume(this->channelVolume[channel]);
		}

		if (this->channelVolume[channel] == 0)
		{
            _channel[channel].setVolume(0);

			continue;
		}

		if (this->channelFrequency[channel] != oldChannelFrequency[channel])
		{
            _channel[channel].setFrequency(this->channelFrequency[channel]);
		}
	}
}

void Ay3_8912_state::selectRegister(uint8_t registerNumber)
{
	this->selectedRegister = registerNumber;
}

void Ay3_8912_state::setRegisterData(uint8_t data)
{
	switch (this->selectedRegister)
	{
	case 0:
		this->finePitchChannelA = data;
		break;
	case 1:
		this->coarsePitchChannelA = data;
		break;
	case 2:
		this->finePitchChannelB = data;
		break;
	case 3:
		this->coarsePitchChannelB = data;
		break;
	case 4:
		this->finePitchChannelC = data;
		break;
	case 5:
		this->coarsePitchChannelC = data;
		break;
	case 6:
		this->noisePitch = data;
		break;
	case 7:
		this->mixer = data;
		break;
	case 8:
		this->volumeChannelA = data;
		break;
	case 9:
		this->volumeChannelB = data;
		break;
	case 10:
		this->volumeChannelC = data;
		break;
	case 11:
		this->envelopeFineDuration = data;
		break;
	case 12:
		this->envelopeCoarseDuration = data;
		break;
	case 13:
		this->envelopeShape = data;
		break;
	case 14:
		this->ioPortA = data;
		break;
	default:
		// invalid register - do nothing
		return;
	}

	this->updated();
}

uint8_t Ay3_8912_state::getRegisterData()
{
	switch (this->selectedRegister)
	{
	case 0:
		return this->finePitchChannelA;
	case 1:
		return this->coarsePitchChannelA;
	case 2:
		return this->finePitchChannelB;
	case 3:
		return this->coarsePitchChannelB;
	case 4:
		return this->finePitchChannelC;
	case 5:
		return this->coarsePitchChannelC;
	case 6:
		return this->noisePitch;
	case 7:
		return this->mixer;
	case 8:
		return this->volumeChannelA;
	case 9:
		return this->volumeChannelB;
	case 10:
		return this->volumeChannelC;
	case 11:
		return this->envelopeFineDuration;
	case 12:
		return this->envelopeCoarseDuration;
	case 13:
		return this->envelopeShape;
	case 14:
		return this->ioPortA;
	default:
		return 0;
	}
}

}
