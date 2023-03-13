#ifndef __BEEPERWAVEFORMGENERATOR_H__
#define __BEEPERWAVEFORMGENERATOR_H__

#include <bitset>

#include "fabgl.h"

using namespace std;

#ifdef ZX128K
#define SAMPLES_PER_FRAME 277
#else
#define SAMPLES_PER_FRAME 273
#endif

namespace Sound
{

class BeeperWaveformGenerator : public WaveformGenerator {
public:
  BeeperWaveformGenerator();

  void setFrequency(int value) override;
  int getSample() override;

  void setState(bool state, uint32_t tStates);
  void newFrame();

private:
  bool _oldState = false;
  int _oldTStatesPos;
  int _currentBufferIndex = 0;
  int8_t _audioBuffer[2][SAMPLES_PER_FRAME];
  int8_t* _currentAudioBuffer;
  bitset<32> _buffer[SAMPLES_PER_FRAME];
  int _index = 0;
};

}

#endif /* __BEEPERWAVEFORMGENERATOR_H__ */
