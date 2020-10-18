#ifndef __SDCARD_H__
#define __SDCARD_H__

#include "FS.h"
#include "FFat.h"

bool loadSnapshotSetup();
bool loadSnapshotLoop();

bool saveSnapshotSetup();
bool saveSnapshotLoop();

#endif /* __SDCARD_H__ */
