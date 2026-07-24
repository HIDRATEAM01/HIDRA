#ifndef SERIAL_WIRE_H
#define SERIAL_WIRE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "debug_control.h"

class SerialComm {
  private:
    HardwareSerial &serial;
    String buffer;
    String jsonString;

    const char* sensorNames[4] = {
      "turbidez",
      "temperatura",
      "condutividade",
      "ph"
    };

  public:
    SerialComm(HardwareSerial &serialPort) : serial(serialPort) {}

    void begin(unsigned long baud = 115200) {
      pinMode(0, OUTPUT);
      serial.begin(baud);
      debug.println("[SerialWire] Porta serial iniciada");
    }

    void requestData() {
      digitalWrite(0, HIGH);
      serial.print("@");
      debug.println("[SerialWire] Pedido de dados enviado '@'");
      digitalWrite(0, LOW);
    }

    bool readData() {
      while (serial.available()) {
        char c = serial.read();
        if (c == '$') {
          debug.println("[SerialWire] Fim da mensagem recebido '$'");
          parseToJson();
          buffer = "";
          return true;
        } else {
          buffer += c;
        }
      }
      return false;
    }

    void parseToJson() {
      DynamicJsonDocument doc(1024);
      JsonObject obj = doc.to<JsonObject>();

      int start = 0;
      int sensorIndex = 0;

      while (sensorIndex < 4) {
        int idx = buffer.indexOf('&', start);
        String val;

        if (idx == -1) {
          val = buffer.substring(start);
        } else {
          val = buffer.substring(start, idx);
        }

        if (sensorIndex == 0 && val.startsWith("#")) {
          val.remove(0, 1);
        }

        if (val.length() > 0) {
          obj[sensorNames[sensorIndex]] = val.toFloat();
        } else {
          obj[sensorNames[sensorIndex]] = nullptr;
        }

        sensorIndex++;
        if (idx == -1) break;
        start = idx + 1;
      }

      while (sensorIndex < 4) {
        obj[sensorNames[sensorIndex]] = nullptr;
        sensorIndex++;
      }

      time_t now;
      struct tm timeinfo;
      if (getLocalTime(&timeinfo)) {
        char timeStr[25];
        strftime(timeStr, sizeof(timeStr), "%d-%m-%Y-%H-%M-%S", &timeinfo);
        obj["timestamp"] = timeStr;
      } else {
        obj["timestamp"] = "00-00-0000-00-00-00";
      }

      jsonString = "";
      serializeJson(obj, jsonString);
      debug.println("[SerialWire] JSON gerado: " + jsonString);
    }

    String getJsonString() {
      return jsonString;
    }
};

#endif
