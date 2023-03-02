#ifndef __SDCARD_H__
#define __SDCARD_H__

void FileSystemInitialize();

bool loadSnapshotSetup(const char* path);
bool loadSnapshotLoop();

bool saveSnapshotSetup(const char* path);
bool saveSnapshotLoop();

bool ReadFromFile(const char* fileName, uint8_t* buffer, size_t size);

#endif /* __SDCARD_H__ */
