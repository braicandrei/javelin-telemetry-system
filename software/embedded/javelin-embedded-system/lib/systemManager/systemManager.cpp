#include "systemManager.h"

#define SD_CS_PIN 3

systemManager::systemManager()
    : logServer(SD_CS_PIN, "ESP32", "12345678", "javelin"), serverOn(false)
{}

systemManager::~systemManager(){}


/*!
    * @brief Initializes the system manager
    * 
    * This function initializes the system manager, including the user interface, data logger, and SD card.
    * 
    * @return void
    */
void systemManager::begin() {
    Serial.begin(115200);
    ui.beginUI();
    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("SD card initialization failed!");
    }
    if (logger.begin() != LOGGER_OK) {
        Serial.println("Error initializing data logger!");
        while (1);
    }
    ui.setSystemTransition(POWER_ON);
}

/*!
    * @brief Updates the system manager
    * 
    * This function updates the system manager, including the user interface and data logger.
    * 
    * @return void
*/
void systemManager::update() {
    if (logger.updateLogger() == SHOCK_DETECTED) {
        if (logger.getLoggerState() == LOGGER_SAMPLING) {
            logger.stopSamplig();
            ui.setSystemTransition(SAMPLE_END);
        }
    }
    switch (ui.updateUI()) {
    case INPUT_1:
        if (logger.getLoggerState() == LOGGER_WAITING && !serverOn) {
            logger.startSamplig();
            ui.setSystemTransition(SAMPLE_BEGIN);
        } else if (logger.getLoggerState() == LOGGER_SAMPLING) {
            logger.stopSamplig();
            ui.setSystemTransition(SAMPLE_END);
        }
        break;
    case INPUT_2:
        if (!serverOn && logger.getLoggerState() == LOGGER_WAITING) {
            logServer.begin();
            serverOn = true;
            ui.setSystemTransition(SERVER_MODE_ON);
        } else {
            logServer.end();
            serverOn = false;
            ui.setSystemTransition(SERVER_MODE_OFF);
        }
        break;
    case INPUT_3:
        if (logger.getLoggerState() == LOGGER_WAITING) {
            logger.setCalibration();
            logger.startSamplig();
            ui.setSystemTransition(SAMPLE_BEGIN);
        }
        break;
    default:
        break;
    }
}