/*!
    * @file AHRS.cpp
    * @brief AHRS class definition
    * @date 2021-02-21
*/

#include "AHRS.h"

AHRS::AHRS() {}
AHRS::~AHRS() {}

uint8_t AHRS::beginAHRSi2c() {

  uint8_t accelConexionTry = 0;
  uint8_t magConexionTry = 0;

  while (!icm20649.begin_I2C() && accelConexionTry < 3) { // Initialize ICM20649 I2C bus
    Serial.println("ICM20649 I2C bus initialization failed!");
    delay(1000); // Wait for 1 second before retrying
    accelConexionTry++;
  }
  if (!icm20649.begin_I2C())
  {
   return 0x01; // Error code for ICM20649 initialization failure
  }

  icm20649.setI2CBypass(true); // Enable I2C bypass

  if (!lis3mdl.begin_I2C() && magConexionTry < 3) { // Initialize LIS3MDL I2C bus
    Serial.println("LIS3MDL I2C bus initialization failed!");
    delay(1000); // Wait for 1 second before retrying
    magConexionTry++;
  }
  if (!lis3mdl.begin_I2C())
  {
    return 0x02; // Error code for LIS3MDL initialization failure
  }

  return 0x00; // Success

}

bool AHRS::configAHRS() {

 
  icm20649.setI2CBypass(true); // Enable I2C bypass
 
  lis3mdl.setPerformanceMode(LIS3MDL_HIGHMODE); // Set magentometer performance mode to high
  lis3mdl.setOperationMode(LIS3MDL_CONTINUOUSMODE); // Set magnetometer operation mode to continuous
  lis3mdl.setDataRate(LIS3MDL_DATARATE_560_HZ); // Set magnetometer data rate to 560 Hz
  lis3mdl.setRange(LIS3MDL_RANGE_4_GAUSS); // Set magnetometer range to 4 gauss
  lis3mdl.loadOffsetsFromEEPROM();  // Load offsets from EEPROM
  lis3mdl.writeOffsetxyz();// Write offset values to magnetometer for hard iron calibration
  icm20649.setI2CBypass(false); // Disable I2C bypass

  
  icm20649.odrAlign(true); // Enable ODR alignment
  icm20649.setAccelRange(ICM20649_ACCEL_RANGE_30_G); // Set accelerometer range to 30 G
  icm20649.setGyroRange(ICM20649_GYRO_RANGE_500_DPS); // Set gyroscope range to 500 DPS
  icm20649.setAccelRateDivisor(20); // Set accelerometer data rate divisor to 254
  icm20649.setGyroRateDivisor(20); // Set gyroscope data rate divisor to 254
  
  icm20649.enableI2CMaster(true); // Enable I2C master
  icm20649.configureI2CMaster(); // Configure I2C master
  icm20649.configI2CSlave0(LIS3MDL_I2CADDR_DEFAULT, LIS3MDL_REG_OUT_X_L, LIS3MDL_OUT_DATA_LEN); // Configure I2C slave 0
  
  icm20649.enableFIFO(true); // Enable FIFO
  icm20649.enableFIFOWatermarkInt(true, false); // Enable FIFO watermark interrupt
  icm20649.selectFIFOData(FIFO_DATA_ACCEL_GYRO_S0); // Select FIFO data
  icm20649.resetFIFO(); // Reset FIFO
  return true; // Success
}

bool AHRS::setAHRSRange(icm20649_accel_range_t accelRange, icm20649_gyro_range_t gyroRange, lis3mdl_range_t magRange) {
    icm20649.setAccelRange(accelRange);
    icm20649.setGyroRange(gyroRange);
    lis3mdl.setRange(magRange);
    return true;
}

ahrs_axes_t AHRS::scaleAxes(icm20x_raw_axes_t raw_axes){
    ahrs_axes_t scaled_axes;

    
    float accel_scale = 1.0;
    float gyro_scale = 1.0;
    float mag_scale = 1.0;
  
    if (gyroRange == ICM20649_GYRO_RANGE_500_DPS)
      gyro_scale = 65.5;
    if (gyroRange == ICM20649_GYRO_RANGE_1000_DPS)
      gyro_scale = 32.8;
    if (gyroRange == ICM20649_GYRO_RANGE_2000_DPS)
      gyro_scale = 16.4;
    if (gyroRange == ICM20649_GYRO_RANGE_4000_DPS)
      gyro_scale = 8.2;
  
    if (accelRange == ICM20649_ACCEL_RANGE_4_G)
      accel_scale = 8192.0;
    if (accelRange == ICM20649_ACCEL_RANGE_8_G)
      accel_scale = 4096.0;
    if (accelRange == ICM20649_ACCEL_RANGE_16_G)
      accel_scale = 2048.0;
    if (accelRange == ICM20649_ACCEL_RANGE_30_G)
      accel_scale = 1024.0;

    if (magRange == LIS3MDL_RANGE_4_GAUSS)
        mag_scale = 6842.0;
    if (magRange == LIS3MDL_RANGE_8_GAUSS)
        mag_scale = 3421.0;
    if (magRange == LIS3MDL_RANGE_12_GAUSS)
        mag_scale = 2281.0;
    if (magRange == LIS3MDL_RANGE_16_GAUSS)
        mag_scale = 1711.0;
  
    scaled_axes.accX = raw_axes.rawAccX / accel_scale;
    scaled_axes.accY = raw_axes.rawAccY / accel_scale;
    scaled_axes.accZ = raw_axes.rawAccZ / accel_scale;

    scaled_axes.gyroX = raw_axes.rawGyroX / gyro_scale;
    scaled_axes.gyroY = raw_axes.rawGyroY / gyro_scale;
    scaled_axes.gyroZ = raw_axes.rawGyroZ / gyro_scale;

    scaled_axes.magX = raw_axes.rawMagX / mag_scale;
    scaled_axes.magY = raw_axes.rawMagY / mag_scale;
    scaled_axes.magZ = raw_axes.rawMagZ / mag_scale;

    return scaled_axes;

}


