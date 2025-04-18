
#include "DataLogger.h"

#define DEBUG_LOGER 1

/**
 * @brief DataLogger constructor
 * 
 * The constructor initializes the data logger object
 */
DataLogger::DataLogger(){}
/**
 * @brief DataLogger destructor
 * 
 * The destructor cleans up the data logger object
 */
DataLogger::~DataLogger(){}


volatile bool DataLogger::inputDataAvailable= false; // Flag for interrupt handling
/**
 * @brief Interrupt handler for data input
 * 
 * This function is called when the interrupt occurs, setting the inputDataAvailable flag to true.
 */
void IRAM_ATTR DataLogger::inputDataInterrupt(){inputDataAvailable = true;} // Set the flag to indicate an interrupt has occurred

/**
 * @brief Initialize the data logger
 * 
 * This function initializes the SD card, RTC, and AHRS sensor.
 * 
 * @return LoggerStatus_t Status of the logger initialization
 */
LoggerStatus_t DataLogger::begin() {
    // Initialize the data logger
    //if (!SD.begin(SD_CS_PIN)) {
    //    #if(DEBUG_LOGER)
    //        Serial.println("SD card initialization failed!");
    //    #endif
    //    return SD_FAILED;
    //}
    #if(DEBUG_LOGER)
        Serial.println("SD card initialized.");
    #endif
    if (!rtc.begin()) {
        #if(DEBUG_LOGER)
            Serial.println("RTC initialization failed!");
        #endif
        return RTC_FAILED;
    }
    #if(DEBUG_LOGER)
        Serial.println("RTC initialized.");
    #endif
    queue = xQueueCreate(1000, sizeof(ahrs_axes_t));//create queue for 500 elements of type ahrs_axes_t
    if (ahrs.beginAHRSi2c()!=0x00) {
        #if(DEBUG_LOGER)
            Serial.println("AHRS initialization failed!");
        #endif
        return AHRS_FAILED;
    }
    ahrs.configAHRS();// Configure ICM20649 and LIS3MDL
    pinMode(AHRS_ITERRUPT_PIN,INPUT);//attach interrupt to pin 4
    attachInterrupt(digitalPinToInterrupt(AHRS_ITERRUPT_PIN), inputDataInterrupt, RISING); //attach interrupt to pin 4
    return LOGGER_OK; // Return logger status
}

/**
 * @brief Generate a file name for data logging
 * 
 * This function generates a file name based on the current date and time.
 * 
 * @return String File name for data logging
 */
String DataLogger::getFileName() {
    // Generate a file name for data log
    DateTime now = rtc.now();

    char fileName[32]; // Buffer size is enough for the full path
    snprintf(fileName, sizeof(fileName), "/datalog_%04d%02d%02d_%02d%02d%02d.csv",
             now.year(), now.month(), now.day(),
             now.hour(), now.minute(), now.second());

    return String(fileName);
}

/**
 * @brief Write a data frame to the SD card
 * 
 * This function writes a data frame to the SD card in CSV format.
 * 
 * @param dataFrame Data frame to be written to the SD card
 */
void DataLogger::writeDataFrameToSD(ahrs_axes_t dataFrame) {
    String data =   String(dataFrame.accX, 4)  + "," +
                    String(dataFrame.accY, 4)  + "," +
                    String(dataFrame.accZ, 4)  + "," +
                    String(dataFrame.gyroX, 4) + "," +
                    String(dataFrame.gyroY, 4) + "," +
                    String(dataFrame.gyroZ, 4) + "," +
                    String(dataFrame.magX, 4)  + "," +
                    String(dataFrame.magY, 4)  + "," +
                    String(dataFrame.magZ, 4)  + "\n";
    dataFile.print(data); // Write the data to the file
}

/**
 * @brief Update the data logger
 * 
 * This function updates the data logger state machine.
 * 
 * @return LoggerStatus_t Status of the data logger update
 */
LoggerStatus_t DataLogger::updateLogger() {

    switch (loggerState)
    {
    case LOGGER_WAITING:
        if (startSampling_flag) {
            startSampling_flag = false; // Reiniciar el flag
            #if(DEBUG_LOGER)
                Serial.println("Start sampling!");
            #endif
            loggerState = LOGGER_PREP;

        }
        break;
    case LOGGER_PREP:
        logPath = logsDirectory + getFileName();
        dataFile = SD.open(logPath, FILE_WRITE); //open file for writing
        if (!dataFile)
        {
            #if(DEBUG_LOGER)
                Serial.println("Error opening datalog.txt");
            #endif
            loggerState = LOGGER_WAITING;
            return SD_FAILED;
        }
        ahrs.icm20649.resetFIFO();// Reset FIFO buffer
        xQueueReset(queue);// Reset the queue
        frameCounter = 0;// Reset frame counter
        loggerState = LOGGER_SAMPLING;
        #if(DEBUG_LOGER)
            Serial.println("Begin sampling in file" + logPath);
        #endif
        startTime = millis();
        break;
    case LOGGER_SAMPLING:
    {   //Serial.print("Frame count: ");
        //Serial.println(ahrs.icm20649.readFIFOCount()); // Print the frame count for debugging
        if (inputDataAvailable && !stopSampling_flag) {
            inputDataAvailable = false;//clear the flag
            //unsigned long t0 = millis(); // Start time for data processing
            uint16_t frameCount = ahrs.icm20649.readFIFOBuffer(raw_axesD);// Read FIFO buffer
            //Serial.println("Time to read FIFO: " + String(millis() - t0) + "ms"); // Debugging time taken to read FIFO
            for (size_t i = 0; i < frameCount; i++) { // Process each frame of data
              ahrs_axes_t scaled_axes = ahrs.scaleAxes(raw_axesD[i]);
              if (xQueueSend(queue, &scaled_axes, 0) != pdPASS) {
                #if(DEBUG_LOGER)
                    Serial.println("Queue full");
                #endif
              }
              frameCounter++;
            }
        }
        if (xQueueReceive(queue, &dataFrame, 0) == pdPASS) { // Check if data is available in the queue
            writeDataFrameToSD(dataFrame); // Write the data frame to SD card
            if (calibration_flag)
            {
                ahrs.magHardIronCalc(dataFrame.magX, dataFrame.magY, dataFrame.magZ); // Perform hard iron calibration
            }
            
        } else if (stopSampling_flag)
        {
            loggerState = LOGGER_END; // Change state to LOGGER_END if stopSampling is true
            stopSampling_flag = false; // Reset the stopSampling flag
        }
        break;
    }
    case LOGGER_END:
        dataFile.close(); // Close the file
        if (calibration_flag){
            calibration_flag = false; // Reset the calibration flag
            ahrs.magCalibWrite(); // Write offset values to magnetometer for hard iron calibration
            ahrs.saveMagCalibToEEPROM(); // Save calibration data to EEPROM
        }
        #if(DEBUG_LOGER)
            Serial.println("End sampling!");
        #endif
        //###################################
        //put the ahrs in low power mode (pending)
        //####################################
        loggerState = LOGGER_WAITING; // Reset state to LOGGER_WAITING
        break;
    default:
        loggerState = LOGGER_WAITING;
        break;
    }
    return LOGGER_OK; // Return logger status
}

/**
 * @brief Start data sampling
 * 
 * This function sets the flag to start data sampling.
 */
void DataLogger::startSamplig() {
    startSampling_flag = true; // Set the flag to start sampling
}
/**
 * @brief Stop data sampling
 * 
 * This function sets the flag to stop data sampling.
 */
void DataLogger::stopSamplig() {
    stopSampling_flag = true; // Set the flag to stop sampling
}
/**
 * @brief Set calibration flag
 * 
 * This function sets the calibration flag to true.
 */
void DataLogger::setCalibration(){
    calibration_flag = true; // Set the calibration flag
}
/**
 * @brief Get the current logger state
 * 
 * This function returns the current state of the data logger.
 * 
 * @return LoggerState_t Current logger state
 */
LoggerState_t DataLogger::getLoggerState(){
    return loggerState; // Return the current logger state
}