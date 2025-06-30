#include "wifi_manager.h"

WiFiManager::WiFiManager()
    : wifiConnected(false),
      apActive(false) {
  hostName = "hidra";
  apSSID = "Hidra";
  apPassword = "HidraGateway";
  apIP = IPAddress(192, 168, 4, 1);
  apGateway = IPAddress(192, 168, 4, 1);
  apSubnet = IPAddress(255, 255, 255, 0);
}

void WiFiManager::updateWiFiMode() {
  if (wifiConnected && apActive) {
    WiFi.mode(WIFI_MODE_APSTA);
    serial.log(LOG_INFO, "[WiFiManager] Modo AP+STA ativado.");
  } else if (wifiConnected) {
    WiFi.mode(WIFI_MODE_STA);
    serial.log(LOG_INFO, "[WiFiManager] Modo STA ativado.");
  } else if (apActive) {
    WiFi.mode(WIFI_MODE_AP);
    serial.log(LOG_INFO, "[WiFiManager] Modo AP ativado.");
  } else {
    WiFi.mode(WIFI_MODE_NULL);
    serial.log(LOG_INFO, "[WiFiManager] Modo OFF ativado.");
  }
}

void WiFiManager::addNetwork(const char *ssid, const char *password) {
  wifiMulti.addAP(ssid, password);
  wifiStore.addSavedNetwork(ssid, password);
  serial.log(LOG_INFO, "[WiFiManager] Rede WiFi adicionada: ", ssid);
}

bool WiFiManager::autoConnectWiFi(uint32_t timeoutMs, int maxAttempts) {
  wifiConnected = true;
  updateWiFiMode();

  uint32_t startTime = millis();
  int attemptCount = 0;

  while (wifiMulti.run() != WL_CONNECTED) {
    if (millis() - startTime > timeoutMs) {
      attemptCount++;
      startTime = millis();
      serial.log(LOG_INFO, "[WiFiManager] Tentando conectar a rede WiFi. Tentativa: ", String(attemptCount).c_str());

      if (attemptCount >= maxAttempts) {
        wifiConnected = false;
        updateWiFiMode();
        serial.log(LOG_WARN, "[WiFiManager] Falha ao conectar na rede WiFi.");
        return false;
      }
    }
    delay(500);
  }

  startMDNS();
  serial.log(LOG_INFO, "[WiFiManager] Conectado na rede WiFi: ", WiFi.SSID().c_str());
  return true;
}

void WiFiManager::disconnectWiFi() {
  WiFi.disconnect();
  serial.log(LOG_INFO, "[WiFiManager] WiFi desconectado.");
  wifiConnected = false;
  updateWiFiMode();
}

void WiFiManager::startAccessPoint() {
  apActive = true;
  updateWiFiMode();
  WiFi.softAP(apSSID.c_str(), apPassword.c_str());
  WiFi.softAPConfig(apIP, apGateway, apSubnet);
  apActive = true;

  serial.log(LOG_INFO, "[WiFiManager] Access Point ativado. SSID: ", apSSID.c_str());
  serial.log(LOG_INFO, "[WiFiManager] IP: ", apIP.toString().c_str());
}

void WiFiManager::stopAccessPoint() {
  WiFi.softAPdisconnect(true);
  apActive = false;
  updateWiFiMode();
  serial.log(LOG_INFO, "[WiFiManager] Access Point desativado.");
}

void WiFiManager::setAcessPoint(const char *ssid, const char *password, IPAddress localIP, IPAddress gateway, IPAddress subnet) {
  apSSID = ssid;
  apPassword = password;
  apIP = localIP;
  apGateway = gateway;
  apSubnet = subnet;
}

bool WiFiManager::isWiFiConnected() {
  return wifiConnected;
}

bool WiFiManager::isAccessPointActive() {
  return apActive;
}

String WiFiManager::getConnectedSSID() {
  if (WiFi.status() != WL_CONNECTED) return "no network";

  return WiFi.SSID();
}

int WiFiManager::getConnectedRSSI() {
  if (WiFi.status() != WL_CONNECTED) return 0;

  return WiFi.RSSI();
}

IPAddress WiFiManager::getWiFiIP() {
  if (WiFi.status() != WL_CONNECTED) return IPAddress(0, 0, 0, 0);

  return WiFi.localIP();
}

IPAddress WiFiManager::getAPIP() {
  return apIP;
}

String WiFiManager::getAPSSID() {
  return apSSID;
}

String WiFiManager::getAPPassword() {
  return apPassword;
}

String WiFiManager::scanNetworks() {
  serial.log(LOG_INFO, "[WiFiManager] Iniciando Scan de redes WiFi...");
  int n = WiFi.scanNetworks();
  serial.log(LOG_INFO, "[WiFiManager] Scan finalizado. Quantidade: ", String(n).c_str());

  JsonDocument doc;
  JsonArray near = doc["near"].to<JsonArray>();

  for (int i = 0; i < n; i++) {
    JsonObject network = near.add<JsonObject>();
    network["id"] = i;
    network["ssid"] = WiFi.SSID(i);
    network["rssi"] = WiFi.RSSI(i);
  }

  String jsonOutput;
  serializeJson(doc, jsonOutput);
  serial.log(LOG_INFO, "[WiFiManager] Redes WiFi escaneadas: ", jsonOutput.c_str());
  return jsonOutput;
}

bool WiFiManager::connectToNetwork(const String &ssid, const String &pass) {
  WiFi.disconnect(true);
  WiFi.begin(ssid.c_str(), pass.c_str());
  serial.log(LOG_INFO, "[WiFiManager] Tentando conectar na rede WiFi: ", ssid.c_str());

  unsigned long startTime = millis();
  const unsigned long timeout = 10000;

  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startTime > timeout) {
      serial.log(LOG_ERROR, "[WiFiManager] Tempo limite excedido ao conectar na rede WiFi: ", ssid.c_str());
      return false;
    }
    delay(500);
  }

  serial.log(LOG_INFO, "[WiFiManager] Conectado na rede WiFi: ", ssid.c_str());
  addNetwork(ssid.c_str(), pass.c_str());
  return true;
}

void WiFiManager::setMDNS(const char *hostName) {
  this->hostName = hostName;
  serial.log(LOG_INFO, "[WiFiManager] mDNS redefinido: ", hostName);
}

void WiFiManager::startMDNS() {
  if (!MDNS.begin(hostName)) {
    serial.log(LOG_ERROR, "[WiFiManager] Falha ao iniciar mDNS.");
  } else {
    serial.log(LOG_INFO, "[WiFiManager] mDNS iniciado em: http://", hostName, ".local");
  }
}