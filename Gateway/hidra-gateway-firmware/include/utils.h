#ifndef utils_h
#define utils_h

#include <Arduino.h>

class Utils {
  public:
  static int inLimit(int value, int min, int max);

  static float toFixed(float value, int precision);

  static String ip2Str(IPAddress ip);
  static String formatBytes(size_t bytes);
  static String getContentType(String filename);
};

extern Utils utils;

#endif