#include <Arduino.h>

#include <Adafruit_ICM20X.h>
#include <Adafruit_ICM20649.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Adafruit_LIS3MDL.h>
#include <AHRS.h>
//#include <SD.h>
//#include <FS.h>

Adafruit_ICM20649 icm;
Adafruit_LIS3MDL lis;
AHRS ahrs;
//SemaphoreHandle_t xMutex = NULL;
icm20x_raw_axes_t raw_axesD[ICM20X_FIFO_SIZE/6];



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
  //// Try to initialize!
  //if (!icm.begin_I2C(0x68)) {
  //  Serial.println("Failed to find ICM20649 chip");
  //  while (1) {
  //    delay(10);
  //  }
  //}
  ////icm.enableI2CMaster(true);
  ////icm.configureI2CMaster();
  //icm.setI2CBypass(true);
//
  //if (!lis.begin_I2C(0x1E)) {
  //  Serial.println("Failed to find LIS3MDL chip");
  //  while (1) {
  //    delay(10);
  //  }
  //}
  //icm.setI2CBypass(true);

  //icm.writeExternalRegister(0x30, 0x1A, 0xFF);
  //icm.writeExternalRegister(0x30, 0x1B, 0x80);
  //icm.writeExternalRegister(0x30, 0x1D, 0x10);
  //icm.configI2CSlave0(0x30,0x05,0x02);
  
  //icm.setFIFO(FIFO_DATA_ACCEL_GYRO);


}

void loop() {

  //lis.setPerformanceMode(LIS3MDL_HIGHMODE);
  //Serial.print("Performance mode set to: ");
  //switch (lis.getPerformanceMode()) {
  //  case LIS3MDL_LOWPOWERMODE: Serial.println("Low"); break;
  //  case LIS3MDL_MEDIUMMODE: Serial.println("Medium"); break;
  //  case LIS3MDL_HIGHMODE: Serial.println("High"); break;
  //  case LIS3MDL_ULTRAHIGHMODE: Serial.println("Ultra-High"); break;
  //}

 uint16_t frameCoun = ahrs.icm20649.readFIFOBuffer(raw_axesD);

 if (frameCoun>0) {
  Serial.print("Frames read: ");
  Serial.println(frameCoun);
  for (size_t i = 0; i < frameCoun; i++) {
    ahrs_axes_t scaled_axes = ahrs.scaleAxes(raw_axesD[i]);
    Serial.print("AccX: ");
    Serial.print(scaled_axes.accX);
    Serial.print(", AccY: ");
    Serial.print(scaled_axes.accY);
    Serial.print(", AccZ: ");
    Serial.print(scaled_axes.accZ);
    Serial.print(", GyroX: ");
    Serial.print(scaled_axes.gyroX);
    Serial.print(", GyroY: ");
    Serial.print(scaled_axes.gyroY);
    Serial.print(", GyroZ: ");
    Serial.print(scaled_axes.gyroZ);
    Serial.print(", MagX: ");
    Serial.print(scaled_axes.magX);
    Serial.print(", MagY: ");
    Serial.print(scaled_axes.magY);
    Serial.print(", MagZ: ");
    Serial.println(scaled_axes.magZ);

  }
 } else
  {
    Serial.println("No frames read");
  }
  
  //Serial.print("MagX: ");
  //Serial.print(raw_axesD[0].rawMagX);
  //Serial.print(", MagY: ");
  //Serial.print(raw_axesD[0].rawMagY);
  //Serial.print(", MagZ: ");
  //Serial.println(raw_axesD[0].rawMagZ);
  
  delay(500);
}