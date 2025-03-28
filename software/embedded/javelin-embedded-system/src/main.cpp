#include <Arduino.h>

#include <Adafruit_ICM20X.h>
#include <Adafruit_ICM20649.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Adafruit_LIS3MDL.h>

Adafruit_ICM20649 icm;
Adafruit_LIS3MDL lis;


void setup(void) {
  Serial.begin(115200);
  Serial.println("Adafruit ICM20649 test!");
  pinMode(4,INPUT);
  // Try to initialize!
  icm.begin_I2C();
  //icm.enableI2CMaster(true);
  //icm.configureI2CMaster();
 
  if (!lis.begin_I2C()) {
    Serial.println("Failed to find LIS3MDL chip");
  }
  icm.setI2CBypass(true);

  //icm.writeExternalRegister(0x30, 0x1A, 0xFF);
  //icm.writeExternalRegister(0x30, 0x1B, 0x80);
  //icm.writeExternalRegister(0x30, 0x1D, 0x10);
  //icm.configI2CSlave0(0x30,0x05,0x02);
  
  //icm.setFIFO(FIFO_DATA_ACCEL_GYRO);
}

void loop() {

  lis.setPerformanceMode(LIS3MDL_HIGHMODE);
  Serial.print("Performance mode set to: ");
  switch (lis.getPerformanceMode()) {
    case LIS3MDL_LOWPOWERMODE: Serial.println("Low"); break;
    case LIS3MDL_MEDIUMMODE: Serial.println("Medium"); break;
    case LIS3MDL_HIGHMODE: Serial.println("High"); break;
    case LIS3MDL_ULTRAHIGHMODE: Serial.println("Ultra-High"); break;
  }
  delay(500);
}