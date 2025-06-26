#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiMulti.h>

#include "serial_manager.h"

class WiFiManager {
  public:
  WiFiManager();
  void updateWiFiMode();

  void addNetwork(const char *ssid, const char *password);
  bool autoConnectWiFi(uint32_t timeoutMs = 5000, int maxAttempts = 5);
  void disconnectWiFi();

  void startAccessPoint();
  void stopAccessPoint();
  void setAcessPoint(const char *ssid,
                     const char *password,
                     IPAddress localIP = IPAddress(192, 168, 4, 1),
                     IPAddress gateway = IPAddress(192, 168, 4, 1),
                     IPAddress subnet = IPAddress(255, 255, 255, 0));

  bool isWiFiConnected();
  bool isAccessPointActive();

  String getConnectedSSID();
  IPAddress getWiFiIP();
  IPAddress getAPIP();
  int getSignalStrength();

  String scanNetworks();
  bool connectToNetwork(const String &ssid, const String &pass);

  void setMDNS(const char *hostName);
  void startMDNS();

  private:
  WiFiMulti wifiMulti;
  bool wifiConnected;
  bool apActive;

  String apSSID;
  String apPassword;
  IPAddress apIP;
  IPAddress apGateway;
  IPAddress apSubnet;

  const char *hostName;
};

#endif
