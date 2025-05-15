#include <Arduino.h>
#include <systemManager.h>

systemManager sysManager;

void setup() {
    sysManager.begin();
}

void loop() {
    sysManager.update();
}