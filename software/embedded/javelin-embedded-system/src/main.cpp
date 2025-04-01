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

const int magCalArraySize = 1000;
float xmag[magCalArraySize],
      ymag[magCalArraySize],
      zmag[magCalArraySize];
float xmagR,
ymagR,
zmagR;

int idx = 0;
unsigned long startTime = 0;
#define EEPROM_SIZE 64
void setup(void) {
  Serial.begin(115200);
  if (!EEPROM.begin(EEPROM_SIZE)) {
    Serial.println("failed to initialize EEPROM");
    delay(1000000);
  }
  ahrs.beginAHRSi2c();
  ahrs.configAHRS();
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
    //Serial.print("FIFO count: ");
    //Serial.println(ahrs.icm20649.readFIFOCount());
    uint16_t frameCoun = ahrs.icm20649.readFIFOBuffer(raw_axesD);
    //Serial.print("Frames read: ");
    //Serial.println(frameCoun);

    for (size_t i = 0; i < frameCoun; i++) {
      unsigned long t0 = millis();
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

      //Serial.print("Raw:");
      //Serial.print(0); Serial.print(",");
      //Serial.print(0); Serial.print(",");
      //Serial.print(0); Serial.print(",");
      //Serial.print(0); Serial.print(",");
      //Serial.print(0); Serial.print(",");
      //Serial.print(0); Serial.print(",");
      //Serial.print(scaled_axes.magX,4);  Serial.print(",");
      //Serial.print(scaled_axes.magY,4); Serial.print(",");
      //Serial.println(scaled_axes.magZ,4);

      if (idx<magCalArraySize)
      {
        xmag[idx] = scaled_axes.magX;
        ymag[idx] = scaled_axes.magY;
        zmag[idx] = scaled_axes.magZ;
        idx++;
      }
      


      unsigned long t1 = millis();
      
  }
  //Serial.println(ahrs.icm20649.readFIFOCount());
  
  if (millis()-startTime>20000) {
    myFile.close();
    Serial.println("Datalogging ended");
    ahrs.icm20649.enableI2CMaster(false);
    ahrs.icm20649.setI2CBypass(true);
    delay(10);
    if (!ahrs.lis3mdl.hardIronCalib(xmag,ymag,zmag,magCalArraySize))
    {
      Serial.println("Failed to write Hardiron");
    }
    
    ahrs.lis3mdl.readCalibrationOffsets(&xmagR, &ymagR, &zmagR);
    Serial.print("X calibration: ");
    Serial.println(xmagR);
    Serial.print("Y calibration: ");
    Serial.println(ymagR);
    Serial.print("Z calibration: ");
    Serial.println(zmagR);
    while (true) {
      delay(10);
    }
  }
  //delay(50);
}
}
