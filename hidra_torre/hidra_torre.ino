#include "debug_control.h"
#include "camera_control.h"
#include "firebase_uploader.h"
#include "serial_wire.h"
#include "secrets.h"

CameraControl camera;
FirebaseUploader firebase;

DebugControl debug;
SerialComm stm(Serial);


int counter = 0;
unsigned long lastRun = 0;
bool debugRead = false;

void setup() {
  debug.addNetwork(WIFI_SSID, WIFI_PASS);
  debug.addNetwork(PHONE_SSID, PHONE_PASS); 
  debug.addNetwork(DEF_SSID, DEF_PASS); 

  debug.begin();
  debug.println("[SETUP] Iniciado.");

  //camera.begin();
  stm.begin(115200);

  firebase.begin(FIREBASE_BUCKET, FIREBASE_RTDB);
}

void loop() {
  debug.handle();

  if (millis() - lastRun >= 1000*60*5) {
    lastRun = millis();
    debug.println("[Coleta] Inicio.");
    stm.requestData();
    debugRead = false;
  }

  if (stm.readData() && !debugRead) {
    ciclo();
  }

  switch (debug.read()) {
    case 'l':
    case 'L':
      debug.println("\n[CTRL] 'L'");
      firebase.begin(FIREBASE_BUCKET, FIREBASE_RTDB);
      break;
    case 'N':
    case 'n':
      debug.println("\n[CTRL] 'N'");
      camera.begin();
      break;
    case 'C':
    case 'c':
      debug.println("\n[CTRL] 'C'");
      if (camera.capture("/foto_" + String(counter) + ".jpg")) counter++;
      break;
    case 'U':
    case 'u':
      debug.println("\n[CTRL] 'U'");
      upload();
      break;
    case 'T':
    case 't':
      firebase.uploadJSON("{\"iniciando\":\"hidra\",\"valor\":" + String(millis()) + "}", "teste");
      break;
    case 'R':
    case 'r':
      debug.println("\n[CTRL] 'R'");
      debug.println("[DBG] Reiniciando.");
      ESP.restart();
      break;
    case '1':
      debug.println("\n[CTRL] '1'");
      debug.println("[DBG] Requisita leituras.");
      stm.requestData();
      debugRead = true;
      break;
    case '2':
      debug.println("\n[CTRL] '2'");
      debug.println("[DBG] Envio forcado.");
      firebase.uploadJSON(stm.getJsonString(), "values");
      debugRead = false;
      break;
    case '3':
      debug.println("\n[CTRL] '3'");
      debug.println("[DBG] Ciclo forcado.");
      stm.requestData();
      debugRead = false;
      break;
  }
}

void upload() {
  String path = "/foto_" + String(counter - 1) + ".jpg";
  firebase.uploadFile(path.c_str());
}

void ciclo() {
  String data = stm.getJsonString();
  if (!data.length() > 0) {
    debug.println("[Coleta] Nenhum dado recebido do STM.");
    return;
  }

  debug.println("[Coleta] Leitura realizada.");

  if (firebase.uploadJSON(data, "values")) {
    debug.println("[Coleta] Envio Leitura.");
  } else {
    debug.println("[Coleta] Falha no envio da leitura.");
  }

  if(camera.capture("/foto_" + String(counter) + ".jpg")) {
    counter++;
    debug.println("[Coleta] Retrato.");
    upload();
    debug.println("[Coleta] Envio retrato.");
  } else {
    debug.println("[Coleta] Falha no envio da foto.");
  }

  debug.println("[Coleta] Finalizado.");

}