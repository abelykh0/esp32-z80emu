#ifndef _SPECTRUMSCREENDATA_H
#define _SPECTRUMSCREENDATA_H

#include <stdint.h>

#define SPECTRUM_WIDTH  32
#define SPECTRUM_HEIGHT 24

typedef struct 
{
	uint8_t*  Pixels;
	uint16_t* Attributes;
} SpectrumScreenData;

#endif
