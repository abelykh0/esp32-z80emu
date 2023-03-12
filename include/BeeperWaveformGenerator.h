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
  bool _state = false;
  int64_t _countChangeOrSample = 0;
  int _accumZero = 0;
  int _accumOne = 0;
};

}

#endif /* __BEEPERWAVEFORMGENERATOR_H__ */
