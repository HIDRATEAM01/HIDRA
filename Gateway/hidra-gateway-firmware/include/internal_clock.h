#ifndef INTERNAL_CLOCK_H
#define INTERNAL_CLOCK_H

#include <Arduino.h>
#include <time.h>

#include "serial_manager.h"

enum DateFormats {
  DD,
  HH,
  DH
};

class InternalClock {
  public:
  InternalClock(long timezone, byte daysavetime);

  void setTimeZone(long timezone, byte daysavetime = 0);
  void setClockByServer();
  void setClock(int day, int month, int year, int hour, int minute, int second);
  String getClock(DateFormats dateformat);

  private:
  long timezone;
  byte daysavetime;
};

extern InternalClock rtc;

#endif