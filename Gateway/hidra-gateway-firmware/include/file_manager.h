#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>

#include "serial_manager.h"
#include "utils.h"

#define FILE_SYSTEM LittleFS

class FileManager {
  public:
  FileManager();
  bool begin();
  bool createFile(const String &path);
  bool saveFile(const char *path, const uint8_t *data, size_t length);
  bool readFile(const char *path, uint8_t *buffer, size_t bufferSize);
  bool writeToFile(const String &path, const uint8_t *data, size_t len);

  bool exists(const String path);
  File getFile(const char *path, const char *mode = "r");

  void listFiles(const char *path = "/", int maxDepth = 4);
  bool removeRecursive(const char *path);

  void end();
};

extern FileManager fm;

#endif