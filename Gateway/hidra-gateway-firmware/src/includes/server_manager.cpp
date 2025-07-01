#include "server_manager.h"

#pragma region static headers
static void getConfig();
static void getWifiStatus();
static void getWifiNetworks();
static void getWebserverConfig();
static void getModules();
static void postWifiToggle();
static void postWebserverToggle();
static void postWifiAdd();
static void postConfigTime();
static void postWebserverConfig();
static const char *getUploadPage();
#pragma endregion

ServerManager::ServerManager() : webserver(80) {}

void ServerManager::startServer() {
  setupRoutes();
  webserver.begin();
  serial.log(LOG_INFO, "[ServerManager] Servidor iniciado.");
}

void ServerManager::restartServer() {
  serial.log(LOG_INFO, "[ServerManager] Reiniciando servidor.");
  webserver.close();
  startServer();
}

void ServerManager::handleClient() {
  webserver.handleClient();
}

void ServerManager::end() {
  webserver.close();
  serial.log(LOG_INFO, "[ServerManager] Servidor encerrado.");
}

void ServerManager::raiseError(String errorMessage) {
  webserver.send(400, "text/plain", errorMessage);
  serial.log(LOG_ERROR, "[SERVER] ", errorMessage.c_str());
}

void ServerManager::raiseSuccess(const char *route, String successMessage, bool minimal) {
  webserver.send(200, "application/json", successMessage);
  if (minimal)
    serial.log(LOG_INFO, "[SERVER] [", route, "]: ", "Sucesso.");
  else
    serial.log(LOG_INFO, "[SERVER] [", route, "]: ", successMessage.c_str());
}

void ServerManager::setupRoutes() {
  setupGetRoutes();
  setupPostRoutes();
  setupStaticRoutes();
  setupNotFoundHandler();
}

void ServerManager::setupGetRoutes() {
  webserver.on("/config", HTTP_GET, getConfig);
  webserver.on("/wifi/status", HTTP_GET, getWifiStatus);
  webserver.on("/wifi/networks", HTTP_GET, getWifiNetworks);
  webserver.on("/server/config", HTTP_GET, getWebserverConfig);
  webserver.on("/modules", HTTP_GET, getModules);
}

void ServerManager::setupPostRoutes() {
  webserver.on("/wifi/toggle", HTTP_POST, postWifiToggle);
  webserver.on("/webserver/toggle", HTTP_POST, postWebserverToggle);
  webserver.on("/wifi/add", HTTP_POST, postWifiAdd);
  webserver.on("/config/time", HTTP_POST, postConfigTime);
  webserver.on("/webserver/config", HTTP_POST, postWebserverConfig);

  webserver.on("/upload", HTTP_POST, [this]() { server.raiseSuccess("/upload", "{ \"message\": \"Sucesso ao enviar.\"}"); }, [this]() {
        String result = handleFileUpload(webserver.upload());
        if (result == "") {
          webserver.send(500, "text/plain", "Erro ao salvar arquivo.");
        } else if (result != "START" && result != "IN_PROGRESS") {
          webserver.send(200, "text/plain", "Arquivo recebido: " + result);
        } });
}

void ServerManager::setupStaticRoutes() {
  webserver.on("/", HTTP_GET, [this]() {
    String indexPath = "/index.html.gz";
    handleFileRead(indexPath);
  });

  webserver.on("/sendfiles", HTTP_GET, [this]() {
    webserver.send(200, "text/html", getUploadPage());
  });
}

void ServerManager::setupNotFoundHandler() {
  webserver.onNotFound([this]() {
    String path = webserver.uri();

    if (!path.endsWith(".ico") && !path.endsWith(".png")) {
      String gzPath = path + ".gz";
      if (handleFileRead(gzPath)) return;
    }

    if (handleFileRead(path)) return;

    webserver.send(404, "text/plain", "Arquivo não encontrado.");
    serial.log(LOG_WARN, "[ServerManager] Requisição não atendida: ", path.c_str());
  });
}

bool ServerManager::handleFileRead(String &filePath) {
  File file = fm.getFile(filePath.c_str(), "r");
  if (!file) return false;

  String contentType = utils.getContentType(
      filePath.endsWith(".gz") ? filePath.substring(0, filePath.length() - 3) : filePath);

  if (filePath.endsWith(".gz")) {
    // webserver.sendHeader("Content-Encoding", "gzip");  //Aparentemente sendifles já envia o cabealho de codificação gzip
  }

  webserver.streamFile(file, contentType);
  file.close();

  serial.log(LOG_INFO, "[ServerManager] Arquivo enviado: ", filePath.c_str());
  return true;
}

bool ServerManager::handlePostPayload(JsonDocument &doc) {
  if (!webserver.hasArg("plain")) {
    raiseError("Conteúdo ausente.");
    return false;
  }

  DeserializationError error = deserializeJson(doc, webserver.arg("plain"));
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

#pragma region static definitions
// send the current date and time in JSON format
// Example: {"date": "2023-10-01", "time": "12:30:00", "address": "N/A"}
static void getConfig() {
  JsonDocument doc;
  doc["date"] = rtc.getClock(DD);
  doc["time"] = rtc.getClock(HH);
  doc["address"] = "N/A";  // TODO: Corrigir endereço

  String jsonOutput;
  serializeJson(doc, jsonOutput);
  server.raiseSuccess("/config", jsonOutput);
}

// send the current WiFi status in JSON format
// Example: {"ssid": "MyNetwork", "rssi": -70, "ip": "192.168.1.100"}
static void getWifiStatus() {
  JsonDocument doc;
  doc["ssid"] = wm.getConnectedSSID();
  doc["rssi"] = wm.getConnectedRSSI();
  doc["ip"] = utils.ip2Str(wm.getWiFiIP());
  doc["status"] = wm.isWiFiConnected() ? 1 : 0;

  String jsonOutput;
  serializeJson(doc, jsonOutput);
  server.raiseSuccess("/wifi/status", jsonOutput);
}

// send the list of saved WiFi networks and nearby networks in JSON format
// Example: {"saved": [{"id": 0, "ssid": "Network1", "password": "pass1"}, ...],
//           "near": [{"ssid": "Network2", "rssi": -70}, ...]}
static void getWifiNetworks() {
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
  server.raiseSuccess("/wifi/networks", jsonOutput, true);
}

// send the current webserver configuration in JSON format
// Example: {"ssid": "MyAP", "pass": "mypassword", "ip": "192.168.1.1"}
static void getWebserverConfig() {
  JsonDocument doc;
  doc["ssid"] = wm.getAPSSID();
  doc["pass"] = wm.getAPPassword();
  doc["ip"] = utils.ip2Str(wm.getAPIP());
  doc["status"] = wm.isAccessPointActive() ? 1 : 0;

  String jsonOutput;
  serializeJson(doc, jsonOutput);
  server.raiseSuccess("/server/config", jsonOutput);
}

// send a list of modules in JSON format
// Example: {"modules": [{"id": 0, "name": "Module1", "recieve-date": "2025-23-06", "recieve-time": "14:19:00", "address": "123", "bat": 45}, ...]}
static void getModules() {
  // TODO: Implement a real data retrieval mechanism
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

  server.raiseSuccess("/modules", jsonOutput, true);
}

// send WiFi connection status based on the provided status in the JSON payload
// Example: {"status": 1} to connect, {"status": 0} to disconnect
static void postWifiToggle() {
  JsonDocument doc;
  if (!server.handlePostPayload(doc)) return;

  int status = doc["status"].as<int>();
  if (status == 1)
    wm.autoConnectWiFi();
  else if (status == 0)
    wm.disconnectWiFi();
  else {
    server.raiseError("Status inválido.");
    return;
  }

  server.raiseSuccess("/wifi/toggle", "{\"status\": " + String(status) + "}");
}

// send the webserver access point status based on the provided status in the JSON payload
// Example: {"status": 1} to start the access point, {"status": 0} to stop it
static void postWebserverToggle() {
  JsonDocument doc;
  if (!server.handlePostPayload(doc)) return;

  int status = doc["status"];
  if (status == 1)
    wm.startAccessPoint();
  else if (status == 0)
    wm.stopAccessPoint();
  else {
    server.raiseError("Status inválido.");
    return;
  }

  server.raiseSuccess("/webserver/toggle", "{\"status\": " + String(status) + "}");
}

// Add a new WiFi network to the saved networks list
// Example: {"ssid": "MyNetwork", "password": "mypassword"}
static void postWifiAdd() {
  JsonDocument doc;
  if (!server.handlePostPayload(doc)) return;

  String ssid = doc["ssid"].as<String>();
  String password = doc["password"].as<String>();

  if (ssid.isEmpty()) {
    server.raiseError("SSID ausente.");
    return;
  }

  // TODO: Corrigir método de adição de rede WiFi
  wifiStore.addSavedNetwork(ssid, password);
  server.raiseSuccess("/wifi/add", "{\"ssid\": \"" + ssid + "\"}");
}

// Set the internal clock using the provided date and time in the JSON payload
// Example: {"date": "10/01/2025", "time": "12:30:00"}
static void postConfigTime() {
  JsonDocument doc;
  if (!server.handlePostPayload(doc)) return;

  String date = doc["date"].as<String>();
  String time = doc["time"].as<String>();

  if (date.isEmpty() || time.isEmpty()) {
    server.raiseError("Data ou hora ausente.");
    return;
  }

  rtc.setClockByString(date, time);
  server.raiseSuccess("/config/time", "{\"date\": \"" + date + "\", \"time\": \"" + time + "\"}");
}

// Configure the webserver access point with the provided SSID and password in the JSON payload
// Example: {"ssid": "MyAP", "pass": "mypassword"}
static void postWebserverConfig() {
  JsonDocument doc;
  if (!server.handlePostPayload(doc)) return;

  String ssid = doc["ssid"].as<String>();
  String password = doc["pass"].as<String>();
  if (ssid.isEmpty()) {
    server.raiseError("SSID ausente.");
    return;
  }
  if (ssid.length() < 3 || ssid.length() > 16) {
    server.raiseError("SSID deve ter entre 3 e 16 caracteres.");
    return;
  }
  if (password.length() < 8 || password.length() > 16) {
    server.raiseError("Senha deve ter entre 8 e 16 caracteres.");
    return;
  }
  wm.setAccessPoint(ssid.c_str(), password.c_str());
  wm.restartAccessPoint();
  server.raiseSuccess("/webserver/config", "{\"ssid\": \"" + ssid + "\", \"pass\": \"" + password + "\"}");
}

// HTML page for file upload
// This page allows users to upload files to the ESP32's filesystem
static const char *getUploadPage() {
  return R"rawliteral(
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
  )rawliteral";
}

#pragma endregion
