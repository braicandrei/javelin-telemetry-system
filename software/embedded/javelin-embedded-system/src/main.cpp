#include <Arduino.h>

#include <Adafruit_ICM20X.h>
#include <Adafruit_ICM20649.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Adafruit_LIS3MDL.h>
#include <AHRS.h>
#include <SD.h>
#include <FS.h>
#include <EEPROM.h>

QueueHandle_t queue = NULL;

Adafruit_ICM20649 icm;
Adafruit_LIS3MDL lis;
AHRS ahrs;
//SemaphoreHandle_t xMutex = NULL;
icm20x_raw_axes_t raw_axesD[ICM20X_FIFO_SIZE/6];
File myFile;

volatile bool interruptOccurred = false;

void IRAM_ATTR handleInterrupt() {
  interruptOccurred = true;
}

unsigned long startTime = 0;
#define EEPROM_SIZE 64

void setup(void) {
  Serial.begin(115200);//start serial for debugging
  if (!EEPROM.begin(EEPROM_SIZE)) {//initialize EEPROM
    Serial.println("failed to initialize EEPROM");
    delay(1000000);
  }
  queue = xQueueCreate(500, sizeof(ahrs_axes_t));//create queue for 500 elements of type ahrs_axes_t
  if (!SD.begin()) {// initialize SD card
    Serial.println("SD card initialization failed!");
    while (true) {
      delay(10);
    }
  } else {
    Serial.println("SD card initialized successfully");
  }
  
  myFile = SD.open("/datalog.txt", FILE_WRITE);//open file for writing
  if (!myFile) {
    Serial.println("Error opening datalog.txt");
    while (true) {
      delay(10);
    }
  } else {
    Serial.println("datalog.txt opened successfully");
  }

  ahrs.beginAHRSi2c(); // Initialize ICM20649 and LIS3MDL
  ahrs.configAHRS();// Configure ICM20649 and LIS3MDL
  pinMode(4,INPUT);
  attachInterrupt(digitalPinToInterrupt(4), handleInterrupt, RISING); //attach interrupt to pin 4

  ahrs.lowPowerMode(true);
  Serial.println("Low power mode active....");
  delay(2000);
  ahrs.lowPowerMode(false);
  Serial.println("Low power mode deactivate....");
  startTime = millis();
}

void loop() {

  
  if (interruptOccurred) {
    interruptOccurred = false;
    Serial.println("Interrupción detectada en el pin 4");
    //Serial.print("FIFO count: ");
    //Serial.println(ahrs.icm20649.readFIFOCount());
    uint16_t frameCoun = ahrs.icm20649.readFIFOBuffer(raw_axesD);
    //Serial.print("Frames read: ");
    //Serial.println(frameCoun);

    for (size_t i = 0; i < frameCoun; i++) {
      unsigned long t0 = micros();
      ahrs_axes_t scaled_axes = ahrs.scaleAxes(raw_axesD[i]);
      //write to SD card only the axex values in csv format
      //myFile.print(scaled_axes.accX,4); myFile.print(",");
      //myFile.print(scaled_axes.accY,4); myFile.print(",");
      //myFile.print(scaled_axes.accZ,4); myFile.print(",");
      //myFile.print(scaled_axes.gyroX,4); myFile.print(",");
      //myFile.print(scaled_axes.gyroY,4); myFile.print(",");
      //myFile.print(scaled_axes.gyroZ,4); myFile.print(",");
      //myFile.print(scaled_axes.magX,4);  myFile.print(",");
      //myFile.print(scaled_axes.magY,4); myFile.print(",");
      //myFile.println(scaled_axes.magZ,4);

      String data = String(scaled_axes.accX, 4) + "," +
              String(scaled_axes.accY, 4) + "," +
              String(scaled_axes.accZ, 4) + "," +
              String(scaled_axes.gyroX, 4) + "," +
              String(scaled_axes.gyroY, 4) + "," +
              String(scaled_axes.gyroZ, 4) + "," +
              String(scaled_axes.magX, 4) + "," +
              String(scaled_axes.magY, 4) + "," +
              String(scaled_axes.magZ, 4);
      myFile.println(data);        
      unsigned long t1 = micros();
      //Serial.print("Time taken: ");
      //Serial.println(t1-t0);
  }
  //Serial.println(ahrs.icm20649.readFIFOCount());
  
  if (millis()-startTime>20000) {
    myFile.close();
    Serial.println("Datalogging ended");
    //ahrs.icm20649.enableI2CMaster(false);
    //ahrs.icm20649.setI2CBypass(true);
    //delay(10);
    //if (!ahrs.lis3mdl.hardIronCalib(xmag,ymag,zmag,magCalArraySize))
    //{
    //  Serial.println("Failed to write Hardiron");
    //}
    //
    //ahrs.lis3mdl.readCalibrationOffsets(&xmagR, &ymagR, &zmagR);
    //Serial.print("X calibration: ");
    //Serial.println(xmagR);
    //Serial.print("Y calibration: ");
    //Serial.println(ymagR);
    //Serial.print("Z calibration: ");
    //Serial.println(zmagR);
    while (true) {
      delay(10);
    }
  }
  //delay(50);
}
}
