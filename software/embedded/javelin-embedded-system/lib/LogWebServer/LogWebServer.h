#ifndef LOGWEBSERVER_H
#define LOGWEBSERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <SPI.h>
#include <SD.h>
#include <LittleFS.h>

class LogWebServer {
public:
  LogWebServer(uint8_t sdCsPin, const char* ssid, const char* password, const char* mdnsName);
  void begin();
  void end();

private:
  uint8_t _sdCsPin;
  const char* _ssid;
  const char* _password;
  const char* _mdnsName;

  AsyncWebServer _server;
  bool _running;
  volatile bool isProcessingRequest = false;
  void initRoutes();
};

#endif
