#ifndef __BEEPERWAVEFORMGENERATOR_H__
#define __BEEPERWAVEFORMGENERATOR_H__

#include "fabgl.h"

namespace Sound
{

class BeeperWaveformGenerator : public WaveformGenerator {
public:
  BeeperWaveformGenerator();

  void setFrequency(int value) override;
  int getSample() override;

  void setState(bool state);
  void newFrame();

private:
  int8_t* _buffer1;
  int8_t* _buffer2;
  int8_t* _currentBuffer;
  int8_t* _shadowBuffer;

  int _index = 0; 
  bool _state = false;
  uint32_t _countChangeOrSample = 0;
  int _accumZero = 0;
  int _accumOne = 0;
};

}

#endif /* __BEEPERWAVEFORMGENERATOR_H__ */