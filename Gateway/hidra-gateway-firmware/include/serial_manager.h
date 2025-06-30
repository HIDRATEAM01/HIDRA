#ifndef SERIAL_MANAGER_H
#define SERIAL_MANAGER_H

#include <Arduino.h>
#include <WiFiUdp.h>

enum LogLevel {
  LOG_NONE,
  LOG_ERROR,
  LOG_WARN,
  LOG_INFO,
  LOG_ALL
};

class SerialManager {
  public:
  SerialManager();

  void begin(bool enableSerial = true, bool enableTelnet = false, LogLevel level = LOG_ALL);
  void log(LogLevel level, const char *message, const char *arg1 = "", const char *arg2 = "", const char *arg3 = "");

  void enableSerial(bool enable);
  void setLogLevel(LogLevel level);

  private:
  bool serialEnabled;
  LogLevel currentLogLevel;
  const char *logPrefix(LogLevel level);
};

extern SerialManager serial;

#endif
