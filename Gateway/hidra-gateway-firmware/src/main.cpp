#include "file_manager.h"
#include "internal_clock.h"
#include "serial_manager.h"
#include "server_manager.h"
#include "store_manager.h"
#include "wifi_manager.h"

SerialManager serial;
InternalClock rtc(-3, 0);
FileManager fm;
WiFiManager wm;
StoreManager wifiStore;

ServerManager server;

void setup() {
  serial.begin();
  serial.log(LOG_INFO, "Iniciando sistema.");

  fm.begin();
  fm.listFiles();
  wifiStore.begin("wifi");

  rtc.setClockByServer();
  wm.startAccessPoint();
  server.startServer();
  serial.log(LOG_INFO, "Sistema iniciado.");
}

void loop() {
}
