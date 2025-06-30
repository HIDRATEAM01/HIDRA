#include "server_manager.h"

ServerManager::ServerManager() : server(80) {}

void ServerManager::startServer() {
  setupRoutes();
  server.begin();
  serial.log(LOG_INFO, "[ServerManager] Servidor iniciado.");
}

void ServerManager::restartServer() {
  serial.log(LOG_INFO, "[ServerManager] Reiniciando servidor.");
  server.close();
  startServer();
}

void ServerManager::handleClient() {
  server.handleClient();
}

void ServerManager::end() {
  server.close();
  serial.log(LOG_INFO, "[ServerManager] Servidor encerrado.");
}

void ServerManager::raiseError(String errorMessage) {
  server.send(400, "text/plain", errorMessage);
  serial.log(LOG_ERROR, "[SERVER] ", errorMessage.c_str());
}

void ServerManager::raiseSucess(String sucessMessage) {
  server.send(200, "text/plain", sucessMessage);
  serial.log(LOG_INFO, "[SERVER] ", sucessMessage.c_str());
}

void ServerManager::setupRoutes() {
  setupGetRoutes();
  setupPostRoutes();
  setupStaticRoutes();
  setupNotFoundHandler();
}

// TODO: Adicionar funções static
void ServerManager::setupGetRoutes() {
  server.on("/config", HTTP_GET, [this]() {
    JsonDocument doc;
    doc["date"] = rtc.getClock(DD);
    doc["time"] = rtc.getClock(HH);
    doc["address"] = "N/A";

    String jsonOutput;
    serializeJson(doc, jsonOutput);
    serial.log(LOG_INFO, "[ServerManager] [/config]: Sucesso. JSON: ", jsonOutput.c_str());
    server.send(200, "application/json", jsonOutput);
  });

  server.on("/wifi/status", HTTP_GET, [this]() {
    JsonDocument doc;
    doc["ssid"] = wm.getConnectedSSID();
    doc["rssi"] = wm.getConnectedRSSI();
    doc["ip"] = utils.ip2Str(wm.getWiFiIP());
    doc["status"] = wm.isWiFiConnected() ? 1 : 0;

    String jsonOutput;
    serializeJson(doc, jsonOutput);
    serial.log(LOG_INFO, "[ServerManager] [/wifi/status]: Sucesso. JSON: ", jsonOutput.c_str());
    server.send(200, "application/json", jsonOutput);
  });

  server.on("/wifi/networks", HTTP_GET, [this]() {
    String networks = wm.scanNetworks();
    std::vector<WifiCredential> savedNetworks = wifiStore.getSavedNetworks();

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, networks);

    if (error) {
      serial.log(LOG_ERROR, "[ServerManager] [/wifi/networks]: Erro ao desserializar JSON: ", error.c_str());
      return;
    }

    JsonArray saved = doc["saved"].to<JsonArray>();
    JsonArray near = doc["near"].as<JsonArray>();

    int i = 0;
    for (const auto &network : savedNetworks) {
      JsonObject obj = saved.add<JsonObject>();
      obj["id"] = i++;
      obj["ssid"] = network.ssid;
      obj["password"] = network.password;
    }

    String jsonOutput;
    serializeJson(doc, jsonOutput);
    serial.log(LOG_INFO, "[ServerManager] [/wifi/networks]: Sucesso.");
    server.send(200, "application/json", jsonOutput);
  });

  server.on("/server/config", HTTP_GET, [this]() {
    JsonDocument doc;
    doc["ssid"] = wm.getAPSSID();
    doc["pass"] = wm.getAPPassword();
    doc["ip"] = utils.ip2Str(wm.getAPIP());
    doc["status"] = wm.isAccessPointActive() ? 1 : 0;

    String jsonOutput;
    serializeJson(doc, jsonOutput);
    serial.log(LOG_INFO, "[ServerManager] [/server/config]: Sucesso. JSON: ", jsonOutput.c_str());
    server.send(200, "application/json", jsonOutput);
  });

  server.on("/modules", HTTP_GET, [this]() {
    String jsonOutput = R"rawliteral(
      {
        "modules": [
          {
            "id": 0,
            "name": "rio azul 2",
            "recieve-date": "2025-23-06",
            "recieve-time": "14:19:00",
            "address": "123",
            "bat": 45
          },
          {
            "id": 1,
            "name": "rio mais verde",
            "recieve-date": "2025-23-06",
            "recieve-time": "12:29:00",
            "address": "456",
            "bat": 36
          },
          {
            "id": 2,
            "name": "vila nova sol",
            "recieve-date": "2025-23-06",
            "recieve-time": "13:02:00",
            "address": "789",
            "bat": 51
          },
          {
            "id": 3,
            "name": "parque das flores",
            "recieve-date": "2025-23-06",
            "recieve-time": "11:44:00",
            "address": "321",
            "bat": 62
          },
          {
            "id": 4,
            "name": "monte claro",
            "recieve-date": "2025-23-06",
            "recieve-time": "15:10:00",
            "address": "654",
            "bat": 39
          },
          {
            "id": 5,
            "name": "bairro do sol",
            "recieve-date": "2025-23-06",
            "recieve-time": "10:58:00",
            "address": "987",
            "bat": 28
          },
          {
            "id": 6,
            "name": "estrada verde",
            "recieve-date": "2025-23-06",
            "recieve-time": "09:37:00",
            "address": "159",
            "bat": 73
          },
          {
            "id": 7,
            "name": "eco vila 1",
            "recieve-date": "2025-23-06",
            "recieve-time": "08:23:00",
            "address": "753",
            "bat": 33
          }
        ]
      }
      )rawliteral";

    serial.log(LOG_INFO, "[ServerManager] [/modules]: Sucesso. JSON: ", jsonOutput.c_str());
    server.send(200, "application/json", jsonOutput);
  });
}

void ServerManager::setupPostRoutes() {
  server.on(
      "/upload", HTTP_POST,
      [this]() {
        server.send(200, "text/plain", "Arquivo recebido com sucesso de forma completa!");
      },
      [this]() {
        String result = handleFileUpload(server.upload());
        if (result == "") {
          server.send(500, "text/plain", "Erro ao salvar arquivo.");
        } else if (result != "START" && result != "IN_PROGRESS") {
          server.send(200, "text/plain", "Arquivo recebido: " + result);
        }
      });

  server.on("/wifi/toggle", HTTP_POST, [this]() {
    if (!server.hasArg("plain")) {
      raiseError("Conteúdo ausente.");
      return;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    if (error) {
      raiseError("Falha ao analisar JSON.");
      return;
    }

    int status = doc["status"].as<int>();
    if (status == 1)
      wm.autoConnectWiFi();
    else if (status == 0)
      wm.disconnectWiFi();
    else {
      raiseError("Status inválido.");
      return;
    }

    server.send(200, "text/plain", "{\"novo status\": " + String(status) + "}");
    serial.log(LOG_INFO, "[ServerManager] [/wifi/toggle]: Sucesso. Novo status: ", String(status).c_str());
  });

  server.on("/server/toggle", HTTP_POST, [this]() {
    if (!server.hasArg("plain")) {
      raiseError("Conteúdo ausente.");
      return;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    if (error) {
      raiseError("Falha ao analisar JSON.");
      return;
    }

    int status = doc["status"];
    if (status == 1)
      wm.startAccessPoint();
    else if (status == 0)
      wm.stopAccessPoint();
    else {
      raiseError("Status inválido.");
      return;
    }

    server.send(200, "text/plain", "{\"novo status\": " + String(status) + "}");
    serial.log(LOG_INFO, "[ServerManager] [/server/toggle]: Sucesso. Novo status: ", String(status).c_str());
  });

  // TODO: Corrigir método de adição de rede WiFi
  server.on("/wifi/add", HTTP_POST, [this]() {
    JsonDocument doc;
    if (!handlePostPayload(doc)) return;

    String ssid = doc["ssid"].as<String>();
    String password = doc["password"].as<String>();

    if (ssid.isEmpty()) {
      raiseError("SSID ausente.");
      return;
    }

    wifiStore.addSavedNetwork(ssid, password);
    server.send(200, "text/plain", "Rede adicionada com sucesso.");
    serial.log(LOG_INFO, "[ServerManager] [/wifi/add]: Sucesso. SSID: ", ssid.c_str());
  });

  server.on("/config/time", HTTP_POST, [this]() {
    JsonDocument doc;
    if (!handlePostPayload(doc)) return;

    String date = doc["date"].as<String>();
    String time = doc["time"].as<String>();

    if (date.isEmpty() || time.isEmpty()) {
      raiseError("Data ou hora ausente.");
      return;
    }

    rtc.setClockByString(date, time);

    server.send(200, "text/plain", "Hora configurada com sucesso.");
    serial.log(LOG_INFO, "[ServerManager] [/config/time]: Sucesso. Data: ", date.c_str(), " Hora: ", time.c_str());
  });

  server.on("/server/config", HTTP_POST, [this]() {
    JsonDocument doc;
    if (!handlePostPayload(doc)) return;

    String ssid = doc["ssid"].as<String>();
    String password = doc["pass"].as<String>();
    if (ssid.isEmpty()) {
      raiseError("SSID ausente.");
      return;
    }
    if (ssid.length() < 3 || ssid.length() > 16) {
      raiseError("SSID deve ter entre 3 e 16 caracteres.");
      return;
    }
    if (password.length() < 8 || password.length() > 16) {
      raiseError("Senha deve ter entre 8 e 16 caracteres.");
      return;
    }
    wm.setAccessPoint(ssid.c_str(), password.c_str());
    wm.restartAccessPoint();

    server.send(200, "text/plain", "Configuração do servidor atualizada com sucesso.");
    serial.log(LOG_INFO, "[ServerManager] [/server/config]: Sucesso. SSID: ", ssid.c_str(), " Senha: ", password.c_str());
  });
}

void ServerManager::setupStaticRoutes() {
  server.on("/", HTTP_GET, [this]() {
    String indexPath = "/index.html.gz";
    handleFileRead(indexPath);
  });

  server.on("/sendfiles", HTTP_GET, [this]() {
    server.send(200, "text/html", R"rawliteral(
    <!DOCTYPE html>
    <html lang="pt-BR">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Upload de Arquivo</title>
        <style>
            body {
                font-family: Arial, sans-serif;
                margin: 30px;
                background-color: #f4f4f4;
                color: #333;
            }
            h1 {
                color: #444;
                text-align: center;
            }
            form {
                background: white;
                margin:auto;
                padding: 20px;
                border-radius: 8px;
                box-shadow: 0 2px 5px rgba(0,0,0,0.1);
                max-width: 400px;
            }
            input[type="file"], button {
                margin-top: 10px;
                width: 100%;
                padding: 10px;
                font-size: 16px;
            }
            #response {
                margin-top: 20px;
                text-align: center;
                justify-content: center;
                padding: 10px;
                border-radius: 5px;
                background-color: #e0e0e0;
                display: none;
            }
        </style>
    </head>
    <body>
        <h1>Enviar Arquivo para ESP32</h1>
        <form id="uploadForm">
            <input type="file" name="file" id="fileInput" required>
            <button type="submit">Enviar</button>
        </form>
        <div id="response"></div>

        <script>
            const form = document.getElementById('uploadForm');
            const fileInput = document.getElementById('fileInput');
            const responseDiv = document.getElementById('response');

            form.addEventListener('submit', async (e) => {
                e.preventDefault();
                const file = fileInput.files[0];
                if (!file) return;

                const formData = new FormData();
                formData.append('file', file);

                try {
                    const res = await fetch('/upload', {
                        method: 'POST',
                        body: formData
                    });

                    const text = await res.text();
                    responseDiv.innerHTML += `<div>${text}</div>`;
                    responseDiv.style.display = 'block';
                } catch (err) {
                    responseDiv.innerHTML += `<div>Erro ao enviar o arquivo.</div>`;
                    responseDiv.style.display = 'block';
                }
            });
        </script>
    </body>
    </html>
  )rawliteral");
  });
}

void ServerManager::setupNotFoundHandler() {
  server.onNotFound([this]() {
    String path = server.uri();

    if (!path.endsWith(".ico") && !path.endsWith(".png")) {
      String gzPath = path + ".gz";
      if (handleFileRead(gzPath)) return;
    }

    if (handleFileRead(path)) return;

    server.send(404, "text/plain", "Arquivo não encontrado.");
    serial.log(LOG_WARN, "[ServerManager] Requisição não atendida: ", path.c_str());
  });
}

bool ServerManager::handleFileRead(String &filePath) {
  File file = fm.getFile(filePath.c_str(), "r");
  if (!file) return false;

  String contentType = utils.getContentType(
      filePath.endsWith(".gz") ? filePath.substring(0, filePath.length() - 3) : filePath);

  if (filePath.endsWith(".gz")) {
    // server.sendHeader("Content-Encoding", "gzip");  //Aparentemente sendifles já envia o cabealho de codificação gzip
  }

  server.streamFile(file, contentType);
  file.close();

  serial.log(LOG_INFO, "[ServerManager] Arquivo enviado: ", filePath.c_str());
  return true;
}

bool ServerManager::handlePostPayload(JsonDocument &doc) {
  if (!server.hasArg("plain")) {
    raiseError("Conteúdo ausente.");
    return false;
  }

  DeserializationError error = deserializeJson(doc, server.arg("plain"));
  if (error) {
    raiseError("Falha ao analisar JSON: " + String(error.c_str()));
    return false;
  }

  return true;
}

String ServerManager::handleFileUpload(HTTPUpload &upload) {
  static String filename;

  if (upload.status == UPLOAD_FILE_START) {
    filename = "/" + upload.filename;
    serial.log(LOG_INFO, "[FileManager] Iniciando upload: ", filename.c_str());

    if (!fm.createFile(filename)) {
      serial.log(LOG_ERROR, "[FileManager] Erro ao criar arquivo: ", filename.c_str());
      return "";
    }
    return "START";
  }

  if (upload.status == UPLOAD_FILE_WRITE) {
    if (!fm.writeToFile(filename, upload.buf, upload.currentSize)) {
      serial.log(LOG_ERROR, "[FileManager] Erro ao gravar no arquivo.");
      return "";
    }
  }

  if (upload.status == UPLOAD_FILE_END) {
    serial.log(LOG_INFO, "[FileManager] Upload concluído: ", filename.c_str());
    return filename;
  }

  return "IN_PROGRESS";
}
