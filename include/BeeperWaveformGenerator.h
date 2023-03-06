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
  void newFrame(int frame);

private:
  int _state;
  int _index;
};

}

#endif /* __BEEPERWAVEFORMGENERATOR_H__ */
