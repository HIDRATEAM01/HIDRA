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

void ServerManager::setupGetRoutes() {
  server.on("/deletedist", HTTP_GET, [this]() {
    serial.log(LOG_INFO, "[ServerManager] Iniciando remoção do dist.tar.gz");
    if (fm.removeRecursive("/dist.tar.gz"))
      raiseSucess("Arquivo removido com sucesso!");
    else
      raiseError("Erro ao remover arquivo.");
  });

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
    serial.log(LOG_INFO, "[ServerManager] [/wifi/networks]: Sucesso. JSON: ", jsonOutput.c_str());
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
            "name": "bairro do sol nascente",
            "recieve-date": "2025-23-06",
            "recieve-time": "10:58:00",
            "address": "987",
            "bat": 28
          },
          {
            "id": 6,
            "name": "estrada das palmeiras",
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
        server.send(200, "text/plain", "Arquivo recebido!");
      },
      [this]() {
        if (!handleFileUpload(server.upload())) {
          server.send(500, "text/plain", "Erro ao salvar arquivo.");
        }
      });
}

void ServerManager::setupStaticRoutes() {
  server.on("/", HTTP_GET, [this]() {
    handleFileRead("/index.html.gz");
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
                    responseDiv.textContent = text;
                    responseDiv.style.display = 'block';
                } catch (err) {
                    responseDiv.textContent = 'Erro ao enviar o arquivo.';
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

    // Tenta .gz primeiro, se não for imagem
    if (!path.endsWith(".ico") && !path.endsWith(".png")) {
      String gzPath = path + ".gz";
      if (fm.exists(gzPath)) {
        handleFileRead(gzPath);
        return;
      }
    }

    handleFileRead(path);
  });
}

void ServerManager::handleFileRead(String filePath) {
  File file = fm.getFile(filePath.c_str(), "r");
  String contentType = utils.getContentType(
      filePath.endsWith(".gz")
          ? filePath.substring(0, filePath.length() - 3)
          : filePath);

  if (file) {
    if (filePath.endsWith(".gz")) {
      // server.sendHeader("Content-Encoding", "gzip");
    }
    server.streamFile(file, contentType);
    file.close();
    serial.log(LOG_INFO, "[ServerManager] Arquivo enviado: ", filePath.c_str());
  } else {
    raiseError("Arquivo não encontrado.");
  }
}

bool ServerManager::handleFileUpload(HTTPUpload &upload) {
  static String filename;

  if (upload.status == UPLOAD_FILE_START) {
    filename = "/" + upload.filename;
    serial.log(LOG_INFO, "[FileManager] Iniciando upload: ", filename.c_str());

    if (!fm.createFile(filename)) {
      serial.log(LOG_ERROR, "[FileManager] Erro ao criar arquivo: ", filename.c_str());
      return false;
    }

    return true;
  }

  if (upload.status == UPLOAD_FILE_WRITE) {
    if (!fm.writeToFile(filename, upload.buf, upload.currentSize)) {
      serial.log(LOG_ERROR, "[FileManager] Erro ao gravar no arquivo.");
      return false;
    }
  }

  if (upload.status == UPLOAD_FILE_END) {
    serial.log(LOG_INFO, "[FileManager] Upload concluído: ", filename.c_str());
  }

  return true;
}
