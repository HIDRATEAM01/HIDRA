#ifndef SERVER_MANAGER_H
#define SERVER_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <WebServer.h>

#include "file_manager.h"
#include "internal_clock.h"
#include "serial_manager.h"
#include "store_manager.h"
#include "utils.h"
#include "wifi_manager.h"

class ServerManager {
  public:
  ServerManager();
  void startServer();
  void restartServer();
  void handleClient();
  void end();

  void raiseError(String errorMessage);
  void raiseSucess(String sucessMessage);

  private:
  WebServer server;
  void setupRoutes();
  void setupGetRoutes();
  void setupPostRoutes();
  void setupStaticRoutes();
  void setupNotFoundHandler();
  void handleFileRead(String path);
  bool handleFileUpload(HTTPUpload &upload);
};

#endif