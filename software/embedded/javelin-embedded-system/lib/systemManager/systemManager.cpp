#include "systemManager.h"

#define SD_CS_PIN 3
#define DEBUG_SYSTEM_MANAGER 0

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
        #if (DEBUG_SYSTEM_MANAGER)
            Serial.println("[SystemManager] Shock detected!");
        #endif
        if (logger.getLoggerState() == LOGGER_SAMPLING) {
            #if (DEBUG_SYSTEM_MANAGER)
            Serial.println("[SystemManager] Stopping sampling.");
            #endif
            logger.stopSamplig();
            ui.setSystemTransition(SAMPLE_END);
        }
    }
    switch (ui.updateUI()) {
    case INPUT_1:
        if (logger.getLoggerState() == LOGGER_WAITING && !serverOn) {
            logger.startSamplig();
            ui.setSystemTransition(SAMPLE_BEGIN);
            #if (DEBUG_SYSTEM_MANAGER)
            Serial.println("[SystemManager] INPUT1 Starting sampling.");
            #endif
        } else if (logger.getLoggerState() == LOGGER_SAMPLING) {
            logger.stopSamplig();
            ui.setSystemTransition(SAMPLE_END);
            #if (DEBUG_SYSTEM_MANAGER)
            Serial.println("[SystemManager] INPUT1 Stopping sampling.");
            #endif
        }
        break;
    case INPUT_2:
        if (!serverOn && logger.getLoggerState() == LOGGER_WAITING) {
            logServer.begin();
            serverOn = true;
            ui.setSystemTransition(SERVER_MODE_ON);
            #if (DEBUG_SYSTEM_MANAGER)
            Serial.println("[SystemManager] INPUT2 Starting server mode.");
            #endif
        } else {
            logServer.end();
            serverOn = false;
            ui.setSystemTransition(SERVER_MODE_OFF);
            #if (DEBUG_SYSTEM_MANAGER)
            Serial.println("[SystemManager] INPUT2 Stopping server mode.");
            #endif
        }
        break;
    case INPUT_3:
        if (logger.getLoggerState() == LOGGER_WAITING) {
            logger.setCalibration();
            logger.startSamplig();
            ui.setSystemTransition(SAMPLE_BEGIN);
            #if (DEBUG_SYSTEM_MANAGER)
            Serial.println("[SystemManager] INPUT3 Starting calibration.");
            #endif
        }
        break;
    default:
        break;
    }
}