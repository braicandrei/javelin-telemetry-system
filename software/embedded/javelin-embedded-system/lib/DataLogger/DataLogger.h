#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H


#include <Arduino.h>
#include <AHRS.h>
#include <SD.h>
#include <FS.h>
#include <RTClib.h>

#define AHRS_ITERRUPT_PIN 4 // Pin for AHRS interrupt
#define SD_CS_PIN 3 // Chip select pin for SD card
#define SD_MISO_PIN 8
#define SD_MOSI_PIN 9
#define SD_CLK_PIN 7

#define FRAME_BUFFER_LENGTH 4
#define SAMPLE_RATE 100 // Sampling rate in Hz

typedef enum {
    LOGGER_OK,
    AHRS_FAILED,
    SD_FAILED,
    RTC_FAILED,
    SHOCK_DETECTED
} LoggerStatus_t;

typedef enum {
    LOGGER_WAITING,
    LOGGER_PREP,
    LOGGER_SAMPLING,
    LOGGER_END,
} LoggerState_t;

class DataLogger
{
public:
    DataLogger();
    ~DataLogger();
    LoggerStatus_t begin(); // Initialize the data logger
    LoggerStatus_t updateLogger(); // Update the data logger
    void startSamplig(); // Start data sampling
    void stopSamplig(); // End data sampling
    void setCalibration(); // Set calibration flag
    LoggerState_t getLoggerState();
private:
    AHRS ahrs; // AHRS object for sensor data
    uint16_t frameCounter = 0;
    //QueueHandle_t queue = NULL;
    //static volatile bool inputDataAvailable; // Flag for interrupt handling
    //static void IRAM_ATTR inputDataInterrupt(); // Interrupt handler function
    icm20x_raw_axes_t raw_axesD[ICM20X_FIFO_SIZE/6]; // Buffer for raw sensor data
    RTC_DS3231 rtc; // RTC object for real-time clock
    String getFileName(); // Generate a file name for data logging
    
    File dataFile; // File object for SD card data logging
    const String logsDirectory = "/logs"; // Directory for logs 
    String logPath; // File name for data logging
    ahrs_axes_t dataFrame; // Data frame for sensor data

    ahrs_axes_t last_axes;
    ahrs_orientation_t last_orientation; // Last orientation data
    bool shockDetected = false;
    void writeMetaDataToSD(); // Write metadata to SD card
    void writeDataFrameToSD(ahrs_axes_t dataFrame); // Write data frame to SD card
    void writeDataFrameToSD(ahrs_axes_t dataFrame, ahrs_orientation_t orientation);
    void writeDataFrameToSD(ahrs_axes_t dataFrame, ahrs_angles_t angles);
    unsigned long packetCounter = 0;
    ahrs_axes_t dataFrameBuffer[FRAME_BUFFER_LENGTH];
    bool shockCheck(ahrs_axes_t dataFrame);
    const float shockThreshold = 15.0;
    
    bool getShockDetected();
    
    LoggerState_t loggerState = LOGGER_WAITING; // State of the data logger
    unsigned long startTime = 0; // Start time for data logging
    bool startSampling_flag = false; // Flag to indicate logger start
    bool stopSampling_flag = false; // Flag to indicate logger stop
    bool calibration_flag = false; // Flag to indicate calibration

};


#endif