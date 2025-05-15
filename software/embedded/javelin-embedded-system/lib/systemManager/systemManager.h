#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H

#include <Arduino.h>
#include <DataLogger.h>
#include <UserInterface.h>
#include <LogWebServer.h>
#include <SD.h>

class systemManager
{
public:
    systemManager();
    ~systemManager();

    void begin();
    void update();

private:
    UserInterface ui;
    DataLogger logger;
    LogWebServer logServer;
    bool serverOn;
};

#endif