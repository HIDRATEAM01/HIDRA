#include "serial_manager.h"

SerialManager::SerialManager() {
  serialEnabled = false;
  currentLogLevel = LOG_NONE;
}

void SerialManager::begin(bool enableSerial, bool enableTelnet, LogLevel level) {
  currentLogLevel = level;
  serialEnabled = enableSerial;

  if (serialEnabled) {
    Serial.begin(115200);
    log(LOG_INFO, "[SerialManager] Serial habilitado.");
  }
}

void SerialManager::log(LogLevel level, const char *message, const char *arg1, const char *arg2, const char *arg3) {
  if (level > currentLogLevel) return;

  const char *prefix = logPrefix(level);

  if (serialEnabled) {
    Serial.print(prefix);
    Serial.print(message);
    Serial.print(arg1);
    Serial.print(arg2);
    Serial.println(arg3);
  }
}

void SerialManager::enableSerial(bool enable) {
  serialEnabled = enable;
  if (serialEnabled) log(LOG_INFO, "[SerialManager] Serial ativado.");
}

void SerialManager::setLogLevel(LogLevel level) {
  currentLogLevel = level;
}

const char *SerialManager::logPrefix(LogLevel level) {
  switch (level) {
  case LOG_INFO:
    return "[INFO] ";
  case LOG_WARN:
    return "[WARN] ";
  case LOG_ERROR:
    return "[ERROR] ";
  default:
    return "";
  }
}
