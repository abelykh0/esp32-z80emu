#ifndef _SPECTRUMSCREENDATA_H
#define _SPECTRUMSCREENDATA_H

#include <stdint.h>

typedef struct 
{
	uint8_t  Pixels[32 * 8 * 24];
	uint16_t Attributes[32 * 24];
} SpectrumScreenData;

#endif
