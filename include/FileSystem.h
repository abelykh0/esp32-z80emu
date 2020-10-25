#ifndef __SDCARD_H__
#define __SDCARD_H__

#include "FS.h"

void FileSystemInitialize(fs::FS* fileSystem);

bool loadSnapshotSetup(const char* path);
bool loadSnapshotLoop();

bool saveSnapshotSetup();
bool saveSnapshotLoop();

#endif /* __SDCARD_H__ */
