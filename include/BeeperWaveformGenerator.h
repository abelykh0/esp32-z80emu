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

private:
  int _state;
};

}

#endif /* __BEEPERWAVEFORMGENERATOR_H__ */
