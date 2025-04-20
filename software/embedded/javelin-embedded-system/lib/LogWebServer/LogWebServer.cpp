#include "LogWebServer.h"

#define DEBUG_WEBSERVER 0
LogWebServer::LogWebServer(uint8_t sdCsPin, const char* ssid, const char* password, const char* mdnsName)
  : _sdCsPin(sdCsPin), _ssid(ssid), _password(password), _mdnsName(mdnsName), _server(80), _running(false) {}

void LogWebServer::begin() {
  #if (DEBUG_WEBSERVER)
    Serial.println("[WebServer] Iniciando servidor web...");
  #endif

  if (!LittleFS.begin(true)) {
    #if (DEBUG_WEBSERVER)
      Serial.println("[WebServer] Error al montar LittleFS");
    #endif
    return;
  }

  WiFi.mode(WIFI_AP);
  WiFi.softAP(_ssid, _password);
    #if (DEBUG_WEBSERVER)
        Serial.println("[WebServer] Iniciando AP...");
        Serial.println(WiFi.softAPIP());
    #endif

  if (!MDNS.begin(_mdnsName)) {
    #if (DEBUG_WEBSERVER)
      Serial.println("[WebServer] Error al iniciar mDNS");
    #endif
    return;
  }

  initRoutes();
  _server.begin();
  _running = true;
  #if (DEBUG_WEBSERVER)
      Serial.println("[WebServer] Servidor iniciado ");
  #endif
}

void LogWebServer::end() {
  if (_running) {
    _server.end();
    MDNS.end();
    delay(100);
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    _running = false;
    #if (DEBUG_WEBSERVER)
      Serial.println("[WebServer] Servidor detenido");
    #endif
  }
}

void LogWebServer::initRoutes() {
  // 0) Bloquear cualquier .map en /libs (evita VFS errors)
  _server.on("/libs/*.map", HTTP_GET, [](AsyncWebServerRequest *req){
    #if (DEBUG_WEBSERVER)
      Serial.printf("Stub .map request: %s\n", req->url().c_str());
    #endif
    req->send(204, "text/plain", "");  // 204 No Content
  });

  // 1) Listar CSV desde la SD
  _server.on("/list", HTTP_GET, [](AsyncWebServerRequest *request){
    File dir = SD.open("/logs");
    String json = "[";
    bool first = true;
    while (true) {
      File entry = dir.openNextFile();
      if (!entry) break;
      String name = String(entry.name());
      if (!entry.isDirectory() && name.endsWith(".csv")) {
        if (!first) json += ",";
        json += "\"" + name + "\"";
        first = false;
      }
    }
    json += "]";
    request->send(200, "application/json", json);
  });

  // 2) Servir CSV desde la SD
  _server.on("/logs/*", HTTP_GET, [this](AsyncWebServerRequest *request) {
    if (isProcessingRequest) {
        request->send(503, "text/plain", "Servidor ocupado, intente más tarde.");
        return;
    }

    isProcessingRequest = true;

    String url = request->url();
    String filename = url.substring(strlen("/logs/"));
    String path = "/logs/" + filename;
    #if (DEBUG_WEBSERVER)
      Serial.printf("Petición CSV SD: %s\n", path.c_str());
    #endif

    if (SD.exists(path)) {
        // Enviar archivo
        request->send(SD, path.c_str(), "text/csv");

        // Esperar a que se cierre la conexión
        request->onDisconnect([this]() {
            isProcessingRequest = false;
            #if (DEBUG_WEBSERVER)
              Serial.println(" Archivo enviado. Listo para nueva petición.");
            #endif
        });

    } else {
        request->send(404, "text/plain", "Archivo no encontrado");
        isProcessingRequest = false;
    }
});

  // 3) Librerías JS/CSS desde LittleFS
  _server.serveStatic("/libs", LittleFS, "/libs");

  // 4) Página y recursos estáticos desde LittleFS
  _server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

  // 5) Resto: 404 con log
  _server.onNotFound([](AsyncWebServerRequest *request) {
    #if (DEBUG_WEBSERVER)
      Serial.printf("404 Ruta no encontrada: %s\n", request->url().c_str());
    #endif
    request->send(404, "text/plain", "Ruta no encontrada");
  });
}