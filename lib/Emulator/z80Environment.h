#ifndef __Z80ENVIRONMENT_INCLUDED__
#define __Z80ENVIRONMENT_INCLUDED__

// Class for the Z80 enviroment (ROM, RAM, I/O)

#include <stdint.h>
#include "SpectrumScreen.h"

using namespace Display;

class Z80Environment
{
private:
	SpectrumScreen* _spectrumScreen = nullptr;

public:
	void Setup(SpectrumScreen* spectrumScreen);

    uint8_t ReadByte(uint16_t address);
	uint16_t ReadWord(uint16_t address);
	void WriteByte(uint16_t address, uint8_t data);
	void WriteWord(uint16_t address, uint16_t data);
	uint8_t Input(uint8_t portLow, uint8_t portHigh);
	void Output(uint8_t portLow, uint8_t portHigh, uint8_t data);
};

#endif

