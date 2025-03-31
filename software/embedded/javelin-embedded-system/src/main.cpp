#include <Arduino.h>

#include <Adafruit_ICM20X.h>
#include <Adafruit_ICM20649.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Adafruit_LIS3MDL.h>
#include <AHRS.h>
#include <SD.h>
#include <FS.h>

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

unsigned long getTimeStamp() {
  //temporary function to get time stamp until RTC is implemented
  return millis();
}

unsigned long startTime = 0;

void setup(void) {
  Serial.begin(115200);
  uint8_t beginE =  ahrs.beginAHRS();
  if (beginE != 0x00) {
    Serial.print("AHRS begin failed with error code: ");
    Serial.println(beginE, HEX);
    while (1) {
      delay(10);
    }
  } else {
    Serial.println("AHRS begin success!");
  }
  //Serial.println("Adafruit ICM20649 test!");
  pinMode(4,INPUT);
  attachInterrupt(digitalPinToInterrupt(4), handleInterrupt, RISING); // Interrupción activa en HIGH


  if (!SD.begin()) {
    Serial.println("SD card initialization failed!");
    while (true) {
      delay(10);
    }
  } else {
    Serial.println("SD card initialized successfully");
  }

  myFile = SD.open("/datalog.txt", FILE_WRITE);
  if (!myFile) {
    Serial.println("Error opening datalog.txt");
    while (true) {
      delay(10);
    }
  } else {
    Serial.println("datalog.txt opened successfully");
  }

  startTime = millis();
}

void loop() {

  if (interruptOccurred) {
    interruptOccurred = false;
    Serial.println("Interrupción detectada en el pin 4");
    Serial.print("FIFO count: ");
    Serial.println(ahrs.icm20649.readFIFOCount());
    uint16_t frameCoun = ahrs.icm20649.readFIFOBuffer(raw_axesD);
    Serial.print("Frames read: ");
    Serial.println(frameCoun);

    for (size_t i = 0; i < frameCoun; i++) {
      unsigned long t0 = millis();
      ahrs_axes_t scaled_axes = ahrs.scaleAxes(raw_axesD[i]);
      //write to SD card only the axex values in csv format
      myFile.print(scaled_axes.accX,4); myFile.print(",");
      myFile.print(scaled_axes.accY,4); myFile.print(",");
      myFile.print(scaled_axes.accZ,4); myFile.print(",");
      myFile.print(scaled_axes.gyroX,4); myFile.print(",");
      myFile.print(scaled_axes.gyroY,4); myFile.print(",");
      myFile.print(scaled_axes.gyroZ,4); myFile.print(",");
      myFile.print(scaled_axes.magX,4);  myFile.print(",");
      myFile.print(scaled_axes.magY,4); myFile.print(",");
      myFile.println(scaled_axes.magZ,4);
      unsigned long t1 = millis();
      Serial.print("Time taken for frame ");
      Serial.println(t1-t0);
    }
  }
  //Serial.println(ahrs.icm20649.readFIFOCount());
  
  if (millis()-startTime>20000) {
    myFile.close();
    Serial.println("Datalogging ended");
    while (true) {
      delay(10);
    }
  }
  //delay(50);
}