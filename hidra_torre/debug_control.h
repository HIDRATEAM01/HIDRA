#ifndef DEBUG_CONTROL_H
#define DEBUG_CONTROL_H

#include <WiFiMulti.h>
#include <ArduinoOTA.h>

class DebugControl {
    private:
      WiFiMulti wifiMulti;
      bool connected;

      WiFiServer server;
      WiFiClient client;

      void connectWiFi() {
        WiFi.mode(WIFI_STA);

        unsigned long startAttemptTime = millis();
        while (wifiMulti.run() != WL_CONNECTED && millis() - startAttemptTime < 15000) {
          delay(500);
        }

        connected = (WiFi.status() == WL_CONNECTED);
      }

    void setupOTA() {
      ArduinoOTA.setHostname("H.I.D.R.A.");
      ArduinoOTA.setPassword("hidra");

      ArduinoOTA
        .onStart([this]() {
          println("[OTA] Atualizando...");
        })
        .onEnd([this]() {
          println("[OTA] Finalizado.");
        })
        .onProgress([this](unsigned int progress, unsigned int total) {
          if (client && client.connected()) {
            client.printf("[OTA] Progresso: %u%%\r", (progress / (total / 100)));
          }
        })
        .onError([this](ota_error_t error) {
          print("Erro OTA [%u]: ");
          print("error");
          if (error == OTA_AUTH_ERROR) println("[OTA] Falha na autenticação");
          else if (error == OTA_BEGIN_ERROR) println("[OTA] Falha ao iniciar");
          else if (error == OTA_CONNECT_ERROR) println("[OTA] Falha de conexão");
          else if (error == OTA_RECEIVE_ERROR) println("[OTA] Falha ao receber");
          else if (error == OTA_END_ERROR) println("[OTA] Falha ao finalizar");
        });

      ArduinoOTA.begin();
    }

    void setupTelnet() {
      server.begin();
      server.setNoDelay(true);
    }

    public:
        DebugControl(uint16_t port = 23) 
          : connected(false), server(port) {}

    void begin() {
      connectWiFi();
      if (!connected) return;

      configTime(3600 * -3, 0, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org");

      setupTelnet();
      setupOTA();
    }

    void addNetwork(const char* ssid, const char* password) {
      wifiMulti.addAP(ssid, password);
    }

    void handle() {
      if (!connected) return;

      ArduinoOTA.handle();

      if (!client || !client.connected()) {
        client = server.available();
        if (client) {
          client.println("[DBG] Conectado.");
        }
      }

      // Envia log periódico
      static unsigned long last = 0;
      if (millis() - last > 5000) {
        println("[DBG] [" + String(millis()) + "] Mensagem periodica.");
        last = millis();
      }
    }

    bool available() {
      return client && client.available();
    }

    char read() {
      if (available()) return client.read();
      return -1;
    }

    void println() { println(""); }

    void println(const String &msg) {
      if (client && client.connected()) client.println(msg);
    }

    void print(const String &msg) {
      if (client && client.connected()) client.print(msg);
    }
};

extern DebugControl debug;

#endif
