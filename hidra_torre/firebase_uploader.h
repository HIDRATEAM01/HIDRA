#ifndef FIREBASE_UPLOADER_H
#define FIREBASE_UPLOADER_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <FS.h>
#include <SPIFFS.h>
#include "debug_control.h"
#include "secrets.h"
#include <ArduinoJson.h>
#include <Preferences.h>

Preferences prefs;
int FBcounter = 0;

class FirebaseUploader {
private:
    String bucketUrl;
    String authToken;
    String databaseUrl;

    bool login() {
        if (FIREBASE_EMAIL == "" || FIREBASE_PASS == "" || FIREBASE_API == "") {
            debug.println("[Firebase] Credenciais não definidas");
            return false;
        }

        HTTPClient http;
        String url = String("https://identitytoolkit.googleapis.com/v1/accounts:signInWithPassword?key=") + FIREBASE_API;

        String body = "{\"email\":\"" + String(FIREBASE_EMAIL) + "\",\"password\":\"" + String(FIREBASE_PASS) + "\",\"returnSecureToken\":true}";

        http.begin(url);
        http.addHeader("Content-Type", "application/json");

        debug.println("[Firebase] URL: " + url);
        debug.println("[Firebase] Body: " + body);
        debug.println("[Firebase] WiFi status: " + String(WiFi.status()));

        int code = http.POST(body);
        if (code != 200) {
            debug.println("[Firebase] Falha no login: " + String(code));
            debug.println(http.getString());
            http.end();
            return false;
        }

        String resp = http.getString();
        http.end();

        DynamicJsonDocument doc(1024);
        deserializeJson(doc, resp);
        authToken = doc["idToken"].as<String>();
        debug.println("[Firebase] Login OK, ID token obtido");
        return true;
    }

public:
    FirebaseUploader() {}

    void begin(const String& bucket, const String& rtdb) {
        bucketUrl = bucket;
        databaseUrl = rtdb;

        prefs.begin("firebase", false);
        FBcounter = prefs.getInt("FBcounter", 0); 
        debug.println("[Firebase] Iniciando contador em: " + String(FBcounter));
        login();
    }

    bool uploadJSON(const String& jsonData, const String& path) {
        if (WiFi.status() != WL_CONNECTED) {
            debug.println("[Firebase] WiFi não conectado");
            return false;
        }

        if (!jsonData || jsonData.length() == 0) {
            debug.println("[Firebase] Mensagem Vazia");
            return false;
        }

        HTTPClient http;
        String url = "https://" + databaseUrl + "/" + path + "/" + String(FBcounter) + ".json?auth=" + authToken;

        http.begin(url);
        http.addHeader("Content-Type", "application/json");

        int code = http.PUT(jsonData);

        if (code > 0) {
            debug.println("[Firebase] Upload JSON, code: " + String(code));
            debug.println(http.getString());

            if (code == 200 || code == 201) {
                FBcounter++;
                prefs.putInt("FBcounter", FBcounter);
            }
        } else {
            debug.println("[Firebase] Erro no upload JSON: " + http.errorToString(code));
        }

        http.end();
        return (code == 200 || code == 201);
    }

    bool uploadFile(const char* path, const String& contentType = "image/jpeg") {
        if (WiFi.status() != WL_CONNECTED) {
            debug.println("[Firebase] WiFi não conectado");
            return false;
        }

        File file = SPIFFS.open(path, "r");
        if (!file || file.isDirectory()) {
            debug.println("[Firebase] Erro ao abrir arquivo: " + String(path));
            return false;
        }

        String fileName = "foto_" + String(FBcounter)+ ".jpg";

        debug.println("[Firebase] Upload: " + String(path));
        debug.println("[Firebase] Tamanho: " + String(file.size()) + " bytes");
        debug.println("[Firebase] Envio: " + fileName);

        WiFiClientSecure client;
        client.setInsecure();

        if (!client.connect("firebasestorage.googleapis.com", 443)) {
            debug.println("[Firebase] Falha ao conectar no servidor");
            file.close();
            return false;
        }

        // Monta URL relativa
        String url = "/v0/b/" + bucketUrl + "/o?uploadType=media&name=" + fileName;

        // Cabeçalho HTTP
        client.println("POST " + url + " HTTP/1.1");
        client.println("Host: firebasestorage.googleapis.com");
        if (authToken != "") {
            client.println("Authorization: Bearer " + authToken);
        }
        client.println("Content-Type: " + contentType);
        client.println("Content-Length: " + String(file.size()));
        client.println();

        uint8_t buf[512];
        while (file.available()) {
            size_t len = file.read(buf, sizeof(buf));
            client.write(buf, len);
        }
        file.close();

        // Lê resposta
        String response;
        while (client.connected()) {
            String line = client.readStringUntil('\n');
            if (line == "\r") break; // fim dos headers
        }
        while (client.available()) {
            response += client.readString();
        }

        debug.println("[Firebase] Resposta: " + response);

        // Checa se deu certo
        return (response.indexOf("\"bucket\"") != -1 || response.indexOf("\"name\"") != -1);
    }
};

#endif
