#include "internal_clock.h"

InternalClock::InternalClock(long timezone, byte daysavetime) {
  setTimeZone(timezone, daysavetime);
}

void InternalClock::setTimeZone(long timezone, byte daysavetime) {
  this->timezone = timezone;
  this->daysavetime = daysavetime;
}

void InternalClock::setClockByServer() {
  serial.log(LOG_INFO, "[InternalClock] Configurando relógio pelo servidor.");
  configTime(3600 * timezone, daysavetime * 3600, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org");
  delay(500);
  serial.log(LOG_INFO, "[InternalClock] Relógio configurado: ", getClock(DH).c_str());
}

void InternalClock::setClock(int day, int month, int year, int hour, int minute, int second) {
  struct tm timeinfo;
  timeinfo.tm_year = year - 1900;  // Ano - 1900
  timeinfo.tm_mon = month - 1;     // Mês (0-11, janeiro = 0)
  timeinfo.tm_mday = day;          // Dia do mês (1-31)
  timeinfo.tm_hour = hour;         // Hora (0-23)
  timeinfo.tm_min = minute;        // Minuto (0-59)
  timeinfo.tm_sec = second;        // Segundo (0-59)

  // Converter a estrutura tm para um timestamp UNIX
  time_t timestamp = mktime(&timeinfo);
  // Configurar o RTC do ESP32 com o timestamp UNIX
  timeval tv = {timestamp, 0};
  settimeofday(&tv, nullptr);
}

String InternalClock::getClock(DateFormats dateformat) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    serial.log(LOG_ERROR, "[InternalClock] Relógio não disponível. Configurando data padrão.");
    setClock(1, 1, 2025, 0, 0, 0);
    getLocalTime(&timeinfo);
  }

  char buffer[30];
  if (dateformat == DD) {
    strftime(buffer, sizeof(buffer), "%d/%m/%Y", &timeinfo);
  } else if (dateformat == HH) {
    strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeinfo);
  } else if (dateformat == DH) {
    strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", &timeinfo);
  } else {
    serial.log(LOG_ERROR, "[InternalClock] Formato de data inválido.");
    return "date format ERROR";
  }
  return String(buffer);
}
