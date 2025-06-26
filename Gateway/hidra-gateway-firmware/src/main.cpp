#include <Arduino.h>
#include <ArduinoJson.h>

#include "serial_manager.h"
#include "wifi_manager.h"

SerialManager serial;
WiFiManager wm;

void setup() {
  serial.begin();
  serial.log(LOG_INFO, "Iniciando sistema.");
  wm.scanNetworks();
}

void loop() {
}
