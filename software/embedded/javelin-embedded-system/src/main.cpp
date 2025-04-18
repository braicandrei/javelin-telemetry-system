#include <Arduino.h>


#include <DataLogger.h>
#include <UserInterface.h>
#include <LogWebServer.h>
#include <SD.h>
#define SD_CS_PIN 3


UserInterface ui;
DataLogger logger;

LogWebServer logServer(SD_CS_PIN, "ESP32", "12345678", "javelin");
bool serverOn = false;
void setup(void) {
  Serial.begin(115200);//start serial for debugging
  ui.beginUI();//initialize user interface
  if (!SD.begin(SD_CS_PIN)) {
  Serial.println("SD card initialization failed!");
}
  if (logger.begin()!=LOGGER_OK) {//initialize data logger
    Serial.println("Error initializing data logger!");
    while (1); // Stop execution if initialization fails
  }
}

void loop() {

  if (logger.updateLogger()!=LOGGER_OK) {//update data logger
    Serial.println("Error updating data logger!");
    while (1); // Stop execution if update fails
    delay(1000); // Delay to avoid rapid error messages
  }
  switch (ui.updateUI()) {
  case THREE_TOUCHES:
    if (logger.getLoggerState() == LOGGER_WAITING && !serverOn) {
      // Handle three touches detected
      logger.startSamplig(); // Start data sampling
      ui.setSystemTransition(SAMPLE_BEGIN); // Set system transition to sample begin
    } else if (logger.getLoggerState() == LOGGER_SAMPLING) {
      // Handle three touches detected while sampling
      logger.stopSamplig(); // Stop data sampling
      ui.setSystemTransition(SAMPLE_END); // Set system transition to sample end
    }
    break;
  case FIVE_TOUCHES:
    //if (logger.getLoggerState() == LOGGER_WAITING) {
    //  // Handle three touches detected
    //  logger.setCalibration(); // Set calibration flag
    //  logger.startSamplig(); // Start data sampling
    //  ui.setSystemTransition(SAMPLE_BEGIN); // Set system transition to sample begin
    //}
    if (!serverOn && logger.getLoggerState() == LOGGER_WAITING)
    {
      logServer.begin(); // Start the web server
      serverOn = true; // Set serverOn flag to true
      ui.setSystemTransition(SERVER_MODE_ON); // Set system transition to server mode on
    } else
    {
      logServer.end(); // Stop the web server
      serverOn = false; // Set serverOn flag to false
      ui.setSystemTransition(SERVER_MODE_OFF);
    }
    
    break;
  default:
    // No action needed for other cases
    break;
  }
}


//#include <AHRS.h>
//
//AHRS ahrs; // Create an instance of the AHRS class
//icm20x_raw_axes_t raw_axesD[1000]; // Buffer to store raw axes data
//volatile bool interruptOccurred = false;
//
//void IRAM_ATTR handleInterrupt() {
//  interruptOccurred = true;
//}
//void setup() {
//  Serial.begin(115200); // Start serial communication for debugging
//  delay(1000); // Wait for a second before starting
//  pinMode(4,INPUT);
//  attachInterrupt(digitalPinToInterrupt(4), handleInterrupt, RISING); //attach interrupt to pin 4
//  ahrs.beginAHRSi2c(); // Initialize the AHRS sensor
//  ahrs.configAHRS(); // Configure the AHRS sensor
//  ahrs.icm20649.resetFIFO();
//}
//
//void loop() {
//  if (interruptOccurred) { // Check if an interrupt has occurred
//    interruptOccurred = false; // Reset the interrupt flag
//    //unsigned long t0 = millis(); // Start time for data processing
//    //Serial.print("Frame count before: ");
//    //uint32_t countBefore =  ahrs.icm20649.readFIFOCount();
//    ahrs.icm20649.readFIFOBuffer(raw_axesD); // Read FIFO buffer
//    //uint32_t countAfter =  ahrs.icm20649.readFIFOCount();
//    //Serial.print("Time to read FIFO: " + String(millis() - t0) + "ms"); // Debugging time taken to read FIFO
//    //Serial.println("Bytes readed from fifo: "+ String(countBefore-countAfter)+"--Ramainging: "+String(countAfter)); // Print the frame count for debugging
//  }
// // delay(100); // Delay for 100 milliseconds
//
//}