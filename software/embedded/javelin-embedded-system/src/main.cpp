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
  lis.begin_I2C();
icm.setAccelRange(ICM20649_ACCEL_RANGE_30_G);
  Serial.print("Accelerometer range set to: ");
  switch (icm.getAccelRange()) {
  case ICM20649_ACCEL_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case ICM20649_ACCEL_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case ICM20649_ACCEL_RANGE_16_G:
    Serial.println("+-16G");
    break;
  case ICM20649_ACCEL_RANGE_30_G:
    Serial.println("+-30G");
    break;
  }

  icm.setGyroRange(ICM20649_GYRO_RANGE_500_DPS);
  Serial.print("Gyro range set to: ");
  switch (icm.getGyroRange()) {
  case ICM20649_GYRO_RANGE_500_DPS:
    Serial.println("500 degrees/s");
    break;
  case ICM20649_GYRO_RANGE_1000_DPS:
    Serial.println("1000 degrees/s");
    break;
  case ICM20649_GYRO_RANGE_2000_DPS:
    Serial.println("2000 degrees/s");
    break;
  case ICM20649_GYRO_RANGE_4000_DPS:
    Serial.println("4000 degrees/s");
    break;
  }

  icm.setAccelRateDivisor(254);
  uint16_t accel_divisor = icm.getAccelRateDivisor();
  float accel_rate = 1125 / (1.0 + accel_divisor);

  Serial.print("Accelerometer data rate divisor set to: ");
  Serial.println(accel_divisor);
  Serial.print("Accelerometer data rate (Hz) is approximately: ");
  Serial.println(accel_rate);

  icm.setGyroRateDivisor(254);
  uint8_t gyro_divisor = icm.getGyroRateDivisor();
  float gyro_rate = 1125 / (1.0 + gyro_divisor);

  Serial.print("Gyro data rate divisor set to: ");
  Serial.println(gyro_divisor);
  Serial.print("Gyro data rate (Hz) is approximately: ");
  Serial.println(gyro_rate);
  Serial.println();

  //icm.enableI2CMaster(true);
  //icm.configureI2CMaster();
  icm.setI2CBypass(true);
  //icm.writeExternalRegister(0x30, 0x1A, 0xFF);
  //icm.writeExternalRegister(0x30, 0x1B, 0x80);
  //icm.writeExternalRegister(0x30, 0x1D, 0x10);
  //icm.configI2CSlave0(0x30,0x05,0x02);
  
  //icm.setFIFO(FIFO_DATA_ACCEL_GYRO);
}

void loop() {

  //  /* Get a new normalized sensor event */
  
  //sensors_event_t accel;
  //sensors_event_t gyro;
  //sensors_event_t temp;
  //icm.getEvent(&accel, &gyro, &temp);
//
  //Serial.print("\t\tTemperature ");
  //Serial.print(temp.temperature);
  //Serial.println(" deg C");
//
  //// Display the results (acceleration is measured in m/s^2) 
  //Serial.print("\t\tAccel X: ");
  //Serial.print(accel.acceleration.x);
  //Serial.print(" \tY: ");
  //Serial.print(accel.acceleration.y);
  //Serial.print(" \tZ: ");
  //Serial.print(accel.acceleration.z);
  //Serial.println(" m/s^2 ");
//
  //// Display the results (acceleration is measured in m/s^2) 
  //Serial.print("\t\tGyro X: ");
  //Serial.print(gyro.gyro.x);
  //Serial.print(" \tY: ");
  //Serial.print(gyro.gyro.y);
  //Serial.print(" \tZ: ");
  //Serial.print(gyro.gyro.z);
  //Serial.println(" radians/s ");
  //Serial.println();
  //Serial.print("FIFO COUNT: ");
  //uint32_t fifo_count = icm.readFIFOCount();
  //Serial.println(fifo_count);
  //while (icm.readFIFOCount() > 12)
  //{
  //  icm.readFIFO();
  //}
  //Serial.println(icm.readExternalRegister(0x1C, 0x0F));
  //Serial.println(icm.readFIFOByte());

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