#include "LogWebServer.h"

#define DEBUG_WEBSERVER 0
LogWebServer::LogWebServer(uint8_t sdCsPin, const char* ssid, const char* password, const char* mdnsName)
  : _sdCsPin(sdCsPin), _ssid(ssid), _password(password), _mdnsName(mdnsName), _server(80), _running(false) {}

void LogWebServer::begin() {
  #if (DEBUG_WEBSERVER)
    Serial.println("[WebServer] Iniciando servidor web...");
  #endif

  WiFi.mode(WIFI_AP);
  WiFi.softAP(_ssid, _password);
    #if (DEBUG_WEBSERVER)
        Serial.println("[WebServer] Iniciando AP...");
    #endif
  Serial.println(WiFi.softAPIP());

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
  _server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    File file = SD.open("/web/index.html");
    if (!file) {
      request->send(500, "text/plain", "No se pudo cargar el HTML");
      return;
    }
    String html;
    while (file.available()) html += (char)file.read();
    file.close();
    request->send(200, "text/html", html);
  });

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

  _server.onNotFound([this](AsyncWebServerRequest *request) {
    String url = request->url();

    if (url.startsWith("/logs/")) {
      String filename = url.substring(strlen("/logs/"));
      String path = "/logs/" + filename;

      if (SD.exists(path)) {
        request->send(SD, path.c_str(), "text/csv");
      } else {
        request->send(404, "text/plain", "Archivo no encontrado");
      }
    } else {
      request->send(404, "text/plain", "Ruta no encontrada");
    }
  });

  _server.serveStatic("/libs", SD, "/libs");
}
